#include "eosiolib/eosio.hpp"
#include "eosiolib/print.hpp"
#include "eosiolib/types.h"
#include "eosiolib/crypto.h"

using namespace eosio;

class verifier : public contract {
  public:
      using contract::contract;

      [[eosio::action]]
      void assrtrcvrky(const capi_checksum256 digest, const char sig, size_t siglen, const char pub, size_t publen ) {
        capi_checksum256 result;
        const char* myString = "yo waddup";
        // below generates and unpacks a valid checksum256
        sha256(myString, sizeof(myString), &result);
      }

  private:
    struct [[eosio::table]] pkeytoutxo {
      uint64_t key;
      std::string publicKeyFrom;
      std::string publicKeyTo;
      float amount;
      capi_checksum256 signature;
      uint64_t primary_key() const { return key; }

      std::string get_by_public_key_from() const { return publicKeyTo; }
      std::string get_by_public_key_to() const { return publicKeyFrom; }
    };

    typedef eosio::multi_index<name("pkeytoutxo"), pkeytoutxo,
      indexed_by<name("pblckyfrm"), const_mem_fun<pkeytoutxo, std::string, &pkeytoutxo::get_by_public_key_from>>,
      indexed_by<name("pblckyto"), const_mem_fun<pkeytoutxo, std::string, &pkeytoutxo::get_by_public_key_to>>
    > txtable;

};
EOSIO_DISPATCH(verifier, (assrtrcvrky))
