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
  const isNotFuckedUp = ecc.verify(signature, stringifiedData, keys.from.public);
  console.log('does god care?', isNotFuckedUp);
  
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
  '["EOS7PcgxVxfBLSFMhrgYn7LtvP8eLg2BYjCx5yesGw2bdoiabWoY7", "EOS8MCxWjToU77qy8RhqFcXd3xNWAihb2rULGLuZ6rdhEZWdxC9RJ", "2 UTXO", "1 UTXO", "transfer from second account to first account"]',
   "5KFXjPG8VM6JeAijjp8aCzPrM1MQYcdThDfN4vjXSHUE4TV3grS"
   );
console.log('sig: ', sig)