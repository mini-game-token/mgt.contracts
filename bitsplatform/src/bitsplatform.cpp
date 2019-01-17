#include <eosiolib/eosio.hpp>
#include <eosiolib/time.hpp>
#include <math.h>
#include "include/bitsplatform.hpp"
#include "include/tools/tool.hpp"

#include "token.cpp"
#include "referrer.cpp"
#include "consume.cpp"
#include "bonus.cpp"

namespace egame {

   platform::platform( name receiver, name code, datastream<const char*> ds ) : eosio::contract( receiver, code, ds ),
         _users(_self, _self.value),
         _games(_self, _self.value),
         _global_table(_self, _self.value),
         _ref_status_table(_self, _self.value),
         _con_status_table(_self, _self.value),
         _bon_status_table(_self, _self.value)
   {
      _sys_precision = uint32_t( pow(10, SYS.precision()) );
      _global = _global_table.exists() ? _global_table.get() : get_default_state();
      _ref_status = _ref_status_table.exists() ? _ref_status_table.get() : get_default_ref_status();
      _con_status = _con_status_table.exists() ? _con_status_table.get() : get_default_consume_status();
      _bon_status = _bon_status_table.exists() ? _bon_status_table.get() : get_default_bonus_status();
      //_bon_status.last_dividend_time=1545915720000000;
   }

   bool platform::check_point()
   {
      asset all_point = (_global.total_recharge - _global.total_withdraw) * _global.point_rate / _sys_precision;
      if( _global.total_point.amount != all_point.amount + _global.total_gift_recharge.amount) {
         _global.pause_recharge = true;
         _global.pause_withdraw = true;
         return false;
      }
      return true;
   }

   void platform::deposit( name from, name to, asset quantity, string memo )
   {
      //print("=========deposit");
      if( _self != GAME_PLATFORM ) return;
      if( from == _self ) return;
      if( to != _self ) return;
      if( memo.find(string("mg:deposit")) == -1 ) return;

      eosio_assert( _global.pause_recharge == false, "The recharge has been suspended." );
      eosio_assert( from != to, "cannot transfer to self" );
      require_auth( from );
      //eosio_assert( to == _self, "target account is not platform");
      
      eosio_assert( quantity.symbol == SYS, "asset must be system token" );
      eosio_assert( quantity.is_valid(), "invalid quantity" );
      eosio_assert( quantity.amount > 0, "must transfer positive quantity for platform" );

      asset point = quantity * _global.point_rate / _sys_precision;
      point = asset( point.amount, POINT );
      eosio_assert( point.is_valid(), "invalid quantity" );
      eosio_assert( point.amount > 0, "must deposit positive quantity" );

      asset value = quantity;
      auto den = _sys_precision / _global.point_rate;
      if( den > 0 )
         value = value / den * den;
      
      auto gameItr = _games.find( from.value );
      if( gameItr != _games.end() ) { //income
         _global.total_recharge += value;
         _global.total_point += point;
         return;
      }

      auto param = split( memo, ":" );
      name referrer;
      if( param.size() > 2 ) {
         referrer = name( param[2] );
      }

      auto uitr = _users.find( from.value );
      if( uitr == _users.end() ) {
         //reg and recharge
         uitr = _users.emplace( _self, [&]( auto& u ) {
            u.name = from;
            u.nickname = from.to_string();
            u.point = point + _global.gift_recharge;
            u.recharge_cot = point;
            u.UpdateLastTime( false );
         });
         _global.total_users += 1;
         _global.max_users += 1;
      } else {
         //recharge
         _users.modify( uitr, same_payer, [&]( auto& u ) {
            u.point += point;
            u.recharge_cot += point;
            u.UpdateLastTime( _bon_status.paying );
         });
      }
      
      _global.total_recharge += value;
      _global.total_point += point + _global.gift_recharge;
      _global.total_gift_recharge += _global.gift_recharge;

      if( _ref_status.is_open ) { //referrer
         bind_referrer( from, referrer );
         reward_referrer( from, 0, value.amount );
      }
      if( _con_status.is_open ) { //consume
         reward_consume( from, 0, value.amount );
      }
      if( uitr->is_first_recharge == false ) {
         _users.modify( uitr, same_payer, [&]( auto& u ) {
            u.is_first_recharge = true;
         });
      }
      check_bonus();
   }

   void platform::withdraw( const name& to, const asset& quantity )
   {
      eosio_assert( _global.pause_withdraw == false, "The withdraw has been suspended." );
      
      eosio_assert( is_account( to ), "withdraw:to account does not exist");
      require_auth( to );

      eosio_assert( quantity.symbol == POINT, "asset must be point" );//only point
      eosio_assert( quantity.is_valid(), "invalid quantity" );
      eosio_assert( quantity.amount > 0, "must withdraw positive quantity" );

      auto itr = _users.find( to.value );
      eosio_assert(itr != _users.end(), "the account does not exist on the platform.");
      eosio_assert(itr->status == 0, "the account in the game or locked cannot be withdraw.");

      eosio_assert( itr->point >= quantity, "insufficient balance" );
      eosio_assert( quantity <= _global.total_point - _global.income, "invalid quantity" );

      //check point and SYS
      eosio_assert( check_point(), "platform data verification failed." );

      asset sys = asset( quantity.amount * _sys_precision / _global.point_rate, SYS );
      //updata state data
      auto den = _global.point_rate / _sys_precision;
      asset qt = quantity;
      if( den > 0 )
         qt = qt / den * den;
      _global.total_point -= qt;
      _global.total_withdraw += sys;
      //updata user data
      _users.modify( itr, same_payer, [&]( auto& acnt ) {
         //eosio_assert( acnt.point >= qt, "insufficient balance" );
         acnt.point -= qt;
         acnt.withdraw_cot += qt;
         acnt.UpdateLastTime( _bon_status.paying );
      });

      action(
         permission_level{ _self, ACTIVE_PERMISSION },
         NAME_EOSIO_TOKEN, ACTION_NAME_TRANSFER,
         std::make_tuple(_self, to, sys, std::string("mg:withdraw"))
      ).send();

      if( itr->is_empty() ) {
         //TODO:Do not delete
         //_users.erase( itr );
         //_global.total_users -= 1;
      }
      check_bonus();
   }

   void platform::settle( const name& game,  const uint64_t table, const vector<settlement>& data )
   {
      require_auth( game );//Custom game permissions
      
      //check game
      auto g_itr = _games.find( game.value );
      eosio_assert( g_itr != _games.end(), "the game does not exist on the platform." );
      //eosio_assert( g_itr->point_game, "this game is not a point game." );
      eosio_assert( g_itr->status == 0, "this game is off status." );

      asset winner;
      asset loser;

      if ( g_itr->point_game ) {
         //check point and SYS
         eosio_assert( check_point(), "platform data verification failed." );

         //check settlement
         eosio_assert( data.size() >= 2, "invalid settlement data" );

         winner = asset( 0, POINT );
         loser = asset( 0, POINT );
      } else {
         //check settlement
         eosio_assert( data.size() >= 1, "invalid settlement data" );

         winner = asset( 0, g_itr->income_cot.symbol );
         loser = asset( 0, g_itr->income_cot.symbol );
      }
      
      for( auto& s : data ) {
         //check account
         eosio_assert( is_account( s.user ), "settle:to account does not exist");
         auto itr = _users.find( s.user.value );
         
         //check quantity
         if( g_itr->point_game ) {
            eosio_assert( itr != _users.end(), "the account does not exist on the platform.");
            eosio_assert( itr->game_id == game, "the user is not in the specified game." );
            eosio_assert( itr->table_id == table, "the user is not in the specified table." );
            eosio_assert( s.quantity.symbol == POINT, "asset must be point" );
         } else {
            eosio_assert( s.quantity.symbol == SYS || s.quantity.symbol == MGT, "asset must be system token" );
         }
         eosio_assert( s.quantity.is_valid(), "invalid quantity" );
         eosio_assert( s.quantity.amount > 0, "must settle positive quantity" );

         if( s.winner ) {
            winner += s.quantity;
         } else {
            if( g_itr->point_game ) {
               eosio_assert( itr->point >= s.quantity, "insufficient balance" );
            }
            loser += s.quantity;
         }
      }
      if ( g_itr->point_game )
         eosio_assert( winner.amount == loser.amount, "invalid settlement amount" );

      //update points
      asset income;
      if( g_itr->point_game ) {
         income = asset( 0, POINT );
         asset fee = asset( 0, POINT );
         for( auto& su : data ) {
            fee = su.quantity * g_itr->fee_rate / 100;
            auto itr = _users.find( su.user.value );
            _users.modify( itr, same_payer, [&]( auto& acnt ) {
               if( su.winner ) {
                  acnt.point += su.quantity;
                  if( g_itr->fee_rate > 0 ) {
                     acnt.point -= fee;
                     income += calc_income( fee );
                  }
               } else {
                  acnt.point -= su.quantity;
               }
               acnt.UpdateLastTime( _bon_status.paying );
               if( acnt.status == 1 ) acnt.status = 0;
               acnt.game_id = name();
               acnt.table_id = 0;
            });
            //reward referrer fee_rate
            if( su.winner && g_itr->fee_rate > 0 ) {
               add_point( fee );
               auto fee_amount = fee.amount * _sys_precision / _global.point_rate;
               if( fee_amount > 0 && itr->referrer.value > 0 && _ref_status.is_open && g_itr->fee_rate > 0 ) {
                  reward_referrer( su.user, 1, int64_t(fee_amount) );
               }
               if( fee_amount > 0 && _con_status.is_open && g_itr->fee_rate > 0 ) {
                  reward_consume( su.user, 1, int64_t(fee_amount) );
               }
            }
         }
      } else {
         income = asset( 0, data[0].quantity.symbol );
         if( g_itr->fee_rate > 0 ) {
            for( auto& su : data ) {
               if( su.winner ) {
                  auto sysfee = su.quantity * g_itr->fee_rate / 100;
                  if( su.quantity.symbol == SYS ) {
                     income += calc_income( sysfee );
                     //SYS
                     asset fee_sys = sysfee * _global.point_rate / _sys_precision;
                     //POINT
                     asset fee_point = asset( fee_sys.amount, POINT );
                     add_point( fee_point );
                  } else {
                     income += sysfee;
                  }
               }
            }
         }
      }

      //update game data
      _games.modify( g_itr, same_payer, [&]( auto& g ) {
         g.income_cot += income;
         g.cur_bout_cot -=1;
      });
      
   }

   void platform::pause()
   {
      require_auth( _self );
      _global.pause_recharge = true;
      _global.pause_withdraw = true;
   }

   void platform::open()
   {
      require_auth( _self );
      _global.pause_recharge = false;
      _global.pause_withdraw = false;
      check_bonus();
   }

   void platform::updateuser( const name& u, const string& nickname, const uint8_t gender, const uint32_t head)
   {
      require_auth( u );
      eosio_assert( is_account( u ), "u account does not exist");

      auto itr = _users.find( u.value );
      eosio_assert(itr != _users.end(), "the account does not exist on the platform.");

      if(nickname != "") {
         size_t len = nickname.length();
         eosio_assert( (len >= 3 && len <= 24), "invalid nickname" );

         auto chinese = IsChinese(nickname);
         auto str = IsWord(nickname);
         eosio_assert( chinese || str, "invalid nickname" );
      }

      _users.modify( itr, same_payer, [&]( auto& acnt ) {
         if(nickname != "")
            acnt.nickname = nickname;
         acnt.gender = gender;
         acnt.head = head;
         acnt.UpdateLastTime( _bon_status.paying );
      });
   }

   void platform::addgame( const game& g)
   {
      require_auth( _self );
      //require_auth( _self.value, name("game").value );

      auto itr = _games.find( g.name.value );
      eosio_assert( itr == _games.end(), "This game is already on the platform.");

      size_t len = g.display_name.length();
      eosio_assert( (len >= 3 && len <= 30), "invalid display name" );

      len = g.description.length();
      eosio_assert( (len >= 30 && len <= 200), "invalid description" );

      eosio_assert( g.ext_json.length() < 1000, "ext too long.");

      eosio_assert( g.fee_rate >= 0 && g.fee_rate <= 100, "invalid fee_rate" );

      eosio_assert( g.max_multiple > 0, "invalid max multiple");

      if( g.point_game ) {
         eosio_assert( g.income_cot.symbol == POINT, "asset must be point" );
      } else {
         eosio_assert( g.income_cot.symbol == SYS || g.income_cot.symbol == MGT, "asset must be system token" );
      }

      _games.emplace( _self, [&](auto& _g){
         _g.name = g.name;
         _g.display_name = g.display_name;
         _g.description = g.description;
         _g.status = g.status;
         _g.big_category = g.big_category;
         _g.small_category = g.small_category;
         _g.ext_json = g.ext_json;
         _g.fee_fixed = g.fee_fixed;
         _g.fee_rate = g.fee_rate;
         _g.point_game = g.point_game;
         _g.min_point = g.min_point;
         _g.income_cot = g.income_cot;
         _g.base_bet = g.base_bet;
      });
      check_bonus();
   }

   void platform::updategame( const game& g)
   {
      //require_auth2( _self.value, name("game").value );
      require_auth( _self );

      auto itr = _games.find( g.name.value );
      eosio_assert(itr != _games.end(), "the game does not exist on the platform.");

      size_t len = g.display_name.length();
      eosio_assert( (len >= 3 && len <= 30), "invalid display name" );

      len = g.description.length();
      eosio_assert( (len >= 30 && len <= 200), "invalid description" );

      eosio_assert( g.ext_json.length() < 1000, "ext too long.");

      eosio_assert( g.max_multiple > 0, "invalid max multiple");

      if( g.point_game ) {
         eosio_assert( g.income_cot.symbol == POINT, "asset must be point" );
      } else {
         eosio_assert( g.income_cot.symbol == SYS || g.income_cot.symbol == MGT, "asset must be system token" );
      }

      _games.modify( itr, same_payer, [&]( auto& _g ) {
         if( g.display_name != "" )
            _g.display_name = g.display_name;
         if( g.description != "" )
            _g.description = g.description;
         if( g.status > -1 )
            _g.status = g.status;
         if( g.big_category > -1 )
            _g.big_category = g.big_category;
         if( g.small_category > -1 )
            _g.small_category = g.small_category;
         if( g.ext_json != "" )
            _g.ext_json = g.ext_json;
         _g.fee_fixed = g.fee_fixed;
         _g.fee_rate = g.fee_rate;
         _g.min_point = g.min_point;
         _g.base_bet = g.base_bet;
      });
      check_bonus();
   }

   void platform::delgame( const name& game_name )
   {
      //require_auth2( _self.value, name("game").value );
      require_auth( _self );

      auto itr = _games.find( game_name.value );
      eosio_assert( itr != _games.end(), "the game does not exist on the platform.");

      _games.erase( itr );
      check_bonus();
   }

   void platform::gamenter( const name& gname, const vector<name>& users, const uint64_t table )
   {
      //require_auth2( gname.value, GAME_PERMISSION.value );
      require_auth( gname );

      auto gitr = _games.find( gname.value );
      eosio_assert( gitr != _games.end(), "the game does not exist on the platform.");
      //eosio_assert( gitr->point_game, "this game is not a point game." );
      eosio_assert( gitr->status == 0, "this game is off status." );

      eosio_assert( users.size() > 0, "invalid users" );

      for( auto& user : users ) {
         eosio_assert( is_account( user ), "gamenter:to account does not exist");
         if( gitr->point_game ) {
            auto uite = _users.find( user.value );
            eosio_assert( uite != _users.end(), "the account does not exist on the platform." );
            auto minpoint = max( gitr->min_point, gitr->fee_fixed );
            eosio_assert( uite->point >= ( minpoint ), "the balance cannot be less than the minimum min_point balance.");
         }
      }

      //fee and update user data
      if( gitr->point_game ) {
         for( auto& user : users ) {
            auto uite = _users.find( user.value );
            _users.modify( uite, same_payer, [&]( auto& acnt ) {
               acnt.game_id = gname;
               acnt.table_id = table;
               acnt.status = 1;
               acnt.UpdateLastTime( _bon_status.paying );
               if( gitr->fee_fixed.amount > 0 && gitr->point_game ) {
                  acnt.point -= gitr->fee_fixed;
               }
            });
            //reward 
            if( gitr->fee_fixed.amount > 0 ) {
               add_point( gitr->fee_fixed );
               auto fee_amount = gitr->fee_fixed.amount * _sys_precision / _global.point_rate;
               if( fee_amount > 0 ) {
                  //referrer
                  if( uite->referrer.value > 0 && _ref_status.is_open ) {
                     reward_referrer( user, 1, int64_t(fee_amount) );
                  }
                  //consume
                  if( _con_status.is_open ) {
                     reward_consume( user, 1, int64_t(fee_amount) );
                  }
               }
            }
         }
      }

      //update game data
      _games.modify( gitr, same_payer, [&]( auto& g ) {
         if( g.fee_fixed.amount > 0 ) {
            asset income = gitr->fee_fixed * users.size();
            if( g.fee_fixed.symbol == SYS ) {
               g.income_cot += calc_income( income );
            } else {
               g.income_cot += income;
            }
         }
         g.bout_cot += 1;
         g.cur_bout_cot +=1;
         g.total_person_time += users.size();
      });
      check_bonus();
   }

   void platform::lockuser( const name& gname, const vector<name> users )
   {
      //require_auth2( gname.value, GAME_PERMISSION.value );
      require_auth( gname );
      eosio_assert( users.size() > 0, "users size must be than 0" );

      auto gitr = _games.find( gname.value );
      eosio_assert( gitr != _games.end(), "the game does not exist on the platform.");

      for( auto& u : users ) {
         auto uitr = _users.find( u.value );
         if( uitr != _users.end() ) {
            _users.modify( uitr, same_payer, [&]( auto& user ) {
               user.status = 3;
            });
         }
      }
      check_bonus();
   }

   void platform::claimincome( const name& to, const asset& quantity )
   {
      require_auth( _self );

      eosio_assert( is_account( to ), "claimincome:to account does not exist");
      eosio_assert( quantity.symbol == POINT, "asset must be point" );//only point
      eosio_assert( quantity.is_valid(), "invalid quantity" );
      eosio_assert( quantity.amount > 0, "must withdraw positive quantity" );
      eosio_assert( quantity.amount <= _global.income.amount, "must be less than or equal to the amount of income" );
      //check point and SYS
      eosio_assert( check_point(), "platform data verification failed." );

      asset sys = asset( quantity.amount * _sys_precision / _global.point_rate, SYS );
      //updata state data
      auto den = _global.point_rate / _sys_precision;
      asset qt = quantity;
      if( den > 0 )
         qt = qt / den * den;
      _global.total_point -= qt;
      _global.income -= qt;
      _global.total_withdraw += sys;

      action(
         permission_level{ _self, ACTIVE_PERMISSION },
         NAME_EOSIO_TOKEN, ACTION_NAME_TRANSFER,
         std::make_tuple(_self, to, sys, std::string("mg:claimincome"))
      ).send();
   }

   void platform::OnError( const onerror& error )
   {
      uint64_t id = get_send_id( error.sender_id );
      if( id == ("mg.clearbon"_n).value || id == ("mg.bonus"_n).value ) {
         uint128_t newid = (uint128_t(id) << 64) | current_time();
         transaction dtrx = error.unpack_sent_trx();
         dtrx.send(newid, _self);
      }
   }

   void platform::reset(){
      require_auth( _self );
      _global = get_default_state();
      _ref_status = get_default_ref_status();
      _con_status = get_default_consume_status();
      _bon_status = get_default_bonus_status();

      clear_table(_games);
      
      tokenStats statstable( _self, MGT.code().raw() );
      clear_table(statstable);

      ReferrerIndex referrerTable( _self, _self.value );
      clear_table(referrerTable);

      RefUserIndex refUserTable( _self, _self.value );
      clear_table(refUserTable);

      BonRecordIndex bonRecord( _self, _self.value );
      clear_table(bonRecord);

      uint32_t m = 0;
      auto uItr = _users.begin();
      while( uItr != _users.end() && m < 500 ) {
         balances acnts( _self, uItr->name.value );
         auto it = acnts.find( MGT.code().raw() );
         if( it != acnts.end() ) {
            acnts.erase( it );
         }
         uItr = _users.erase( uItr );
         m++;
      }
   }
}

EOS_DISPATCH( egame::platform, (deposit)
                     (withdraw)
                     (settle)
                     (pause)
                     (open)
                     (updateuser)
                     (addgame)
                     (updategame)
                     (delgame)
                     (gamenter)
                     (lockuser)
                     (claimincome)
                     (reset) 
                     /*****token*****/
                     (create)
                     (issue)
                     (retire)
                     (transfer)
                     (claim)
                     (closetoken)
                     /*****token end*****/

                     /*****referrer*****/
                     (bindref)
                     (addrefpool)
                     /*****referrer end*****/

                     /*****consume*****/
                     (reward)
                     (addconpool)
                     /*****consume end*****/

                     /*****bonus*****/
                     (clearbon)
                     (bonus)
                     /*****bonus end*****/
                     )