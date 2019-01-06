# wallet setup
rm ~/eosio-wallet/default.wallet
cleos wallet create --to-console
cleos wallet import --private-key 5KQwrPbwdL6PhXujxW37FSSQZ1JiwsST4cqQzDeyXtP79zkvFD3 # EOS key
cleos wallet import --private-key 5J5U8vCjHSkEcqCXHYDiQPPfCP3amCsGa2uWaDXsc3KEXZGWprd # relayer

# account setup
cleos create account eosio relayer EOS7C6WfvDVzAifCXyAwUtq2bLmDZ6sWXoeiWGY7vcSXane7hB1jB

# compilation and setting
eosio-cpp -w -o verifier.wasm verifier.cpp
cleos set contract relayer ../verifier

# create
cleos push action relayer create '["EOS8S4YmB2fPSZDopZ2pCHFy3p4gCmwHipo4NqwTAgLuggnqRHBAp", "100 UTXO"]' -p relayer
cleos get table relayer 340784338176 stats

# issue
cleos push action relayer issue '["EOS7i2MKCvs5JdcSp8vctKJ3QKvs5qLiAJC9EwcRb1x5a9vyLsspy", "10 UTXO", "issue to first account"]' -p relayer
cleos push action relayer issue '["EOS6n6PDUvTYvJzgJoU3dnd1fps9muDTx56zRRGvPafe3vwCTwFYR", "2 UTXO", "issue to second account"]' -p relayer
cleos get table relayer relayer accounts

# transfer
cleos push action relayer transfer '["EOS7i2MKCvs5JdcSp8vctKJ3QKvs5qLiAJC9EwcRb1x5a9vyLsspy", "EOS6n6PDUvTYvJzgJoU3dnd1fps9muDTx56zRRGvPafe3vwCTwFYR", "SIG_K1_KY9nyphcZVuQRu1qMRJjRuNXmpgg3VnfjWoyaoysJ453kYChHh6HXEsNt1PKEgDuoXxEbsy1tbY7FAwUCkJYVNFYXcq5C2", "2 UTXO", "0 UTXO", "transfer from first account to second account"]' -p relayer
cleos get table relayer relayer accounts

# fee
cleos push action relayer transfer '["EOS7i2MKCvs5JdcSp8vctKJ3QKvs5qLiAJC9EwcRb1x5a9vyLsspy", "EOS6n6PDUvTYvJzgJoU3dnd1fps9muDTx56zRRGvPafe3vwCTwFYR", "SIG_K1_KkVrHaYUegByWTCD4LVWH11nse687kKc3DgHucQn6wgwEr66z252dDLz3jjd3Y3WN3J4xFDgFBLqd3CbtDQEHmFf6CcqSG", "2 UTXO", "1 UTXO", "transfer from first account to second account"]' -p relayer
cleos get table relayer relayer accounts
