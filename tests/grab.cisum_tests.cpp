#include <boost/test/unit_test.hpp>
#include <contracts.hpp>
#include <eosio/testing/tester.hpp>
#include <fc/variant_object.hpp>

using namespace eosio::testing;
using namespace eosio;
using namespace eosio::chain;
using namespace eosio::testing;
using namespace fc;
using namespace std;


using mvo = fc::mutable_variant_object;

class grab_cisum_tester: public validating_tester {
public:
    name contract_account = "grabcisum"_n;
    name admin = "adminacc"_n;
    abi_serializer abi_ser;

    grab_cisum_tester() {
        create_accounts( { contract_account, admin } );
        set_code(contract_account, contracts::grabcisum_wasm());
        set_abi(contract_account, contracts::grabcisum_abi().data());

        produce_blocks();

        const auto& accnt = control->db().get<account_object,by_name>( contract_account );
        abi_def abi;
        BOOST_REQUIRE_EQUAL(abi_serializer::to_abi(accnt.abi, abi), true);
        abi_ser.set_abi(abi, abi_serializer::create_yield_function(abi_serializer_max_time));

        init(admin);
    }

    void init(const name& admin) {
        push_action(contract_account, "init"_n, mvo()("admin", admin));
    }

   fc::variant get_global_state() {
      vector<char> data = get_row_by_account( contract_account, contract_account, "global"_n, "global"_n );
      wdump((data));
      return data.empty() ? fc::variant() : abi_ser.binary_to_variant( "global_t", data, abi_serializer::create_yield_function(abi_serializer_max_time) );
   }

    action_result push_action( const account_name& signer, const action_name &name, const variant_object &data ) {
      string action_type_name = abi_ser.get_action_type(name);
      action act;
      act.account = contract_account;
      act.name    = name;
      act.data    = abi_ser.variant_to_binary( action_type_name, data, abi_serializer::create_yield_function(abi_serializer_max_time) );

      return base_tester::push_action( std::move(act), signer.to_uint64_t() );
   }
    // Add more helper methods for other actions and table queries as needed
};

BOOST_AUTO_TEST_SUITE(grab_cisum_tests)


BOOST_FIXTURE_TEST_CASE( test_init_admin, grab_cisum_tester ) try {

    produce_blocks();

    // Read global table and check admin field
    auto global = get_global_state();
    BOOST_REQUIRE(global.is_object());
    BOOST_CHECK(global["admin"].as_string() == admin.to_string());
} FC_LOG_AND_RETHROW()

BOOST_AUTO_TEST_CASE(test_addrushsale) {
    // Set admin, create rush sale
    // auto params = ...; // Fill with valid rush sale params
    // tester.push_action("grabcisum", "addrushsale", {"adminacc"}, params);
    // auto sale = tester.get_table_row<rush_sale>(...);
    // BOOST_CHECK(sale.show_id == params.show_id);
}

BOOST_AUTO_TEST_CASE(test_updrushsale) {
    // Update rush sale parameters
    // tester.push_action("grabcisum", "updrushsale", {"adminacc"}, {rush_sale_id, win_ratio, ...});
    // auto sale = tester.get_table_row<rush_sale>(...);
    // BOOST_CHECK(sale.win_ratio == new_win_ratio);
}

BOOST_AUTO_TEST_CASE(test_delrushsale) {
    // Delete rush sale
    // tester.push_action("grabcisum", "delrushsale", {"adminacc"}, {rush_sale_id, false});
    // auto sale = tester.get_table_row<rush_sale>(...);
    // BOOST_CHECK(sale == nullptr);
}

BOOST_AUTO_TEST_CASE(test_delusers) {
    // Batch delete users
    // tester.push_action("grabcisum", "delusers", {"adminacc"}, {rush_sale_id, 10});
    // auto users = tester.get_table("stat", rush_sale_id);
    // BOOST_CHECK(users.size() == expected);
}

BOOST_AUTO_TEST_CASE(test_on_transfer) {
    // Simulate user transfer and ticket allocation
    // tester.push_action("nestar.cisum", "transfer", {"useracc"}, {"useracc", "grabcisum", quantity, "grab:1"});
    // auto user = tester.get_table_row<users>(rush_sale_id, "useracc");
    // BOOST_CHECK(user.tickets > 0);
}

BOOST_AUTO_TEST_SUITE_END()
