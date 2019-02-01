const ecc = require('eosjs-ecc');
//// const hash = require('eosjs-ecc/src/hash')
//
//// generate our keys
//
//function ascii_to_hexa(str) {
//	var arr1 = [];
//	for (var n = 0, l = str.length; n < l; n ++) {
//		var hex = Number(str.charCodeAt(n)).toString(16);
//		arr1.push(hex);
//	}
//	return arr1.join('');
//}
//
//function getMyKeys() {
//  return Promise.all([ecc.randomKey(), ecc.randomKey()])
//}
//
//function createKeyPair(privateKey) {
//  return {
//    public: ecc.privateToPublic(privateKey),
//    private: privateKey
//  }
//}
//async function main() {
//  const myKeys = await getMyKeys();
//  console.log('my keys:', myKeys);
//  const keys = {};
//  keys.to = createKeyPair(myKeys[0])
//  keys.from = createKeyPair(myKeys[1])
//
//  // create  raw transaction json
//  const rawData = createRawData(keys.from.public, keys.to.public);
//  // convert to hex
//  const hexedData = ascii_to_hexa(JSON.stringify(rawData));
//  // create real transaction json
//  const transactionJSON = createTransaction(hexedData);
//
//  // now to sign the data
//  // const stringifiedData = JSON.stringify(transactionJSON)
//  const stringifiedData = ""
//
//  const signature = ecc.sign(stringifiedData, keys.from.private);
//  console.log('signature', signature);
//  const verified = ecc.verify(signature, stringifiedData, keys.from.public);
//  console.log('does god care?', verified);
//}
//
//// main();
//
//const createRawData = function(from, to) {
//  return {
//    "from": from,
//    "to": to,
//    "amount": "2 UTXO",
//    "fee": "0 UTXO",
//    "memo": "transferring 2 utxo from pub key 1 to pub key 2"
//  };
//}
//
//const createTransaction = function(hexedData) {
//  return {
//    "actions": [
//      {
//        "data": hexedData
//      }
//    ],
//  }
//}
//
//
//const sig = ecc.sign(
//  '{"amount":"2. UTXO","fee":"1. UTXO","from":"EOS7i2MKCvs5JdcSp8vctKJ3QKvs5qLiAJC9EwcRb1x5a9vyLsspy","memo":"transfer from first account to second account","to":"EOS6n6PDUvTYvJzgJoU3dnd1fps9muDTx56zRRGvPafe3vwCTwFYR"}',
//  "5JnbNk8MsC9vK5dVcwYgW6kr2hRg5utraXxba1k15D16BX4wJBT"
//);
//console.log('sig: ', sig)

const base58 = require('bs58');

const from = "EOS7DNtsH1hh7y7DuFEpBhFCMS4HEogVMivpVaEVSYemT76z3oKvS";
const relayer = from;
const to = "EOS6n6PDUvTYvJzgJoU3dnd1fps9muDTx56zRRGvPafe3vwCTwFYR";
const amount = 2;
const fee = 0;
const nonce = 1;
const memo = "transfer from first account to second account"
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
const sender_wif = "5Kh33Z8Nwnfwnyifrh2ULBNRcP2uTrbBwTMvsiY1gbsoBuiawjJ";
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

