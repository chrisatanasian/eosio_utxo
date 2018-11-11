#include <eosiolib/eosio.hpp>
#include <eosiolib/print.hpp>
#include <eosiolib/types.h>
#include <eosiolib/crypto.h>
#include <eosiolib/asset.hpp>

using namespace eosio;
using namespace std;

class verifier : public contract {
  public:
      using contract::contract;

      [[eosio::action]]
      void addutxo(name relayer,
                   const string pkeyFrom,
                   const string pkeyTo,
                   const string pkeyFee,
                   double amount,
                   double fee) {
        require_auth(relayer);

        capi_checksum256 digest;
        const char* data = "test_data";
        sha256(data, sizeof(data), &digest);

        const string signature = "SIG_K1_KfQ57wLFFiPR85zjuQyZsn7hK3jRicHXg4qETxLvxHQTHHejveGtiYBSx6Z68xBZYrY9Fihz74makocnSLQFBwaHTg6Aaa";
        eosio_assert(recover_key(digest, signature, 101, pkeyFrom, 53), "digest and signature do not match");
      }

  private:
    bool recover_key(const capi_checksum256 digest,
                     const string signature,
                     size_t siglen,
                     const string pub,
                     size_t publen) {
      return false;
    }

    struct [[eosio::table]] transfargs {
      uint64_t key;
      string from;
      string to;
      asset quantity;
      asset fee;
      string memo;

      uint64_t primary_key() const { return key; }

      string publickeyfrom() const { return from; }
      string publickeyto() const { return to; }
    };

    typedef eosio::multi_index<"transfargs"_n, transfargs> txtable;

};
EOSIO_DISPATCH(verifier, (addutxo))
