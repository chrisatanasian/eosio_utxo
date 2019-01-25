#include <eosiolib/eosio.hpp>
#include <eosiolib/print.hpp>
#include <eosiolib/types.h>
#include <eosiolib/crypto.h>
#include <eosiolib/asset.hpp>
#include "json.hpp"
#include "../eosio.lost/trezor-crypto/base58.c"
#include "../eosio.lost/trezor-crypto/memzero.c"
#include "../eosio.lost/trezor-crypto/ripemd160.c"

using namespace eosio;
using namespace std;
using json = nlohmann::json;

class verifier : public contract {
  public:
      using contract::contract;

      [[eosio::action]]
      void create(public_key issuer,
                  asset maximum_supply) {
        require_auth(_self);

        auto symbol = maximum_supply.symbol;
        eosio_assert(symbol.is_valid(), "invalid symbol name");
        eosio_assert(maximum_supply.is_valid(), "invalid supply");
        eosio_assert(maximum_supply.amount > 0, "max-supply must be positive");

        stats statstable(_self, symbol.raw());
        auto existing = statstable.find(symbol.raw());
        eosio_assert(existing == statstable.end(), "token with symbol already exists");

        statstable.emplace(_self, [&](auto& s) {
          s.supply.symbol = maximum_supply.symbol;
          s.max_supply    = maximum_supply;
          s.issuer        = issuer;
        });
      }

      [[eosio::action]]
      void issue(public_key to, asset quantity, const string memo) {
        require_auth(_self);

        auto symbol = quantity.symbol;
        eosio_assert(symbol.is_valid(), "invalid symbol name");
        eosio_assert(memo.size() <= 256, "memo has more than 256 bytes");

        stats statstable(_self, symbol.raw());
        auto existing = statstable.find(symbol.raw());
        eosio_assert(existing != statstable.end(), "token with symbol does not exist, create token before issue");
        const auto& st = *existing;

        eosio_assert(quantity.is_valid(), "invalid quantity");
        eosio_assert(quantity.amount > 0, "must issue positive quantity");

        eosio_assert(quantity.symbol == st.supply.symbol, "symbol precision mismatch");
        eosio_assert(quantity.amount <= st.max_supply.amount - st.supply.amount, "quantity exceeds available supply");

        statstable.modify(st, _self, [&](auto& s) {
           s.supply += quantity;
        });

        add_balance(st.issuer, quantity);

        asset fee = asset(int64_t(0), symbol);
        if (to != st.issuer) {
          transfer_internal(st.issuer, to, quantity, fee, memo);
        }
      }

      [[eosio::action]]
      void transfer(public_key pkeyFrom,
                    public_key pkeyTo,
                    signature sig,
                    asset amount,
                    asset fee,
                    string memo) {
        json digestJSON;
        digestJSON["from"] = public_key_to_string(pkeyFrom);
        digestJSON["to"] = public_key_to_string(pkeyTo);
        digestJSON["amount"] = amount.to_string();
        digestJSON["fee"] = fee.to_string();
        digestJSON["memo"] = memo;

        const string str = public_key_to_string(pkeyFrom);

        const string digestJSONString = digestJSON.dump();
        checksum256 digest = sha256(digestJSONString.c_str(), digestJSONString.size());

        assert_recover_key(digest, sig, pkeyFrom);
        transfer_internal(pkeyFrom, pkeyTo, amount, fee, memo);
      }

  private:

    static const string public_key_to_string(public_key pkey) {
      public_key pkeycopy = pkey;
      unsigned char to_encode[37];
      memcpy(to_encode, pkeycopy.data.data(), 33);

      // add ripemd160 checksum to end of key
      uint8_t hash_output[20];
      ripemd160((uint8_t *)pkeycopy.data.begin(), 33, hash_output);
      memcpy(to_encode + 33, hash_output, 4);

      // convert to base58
      char b58[51];
      size_t b58sz = 51;
      b58enc(b58, &b58sz, (const uint8_t *)to_encode, 37);

      string pkeyString(b58);
      return "EOS" + pkeyString;
    }

    static fixed_bytes<32> public_key_to_fixed_bytes(const public_key publickey) {
      return sha256(publickey.data.begin(), 33);
    }

    void sub_balance(public_key owner, asset value) {
      accounts from_acts(_self, _self.value);

      auto accounts_index = from_acts.get_index<name("bypk")>();
      const auto& from = accounts_index.get(public_key_to_fixed_bytes(owner), "no public key object found");
      eosio_assert(from.balance.amount >= value.amount, "overdrawn balance");

      if (from.balance.amount == value.amount) {
        from_acts.erase(from);
      } else {
        from_acts.modify(from, _self, [&]( auto& a ) {
          a.balance -= value;
        });
      }
    }

    void add_balance(public_key recipientKey, asset value) {
      accounts to_acts(_self, _self.value);

      auto accounts_index = to_acts.get_index<name("bypk")>();
      auto to = accounts_index.find(public_key_to_fixed_bytes(recipientKey));

      if (to == accounts_index.end()) {
        to_acts.emplace(_self, [&]( auto& a ){
          a.key = to_acts.available_primary_key();
          a.balance = value;
          a.publickey = recipientKey;
        });
      } else {
        accounts_index.modify(to, _self, [&]( auto& a ) {
          a.balance += value;
        });
      }
    }

    void transfer_internal(public_key pkeyFrom,
                           public_key pkeyTo,
                           asset amount,
                           asset fee,
                           string memo) {
      require_auth(_self);
      eosio_assert(pkeyFrom != pkeyTo, "cannot transfer to self");

      auto sym = amount.symbol.raw();
      stats statstable(_self, sym);
      const auto& st = statstable.get(sym);

      eosio_assert(amount.is_valid(), "invalid quantity" );
      eosio_assert(fee.is_valid(), "invalid quantity" );
      eosio_assert(amount.amount > 0, "must transfer positive quantity");
      eosio_assert(fee.amount >= 0, "must transfer non negative quantity");
      eosio_assert(amount.symbol == st.supply.symbol, "symbol precision mismatch");
      eosio_assert(fee.symbol == st.supply.symbol, "symbol precision mismatch");
      eosio_assert(memo.size() <= 256, "memo has more than 256 bytes");

      // once for the amount from to to
      sub_balance(pkeyFrom, amount);
      add_balance(pkeyTo, amount);

      // second time to give the relayer the fee
      if (fee.amount > 0) {
        sub_balance(pkeyFrom, fee);
        add_balance(st.issuer, fee);
      }
    }

    struct [[eosio::table]] account {
      uint64_t key;
      public_key publickey;
      asset balance;

      uint64_t primary_key() const { return key; }
      fixed_bytes<32> bypk() const {
        return public_key_to_fixed_bytes(publickey);
      };
    };

    struct [[eosio::table]] currstats {
      asset supply;
      asset max_supply;
      public_key issuer;

      uint64_t primary_key() const { return supply.symbol.raw(); }
    };

    typedef eosio::multi_index<"accounts"_n,
                               account,
                               indexed_by<"bypk"_n, const_mem_fun<account, fixed_bytes<32>, &account::bypk>>
                              > accounts;
    typedef eosio::multi_index<"stats"_n, currstats> stats;

};
EOSIO_DISPATCH(verifier, (create)(issue)(transfer))
