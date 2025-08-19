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

struct nsymbol {
    uint32_t id     = 0;
    uint32_t pid    = 0;

    nsymbol() = default;
    explicit nsymbol(uint32_t i, uint32_t p = 0): id(i),pid(p) {
    }

    explicit nsymbol(uint64_t raw) {
        pid = raw / U1E9;
        id  = raw - pid * U1E9;
    }

    friend bool operator==(const nsymbol& a, const nsymbol& b) {
        return( a.id == b.id && a.pid == b.pid );
    }

    uint64_t raw()const { return( (uint64_t) pid * U1E9 + id ); }

    static constexpr uint32_t U1E9  = 10'0000'0000UL;
};

FC_REFLECT( nsymbol, (id)(pid) )
struct nasset {
    int64_t         amount  = 0;
    nsymbol         symbol;

    nasset() = default;
    explicit nasset(uint32_t id): amount(0), symbol(id) {}
    explicit nasset(uint32_t id, uint32_t pid, int64_t amount = 0): amount(amount), symbol(id, pid) {}
    explicit nasset(int64_t amount, const nsymbol& symb): amount(amount), symbol(symb) {}

    nasset& operator+=(const nasset& quantity) {
        EOS_ASSERT( quantity.symbol.raw() == this->symbol.raw(), symbol_type_exception, "nsymbol mismatch" );
        this->amount += quantity.amount; return *this;
    }
    nasset& operator-=(const nasset& quantity) {
        EOS_ASSERT( quantity.symbol.raw() == this->symbol.raw(), symbol_type_exception, "nsymbol mismatch" );
        this->amount -= quantity.amount; return *this;
    }

    friend bool operator==(const nasset& a, const nasset& b) {
        return( a.amount == b.amount && a.symbol == b.symbol );
    }

};

FC_REFLECT( nasset, (amount)(symbol) )

class grab_cisum_tester: public validating_tester {
public:
    name contract_account = "grabcisum"_n;
    name admin = "adminacc"_n;
    abi_serializer abi_ser;

    grab_cisum_tester() {
        create_accounts( { contract_account, admin } );
        set_code(contract_account, contracts::grab_cisum_wasm());
        set_abi(contract_account, contracts::grab_cisum_abi().data());

        produce_blocks();

        const auto& accnt = control->db().get<account_object,by_name>( contract_account );
        abi_def abi;
        BOOST_REQUIRE_EQUAL(abi_serializer::to_abi(accnt.abi, abi), true);
        abi_ser.set_abi(abi, abi_serializer::create_yield_function(abi_serializer_max_time));

        init(admin);
    }

    void init(const name& admin) {
        push_action(contract_account, "init"_n, contract_account, mvo()("admin", admin));
    }

   fc::variant get_global_state() {
        vector<char> data = get_row_by_account( contract_account, contract_account, "global"_n, "global"_n );
        return data.empty() ? fc::variant() : abi_ser.binary_to_variant( "global_t", data, abi_serializer::create_yield_function(abi_serializer_max_time) );
   }

   fc::variant get_rush_sale_state(uint64_t rush_sale_id) {
        vector<char> data = get_row_by_account( contract_account, contract_account, "rushsales"_n, name(rush_sale_id) );
        wdump((data));
        return data.empty() ? fc::variant() : abi_ser.binary_to_variant( "rush_sale", data, abi_serializer::create_yield_function(abi_serializer_max_time) );
   }

   fc::variant get_user_state(uint64_t rush_sale_id, name user) {
        vector<char> data = get_row_by_account( contract_account, name(rush_sale_id), "users"_n, user );
        return data.empty() ? fc::variant() : abi_ser.binary_to_variant( "user_t", data, abi_serializer::create_yield_function(abi_serializer_max_time) );
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

BOOST_FIXTURE_TEST_CASE(test_grab, grab_cisum_tester) {
    // 1. Instantiate tester and deploy contracts
    auto token_account = "nestar.token"_n;
    auto user = "useracc"_n;
    auto nestar_symbol = symbol(4, "NESTAR");
    int64_t initial_supply = 100000000;
    int64_t grab_price = 10000;

    // Deploy nestar.token and create/issue NESTAR
    create_accounts({token_account, user});
    set_code(token_account, contracts::nestar_token_wasm());
    set_abi(token_account, contracts::nestar_token_abi().data());

    push_action(token_account, "init"_n, token_account, mvo()
        ("issuer", token_account)
        ("admin", token_account)
        ("artists_contract", token_account)
        ("badgestore_contract", token_account));

    push_action(token_account, "create"_n, token_account, mvo()
        ("issuer", token_account)
        ("maximum_supply", asset(initial_supply, nestar_symbol)));
    push_action(token_account, "issue"_n, token_account, mvo()
        ("to", user)
        ("quantity", asset(initial_supply, nestar_symbol))
        ("memo", "initial issue"));

    push_action(token_account, "addwhitelist"_n, token_account, mvo()
        ("account", contract_account));

    // 3. Admin creates rush sale
    push_action(contract_account, "addrushsale"_n, admin, mutable_variant_object()
        ("show_id", nsymbol(1, 1))
        ("ticket_id", nsymbol(1, 2))
        ("started_at", control->head().block_time())
        ("ended_at", control->head().block_time() + fc::seconds(3600))
        ("price", asset(grab_price, nestar_symbol))
        ("max_grabs_per_user", 1)
        ("win_ratio", 10000)
        ("total_tickets", 10));

    // 4. User transfers NESTAR to grab.cisum to grab ticket
    push_action(token_account, "transfer"_n, user, mutable_variant_object()
        ("from", user)
        ("to", contract_account)
        ("quantity", asset(grab_price, nestar_symbol))
        ("memo", "grab:1"));

    // 5. Check user tickets and rush sale status
    auto user_stat = get_user_state(1, user);
    BOOST_REQUIRE_MESSAGE(user_stat.is_object(), "User state not found");
    BOOST_CHECK(user_stat["tickets"].as_int64() >= 0);
    auto rush_sale = get_rush_sale_state(1);
    BOOST_REQUIRE_MESSAGE(rush_sale.is_object(), "Rush sale state not found");
    BOOST_CHECK(rush_sale["sold_tickets"].as_int64() >= 0);
}
BOOST_AUTO_TEST_SUITE_END()
