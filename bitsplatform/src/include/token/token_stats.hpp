#pragma once

#include <eosiolib/asset.hpp>
#include <eosiolib/eosio.hpp>

namespace egame {

   struct [[eosio::table, eosio::contract("bitsplatform")]] TokenStat {
      asset supply;
      asset max_supply;
      name issuer;
      asset team;
      asset team_claim;

      uint64_t primary_key()const { return supply.symbol.code().raw(); }

   };

   typedef eosio::multi_index< "stat"_n, TokenStat > tokenStats;
}