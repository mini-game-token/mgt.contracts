#pragma once

#include <eosiolib/eosio.hpp>
#include <eosiolib/asset.hpp>

using namespace eosio;

namespace egame {

   #define POINT symbol(symbol_code("POINT"), 0)
   #define SYS symbol(symbol_code("EOS"), 4)
   #define MGT symbol(symbol_code("MGT"), 4)

   #define GAME_PLATFORM ("bitsplatform"_n)
   #define NAME_EOSIO_TOKEN name("eosio.token")

   #define GAME_PERMISSION name("game")
   #define ACTIVE_PERMISSION name("active")

   #define ACTION_NAME_TRANSFER name("transfer")
   #define ACTION_NAME_GAMENTER name("gamenter")
   #define ACTION_NAME_SETTLE name("settle")
   #define ACTION_NAME_LOCKUSER name("lockuser")

   #define EOS_DISPATCH( TYPE, MEMBERS ) \
   extern "C" { \
      void apply( uint64_t receiver, uint64_t code, uint64_t action ) { \
         if( code == name("eosio.token").value && action == name("transfer").value ) { \
            eosio::execute_action( eosio::name(receiver), eosio::name(code), &TYPE::deposit ); \
         } else if( code == name("eosio").value && action == name("onerror").value ) { \
            eosio::execute_action( eosio::name(receiver), eosio::name(code), &TYPE::OnError ); \
         } else if( code == receiver ) { \
            switch( action ) { \
               EOSIO_DISPATCH_HELPER( TYPE, MEMBERS ) \
               default: \
                  eosio_assert( false, "Invalid action request" ); \
                  break; \
            } \
            /* does not allow destructor of thiscontract to run: eosio_exit(0); */ \
         } \
      } \
   } \

   #define POINT_DISPATCH( TYPE, MEMBERS ) \
   extern "C" { \
      void apply( uint64_t receiver, uint64_t code, uint64_t action ) { \
         if( code == name("eosio").value && action == name("onerror").value ) { \
            eosio::execute_action( eosio::name(receiver), eosio::name(code), &TYPE::OnError ); \
         } else if( code == receiver ) { \
            switch( action ) { \
               EOSIO_DISPATCH_HELPER( TYPE, MEMBERS ) \
               default: \
                  eosio_assert( false, "Invalid action request" ); \
                  break; \
            } \
            /* does not allow destructor of thiscontract to run: eosio_exit(0); */ \
         } \
      } \
   } \

   #define MGT_DISPATCH( TYPE, MEMBERS ) \
   extern "C" { \
      void apply( uint64_t receiver, uint64_t code, uint64_t action ) { \
         if( code == name("bitsplatform").value && action == name("transfer").value ) { \
            eosio::execute_action( eosio::name(receiver), eosio::name(code), &TYPE::deposit ); \
         } else if( code == name("eosio").value && action == name("onerror").value ) { \
            eosio::execute_action( eosio::name(receiver), eosio::name(code), &TYPE::OnError ); \
         } else if( code == receiver ) { \
            switch( action ) { \
               EOSIO_DISPATCH_HELPER( TYPE, MEMBERS ) \
               default: \
                  eosio_assert( false, "Invalid action request" ); \
                  break; \
            } \
            /* does not allow destructor of thiscontract to run: eosio_exit(0); */ \
         } \
      } \
   } \
   
   struct [[eosio::contract("bitsplatform")]] settlement {
      name user;
      bool winner = false;
      asset quantity;

      //EOSLIB_SERIALIZE( settlement, (user) (winner) (quantity) )
   };

   struct [[eosio::contract("bitsplatform")]] bonusitem {
      name user;
      int64_t bonus;

      //EOSLIB_SERIALIZE( bonusitem, (user) (bonus) )
   };
}