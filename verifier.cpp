#include <eosiolib/eosio.hpp>
#include <eosiolib/print.hpp>
#include <eosiolib/types.h>
#include <eosiolib/crypto.h>

using namespace eosio;

class verifier : public contract {
  public:
      using contract::contract;

      [[eosio::action]]
      void assrtrcvrky() {
      }
};
EOSIO_DISPATCH(verifier, (assrtrcvrky))
