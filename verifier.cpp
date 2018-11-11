#include "eosiolib/eosio.hpp"
#include "eosiolib/print.hpp"
#include "eosiolib/types.h"
#include "eosiolib/crypto.h"

using namespace eosio;
using namespace std;

class verifier : public contract {
  public:
      using contract::contract;

      [[eosio::action]]
      void assrtrcvrky(const capi_checksum256 digest,
                       const char sig,
                       size_t siglen,
                       const char pub,
                       size_t publen) {
        capi_checksum256 result;
        const char* myString = "yo waddup";
        // below generates and unpacks a valid checksum256
        sha256(myString, sizeof(myString), &result);

        txtable transactions(_self, _self.value);
      }

  private:
    struct [[eosio::table]] pkeytoutxo {
      uint64_t key;
      string publicKeyFrom;
      string publicKeyTo;
      double amount;
      capi_checksum256 signature;
      uint64_t primary_key() const { return key; }

      string publickeyfrom() const { return publicKeyFrom; }
      string publickeyto() const { return publicKeyTo; }
    };

    typedef eosio::multi_index<"pkeytoutxo"_n, pkeytoutxo,
      indexed_by<"pblckyfrm"_n, const_mem_fun<pkeytoutxo, string, &pkeytoutxo::publickeyfrom>>,
      indexed_by<"pblckyto"_n, const_mem_fun<pkeytoutxo, string, &pkeytoutxo::publickeyto>>
    > txtable;

};
EOSIO_DISPATCH(verifier, (assrtrcvrky))
