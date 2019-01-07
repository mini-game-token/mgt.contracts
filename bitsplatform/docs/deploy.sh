#! /bin/bash

printf "\t=========== Deploy BitsPlatform ===========\n\n"

RED='\033[0;31m'
NC='\033[0m'

SVR_URL=https://eosx.eosinfra.io/
ACTIVE_PUB="EOS5nVzU2u2Zhq1wp4z5W9tVvLoSCgzg37DEkifV7VFSiY86WVzXw"
GAME_PUB="EOS5PTUVpeSBNGWqqrUM5WeDvkeqYC6N6YfL8CbNF81BNMQu8XZwv"

if [ -z $1 ];then
   echo "You can set an API address"
else
   SVR_URL=$1
fi

if [ -z $2 ];then
   echo "You can set an game public key"
else
   GAME_PUB=$2
fi

JSON=`cleos -u ${SVR_URL} get account bitsplatform -j`
JSON=${JSON#*'"perm_name": "active",'}
JSON=${JSON%%'"accounts":'*}
JSON=${JSON#*'"key": "'}
ACTIVE_PUB=${JSON%%'",'*}

AUTH="{\"threshold\":1,\"keys\":[{\"key\":\""${ACTIVE_PUB}"\",\"weight\":1}],\"accounts\":[{\"permission\":{\"actor\":\"bitsplatform\",\"permission\":\"eosio.code\"},\"weight\":1}]}"

cleos -u ${SVR_URL} set contract bitsplatform ./build/bitsplatform -p bitsplatform &&
cleos -u ${SVR_URL} push action bitsplatform create '[ "bitsplatform", "100000000.0000 MGT",40]' -p bitsplatform &&
cleos -u ${SVR_URL} set account permission bitsplatform active ${AUTH} owner -p bitsplatform
cleos -u ${SVR_URL} set account permission bitsplatform game ${GAME_PUB} active -p bitsplatform@active
cleos -u ${SVR_URL} set action permission bitsplatform bitsplatform addgame game -p bitsplatform@active
cleos -u ${SVR_URL} set action permission bitsplatform bitsplatform updategame game -p bitsplatform@active
cleos -u ${SVR_URL} set action permission bitsplatform bitsplatform delgame game -p bitsplatform@active