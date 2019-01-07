#include <boost/test/unit_test.hpp>
#include <eosio/testing/tester.hpp>
#include <eosio/chain/abi_serializer.hpp>
#include <Runtime/Runtime.h>

#include "contracts.hpp"
#include "test_symbol.hpp"

using namespace eosio::testing;
using namespace eosio;
using namespace eosio::chain;

using mvo = fc::mutable_variant_object;

class platform_tester : public tester {
public:
   platform_tester() {
      create_accounts( { N(eosio.token), N(bitsplatform), N(bitlandowner), N(player1) } );
      produce_block();

      set_code( N(bitsplatform), contracts::util::platform_wasm() );
      set_abi( N(bitsplatform), contracts::util::platform_abi().data() );
      produce_blocks();

      const auto& acc = control->db().get<account_object,by_name>( N(bitsplatform) );
      abi_def abi;
      BOOST_REQUIRE_EQUAL(abi_serializer::to_abi(acc.abi, abi), true);
      abi_ser.set_abi(abi, abi_serializer_max_time);

      set_code( N(eosio.token), contracts::token_wasm() );
      set_abi( N(eosio.token), contracts::token_abi().data() );
      produce_blocks();

   }

   action_result push_action( const name& signer, const name& aname, const variant_object &data, const name& contract = N(bitsplatform) ) {
      string action_type_name = abi_ser.get_action_type(aname);

      action act;
      act.account = contract;
      act.name    = aname;
      act.data    = abi_ser.variant_to_binary( action_type_name, data,abi_serializer_max_time );

      return base_tester::push_action( std::move(act), signer.value);
   }

   void create_currency( name manager, asset maxsupply ) {
      auto act =  mutable_variant_object()
         ("issuer",       manager )
         ("maximum_supply", maxsupply );

      base_tester::push_action(N(eosio.token), N(create), N(eosio.token), act );
   }
   void issue( name to, const asset& amount ) {
      base_tester::push_action( N(eosio.token), N(issue), N(eosio), mutable_variant_object()
                                ("to",      to )
                                ("quantity", amount )
                                ("memo", "")
                                );
   }
   void transfer( name from, name to, const string& amount, const string& memo = "" ) {
      base_tester::push_action( N(eosio.token), N(transfer), from, mutable_variant_object()
                                ("from",    from)
                                ("to",      to )
                                ("quantity", core_sym::from_string(amount) )
                                ("memo", memo )
                                );
   }

   fc::variant get_global() {
      vector<char> data = get_row_by_account( N(bitsplatform), N(bitsplatform), N(global), N(global) );
      return data.empty() ? fc::variant() : abi_ser.binary_to_variant( "global", data, abi_serializer_max_time );
   }

   abi_serializer abi_ser;
};

BOOST_AUTO_TEST_SUITE(platform_tests)

BOOST_FIXTURE_TEST_CASE( recharge, platform_tester ) try {
   create_currency( N(eosio), core_sym::from_string("10000000000.0000") );
   issue( N(player1), core_sym::from_string("1000.0000"));
   produce_blocks();

   transfer( N(player1), N(bitsplatform), "1.0000", "mg:deposit" );
   produce_blocks();

   auto g = get_global();
   BOOST_REQUIRE_EQUAL( g["point_rate"].as<share_type>(), asset::from_string( g["total_point"].as_string() ).get_amount() );
   BOOST_REQUIRE_EQUAL( "1.0000 EOS", g["total_recharge"].as_string() );
} FC_LOG_AND_RETHROW()

BOOST_AUTO_TEST_SUITE_END()