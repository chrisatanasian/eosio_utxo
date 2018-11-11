#include "eosiolib/eosio.hpp"
#include "eosiolib/print.hpp"
#include "eosiolib/types.h"
#include "eosiolib/crypto.h"

using namespace eosio;
using namespace std;


class verifier : public contract {
  public:
      using contract::contract;

      static eosio::key256 pk_to_key256(const string& pkstr) {
        key256 returnKey;
        if (input == "") {
            returnKey = key256::make_from_word_sequence<uint64_t>(0ULL, 0ULL, 0ULL, 0ULL);
        }
        else {
            // This is needed for indexing since indexes cannot be done by strings, only max key256's, for now...
            uint64_t p1 = eosio::string_to_name(pkstr.substr(3, 15).c_str());
            uint64_t p2 = eosio::string_to_name(pkstr.substr(15, 27).c_str());
            uint64_t p3 = eosio::string_to_name(pkstr.substr(27, 39).c_str());
            uint64_t p4 = eosio::string_to_name(pkstr.substr(39, 51).c_str());
            returnKey = key256::make_from_word_sequence<uint64_t>(p1, p2, p3, p4);
        }
        return returnKey;
      }

      bool assrtrcvrky(const capi_checksum256 digest,
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

      [[eosio::action]]
      void addutxo( name relayer, const string& pkeyFee,
                    const string& pkeyFrom, const string& pkeyTo,
                    uint_64_t amount, uint64_t fee,     
                    const char sig) {
        
        //basic housekeeping
        require_auth( relayer );
        eosio_assert( amount - fee > 0, "insufficient amount given fee" );
        
        //e.g. https://github.com/EOSIO/eosjs-ecc/issues/20
        //strlen(SIG_K1_KgMrn3yteiHtoUnqBjBcVhjuJRkeXAfwaYFQaCDmMC6sD7mGU5qQRaz3GHWe96Mfvq5Ei56EHBiwjh7sg6GYjBGzcRv81Y) = 101
        //strlen(EOS5dWHMmRX26RBcK46z6YVf4yxtVAamtwyUyuUuWYkEwXZ6L1UDP) = 52
        // -1 b/c sizeof counts the null character '\0'...so:
        eosio_assert( sizeof(sig) == 100 && 
                      sizeof(pkeyFrom) == 52 && 
                      sizeof(pkeyTo) == 52 && 
                      sizeof(pkeyFee) == 52 ); 
        
        //assert will pass if bool helper returns true
        eosio_assert(
          assrtcvrky( capi_checksum256 digest,
                      sig, 101, pkFrom, 53 );
        );
        //signature checks out, let's rule out "double spend"

        
        txtable transactions(_self, _self.value);
        auto txd = transactions.get_index<N(bypk)>();
        
        key256 senderKey = pk_to_key256(pkeyFrom);
        auto sender = txd.find(senderKey);

        eosio_assert (sender != txd.end(), "no sender available");
        eosio_assert(sender.balance - amount >= 0, "double spend attempt");
        
        transactions.modify( sender, same_payer, [&]( auto& tx ) 
        { tx.balance -= amount; });   

        key256 receiverKey = pk_to_key256(pKeyTo);
        auto receiver = txd.find(receiverKey);
        if ( receiver == prop_idx.end() ) 
          transactions.emplace(relayer, [&]( auto& tx ) {
            tx.id = transactions.available_primary_key();
            tx.publickey = pkeyFrom;
            tx.balance = amount - fee;
          }
        else 
          transactions.modify( fee_receiver, same_payer, [&]( auto& tx ) 
          { tx.balance += amount - fee; });
        
        key256 feeKey = pk_to_key256(pKeyFee);
        auto fee_receiver = txd.find(feeKey);
        if ( fee_receiver == prop_idx.end() ) 
          transactions.emplace(relayer, [&]( auto& tx ) {
            tx.id = transactions.available_primary_key();
            tx.publickey = pkeyFee;
            tx.balance = fee;
          });
        else
          transactions.modify( fee_receiver, same_payer, [&]( auto& tx ) 
          { tx.balance += fee; });   
    } 
          
  private:
    struct [[eosio::table]] pkeytoutxo {
      uint64_t key;
      string publicKey;
      uint_64_t amount;
      
      uint64_t primary_key() const { return key; }
      key256 bypk() const { return pk_to_key256(publicKey); }
     
    };

    typedef eosio::multi_index<"pkeytoutxo"_n, pkeytoutxo,
      indexed_by<"bypk"_n, const_mem_fun<pkeytoutxo, eosio::key256, &pkeytoutxo::bypk>>,
    > txtable;

};
EOSIO_DISPATCH(verifier, (assrtrcvrky))
