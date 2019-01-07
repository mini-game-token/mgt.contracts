#! /bin/bash

printf "\t=========== Building egame.contracts ===========\n\n"

RED='\033[0;31m'
NC='\033[0m'

CORES=`getconf _NPROCESSORS_ONLN`

if [ -d ./build ];then
   rm -rf ./build
fi

if [ -z $1 ];then
   echo "Will compile the test"
else
   if [ $1 == "notest" ];then
      NOTEST=$1
   fi
fi

mkdir -p build
pushd build &> /dev/null
cmake -DNOTEST=${NOTEST} ../
make -j${CORES}
popd &> /dev/null
