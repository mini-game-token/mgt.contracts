#include "include/bitsplatform.hpp"
#include "include/data.hpp"
#include "include/bonus/bon_record.hpp"
#include "include/tools/tool.hpp"

#include <eosiolib/transaction.hpp>
#include <string>
#include <math.h>

namespace egame {

   const TokenStat& platform::get_token_stat( const symbol& sym )
   {
      tokenStats statstable( _self, sym.code().raw() );
      const auto& ts = statstable.get( sym.code().raw() );
      return ts;
   }

   bool platform::check_supply_rate( const TokenStat& ts )
   {
      if( ts.supply.amount <= 0 ) return false;
      int16_t msr = int16_t(double(ts.supply.amount) / double(ts.max_supply.amount) * 1000.00f);
      return msr >= _bon_status.min_start_rate;
   }

   asset platform::calc_income( asset& income )
   {
      asset val = income;
      if( val.amount > 0 ) {
         if( _bon_status.pool_rate > 0 && _bon_status.pool_rate < 100 ) {
            val = val * (100 - _bon_status.pool_rate) / 100;
         } else {
            val = asset( 0, val.symbol );
         }
      }
      return val;
   }

   void platform::add_point( asset fee )
   {
      if( fee.amount < 1 ) return;
      eosio_assert( fee.symbol == POINT, "The reward pool can only be POINT" );

      const auto& ts = get_token_stat( MGT );
      if( check_supply_rate(ts) == false ) {
         _global.income += fee;
         return;
      }

      auto pool = fee * _bon_status.pool_rate / 100;
      if( pool.amount > 0 ) {
         _bon_status.bonus_pool += pool;
         _global.income += fee - pool;
      } else {
         _global.income += fee;
      }
   }

   void platform::check_bonus()
   {
      if( _bon_status.paying || _global.pause_recharge || _global.pause_withdraw ) return;
      tokenStats statstable( _self, MGT.code().raw() );
      auto existing = statstable.find( MGT.code().raw() );
      if( existing != statstable.end() ) {
         const auto& ts = *existing;
         int16_t msr = int16_t(double(ts.supply.amount) / double(ts.max_supply.amount) * 1000.00f);
         if( msr >= _bon_status.min_start_rate && _bon_status.bonus_pool.amount >= _bon_status.min_start.amount &&
            current_time() - _bon_status.last_dividend_time >= _bon_status.min_interval ) {
            _bon_status.paying = true;
            _bon_status.dividend_amount = _bon_status.bonus_pool;
            _bon_status.bonus_pool = asset( 0, _bon_status.bonus_pool.symbol );
            _bon_status.per_amount = double(_bon_status.dividend_amount.amount) / double(ts.supply.amount);
            //_bon_status.last_dividend_time = current_time();
            clear_bonus_record();
         }
      }
   }

   void platform::clear_bonus_record()
   {
      auto act = action(
         permission_level{ _self, ACTIVE_PERMISSION },
         GAME_PLATFORM, "clearbon"_n,
         std::make_tuple(_self)
      );
      transaction trx;
      trx.delay_sec = 1;
      trx.actions.emplace_back( act );
      auto sid = (uint128_t(("mg.clearbon"_n).value) << 64 ) | current_time();
      trx.send( sid , _self, true);
   }

   void platform::loop_bonus()
   {
      auto act = action(
         permission_level{ _self, ACTIVE_PERMISSION },
         GAME_PLATFORM, "bonus"_n,
         std::make_tuple(_self)
      );
      transaction trx;
      trx.delay_sec = 5;
      trx.actions.emplace_back( act );
      auto sid = (uint128_t(("mg.bonus"_n).value) << 64 ) | current_time();
      trx.send( sid , _self, true);
   }

/*****************ACTION*******************/

   void platform::clearbon()
   {
      require_auth( _self );
      BonRecordIndex bonRecord( _self, _self.value );
      clear_table<BonRecordIndex>( bonRecord );
      auto itr = bonRecord.begin();
      if( itr != bonRecord.end() ) {
         clear_bonus_record();
      } else {
         loop_bonus();
      }
   }

   void platform::bonus()
   {
      require_auth( _self );

      if( _bon_status.paying == false )
         return;

      auto uItr = _users.begin();
      if( _bon_status.last_name.value > 0 ) {
         uItr = _users.find( _bon_status.last_name.value );
         if( uItr != _users.end() ) {
            uItr++;
         }
      }
      /* if( uItr == _users.end() ) {
         _bon_status.last_name = name(0);
         _bon_status.paying = false;
         _bon_status.last_dividend_time = current_time();
         return;
      } */
      uint32_t count = 0;
      auto bonus = asset( 0, POINT );
      BonRecordIndex bonRecord( _self, _self.value );
      while( uItr != _users.end() && count < _bon_status.max_single_calc ) {
         const auto& uObj = *uItr;
         _bon_status.last_name = uObj.name;
         if( uObj.platform_token.amount < 1 || 
            ( _bon_status.last_dividend_time > 0 && uObj.last_active_time.time_since_epoch().count() >= _bon_status.last_dividend_time )) {
            uItr++;
            count++;
            continue;
         }

         bonus = asset( int64_t( _bon_status.per_amount * double(uObj.platform_token.amount) ), POINT );
         if( bonus.amount > 0 ) {
            if( bonus > _bon_status.dividend_amount ) bonus = _bon_status.dividend_amount;
            _users.modify( uItr, same_payer, [&]( auto& u ) {
               u.point += bonus;
            });
         
            bonRecord.emplace( _self, [&]( auto& b ) {
               b.id = bonRecord.available_primary_key();
               b.owner = uObj.name;
               b.bonus = bonus;
               b.bonus_time = eosio::time_point_sec(now());
            });
            _bon_status.dividend_amount -= bonus;
         }
         
         uItr++;
         count++;
      }
      if( uItr == _users.end() || _bon_status.dividend_amount.amount < 1 ) {
         _bon_status.last_name = name(0);
         _bon_status.paying = false;
         //_bon_status.per_amount = 0;
         _bon_status.last_dividend_time = current_time();
         if( _bon_status.dividend_amount.amount > 0 ) {
            _bon_status.bonus_pool += _bon_status.dividend_amount;
            //_bon_status.dividend_amount = asset( 0, _bon_status.dividend_amount.symbol );
         }
      } else {
         loop_bonus();
      }
   }

   void platform::bonusout( asset total, const vector<bonusitem>& data )
   {
      require_auth( _self );
      //TODO:
   }
}