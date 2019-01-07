#pragma once

#include <eosiolib/eosio.hpp>
#include <eosiolib/time.hpp>
#include <string>



using namespace eosio;

namespace egame {
   
   struct [[eosio::table, eosio::contract("bitsplatform")]] BonRecord {
      uint64_t id;

      name owner;

      asset bonus;

      eosio::time_point_sec bonus_time;

      uint64_t primary_key()const { return id; }

      uint64_t ByOwner()const { return owner.value; }

   };

   typedef eosio::multi_index< "bonrecord"_n, BonRecord,
         indexed_by< "owner"_n, const_mem_fun<BonRecord, uint64_t,  &BonRecord::ByOwner> >
      > BonRecordIndex;
}