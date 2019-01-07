#pragma once

#include <eosiolib/eosio.hpp>
#include <eosiolib/time.hpp>
#include <string>



using namespace eosio;

namespace egame {
   
   struct [[eosio::table, eosio::contract("bitsplatform")]] RefUser {
      uint64_t id;

      name referrer;

      name referred;

      eosio::time_point_sec create_time;

      uint64_t primary_key()const { return id; }

      uint64_t ByReferrer()const { return referrer.value; }

      uint64_t ByReferred()const { return referred.value; }

   };

   typedef eosio::multi_index< "refuser"_n, RefUser,
         indexed_by< "referrer"_n, const_mem_fun<RefUser, uint64_t,  &RefUser::ByReferrer> >,
         indexed_by< "referred"_n, const_mem_fun<RefUser, uint64_t,  &RefUser::ByReferred> >
      > RefUserIndex;
}