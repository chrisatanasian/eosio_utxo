#include <eosiolib/eosio.hpp>
#include <eosiolib/print.hpp>
#include <eosiolib/types.h>
#include <eosiolib/crypto.h>
#include <eosiolib/asset.hpp>
#include <sstream>
#include "json.hpp"

using namespace eosio;
using namespace std;
using json = nlohmann::json;

class verifier : public contract {
  public:
      using contract::contract;

      [[eosio::action]]
      void create(const string issuer,
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
      void issue(const string to, asset quantity, const string memo) {
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

    void sub_balance(const string owner, asset value) {
      accounts from_acts(_self, _self.value);

      uint64_t v;
      istringstream iss(owner);
      iss >> v;

      const auto& from = from_acts.get(v, "no public key object found");
      eosio_assert(from.balance.amount >= value.amount, "overdrawn balance");

      if (from.balance.amount == value.amount) {
        from_acts.erase(from);
      } else {
        from_acts.modify(from, _self, [&]( auto& a ) {
          a.balance -= value;
        });
      }
    }

    void add_balance(const string recipientKey, asset value) {
      accounts to_acts(_self, _self.value);

      uint64_t v;
      istringstream iss(recipientKey);
      iss >> v;

      auto to = to_acts.find(v);

      if (to == to_acts.end()) {
        to_acts.emplace(_self, [&]( auto& a ){
          a.balance = value;
          a.publickey = recipientKey;
        });
      } else {
        to_acts.modify(to, _self, [&]( auto& a ) {
          a.balance += value;
        });
      }
    }

    struct [[eosio::table]] account {
      string publickey;
      asset balance;

      uint64_t primary_key() const {
        uint64_t v;
        istringstream iss(publickey);
        iss >> v;

        return v;
      }
    };

    struct [[eosio::table]] currstats {
      asset supply;
      asset max_supply;
      string issuer;

      uint64_t primary_key() const { return supply.symbol.raw(); }
    };

    typedef eosio::multi_index<"accounts"_n, account> accounts;
    typedef eosio::multi_index<"stats"_n, currstats> stats;

};
EOSIO_DISPATCH(verifier, (create)(issue)(transfer)(printsha)(verifykey)(verifykey2))
