# wallet setup
rm ~/eosio-wallet/default.wallet
cleos wallet create --to-console
cleos wallet import --private-key 5KQwrPbwdL6PhXujxW37FSSQZ1JiwsST4cqQzDeyXtP79zkvFD3
cleos wallet import --private-key 5J5U8vCjHSkEcqCXHYDiQPPfCP3amCsGa2uWaDXsc3KEXZGWprd

# account setup
cleos create account eosio relayer EOS7C6WfvDVzAifCXyAwUtq2bLmDZ6sWXoeiWGY7vcSXane7hB1jB

# compilation and setting
eosio-cpp -w -o verifier.wasm verifier.cpp
cleos set contract relayer ../verifier

# create
cleos push action relayer create '["EOS8S4YmB2fPSZDopZ2pCHFy3p4gCmwHipo4NqwTAgLuggnqRHBAp", "100 UTXO"]' -p relayer
cleos get table relayer 340784338176 stats

# issue
cleos push action relayer issue '["EOS7PcgxVxfBLSFMhrgYn7LtvP8eLg2BYjCx5yesGw2bdoiabWoY7", "5 UTXO", "issue to first account"]' -p relayer
cleos push action relayer issue '["EOS8MCxWjToU77qy8RhqFcXd3xNWAihb2rULGLuZ6rdhEZWdxC9RJ", "2 UTXO", "issue to second account"]' -p relayer
cleos get table relayer relayer accounts

# transfer
cleos push action relayer transfer '["EOS7PcgxVxfBLSFMhrgYn7LtvP8eLg2BYjCx5yesGw2bdoiabWoY7", "EOS8MCxWjToU77qy8RhqFcXd3xNWAihb2rULGLuZ6rdhEZWdxC9RJ", "2 UTXO", "0 UTXO", "transfer from second account to first account"]' -p relayer
cleos get table relayer relayer accounts
