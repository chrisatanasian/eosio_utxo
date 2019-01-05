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
cleos push action relayer transfer '["EOS7i2MKCvs5JdcSp8vctKJ3QKvs5qLiAJC9EwcRb1x5a9vyLsspy", "EOS6n6PDUvTYvJzgJoU3dnd1fps9muDTx56zRRGvPafe3vwCTwFYR", "SIG_K1_K2YizYyASFsvfy6Jmh8jgM3ED4WijzsUhH5X6JCiFYZfHz3iAaj8tCxfDEpAwaGJTfWhj7wftYeLwpL87Aq39NWdxyA8Wv", "766BD7E605A785AF9EE2B39E2265A389A743726019E6A4BCBA8A4D2F4BE5D65F", "2 UTXO", "0 UTXO", "transfer from second account to first account"]' -p relayer
cleos get table relayer relayer accounts

# fee
cleos push action relayer transfer '["EOS7i2MKCvs5JdcSp8vctKJ3QKvs5qLiAJC9EwcRb1x5a9vyLsspy", "EOS6n6PDUvTYvJzgJoU3dnd1fps9muDTx56zRRGvPafe3vwCTwFYR", "SIG_K1_KZtSVgv4vswd6F8DqxkGaaYwKLdAuwTBBRDQGAWt9MZVJ81D2ipGZTYDEw8MmBv4CLqAYQiYBoitKnPuTCx5PXcyHXxCVk", "46FDA4D38F067252F69E5C37193606D8788AB6D67CE9D4860C0C3AA50B6281B1", "2 UTXO", "1 UTXO", "transfer from second account to first account"]' -p relayer
cleos get table relayer relayer accounts
