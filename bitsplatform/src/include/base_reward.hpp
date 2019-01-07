#pragma once

#include "data.hpp"
#include <eosiolib/asset.hpp>
#include <string>

using namespace eosio;

namespace egame {
   
   struct BaseReward {

      asset token_total = asset( 0, MGT );

      asset token_pool = asset( 0, MGT );

      /**
       * Whether to enable the referral function
       */ 
      bool is_open = true;

      asset first_recharge_reward = asset( 0, MGT );

      /**
       * get reward
       * @pre val SYS amount
       */ 
      asset GetReward( int64_t val ) {
         if( token_total.amount <= 0 || val < 1 || token_pool.amount <= 0 ) {
            return asset( 0, token_pool.symbol );
         }
         double scale = double(token_pool.amount) / double(token_total.amount) * 500.00f + 1.00f;
         int64_t amount = int64_t(double(val) * scale);
         auto m = asset( amount, token_pool.symbol );
         if( token_pool >= m ) {
            token_pool -= m;
            return m;
         } else {
            is_open = false;
            m = token_pool;
            token_pool = asset( 0, token_pool.symbol );
            return m;
         }
      }

      void SetPool( asset& val ) {
         token_total += val;
         token_pool += val;
         is_open = true;
      }

   };

}