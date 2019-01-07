#pragma once

#include "data.hpp"
#include <eosiolib/singleton.hpp>
#include <string>

namespace egame {

   struct [[eosio::table, eosio::contract("bitsplatform")]] global {
      /**
       * The ratio of points to EOS.
       */ 
      uint32_t point_rate = 1000;
      /**
       * Withdrawal fee
       */
      asset fee_withdraw = asset( 0, POINT );
      /**
       * Recharge gift points
       */
      asset gift_recharge = asset( 0, POINT );
      
      /**
       * The total amount of points in the platform.
       */
      asset total_point = asset( 0, POINT );
      /**
       * The total amount of EOS is recharged in the platform.
       */
      asset total_recharge = asset( 0, SYS );
      /**
       * The total amount of EOS is withdraw in the platform.
       */
      asset total_withdraw = asset( 0, SYS );
      /**
       * The total amount of games in the platform.
       */
      uint32_t total_games = 0;
      /**
       * The total amount of users in the platform.
       */
      uint32_t total_users = 0;
      /**
       * The max amount of users in the platform.
       */
      uint32_t max_users = 0;
      /**
       * Total withdrawal fee points
       */
      asset total_fee_withdraw = asset( 0, POINT );
      /**
       * total recharge gift points
       */
      asset total_gift_recharge = asset( 0, POINT );
      /**
       * Whether to pause withdrawal
       */
      bool pause_withdraw = false;
      /**
       * Whether to pause recharge
       */
      bool pause_recharge = false;

      asset income = asset( 0, POINT );

      /* EOSLIB_SERIALIZE( global, (point_rate)
                               (fee_withdraw)
                               (gift_recharge)
                               (total_point)
                               (total_recharge)
                               (total_withdraw)
                               (total_games)
                               (total_users)
                               (max_users)
                               (total_fee_withdraw)
                               (total_gift_recharge)
                               (pause_withdraw)
                               (pause_recharge)) */
   };

   typedef eosio::singleton< "global"_n, global> bits_global_singleton;
}