#include "include/bitsplatform.hpp"
#include "include/token/token_stats.hpp"
#include "include/token/token_balance.hpp"



namespace egame {

   void platform::create( name issuer, asset maximum_supply, int16_t team )
   {
      require_auth( _self );

      auto sym = maximum_supply.symbol;
      eosio_assert( sym.is_valid(), "invalid symbol name" );
      eosio_assert( maximum_supply.is_valid(), "invalid supply");
      eosio_assert( maximum_supply.amount > 0, "max-supply must be positive");
      eosio_assert( team >= 0 && team <= 100, "invalid team share" );

      tokenStats statstable( _self, sym.code().raw() );
      auto existing = statstable.find( sym.code().raw() );
      eosio_assert( existing == statstable.end(), "token with symbol already exists" );

      statstable.emplace( _self, [&]( auto& s ) {
         s.supply.symbol        = maximum_supply.symbol;
         s.max_supply           = maximum_supply;
         s.issuer               = issuer;
         s.team                 = maximum_supply * team / 100;
         s.team_claim.symbol    = maximum_supply.symbol;
      });
   }

   void platform::claim( name to, asset quantity )
   {
      require_auth( _self );

      auto sym = quantity.symbol;
      eosio_assert( sym.is_valid(), "invalid symbol name" );
      eosio_assert( quantity.is_valid(), "invalid quantity" );
      eosio_assert( quantity.amount > 0, "must claim positive quantity" );

      tokenStats statstable( _self, sym.code().raw() );
      auto existing = statstable.find( sym.code().raw() );
      eosio_assert( existing != statstable.end(), "token with symbol does not exist, create token before claim" );
      
      const auto& st = *existing;
      eosio_assert( quantity.symbol == st.supply.symbol, "symbol precision mismatch" );
      eosio_assert( quantity.amount <= st.team.amount - st.team_claim.amount, "quantity exceeds available team");

      auto total = _con_status.token_total + _ref_status.token_total;
      auto total_supply = total - _con_status.token_pool - _ref_status.token_pool;
      int32_t rate = (int32_t)(double(total_supply.amount) / double(total.amount) * 100.00f);
      
      auto claim_ava = st.team * rate / 100 - st.team_claim;
      eosio_assert( quantity.amount <= claim_ava.amount, "quantity exceeds available team");

      statstable.modify( st, same_payer, [&]( auto& s ) {
         s.supply += quantity;
         s.team_claim += quantity;
      });
      add_balance( st.issuer, quantity, st.issuer );
      if( to != st.issuer ) {
         SEND_INLINE_ACTION( *this, transfer, { {st.issuer, "active"_n} },
                           { st.issuer, to, quantity, string("team claim") }
         );
      }
   }

   void platform::issue( name to, asset quantity, string memo )
   {
      auto sym = quantity.symbol;
      eosio_assert( sym.is_valid(), "invalid symbol name" );
      eosio_assert( memo.size() <= 256, "memo has more than 256 bytes" );

      tokenStats statstable( _self, sym.code().raw() );
      auto existing = statstable.find( sym.code().raw() );
      eosio_assert( existing != statstable.end(), "token with symbol does not exist, create token before issue" );
      const auto& st = *existing;

      require_auth( st.issuer );
      eosio_assert( quantity.is_valid(), "invalid quantity" );
      eosio_assert( quantity.amount > 0, "must issue positive quantity" );

      eosio_assert( quantity.symbol == st.supply.symbol, "symbol precision mismatch" );
      eosio_assert( quantity.amount <= (st.max_supply.amount - st.team.amount) - (st.supply.amount - st.team_claim.amount) , "quantity exceeds available supply");

      statstable.modify( st, same_payer, [&]( auto& s ) {
         s.supply += quantity;
      });

      add_balance( st.issuer, quantity, st.issuer );

      if( to != st.issuer ) {
         SEND_INLINE_ACTION( *this, transfer, { {st.issuer, "active"_n} },
                           { st.issuer, to, quantity, memo }
         );
      }
   }

   void platform::retire( asset quantity, string memo )
   {
      auto sym = quantity.symbol;
      eosio_assert( sym.is_valid(), "invalid symbol name" );
      eosio_assert( memo.size() <= 256, "memo has more than 256 bytes" );

      tokenStats statstable( _self, sym.code().raw() );
      auto existing = statstable.find( sym.code().raw() );
      eosio_assert( existing != statstable.end(), "token with symbol does not exist" );
      const auto& st = *existing;

      require_auth( st.issuer );
      eosio_assert( quantity.is_valid(), "invalid quantity" );
      eosio_assert( quantity.amount > 0, "must retire positive quantity" );

      eosio_assert( quantity.symbol == st.supply.symbol, "symbol precision mismatch" );

      statstable.modify( st, same_payer, [&]( auto& s ) {
         s.supply -= quantity;
      });

      sub_balance( st.issuer, quantity );
   }

   void platform::transfer( name from, name to, asset quantity, string memo )
   {
      eosio_assert( from != to, "cannot transfer to self" );
      require_auth( from );
      eosio_assert( is_account( to ), "to account does not exist");
      auto sym = quantity.symbol.code();
      tokenStats statstable( _self, sym.raw() );
      const auto& st = statstable.get( sym.raw() );

      require_recipient( from );
      require_recipient( to );

      eosio_assert( quantity.is_valid(), "invalid quantity" );
      eosio_assert( quantity.amount > 0, "must transfer positive quantity for mg token" );
      eosio_assert( quantity.symbol == st.supply.symbol, "symbol precision mismatch" );
      eosio_assert( memo.size() <= 256, "memo has more than 256 bytes" );

      auto payer = has_auth( to ) ? to : from;

      sub_balance( from, quantity );
      add_balance( to, quantity, payer );
   }

   void platform::closetoken( name owner, const asset& symbol )
   {
      auto auth = has_auth( owner ) ? owner : _self;
      require_auth( auth );
      balances acnts( _self, owner.value );
      auto it = acnts.find( symbol.symbol.code().raw() );
      eosio_assert( it != acnts.end(), "Balance row already deleted or never existed. Action won't have any effect." );
      eosio_assert( it->balance.amount == 0, "Cannot close because the balance is not zero." );
      acnts.erase( it );
   }

   void platform::sub_balance( name owner, asset value ) {
      balances from_acnts( _self, owner.value );

      const auto& from = from_acnts.get( value.symbol.code().raw(), "no balance object found" );
      eosio_assert( from.balance.amount >= value.amount, "overdrawn balance" );

      from_acnts.modify( from, owner, [&]( auto& a ) {
         a.balance -= value;
      });

      if( value.symbol == MGT && _bon_status.paying == false ) {
         auto  uitr = _users.find( owner.value );
         if( uitr != _users.end() ) {
            _users.modify( uitr, same_payer, [&]( auto& u ) {
               u.platform_token = from.balance;
            });
         }
      }
   }

   void platform::add_balance( name owner, asset value, name ram_payer )
   {
      balances to_acnts( _self, owner.value );
      auto to = to_acnts.find( value.symbol.code().raw() );
      if( to == to_acnts.end() ) {
         to = to_acnts.emplace( ram_payer, [&]( auto& a ){
            a.balance = value;
         });
      } else {
         to_acnts.modify( to, same_payer, [&]( auto& a ) {
            a.balance += value;
         });
      }

      if( value.symbol == MGT && _bon_status.paying == false ) {
         auto  uitr = _users.find( owner.value );
         if( uitr == _users.end() ) {
            _users.emplace( _self, [&]( auto& u ) {
               u.name = owner;
               u.platform_token = to->balance; 
               u.nickname = owner.to_string();
            });
            _global.total_users += 1;
            _global.max_users += 1;
         } else {
            _users.modify( uitr, same_payer, [&]( auto& u ) {
               u.platform_token = to->balance;
            });
         }
      }
      
   }
}