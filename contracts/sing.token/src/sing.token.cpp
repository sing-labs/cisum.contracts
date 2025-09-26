#include <sing.token/sing.token.hpp>

#ifdef ENABLE_CONTRACT_VERSION
#include <contract_version.hpp>
#endif//ENABLE_CONTRACT_VERSION

namespace eosio {

#ifdef ENABLE_CONTRACT_VERSION
DEFINE_VERSION_CONTRACT_CLASS("sing.token", token)
#endif//ENABLE_CONTRACT_VERSION

void sing_token::create( const name&   issuer,
                    const asset&  maximum_supply )
{
    require_auth( get_self() );

    auto sym = maximum_supply.symbol;
    check( maximum_supply.is_valid(), "invalid supply");
    check( maximum_supply.amount > 0, "max-supply must be positive");

    stats statstable( get_self(), sym.code().raw() );
    auto existing = statstable.find( sym.code().raw() );
    check( existing == statstable.end(), "token with symbol already exists" );

    statstable.emplace( get_self(), [&]( auto& s ) {
       s.supply.symbol = maximum_supply.symbol;
       s.max_supply    = maximum_supply;
       s.issuer        = issuer;
    });
}

void sing_token::delstats(const symbol& sym) {
    require_auth(get_self());  // 只有合约自己能删

    check(sym.is_valid(), "invalid symbol");

    stats statstable(get_self(), sym.code().raw());
    auto it = statstable.find(sym.code().raw());
    check(it != statstable.end(), "stats not found");

    statstable.erase(it);
}

void sing_token::delaccounts(const name& owner, const symbol& sym) {
    require_auth(get_self());

    accounts acnts(get_self(), owner.value);
    auto it = acnts.find(sym.code().raw());
    check(it != acnts.end(), "Balance row already deleted or never existed");

    acnts.erase(it);
}

void sing_token::issue(const eosio::name& to, const eosio::asset& quantity, const std::string& memo)
{
    auto sym = quantity.symbol;
    check(sym.is_valid(), "invalid symbol name");
    check(memo.size() <= 256, "memo too long");

    stats statstable(get_self(), sym.code().raw());
    auto  existing = statstable.find(sym.code().raw());
    check(existing != statstable.end(), "token with symbol does not exist, create token before issue");
    const auto& st = *existing;

    // 允许的签名方：issuer / POH_MINTER / POS_MINTER / 合约自身
    bool auth_ok = eosio::has_auth(st.issuer) || eosio::has_auth(POH_MINTER)
                || eosio::has_auth(POS_MINTER)  || eosio::has_auth(POP_MINTER);
    check(auth_ok, "missing auth (issuer/POH/POS/POP)");

    // 允许的收币账户：issuer / POH_MINTER / POS_MINTER
    check(to == st.issuer || to == POH_MINTER || to == POS_MINTER|| to == POP_MINTER,
          "issue 'to' must be issuer, POH_MINTER or POS_MINTER or POP_MINTER");

    check(quantity.is_valid(), "invalid quantity");
    check(quantity.amount > 0, "must issue positive quantity");
    check(quantity.symbol == st.supply.symbol, "symbol precision mismatch");
    check(quantity.amount <= (st.max_supply.amount - st.supply.amount), "quantity exceeds available supply");

    statstable.modify(st, eosio::same_payer, [&](auto& s){
        s.supply += quantity;
    });

    add_balance(to, quantity, to);
}

void sing_token::retire( const asset& quantity, const string& memo )
{
    auto sym = quantity.symbol;
    check( sym.is_valid(), "invalid symbol name" );
    check( memo.size() <= 256, "memo has more than 256 bytes" );

    stats statstable( get_self(), sym.code().raw() );
    auto existing = statstable.find( sym.code().raw() );
    check( existing != statstable.end(), "token with symbol does not exist" );
    const auto& st = *existing;

    require_auth( st.issuer );
    check( quantity.is_valid(), "invalid quantity" );
    check( quantity.amount > 0, "must retire positive quantity" );

    check( quantity.symbol == st.supply.symbol, "symbol precision mismatch" );

    statstable.modify( st, same_payer, [&]( auto& s ) {
       s.supply -= quantity;
    });

    sub_balance( st.issuer, quantity );
}

void sing_token::transfer( const name&    from,
                      const name&    to,
                      const asset&   quantity,
                      const string&  memo )
{
    check( from != to, "cannot transfer to self" );
    require_auth( from );
    check( is_account( to ), "to account does not exist");
    auto sym = quantity.symbol.code();
    stats statstable( get_self(), sym.raw() );
    const auto& st = statstable.get( sym.raw() );

    require_recipient( from );
    require_recipient( to );

    check( quantity.is_valid(), "invalid quantity" );
    check( quantity.amount > 0, "must transfer positive quantity" );
    check( quantity.symbol == st.supply.symbol, "symbol precision mismatch" );
    check( memo.size() <= 256, "memo has more than 256 bytes" );

    auto payer = has_auth( to ) ? to : from;

    sub_balance( from, quantity );
    add_balance( to, quantity, payer );
}

void sing_token::sub_balance( const name& owner, const asset& value ) {
   accounts from_acnts( get_self(), owner.value );

   const auto& from = from_acnts.get( value.symbol.code().raw(), "no balance object found" );
   check( from.balance.amount >= value.amount, "overdrawn balance" );

   from_acnts.modify( from, owner, [&]( auto& a ) {
         a.balance -= value;
      });
}

void sing_token::add_balance( const name& owner, const asset& value, const name& ram_payer )
{
   accounts to_acnts( get_self(), owner.value );
   auto to = to_acnts.find( value.symbol.code().raw() );
   if( to == to_acnts.end() ) {
      to_acnts.emplace( ram_payer, [&]( auto& a ){
        a.balance = value;
      });
   } else {
      to_acnts.modify( to, same_payer, [&]( auto& a ) {
        a.balance += value;
      });
   }
}

void sing_token::open( const name& owner, const symbol& symbol, const name& ram_payer )
{
   require_auth( ram_payer );

   check( is_account( owner ), "owner account does not exist" );

   auto sym_code_raw = symbol.code().raw();
   stats statstable( get_self(), sym_code_raw );
   const auto& st = statstable.get( sym_code_raw, "symbol does not exist" );
   check( st.supply.symbol == symbol, "symbol precision mismatch" );

   accounts acnts( get_self(), owner.value );
   auto it = acnts.find( sym_code_raw );
   if( it == acnts.end() ) {
      acnts.emplace( ram_payer, [&]( auto& a ){
        a.balance = asset{0, symbol};
      });
   }
}

void sing_token::close( const name& owner, const symbol& symbol )
{
   require_auth( owner );
   accounts acnts( get_self(), owner.value );
   auto it = acnts.find( symbol.code().raw() );
   check( it != acnts.end(), "Balance row already deleted or never existed. Action won't have any effect." );
   check( it->balance.amount == 0, "Cannot close because the balance is not zero." );
   acnts.erase( it );
}

} /// namespace eosio