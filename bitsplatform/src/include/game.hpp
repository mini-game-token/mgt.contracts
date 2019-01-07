#pragma once

#include "data.hpp"
#include <string>

using namespace std;
using namespace eosio;

namespace egame {

   struct [[eosio::table, eosio::contract("bitsplatform")]] game {

      /**
       * the name of the game, type of account_name
       */ 
      name name;
      /**
       * display name
       */ 
      string display_name;
      /**
       * the description of the game
       */ 
      string description;
      /**
       * the status of the game, 0 is normal and 1 is closed
       */ 
      int8_t status = 0;
      /**
       * the income_cot of the game's total income
       */ 
      asset income_cot = asset( 0, POINT );
      /**
       * the bout_cot of the game's total bouts.
       */ 
      uint32_t bout_cot = 0;
      /**
       * the cur_bout_cot of the game's bout going on.
       */ 
      uint32_t cur_bout_cot = 0;
      /**
       * the total_person_time of the game's total person times.
       */ 
      uint32_t total_person_time = 0;
      /**
       * the big_category of the game's main category.
       */ 
      int16_t big_category = 0;
      /**
       * the small_category of the game's sub category.
       */ 
      int16_t small_category = 0;
      /**
       * minimum admittance balance.
       */
      asset min_point = asset( 0, POINT );
      /**
       * base bet
       */ 
      uint64_t base_bet = 1;
      /**
       * the fee_fixed of the game's fixed fee.
       */ 
      asset fee_fixed = asset( 0, POINT );
      /**
       * the fee_rate of the game's rate fee. NOTE: Not used yet.
       */ 
      uint16_t fee_rate = 0;
      /**
       * the max_multiple of the game's max multiple. default 6
       */ 
      uint32_t max_multiple = 6;
      /**
       * the point_game of game, Is it a point game? default true
       */ 
      bool point_game = true;
      /**
       * the ext_json of the game's expansion information 
       */ 
      string ext_json;

      uint64_t primary_key()const { return name.value; }
      uint64_t ByStatus()const { return (uint64_t)status; }
      uint64_t ByBigCategory()const { return (uint64_t)big_category; }
      uint64_t BySmallCategory()const { return (uint64_t)small_category; }

      /* EOSLIB_SERIALIZE( game, (name)
                           (display_name)
                           (description)
                           (status)
                           (income_cot)
                           (bout_cot)
                           (cur_bout_cot)
                           (total_person_time)
                           (big_category)
                           (small_category)
                           (min_point)
                           (base_bet)
                           (fee_fixed)
                           (fee_rate)
                           (max_multiple)
                           (point_game)
                           (ext_json)) */
   };

   typedef eosio::multi_index< "game"_n, game,
         indexed_by< "status"_n, const_mem_fun<game, uint64_t,  &game::ByStatus> >,
         indexed_by< "bigc"_n, const_mem_fun<game, uint64_t,  &game::ByBigCategory> >,
         indexed_by< "smallc"_n, const_mem_fun<game, uint64_t,  &game::BySmallCategory> >
      > game_index;
}