#include "include/bitsplatform.hpp"
#include "include/consume/con_status.hpp"
#include "include/data.hpp"

#include <string>
#include <math.h>

namespace egame {

   void platform::reward_consume( name user, uint8_t type, int64_t amount )
   {
      auto uItr = _users.find( user.value );
      if( uItr == _users.end() ) {
         uItr = _users.emplace( _self, [&]( auto& u ) {
            u.name = user;
            u.nickname = user.to_string();
            u.UpdateLastTime( false );
         });
         _global.total_users += 1;
         _global.max_users += 1;
      }
      switch( type ) {
         case 0:
            if( uItr->is_first_recharge == false ) {
               if( _con_status.is_open && _con_status.token_pool >= _con_status.first_recharge_reward ) {
                  _con_status.token_pool -= _con_status.first_recharge_reward;
                  SEND_INLINE_ACTION( *this, issue, { {_self, "active"_n} },
                     { user, _con_status.first_recharge_reward, std::string("mg:Reward:firstRecharge") }
                  );
               }
            }
            break;
         case 1:
            if( _con_status.is_open && amount > 0 ) {
               asset reward = _con_status.GetReward( amount );
               if( reward.amount > 0 ) {
                  SEND_INLINE_ACTION( *this, issue, { {_self, "active"_n} },
                     { user, reward, std::string("mg:Reward:consume") }
                  );
               }
            }
            break;
         default:
            eosio_assert( false, "Invalid reward consume Type" );
            break;
      }
   }

   /*********actions********/

   void platform::reward( const name& game_name, const name& user, const int64_t amount, const name& ref )
   {
      require_auth( game_name );
      auto g_itr = _games.find( game_name.value );
      eosio_assert( g_itr != _games.end(), "the game does not exist on the platform." );
      eosio_assert( g_itr->status == 0, "this game is off status." );
      eosio_assert( is_account( user ), "Invalid user name" );
      eosio_assert( amount > 0, "Invalid amount" );

      if( is_account( ref ) && _ref_status.is_open ) {
         bind_referrer( user, ref );
      }

      if( _con_status.is_open )
         reward_consume( user, 1, amount );

      if( _ref_status.is_open )
         reward_referrer( user, 1, amount );
   }

   void platform::addconpool( name& user, asset& value )
   {
      eosio_assert( user != _self, "self cannot addconpool" );
      eosio_assert( value.symbol == _con_status.token_pool.symbol, "Invalid symbol" );
      eosio_assert( is_account(user), "Invalid user name" );
      eosio_assert( value.is_valid(), "invalid value" );
      eosio_assert( value.amount > 10000000, "the value is too small" );

      auto sym = value.symbol.code();
      tokenStats statstable( _self, sym.raw() );
      const auto& st = statstable.get( sym.raw() );
      eosio_assert( value.symbol == st.supply.symbol, "symbol precision mismatch" );

      require_auth( user );

      _con_status.SetPool( value );
      
      statstable.modify( st, same_payer, [&]( auto& s ) {
         s.supply -= value;
      });
      sub_balance( user, value );
   }
}