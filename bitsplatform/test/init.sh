cleos wallet unlock

cleos create account eosio eosio.token EOS8M1jvYGgV2XuPqS2nbZaW9tRZfUjT1N88BxJVeF4jVFDLfSAzQ EOS8M1jvYGgV2XuPqS2nbZaW9tRZfUjT1N88BxJVeF4jVFDLfSAzQ &&

cleos create account eosio bitsplatform EOS8M1jvYGgV2XuPqS2nbZaW9tRZfUjT1N88BxJVeF4jVFDLfSAzQ EOS8M1jvYGgV2XuPqS2nbZaW9tRZfUjT1N88BxJVeF4jVFDLfSAzQ &&

cleos create account eosio player1 EOS8M1jvYGgV2XuPqS2nbZaW9tRZfUjT1N88BxJVeF4jVFDLfSAzQ EOS8M1jvYGgV2XuPqS2nbZaW9tRZfUjT1N88BxJVeF4jVFDLfSAzQ &&

cleos create account eosio player2 EOS8M1jvYGgV2XuPqS2nbZaW9tRZfUjT1N88BxJVeF4jVFDLfSAzQ EOS8M1jvYGgV2XuPqS2nbZaW9tRZfUjT1N88BxJVeF4jVFDLfSAzQ &&

cleos set contract eosio /Users/necklace/work/eos/eosio.contracts/build/eosio.bios -p eosio &&

cleos set contract eosio.token /Users/necklace/work/eos/eosio.contracts/build/eosio.token -p eosio.token &&

cleos set contract bitsplatform ./build/bitsplatform -p bitsplatform &&

cleos set account permission bitsplatform active '{"threshold": 1,"keys": [{"key": "EOS8M1jvYGgV2XuPqS2nbZaW9tRZfUjT1N88BxJVeF4jVFDLfSAzQ","weight": 1}],"accounts": [{"permission":{"actor":"bitsplatform","permission":"eosio.code"},"weight":1}]}' owner -p bitsplatform &&

cleos push action eosio.token create '[ "eosio", "1000000000.0000 EOS", 0, 0, 0]' -p eosio.token &&

cleos push action eosio.token issue '[ "player1", "1000.0000 EOS", "memo" ]' -p eosio &&

cleos push action eosio.token issue '[ "player2", "1000.0000 EOS", "memo" ]' -p eosio &&

cleos set account permission bitsplatform game 'EOS8M1jvYGgV2XuPqS2nbZaW9tRZfUjT1N88BxJVeF4jVFDLfSAzQ' active -p bitsplatform@active &&

cleos set action permission bitsplatform bitsplatform addgame game -p bitsplatform@active &&

cleos push action bitsplatform create '[ "bitsplatform", "100000000.0000 MGT",40]' -p bitsplatform &&

#cleos push action bitsplatform issue '[ "player1", "1000.0000 MGT", "memo" ]' -p bitsplatform &&

#cleos get currency balance bitsplatform player1 MGT &&

#cleos get currency balance bitsplatform player2 MGT &&

cleos get currency stats bitsplatform MGT # &&

#cleos transfer player1 player2 -c bitsplatform "100.0000 MGT" "hello minigame"
