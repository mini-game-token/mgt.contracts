cleos transfer player1 bitsplatform "1.0000 EOS" "mg:deposit" &&

cleos transfer player2 bitsplatform "1.0000 EOS" "mg:deposit:player1" &&

cleos get table bitsplatform bitsplatform global &&

cleos get currency stats bitsplatform MGT &&

cleos get currency balance bitsplatform player1 MGT &&

cleos get currency balance bitsplatform player2 MGT &&

cleos create account eosio plat1 EOS8M1jvYGgV2XuPqS2nbZaW9tRZfUjT1N88BxJVeF4jVFDLfSAzQ EOS8M1jvYGgV2XuPqS2nbZaW9tRZfUjT1N88BxJVeF4jVFDLfSAzQ &&

cleos create account eosio plat2 EOS8M1jvYGgV2XuPqS2nbZaW9tRZfUjT1N88BxJVeF4jVFDLfSAzQ EOS8M1jvYGgV2XuPqS2nbZaW9tRZfUjT1N88BxJVeF4jVFDLfSAzQ &&

cleos push action eosio.token issue '[ "plat1", "1000.0000 EOS", "memo" ]' -p eosio &&

cleos push action eosio.token issue '[ "plat2", "1000.0000 EOS", "memo" ]' -p eosio &&

cleos transfer plat1 bitsplatform "1.0000 EOS" "mg:deposit:plat2"

