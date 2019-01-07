#pragma once

#include "data.hpp"
#include <eosiolib/time.hpp>
#include <string>



using namespace eosio;

namespace egame {
   
   struct [[eosio::table, eosio::contract("bitsplatform")]] user {
      name name;

      /**
       * The profile display name set by the user.
       * default = name str
       */ 
      string nickname;
      /**
       * the profile gender set by the user.
       * 1 is male, 0 is female, default 1
       */ 
      uint8_t gender = 1;
      /**
       * User selected profile photo ID
       */ 
      uint32_t head = 0;

      /**
       * Platform internal settlement unit for games.
       */ 
      asset point = asset( 0, POINT );
      /**
       * The total amount of recharge point.
       */ 
      asset recharge_cot = asset( 0, POINT );
      /**
       * the total amount of withdraw point.
       */ 
      asset withdraw_cot = asset( 0, POINT );
      /**
       * The game ID of the user currently.
       */ 
      struct name game_id;
      /**
       * the game table id of the user currently.
       */ 
      uint64_t table_id = 0;
      /**
       * User status: 0 is normal and 1 is locked for game, and 2 is locked for platform, and 3 is locked for cheat
       */ 
      uint8_t status = 0;

      /**
       * referrer
       */ 
      struct name referrer;

      /**
       * true, Already completed the first charge
       */ 
      bool is_first_recharge = false;
      /**
       * the last active time
       */ 
      eosio::time_point last_active_time;

      asset platform_token = asset( 0, MGT );

      uint64_t primary_key()const { return name.value; }

      uint64_t ByReferrer()const { return referrer.value; }

      uint64_t ByLastTime()const { return uint64_t( last_active_time.time_since_epoch().count() ); }

      bool is_empty()const{return point.amount == 0 && status == 0;}

      void UpdateLastTime( bool isPaying ) {
         if( !isPaying )
            last_active_time = eosio::time_point(microseconds(current_time()));
      }
      
      /* EOSLIB_SERIALIZE( user, (name)
                              (nickname)
                              (gender)
                              (head)
                              (point)
                              (recharge_cot)
                              (withdraw_cot)
                              (game_id)
                              (table_id)
                              (status)
                              (referrer)
                              (is_first_recharge)
                              (last_active_time)
                              (platform_token) ) */
   };

   typedef eosio::multi_index< "user"_n, user,
         indexed_by< "referrer"_n, const_mem_fun<user, uint64_t,  &user::ByReferrer> >,
         indexed_by< "lastime"_n, const_mem_fun<user, uint64_t,  &user::ByLastTime> >
      > user_index;
}