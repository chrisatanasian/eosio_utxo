#include "eosiolib/eosio.hpp"
#include "eosiolib/print.hpp"
#include "eosiolib/types.h"
#include "eosiolib/crypto.h"

using namespace eosio;

class verifier : public contract {
  public:
      using contract::contract;

      [[eosio::action]]
      void assrtrcvrky() {
        capi_checksum256 result;
        const char* myString = "yo waddup";
        // below generates and unpacks a valid checksum256 
        sha256(myString, sizeof(myString), &result);
      }

};
EOSIO_DISPATCH(verifier, (assrtrcvrky))
