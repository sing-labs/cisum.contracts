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
    uint64_t value = 0;

    static constexpr uint32_t U1E9  = 10'0000'0000UL;
    nsymbol() = default;

    static uint64_t to_raw_value(uint32_t i, uint32_t p) {
        EOS_ASSERT( p < U1E9, symbol_type_exception, "pid must be below 10**9" );
        EOS_ASSERT( i < U1E9, symbol_type_exception, "id must be below 10**9" );
        return (uint64_t)p * U1E9 + i;
    }

    explicit nsymbol(uint32_t i, uint32_t p): value(to_raw_value(i, p)) {}

    explicit nsymbol(uint64_t raw): value(raw) {}

    friend bool operator==(const nsymbol& a, const nsymbol& b) {
        return( a.value == b.value );
    }

    inline uint32_t id() const {
        return value % U1E9;
    }

    inline uint32_t pid() const {
        return value / U1E9;
    }
};

FC_REFLECT( nsymbol, (value) )
struct nasset {
    int64_t         amount  = 0;
    nsymbol         symbol;

    nasset() = default;
    explicit nasset(uint32_t id): amount(0), symbol(id) {}
    explicit nasset(uint32_t id, uint32_t pid, int64_t amount = 0): amount(amount), symbol(id, pid) {}
    explicit nasset(int64_t amount, const nsymbol& symb): amount(amount), symbol(symb) {}

    nasset& operator+=(const nasset& quantity) {
        EOS_ASSERT( quantity.symbol.value == this->symbol.value, symbol_type_exception, "nsymbol mismatch" );
        this->amount += quantity.amount; return *this;
    }
    nasset& operator-=(const nasset& quantity) {
        EOS_ASSERT( quantity.symbol.value == this->symbol.value, symbol_type_exception, "nsymbol mismatch" );
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

   abi_serializer get_abi_ser(const name& contract) {
       abi_serializer abi_ser;
       const auto& accnt = control->db().get<account_object,by_name>( contract );
       abi_def abi;
       BOOST_REQUIRE_EQUAL(abi_serializer::to_abi(accnt.abi, abi), true);
       abi_ser.set_abi(abi, abi_serializer::create_yield_function(abi_serializer_max_time));
       return abi_ser;
   }

   asset get_balance( const name& contract, const account_name& act, symbol balance_symbol ) {
        auto abi_ser = get_abi_ser(contract);
        vector<char> data = get_row_by_account( contract, act, "accounts"_n, account_name(balance_symbol.to_symbol_code().value) );
        return data.empty() ?
                asset(0, balance_symbol) :
                abi_ser.binary_to_variant("account", data, abi_serializer::create_yield_function(abi_serializer_max_time))["balance"].as<asset>();
   }

   nasset get_balance( const name& contract, const account_name& act, nsymbol balance_symbol ) {
        auto abi_ser = get_abi_ser(contract);
        vector<char> data = get_row_by_account( contract, act, "accounts"_n, account_name(balance_symbol.value) );
        return data.empty() ?
               nasset(0, balance_symbol) :
               abi_ser.binary_to_variant("account", data, abi_serializer::create_yield_function(abi_serializer_max_time))["balance"].as<nasset>();
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
    auto point_contract = "nestar.token"_n;
    auto ticket_contract = "cvticket.nft"_n;
    auto user = "useracc"_n;
    auto nestar_symbol = symbol(4, "NESTAR");
    int64_t initial_supply = 1'0000'0000'0000;
    asset grab_price = asset(1'0000, nestar_symbol);
    nsymbol show_id = nsymbol(1, 1);

    int64_t total_tickets = 10000;
    nsymbol ticket_id = nsymbol(1, 2);
    auto ticket_issuer = ticket_contract;
    auto point_issuer = point_contract;

    // deploy ticket contract
    create_accounts({ticket_contract});
    set_code(ticket_contract, contracts::cvticket_nft_wasm());
    set_abi(ticket_contract, contracts::cvticket_nft_abi().data());

    // create ticket
    push_action( ticket_contract, "create"_n, ticket_contract, mvo()
        ("issuer", ticket_issuer)
        ("maximum_supply", total_tickets)
        ("symbol", ticket_id)
        ("token_uri", "ipfs://ticket_metadata")
        ("ipowner", ticket_issuer) );

    // issue ticket
    push_action(ticket_contract, "issue"_n, ticket_issuer, mvo()
        ("to", ticket_issuer)
        ("quantity", nasset(100, ticket_id))
        ("memo", "issue ticket"));


    // Deploy nestar.token and create/issue NESTAR
    create_accounts({point_contract, user});
    set_code(point_contract, contracts::nestar_token_wasm());
    set_abi(point_contract, contracts::nestar_token_abi().data());

    push_action(point_contract, "init"_n, point_contract, mvo()
        ("issuer", point_issuer)
        ("admin", point_issuer)
        ("artists_contract", point_issuer)
        ("badgestore_contract", point_issuer));

    push_action(point_contract, "create"_n, point_issuer, mvo()
        ("issuer", point_issuer)
        ("maximum_supply", asset(initial_supply, nestar_symbol)));
    push_action(point_contract, "issue"_n, point_issuer, mvo()
        ("to", point_issuer)
        ("quantity", asset(initial_supply, nestar_symbol))
        ("memo", "initial issue"));

    push_action(point_contract, "addwhitelist"_n, point_contract, mvo()
        ("account", point_issuer));
    push_action(point_contract, "addwhitelist"_n, point_contract, mvo()
        ("account", contract_account));

    // transfer point to user
    push_action(point_contract, "transfer"_n, point_issuer, mvo()
        ("from", point_issuer)
        ("to", user)
        ("quantity", asset(100'0000, nestar_symbol))
        ("memo", "add:1" ));

    // Admin creates rush sale
    push_action(contract_account, "addrushsale"_n, admin, mutable_variant_object()
        ("show_id", show_id)
        ("ticket_id", ticket_id)
        ("started_at", control->head().block_time())
        ("ended_at", control->head().block_time() + fc::seconds(3600))
        ("price", grab_price)
        ("max_grabs_per_user", 1)
        ("win_ratio", 10000));

    // transfer tickets to rush sale
    push_action(ticket_contract, "transfer"_n, ticket_issuer, mvo()
        ("from", ticket_issuer)
        ("to", contract_account)
        ("assets", vector<nasset>{nasset(100, ticket_id)})
        ("memo", "add:1" ));

    // User transfers point to grab.cisum to grab ticket
    push_action(point_contract, "transfer"_n, user, mutable_variant_object()
        ("from", user)
        ("to", contract_account)
        ("quantity", grab_price)
        ("memo", "grab:1"));

    // 5. Check user tickets and rush sale status
    auto user_stat = get_user_state(1, user);
    BOOST_REQUIRE_MESSAGE(user_stat.is_object(), "User state not found");
    BOOST_CHECK(user_stat["tickets"]["amount"].as_int64() >= 0);
    auto rush_sale = get_rush_sale_state(1);
    BOOST_REQUIRE_MESSAGE(rush_sale.is_object(), "Rush sale state not found");
    BOOST_CHECK(rush_sale["sold_tickets"]["amount"].as_int64() >= 0);
}
BOOST_AUTO_TEST_SUITE_END()
