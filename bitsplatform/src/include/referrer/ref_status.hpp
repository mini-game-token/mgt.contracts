#pragma once

#include "../base_reward.hpp"
#include <eosiolib/eosio.hpp>
#include <eosiolib/singleton.hpp>
#include <string>



using namespace eosio;
using namespace std;

namespace egame {
   
   struct [[eosio::table("refstatus"), eosio::contract("bitsplatform")]] RefStatus : BaseReward {

      /**
       * title of per level
       */ 
      vector<string> titles;

      /**
       * Number of people meeting the standard
       */ 
      vector<uint32_t> standards;

      /**
       * Token number % of gifts per level
       */ 
      vector<uint32_t> gift_tokens;

   };

   typedef eosio::singleton< "refstatus"_n, RefStatus > RefStatusSingleton;
}