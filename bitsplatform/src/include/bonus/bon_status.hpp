#pragma once

#include <eosiolib/eosio.hpp>
#include <eosiolib/asset.hpp>
#include <eosiolib/singleton.hpp>
#include <string>

using namespace eosio;
using namespace std;

namespace egame {
   
   struct [[eosio::table("bonstatus"), eosio::contract("bitsplatform")]] BonusStatus {

      /**
       * The percentage of distributed tokens that initiate bonus
       */ 
      uint16_t min_start_rate = 3;

      /**
       * the platform revenue into the bonus pool ratio
       */ 
      uint16_t pool_rate = 50;

      /**
       * The minimum starting amount in the bonus pool (POINT)
       */ 
      asset min_start;

      /**
       * Minimum dividend interval (second)
       */ 
      int64_t  min_interval = seconds(24 * 60 * 60).count();

      /**
       * Bonus pool (POINT)
       */ 
      asset bonus_pool;

      /**
       * Whether it is paying dividends
       */ 
      bool paying = false;

      /**
       * Last assigned username
       */ 
      name last_name;

      /**
       * Single maximum calculation
       */ 
      uint32_t max_single_calc = 500;

      int64_t last_dividend_time;

      asset dividend_amount;

      double per_amount = 0;
      
   };

   typedef eosio::singleton< "bonstatus"_n, BonusStatus> BonusStatusSingleton;

   
}