#include "include/bitsplatform.hpp"
#include "include/referrer/referrer.hpp"
#include "include/referrer/refuser.hpp"
#include "include/data.hpp"

#include <string>
#include <math.h>

namespace egame {

   void platform::bind_referrer( name user, name ref )
   {
      if( user == ref || ref.value == 0 ) return;
      auto uItr = _users.find( user.value );
      if( uItr == _users.end() ) {
         _users.emplace( _self, [&]( auto& u ) {
            u.name = user;
            u.nickname = user.to_string();
            u.referrer = ref;
            u.UpdateLastTime( false );
         });
         _global.total_users += 1;
         _global.max_users += 1;
      } else {
         if( uItr->referrer.value <= 0 ) {
            _users.modify( uItr, same_payer, [&]( auto& u ) {
               u.referrer = ref;
               u.UpdateLastTime( _bon_status.paying );
            });
         } else {
            return;
         }
      }

      auto refuItr = _users.find( ref.value );
      if( refuItr == _users.end() ) {
         _users.emplace( _self, [&]( auto& u ) {
            u.name = ref;
            u.nickname = ref.to_string();
            u.UpdateLastTime( false );
         });
         _global.total_users += 1;
         _global.max_users += 1;
      }

      ReferrerIndex referrerTable( _self, _self.value );
      auto refItr = referrerTable.find( ref.value );
      if( refItr == referrerTable.end() ) {
         referrerTable.emplace( _self, [&]( auto& r ) {
            r.owner = ref;
            r.ref_count += 1;
            r.create_time = eosio::time_point_sec(now());
         });
      } else {
         referrerTable.modify( refItr, same_payer, [&]( auto& r ) {
            r.ref_count += 1;
            auto len = _ref_status.standards.size();
            for( int i = 0; i < len; i++ ) {
               if( r.ref_count > _ref_status.standards.at(i) ) {
                  r.current_level = i;
               }
               if(r.ref_count < _ref_status.standards.at(i) ) {
                  break;
               }
            }
         });
      }

      RefUserIndex refUserTable( _self, _self.value );
      refUserTable.emplace( _self, [&]( auto& ru ) {
         ru.id = refUserTable.available_primary_key();
         ru.referrer = ref;
         ru.referred = user;
         ru.create_time = eosio::time_point_sec(now());
      });
   }

   void platform::reward_referrer( name user, uint8_t type, int64_t amount )
   {
      auto uItr = _users.find( user.value );
      if( uItr != _users.end() ) {
         if( uItr->referrer == user ) {
            _users.modify( uItr, same_payer, [&]( auto& u ) {
               u.referrer = name(0);
            });
            return;
         }
         switch( type ) {
            case 0:
               if( uItr->is_first_recharge == false ) {
                  name referrer = uItr->referrer;
                  if( _ref_status.is_open && _ref_status.token_pool >= _ref_status.first_recharge_reward && referrer.value > 0 ) {
                     if( _ref_status.first_recharge_reward.amount > 0 ) {
                        _ref_status.token_pool -= _ref_status.first_recharge_reward;
                        SEND_INLINE_ACTION( *this, issue, { {_self, "active"_n} },
                           { referrer, _ref_status.first_recharge_reward, std::string("mg:RefReward:firstRecharge") }
                        );
                     }
                  }
               }
               break;
            case 1:
               if( uItr->referrer.value > 0 && _ref_status.is_open && amount > 0 ) {
                  ReferrerIndex refs( _self, _self.value );
                  auto refItr = refs.find( uItr->referrer.value );
                  if( refItr != refs.end() ) {
                     eosio_assert( _ref_status.gift_tokens.size() > refItr->current_level, "Invalid gift_tokens config" );
                     auto rate = _ref_status.gift_tokens[refItr->current_level]; 
                     if( rate > 100 ) rate = 100;
                     if( rate <= 0 ) return;
                     amount = amount * rate / 100;
                     asset reward = _ref_status.GetReward( amount );
                     if( reward.amount > 0 ) {
                        SEND_INLINE_ACTION( *this, issue, { {_self, "active"_n} },
                           { uItr->referrer, reward, std::string("mg:RefReward:consume") }
                        );
                     }
                  }
               }
               break;
            default:
               eosio_assert( false, "Invalid reward referrer Type" );
               break;
         }
      }
   }

   /***********actions************/

   void platform::bindref( name user, name ref )
   {
      require_auth( user );

      eosio_assert( user != ref, "cannot referred by self" );
      eosio_assert( is_account(ref), "Invalid referrer name" );

      if( _ref_status.is_open ){
         //auto refObj = _users.get( ref.value, "unable to find referrer account" );
         //eosio_assert( refObj.point.amount > 0, "the referrer did not meet the referral qualification" );

         bind_referrer( user, ref );
      }
   }

   void platform::addrefpool( name& user, asset& value )
   {
      eosio_assert( user != _self, "self cannot addrefpool" );
      eosio_assert( value.symbol == _ref_status.token_pool.symbol, "Invalid symbol" );
      eosio_assert( is_account(user), "Invalid user name" );
      eosio_assert( value.is_valid(), "invalid value" );
      eosio_assert( value.amount > 10000000, "the value is too small" );

      auto sym = value.symbol.code();
      tokenStats statstable( _self, sym.raw() );
      const auto& st = statstable.get( sym.raw() );
      eosio_assert( value.symbol == st.supply.symbol, "symbol precision mismatch" );

      require_auth( user );

      _ref_status.SetPool( value );
      
      sub_balance( user, value );
      add_balance( _self, value, _self );
   }

}