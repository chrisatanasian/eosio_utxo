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
      void transfer(public_key pkeyFrom,
                    public_key pkeyTo,
                    asset amount,
                    asset fee,
                    signature sig,
                    string memo) {
        require_auth(_self);
        eosio_assert(pkeyFrom != pkeyTo, "cannot transfer to self");

        std::string data = "test_data";
        checksum256 digest = sha256(data, sizeof(data), &digest);

        assert_recover_key(digest, sig, pkeyFrom);

      }
};

EOSIO_DISPATCH(verifier, (transfer))
