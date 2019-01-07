#pragma once

#include "../base_reward.hpp"
#include <eosiolib/eosio.hpp>
#include <eosiolib/asset.hpp>
#include <eosiolib/singleton.hpp>
#include <string>

using namespace eosio;
using namespace std;

namespace egame {
   
   struct [[eosio::table("constatus"), eosio::contract("bitsplatform")]] ConsumeStatus : BaseReward {

      
      
   };

   typedef eosio::singleton< "constatus"_n, ConsumeStatus> ConsumeStatusSingleton;
}