#! /bin/bash

printf "\t=========== Building egame.contracts ===========\n\n"

RED='\033[0;31m'
NC='\033[0m'

CORES=`getconf _NPROCESSORS_ONLN`
SYS_NAME="EOS"
NOTEST="notest"

function help()
{
   printf "\\tHelp:\\n\\t-t compile test\\n\\t-s <SYS_NAME>\\n\\t-c clean\\n\\t-h Help\\n"
   exit 1
}

function clean()
{
   if [ -d ./build ];then
      rm -rf ./build
   fi
}

if [ $# -ne 0 ];then
   while getopts ":ts:ch" opt; do
      case "${opt}" in
         t)
            NOTEST=""
         ;;
         s)
            SYS_NAME="${OPTARG}"
         ;;
         c)
            clean
            exit 1
         ;;
         h)
            help
            exit 1
         ;;
         \? )
            printf "\\n\\tInvalid Option: %s\\n" "-${OPTARG}" 1>&2
            help
            exit 1
         ;;
         : )
            printf "\\n\\tInvalid Option: %s requires an argument.\\n" "-${OPTARG}" 1>&2
            help
            exit 1
         ;;
         * )
            help
            exit 1
         ;;
      esac
   done
fi


printf "SYS_NAME = %s\n\n" "${SYS_NAME}" 1>&2
clean
mkdir -p build
pushd build &> /dev/null
cmake -DNOTEST=${NOTEST} -DSYS_NAME="${SYS_NAME}" ../
make -j${CORES}
popd &> /dev/null
