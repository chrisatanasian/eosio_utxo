#include <eosiolib/eosio.hpp>

using namespace eosio;

class verifier : public contract {
public:
  verifier(account_name self) : contract(self) {}

  // @abi action
  int recover_key(const checksum256* digest,
                  const char* sig,
                  size_t siglen,
                  char* pub,
                  size_t publen);

private:
};

EOSIO_ABI(verifier, (recover_key))
