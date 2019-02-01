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
cleos push action relayer create '["relayer", "100 UTXO"]' -p relayer
cleos get table relayer 340784338176 stats

# issue
cleos push action relayer issue '["EOS7i2MKCvs5JdcSp8vctKJ3QKvs5qLiAJC9EwcRb1x5a9vyLsspy", "5 UTXO", "issue to first account"]' -p relayer
cleos push action relayer issue '["EOS7GGGfZoH3kE3DW2LwrZkNbJYfVn3NBwZZDYd39cMd89koK8s7h", "5 UTXO", "issue to second account"]' -p relayer
cleos get table relayer relayer accounts

# transfer with fee
cleos push action relayer transfer '["EOS8S4YmB2fPSZDopZ2pCHFy3p4gCmwHipo4NqwTAgLuggnqRHBAp", "EOS7GGGfZoH3kE3DW2LwrZkNbJYfVn3NBwZZDYd39cMd89koK8s7h", "EOS7i2MKCvs5JdcSp8vctKJ3QKvs5qLiAJC9EwcRb1x5a9vyLsspy", "2 UTXO", "1 UTXO", 1, "transfer from second account to first account", "SIG_K1_Jz9jZkNaBNNviBeHAce3Zi7yBpoDZpMD5ua1vUNCnShHAmhYStsUzf3SGGqZye9jZ6BNGhisB5wBG36kjRhhkhpxynhe1R"]' -p relayer
cleos get table relayer relayer accounts
cleos get table relayer 340784338176 stats
