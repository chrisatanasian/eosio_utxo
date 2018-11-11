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
      void transfer(public_key pkeyfrom,
                    public_key pkeyto,
                    asset amount,
                    asset fee,
                    signature sig,
                    string memo) {
        //require_auth(_self);
        //eosio_assert(string(pkeyFrom.data) != string(pkeyTo.data), "cannot transfer to self");

        std::string data = "test_data";
        checksum256 digest = sha256(data.c_str(), data.length());
        assert_recover_key(digest, sig, pkeyfrom);
      }

};

EOSIO_DISPATCH(verifier, (transfer))
