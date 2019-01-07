#pragma once

#include <eosiolib/asset.hpp>
#include <eosiolib/eosio.hpp>

namespace egame {

   struct [[eosio::table, eosio::contract("bitsplatform")]] Balance {
      asset balance;

      uint64_t primary_key()const { return balance.symbol.code().raw(); }
   };

   typedef eosio::multi_index< "accounts"_n, Balance > balances;
}