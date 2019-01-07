#pragma once

#include <eosiolib/eosio.hpp>
#include <eosiolib/time.hpp>



using namespace eosio;

namespace egame {
   
   struct [[eosio::table, eosio::contract("bitsplatform")]] Referrer {

      name owner;

      uint32_t current_level = 0;

      /**
       * Current referrers
       */ 
      uint32_t ref_count = 0;

      time_point_sec create_time;

      uint64_t primary_key()const { return owner.value; }

   };

   typedef multi_index< "referrer"_n, Referrer > ReferrerIndex;
}