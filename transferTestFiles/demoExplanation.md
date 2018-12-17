# Testing the verification functions


First, we need to establish the transaction JSON. We'll be using the JSON below, with its condensed representation below that.
```
{
  "from": "EOS8X61Y4uzVBYcn3dDFAn63BDtKVi7GpLQ623bQvPN1x9z4UjBfo",
  "to": "EOS5ouREfkFyjRc1fVFCAUWZ43xjNvq35r3tuTwqNKdiqTRSm7hzi",
  "amount": "2 UTXO",
  "fee": "0 UTXO",
  "memo": "transferring 2 utxo from pub key 1 to pub key 2"
}
```


```
"{"from":"EOS8X61Y4uzVBYcn3dDFAn63BDtKVi7GpLQ623bQvPN1x9z4UjBfo","to":"EOS5ouREfkFyjRc1fVFCAUWZ43xjNvq35r3tuTwqNKdiqTRSm7hzi","amount":"2 UTXO","fee":"0 UTXO","memo":"transferring 2 utxo from pub key 1 to pub key 2"}"
```

If you need to produce the digest manually, run the condensed version through a sha256 function. You can do this online, or you can create an action on your contract
that leverages the sha256 function from eosio.
Example digest output:

1. (with all spaces and newlines removed) `"3b4c148632b2c9c23c57ba2536b0db2f454482c09398b7aa5b1bf5119e4acd75"`

2. (sha 256 on pretty print json -- note that this differs from the above!) `"d5dd28753286fe87dc4a050c77d70a2440f73eb8c3d1036647638fec95438db2"`


Now, get the signature by running the below:

**Note:** If you provide a file to the `sign` function, it will strip out spaces and newlines (i.e. if your JSON is pretty printed, the formatting will be removed before signing. This is important to keep in mind when creating your digest, as the data you provide to the sha256 function will need to be in the same format.)

`cleos sign {digestJSONString} || {digestJSONFile} -k {privateKey}`

`cleos sign "{"from":"EOS8X61Y4uzVBYcn3dDFAn63BDtKVi7GpLQ623bQvPN1x9z4UjBfo","to":"EOS5ouREfkFyjRc1fVFCAUWZ43xjNvq35r3tuTwqNKdiqTRSm7hzi","amount":"2 UTXO","fee":"0 UTXO","memo":"transferring 2 utxo from pub key 1 to pub key 2"}" -k "5JvsffwDPDDZLh3fAXUeX5HY88EwcSQAJ8wHSSZriftyDSGBQCU"`

`cleos sign {fileName} -k {privateKey}`

`cleos sign /Users/masonraasch/dev/contracts/eosio_utxo/transferTestFiles/transferTransactionJSON.json -k "5JvsffwDPDDZLh3fAXUeX5HY88EwcSQAJ8wHSSZriftyDSGBQCU"`

Signature Example:

SIG_K1_KYSqPUDrpdA5rtoiHJoTfMM9h8mE3MgPAGFkp9ZwJc58rhUcGFahZyGDKwWFSFJzpTNKfLAURPTs7KAcPzwYTLV9vzfrwT


Copy and paste the below into your terminal to run the action. Sub out the first value with your generated public key.
```
cleos push action relayer verifykey2 '["EOS8X61Y4uzVBYcn3dDFAn63BDtKVi7GpLQ623bQvPN1x9z4UjBfo", "SIG_K1_KYSqPUDrpdA5rtoiHJoTfMM9h8mE3MgPAGFkp9ZwJc58rhUcGFahZyGDKwWFSFJzpTNKfLAURPTs7KAcPzwYTLV9vzfrwT", "{"from":"EOS8X61Y4uzVBYcn3dDFAn63BDtKVi7GpLQ623bQvPN1x9z4UjBfo","to":"EOS5ouREfkFyjRc1fVFCAUWZ43xjNvq35r3tuTwqNKdiqTRSm7hzi","amount":"2 UTXO","fee":"0 UTXO","memo":"transferring 2 utxo from pub key 1 to pub key 2"}"]' -p relayer
```

