#include <eosiolib/eosio.hpp>
#include <eosiolib/print.hpp>
#include <eosiolib/types.h>
#include <eosiolib/crypto.h>
#include <eosiolib/asset.hpp>
#include "json.hpp"

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
        print("Raw symbol: ");
        print(symbol.raw());
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
          action(permission_level{ _self, "active"_n },
                 _self, "transfer"_n,
                 std::make_tuple(st.issuer, to, quantity, fee, memo)
                 ).send();
        }
      }

      [[eosio::action]]
      void transfer(public_key pkeyFrom,
                    public_key pkeyTo,
                    signature sig,
                    asset amount,
                    asset fee,
                    string memo) {
        require_auth(_self);
        eosio_assert(pkeyFrom != pkeyTo, "cannot transfer to self");

        /* CREATING THE DIGEST: */
        // init the digest var
        eosio::checksum256 digest;
        // first put all the data into a json format so that it can be serialized easily (or just convert it to string myself?)
        // json digestJSON;
        // digestJSON["from"] = pkeyFrom;
        // digestJSON["to"] = pkeyTo;
        // digestJSON["amount"] = amount;
        // digestJSON["fee"] = fee;
        // digestJSON["memo"] = memo;

        // get the serialized form of the json object
        // std::string digestJSONString = digestJSON.dump();

        // convert to c_stringg for use by sha256
        // const char* data = digestJSONString.c_str();
        // then sha256 the json string
        // digest = sha256(data, sizeof(data));
        // the digest, pub key, and sig get passed to assert_recover_key or verify -- need to find this

        // assert_recover_key(digest, sig, pkeyFrom);

        // const string signature = "SIG_K1_KfQ57wLFFiPR85zjuQyZsn7hK3jRicHXg4qETxLvxHQTHHejveGtiYBSx6Z68xBZYrY9Fihz74makocnSLQFBwaHTg6Aaa";

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
        // TODO: uncomment this once the string to public_key changes have been fully made
        // because we're taking in public_keys up top these methods have to be updated accordingly.
        // sub_balance(pkeyFrom, amount);
        // add_balance(pkeyTo, amount);

        // second time to give the relayer the fee
        if (fee.amount > 0) {
          sub_balance(pkeyFrom, fee);
          add_balance(st.issuer, fee);
        }
      }

      [[eosio::action]]
      void verifykey(public_key pkeyFrom,
                    signature sig,
                    eosio::checksum256 digest) {
        print("about to check auth -- top of the function");        
        // require_auth(_self);
        print("about to verify");
        assert_recover_key(digest, sig, pkeyFrom);
        print("verification successful");
      }

      [[eosio::action]]
      void verifykey2(public_key pkeyFrom,
                    signature sig,
                    std::string data) {
        eosio::checksum256 digest = sha256(&data[0], data.size());

        print("about to check auth -- top of the function");
        // require_auth(_self);
        print("about to get the public key");
        eosio::public_key recoveredKey = recover_key(digest, sig);
        print("got the public key");
        const char * recoveredKeyData = &recoveredKey.data[0];
        print(recoveredKeyData);
        bool isMatch = recoveredKey == pkeyFrom;
        print("isMatch");
        print(isMatch);
      }

      [[eosio::action]]
      void printsha(std::string jankJSON, uint32_t jsonLength) {
        // runs sha 256 with the provided 2 args and prints the result so we can use it in transfer2
        eosio::checksum256 digest = sha256(jankJSON.c_str(), jsonLength);
        print("digest:");
        print(digest);
      }
  private:

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
EOSIO_DISPATCH(verifier, (create)(issue)(transfer)(printsha)(verifykey)(verifykey2))
