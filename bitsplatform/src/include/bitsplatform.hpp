#pragma once

#include <eosiolib/eosio.hpp>
#include <eosiolib/asset.hpp>
#include <eosiolib/transaction.hpp>
#include "game.hpp"
#include "user.hpp"
#include "global.hpp"
#include "data.hpp"

#include "token/token_stats.hpp"
#include "token/token_balance.hpp"

#include "referrer/ref_status.hpp"
#include "consume/con_status.hpp"
#include "bonus/bon_status.hpp"
#include <string>

using namespace eosio;

namespace egame {

   class [[eosio::contract("bitsplatform")]] platform : public contract {
      private:
         user_index              _users;
         game_index              _games;
         bits_global_singleton   _global_table;
         global                  _global;

         uint32_t                _sys_precision;

         global get_default_state() {
            global g;
            return g;
         }
         bool check_point();

         /*****token*****/
         void sub_balance( name owner, asset value );
         void add_balance( name owner, asset value, name ram_payer );
         /*****token end*****/

         /*****referrer*****/
         RefStatusSingleton      _ref_status_table;
         RefStatus               _ref_status;

         RefStatus get_default_ref_status() {
            RefStatus rs;
            uint32_t s[3] = {0, 15, 65};
            uint32_t g[3] = {5, 10, 20};
            string t[3] = {"初来乍到", "略有小成", "傲视群雄"};
            rs.standards.insert( rs.standards.begin(), s, s+3 );
            rs.gift_tokens.insert( rs.gift_tokens.begin(), g, g+3 );
            rs.titles.insert( rs.titles.begin(), t, t+3 );
            auto refPool = asset( 60000000000, MGT );
            rs.SetPool( refPool );
            rs.first_recharge_reward = asset( 100000, MGT );
            return rs;
         }
         void bind_referrer( name user, name ref );
         /**
          * Perform referrer rewards
          * @pre user the user account name
          * @pre type the reward type,0 first charge,1 consume
          * @pre amount the SYS amount
          */ 
         void reward_referrer( name user, uint8_t type = 0, int64_t amount = 0 );
         /*****referrer end*****/

         /*****consume*****/
         ConsumeStatusSingleton _con_status_table;
         ConsumeStatus _con_status;
         ConsumeStatus get_default_consume_status() {
            ConsumeStatus cs;
            auto conPool = asset( 240000000000, MGT );
            cs.SetPool( conPool );
            cs.first_recharge_reward = asset( 100000, MGT );
            return cs;
         }

         /**
          * Perform consumer rewards
          * @pre user the user account name
          * @pre type the reward type,0 first charge,1 consume
          * @pre amount the SYS amount 
          */ 
         void reward_consume( name user, uint8_t type = 0, int64_t amount = 0 );
         /*****consume end*****/

         /*****bonus*****/
         BonusStatus _bon_status;
         BonusStatusSingleton _bon_status_table;
         BonusStatus get_default_bonus_status() {
            BonusStatus bs;
            bs.min_start = asset( 300000, POINT );
            bs.bonus_pool.symbol = POINT;
            bs.min_start_rate = 3;
            bs.pool_rate = 50;
            //bs.min_interval = 24;
            return bs;
         }
         void add_point( asset fee );
         void check_bonus();
         void clear_bonus_record();
         void loop_bonus();
         asset calc_income( asset& income );
         bool check_supply_rate( const TokenStat& ts );
         const TokenStat& get_token_stat( const symbol& sym );
         /*****bonus end*****/

      public:
         platform( name receiver, name code, datastream<const char*> ds );
         ~platform() {
            _global_table.set( _global, _self );
            _ref_status_table.set( _ref_status, _self );
            _con_status_table.set( _con_status, _self );
            _bon_status_table.set( _bon_status, _self );
         }

      /*****deposit/withdraw*****/ 
      ACTION deposit( name from, name to, asset quantity, string memo );

      ACTION withdraw( const name& to, const asset& quantity );
      /*****deposit/withdraw end*****/

      /*****Platform management*****/
      ACTION settle( const name& game,  const uint64_t table, const vector<settlement>& data );

      ACTION gamenter( const name& gname, const vector<name>& users, const uint64_t table );

      ACTION updateuser( const name& u, const string& nickname, const uint8_t gender, const uint32_t head);

      ACTION addgame( const game& g);

      ACTION updategame( const game& g);

      ACTION delgame( const name& game_name );

      ACTION lockuser( const name& gname, const vector<name> users );

      ACTION pause();

      ACTION open();

      ACTION reset();

      ACTION claimincome( const name& to, const asset& quantity );

      void OnError( const onerror& error );
      /*****Platform management end*****/

      /*****Token management*****/
      ACTION create( name issuer, asset maximum_supply, int16_t team );
      ACTION issue( name to, asset quantity, string memo );
      ACTION retire( asset quantity, string memo );
      ACTION transfer( name from, name to, asset quantity, string memo );
      ACTION claim( name to, asset quantity );
      ACTION closetoken( name owner, const asset& symbol );
      /*****Token management end*****/

      /*****referrer*****/
      ACTION bindref( name user, name ref );
      ACTION addrefpool( name& user, asset& value );
      /*****referrer end*****/

      /*****consume*****/
      ACTION reward( const name& game_name, const name& user, const int64_t amount, const name& ref );
      ACTION addconpool( name& user, asset& value );
      /*****consume end*****/

      /*****bonus*****/
      ACTION bonus();
      ACTION clearbon();
      ACTION bonusout( asset total, const vector<bonusitem>& data );
      /*****bonus end*****/

      public:
         static const user& GetUser( const name& tokenContractAccount, const name& account )
         {
            user_index users( tokenContractAccount, tokenContractAccount.value );
            const auto& acc = users.get( account.value, "unable to find account" );
            return acc;
         }
         static const user FindUser( const name& tokenContractAccount, const name& account )
         {
            user_index users( tokenContractAccount, tokenContractAccount.value );
            user u;
            auto aitr = users.find( account.value );
            if( aitr != users.end() ) {
               u = *aitr;
            }
            return u;
         }
         static const game& GetGame( const name& tokenContractAccount, const name& game )
         {
            game_index games( tokenContractAccount, tokenContractAccount.value );
            return games.get( game.value, "unable to find game" );
         }
         static void CheckUser( const name& tokenContractAccount, const name& account )
         {
            user_index users( tokenContractAccount, tokenContractAccount.value );
            auto itr = users.find( account.value );
            eosio_assert( itr != users.end(), "unknown account" );
            eosio_assert( itr->status == 0, "this account has been locked" );
            eosio_assert( itr->game_id.value == 0, "already in the game" );
         }
         static bool IsChinese(const string& str) {
            bool chinese = true;
            for(int i = 0; i < str.length(); i++){
               if(str[i] >= 0){
                  chinese = false;
                  break;
               }
            }
            if(chinese) {
               return str.length() >= 6 && str.length() <= 24;
            }
            return chinese;
         }
         static bool IsWord(const string& str) {
            if(str.length() == 0) return false;
            char c = str.at(0);
            if(c >= '0' && c <= '9') return false;

            bool word = true;
            for(int i = 0; i < str.length(); i++) {
               c = str.at(i);
               if( !((c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) ) {
                  word = false;
                  break;
               }
            }
            if(word) {
               return str.length() >= 3 && str.length() <= 12;
            }
            return word;
         }

         /*****token*****/
         static asset GetSupply( name tokenContractAccount, symbol_code symCode )
         {
            tokenStats statstable( tokenContractAccount, symCode.raw() );
            const auto& st = statstable.get( symCode.raw() );
            return st.supply;
         }

         static asset GetBalance( name tokenContractAccount, name owner, symbol_code symCode )
         {
            balances accountstable( tokenContractAccount, owner.value );
            const auto& ac = accountstable.get( symCode.raw() );
            return ac.balance;
         }
         /*****token end*****/

   };
}