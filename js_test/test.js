const ecc = require('eosjs-ecc');
const base58 = require('bs58');
const shell = require('shelljs');

function getKeys() {
  return Promise.all([ecc.randomKey(), ecc.randomKey(), ecc.randomKey()]);
}

function createKeyPair(privateKey) {
  return {
    public: ecc.privateToPublic(privateKey),
    private: privateKey,
  };
}

function transferCommand(relayer, fromKeys, to, amount, fee, nonce, memo) {
  const version = 1;
  const length = 92 + memo.length;

  const pkeyFrom = base58.decode(fromKeys.public.substring(3));
  const pkeyTo = base58.decode(to.substring(3));
  const amountBuf = uint64_to_little_endian(amount);
  const feeBuf = uint64_to_little_endian(fee);
  const nonceBuf = uint64_to_little_endian(nonce);
  const memoBuf = Buffer.from(memo);

  // create raw tx
  const buffer = Buffer.alloc(length);
  buffer[0] = version;
  buffer[1] = length;
  pkeyFrom.copy(buffer, 2, 0, 33);
  pkeyTo.copy(buffer, 35, 0, 33);
  amountBuf.copy(buffer, 68, 0, 8);
  feeBuf.copy(buffer, 76, 0, 8);
  nonceBuf.copy(buffer, 84, 0, 8);
  memoBuf.copy(buffer, 92, 0, memoBuf.length);

  // hash raw tx
  const hashed = ecc.sha256(buffer, 'hex');

  // sign transaction
  const sig = ecc.signHash(hashed, fromKeys.private);

  return `cleos push action relayer transfer '["${relayer}", "${fromKeys.public}", "${to}", "${amount} UTXO", "${fee} UTXO", ${nonce}, "${memo}", "${sig}"]' -p relayer`;
}

// due to JS limitaitons, this only has 48-bit precision,
// but that's good enough for what we need
function uint64_to_little_endian(num) {
  const buf = Buffer.alloc(8);
  buf.writeUIntLE(num, 0, 6);
  return buf;
}

async function test() {
  const privateKeys = await getKeys();
  const relayerKeys = createKeyPair(privateKeys[0]);
  const firstAccountKeys = createKeyPair(privateKeys[1]);
  const secondAccountKeys = createKeyPair(privateKeys[2]);

  // wallet setup
  shell.exec('rm ~/eosio-wallet/default.wallet');
  shell.exec('cleos wallet create --to-console');
  shell.exec('cleos wallet import --private-key 5KQwrPbwdL6PhXujxW37FSSQZ1JiwsST4cqQzDeyXtP79zkvFD3'); // EOS key
  shell.exec(`cleos wallet import --private-key ${relayerKeys.private}`);

  // account setup
  shell.exec(`cleos create account eosio relayer ${relayerKeys.public}`);

  // compilation and setting
  shell.exec('eosio-cpp -w -o ../verifier/verifier.wasm ../verifier/verifier.cpp');
  shell.exec('cleos set contract relayer ../verifier');

  // create
  shell.exec('cleos push action relayer create \'["relayer", "100 UTXO"]\' -p relayer');
  shell.exec('cleos get table relayer 340784338176 stats');

  // issue
  shell.exec(`cleos push action relayer issue '["${firstAccountKeys.public}", "5 UTXO", "issue to first account"]' -p relayer`);
  shell.exec(`cleos push action relayer issue '["${secondAccountKeys.public}", "5 UTXO", "issue to second account"]' -p relayer`);
  shell.exec('cleos get table relayer relayer accounts');

  // transfer
  shell.exec(transferCommand(relayerKeys.public, secondAccountKeys, firstAccountKeys.public, 3, 1, 1, "transfer from second account to first"));
  shell.exec('cleos get table relayer relayer accounts');

  // transfer with fee
  shell.exec(transferCommand(relayerKeys.public, firstAccountKeys, secondAccountKeys.public, 1, 0, 1, "transfer from second account to first"));
  shell.exec('cleos get table relayer relayer accounts');

  shell.exec('cleos get table relayer 340784338176 stats');
}

test();
