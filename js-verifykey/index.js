const ecc = require('eosjs-ecc');
// const hash = require('eosjs-ecc/src/hash')

// generate our keys

function ascii_to_hexa(str) {
	var arr1 = [];
	for (var n = 0, l = str.length; n < l; n ++) {
		var hex = Number(str.charCodeAt(n)).toString(16);
		arr1.push(hex);
	}
	return arr1.join('');
}

function getMyKeys() {
  return Promise.all([ecc.randomKey(), ecc.randomKey()])
}

function createKeyPair(privateKey) {
  return {
    public: ecc.privateToPublic(privateKey),
    private: privateKey
  }
}
async function main() {
  const myKeys = await getMyKeys();
  console.log('my keys:', myKeys);
  const keys = {};
  keys.to = createKeyPair(myKeys[0])
  keys.from = createKeyPair(myKeys[1])

  // create  raw transaction json
  const rawData = createRawData(keys.from.public, keys.to.public);
  // convert to hex
  const hexedData = ascii_to_hexa(JSON.stringify(rawData));
  // create real transaction json
  const transactionJSON = createTransaction(hexedData);

  // now to sign the data
  // const stringifiedData = JSON.stringify(transactionJSON)
  const stringifiedData = ""

  const signature = ecc.sign(stringifiedData, keys.from.private);
  console.log('signature', signature);
  const verified = ecc.verify(signature, stringifiedData, keys.from.public);
  console.log('does god care?', verified);
}

// main();

const createRawData = function(from, to) {
  return {
    "from": from,
    "to": to,
    "amount": "2 UTXO",
    "fee": "0 UTXO",
    "memo": "transferring 2 utxo from pub key 1 to pub key 2"
  };
}

const createTransaction = function(hexedData) {
  return {
    "actions": [
      {
        "data": hexedData
      }
    ],
  }
}


const sig = ecc.sign(
  '{"amount":"2. UTXO","fee":"1. UTXO","from":[3,115,83,47,-60,106,-45,115,105,54,-65,107,-61,-18,93,-86,-22,122,-115,-74,-106,22,-119,-53,-109,43,96,-13,30,-112,41,19,56],"memo":"transfer from first account to second account","to":[2,-8,-34,-123,45,70,-110,-69,-28,26,116,-115,23,-126,-110,-89,-2,99,-25,23,-20,-26,14,57,93,-56,126,94,56,-41,-14,-107,29]}',
  "5JnbNk8MsC9vK5dVcwYgW6kr2hRg5utraXxba1k15D16BX4wJBT"
);
console.log('sig: ', sig)
