const ecc = require('eosjs-ecc');
const base58 = require('bs58');

const relayer = "EOS8S4YmB2fPSZDopZ2pCHFy3p4gCmwHipo4NqwTAgLuggnqRHBAp";
const from = "EOS7GGGfZoH3kE3DW2LwrZkNbJYfVn3NBwZZDYd39cMd89koK8s7h";
const to = "EOS7i2MKCvs5JdcSp8vctKJ3QKvs5qLiAJC9EwcRb1x5a9vyLsspy";
const amount = 2;
const fee = 1;
const nonce = 1;
const memo = "transfer from second account to first account"
const version = 1;
const length = 92 + memo.length;

const pkey_from = base58.decode(from.substring(3));
const pkey_to = base58.decode(to.substring(3));
const amount_buf = uint64_to_little_endian(amount);
const fee_buf = uint64_to_little_endian(fee);
const nonce_buf = uint64_to_little_endian(nonce);
const memo_buf = Buffer.from(memo);

// create raw tx
const buffer = Buffer.alloc(length);
buffer[0] = version;
buffer[1] = length;
pkey_from.copy(buffer, 2, 0, 33);
pkey_to.copy(buffer, 35, 0, 33);
amount_buf.copy(buffer, 68, 0, 8);
fee_buf.copy(buffer, 76, 0, 8);
nonce_buf.copy(buffer, 84, 0, 8);
memo_buf.copy(buffer, 92, 0, memo_buf.length);
console.log("Raw tx:", buffer.toString('hex'))

// hash raw tx
const hashed = ecc.sha256(buffer, 'hex');
console.log("SHA-256:", hashed);

// sign transaction
const sender_wif = "5KfKKJ5x1Z51sXvf2PXsiQRBkpzZbYMe7X8Q8uwNr3Pb1eARoGL";
const sig = ecc.signHash(hashed, sender_wif);
console.log("Signature:", sig);

// Output cleos command
console.log("---- CLEOS COMMAND ----");
console.log(`cleos push action relayer transfer '["${relayer}", "${from}", "${to}", "${amount} UTXO", "${fee} UTXO", ${nonce}, "${memo}", "${sig}"]' -p relayer`)

// due to JS limitaitons, this only has 48-bit precision,
// but that's good enough for what we need
function uint64_to_little_endian(num) {
    const buf = Buffer.alloc(8);
    buf.writeUIntLE(num, 0, 6);
    return buf;
}

