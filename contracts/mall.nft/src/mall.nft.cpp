#include "mall.nft.hpp"
#include <algorithm>

namespace mallnft {

void mall_nft::create( const name& issuer, const int64_t& maximum_supply, const nsymbol& symbol, const string& token_uri, const name& ipowner )
{
   require_auth( issuer );

   check( is_account(issuer), "issuer account does not exist" );
   check( is_account(ipowner) || ipowner.length() == 0, "ipowner account does not exist" );
   check( maximum_supply > 0, "max-supply must be positive" );
   check( token_uri.length() < 1024, "token uri length > 1024" );

   creators allowed(get_self(), get_self().value);
   check(allowed.find(issuer.value) != allowed.end(), "creator not authorized");
   check(symbol.is_valid(), "invalid NFT symbol");

   auto nsymb           = symbol;
   auto nstats          = nstats_t::idx_t( get_self(), get_self().value );
   auto idx             = nstats.get_index<"tokenuriidx"_n>();
   auto token_uri_hash  = HASH256(token_uri);
   check( idx.find(token_uri_hash) == idx.end(), "token with token_uri already exists" );
   check( nstats.find(nsymb.nid) == nstats.end(), "token of nsymbol: " + to_string(nsymb.nid) + " alreay exists" );

   nstats.emplace( issuer, [&]( auto& s ) {
      s.supply.symbol   = nsymb;
      s.paused = false;
      s.max_supply      = nasset( maximum_supply, symbol );
      s.token_uri       = token_uri;
      s.ipowner         = ipowner;
      s.issuer          = issuer;
      s.issued_at       = current_time_point();
   });
   issuance_table history(get_self(), get_self().value);
   history.emplace(issuer, [&](auto& r) { r.nft_id = symbol.nid; });
}

void mall_nft::settokenuri(const uint64_t& symbid, const string& url) {

   auto nstats          = nstats_t::idx_t( get_self(), get_self().value );
   auto itr             = nstats.find(symbid);
   check( itr != nstats.end(), "nft not found  "+to_string(symbid) );

   check(has_auth(get_self()) || has_auth(itr->issuer), "non authorized");
   check(url.size() < 1024, "token uri too long");
   auto urls = nstats.get_index<"tokenuriidx"_n>();
   auto duplicate = urls.find(HASH256(url));
   check(duplicate == urls.end() || duplicate->supply.symbol.nid == symbid, "duplicate token uri");
   nstats.modify( itr, same_payer, [&](auto& row){
      row.token_uri     = url;
   });
}

void mall_nft::issue( const name& to, const nasset& quantity, const string& memo )
{
    auto sym = quantity.symbol;
    check( memo.size() <= 256, "memo has more than 256 bytes" );

    auto nstats = nstats_t::idx_t( get_self(), get_self().value );
    auto existing = nstats.find( sym.nid );
    check( existing != nstats.end(), "token with symbol does not exist, create token before issue" );
    const auto& st = *existing;
    check( to == st.issuer, "tokens can only be issued to issuer account" );

    require_auth( st.issuer );
    check(quantity.is_valid(), "invalid quantity");
    check( quantity.amount > 0, "must issue positive quantity" );

    check( quantity.symbol == st.supply.symbol, "symbol mismatch" );
    check( quantity.amount <= st.max_supply.amount - st.supply.amount, "quantity exceeds available supply");
    issuance_table history(get_self(), get_self().value);
    auto h = history.find(sym.nid);
    check(h != history.end(), "issuance history missing");
    {
       check(quantity.amount <= st.max_supply.amount - h->minted, "lifetime issuance exceeded");
       history.modify(h, same_payer, [&](auto& r) {
          r.minted += quantity.amount;
          r.reserved -= std::min(r.reserved, quantity.amount);
       });
    }

   nstats.modify( st, same_payer, [&]( auto& s ) {
      s.supply += quantity;
      s.issued_at = current_time_point();
   });

   add_balance( st.issuer, quantity, st.issuer );
}

void mall_nft::retire( const nasset& quantity, const string& memo )
{
    auto sym = quantity.symbol;
    check( memo.size() <= 256, "memo has more than 256 bytes" );

    auto nstats = nstats_t::idx_t( get_self(), get_self().value );
    auto existing = nstats.find( sym.nid );
    check( existing != nstats.end(), "token with symbol does not exist" );
    const auto& st = *existing;

    require_auth( st.issuer );
    check(quantity.is_valid(), "invalid quantity");
    check( quantity.amount > 0, "must retire positive quantity" );

    check( quantity.symbol == st.supply.symbol, "symbol precision mismatch" );

    nstats.modify( st, same_payer, [&]( auto& s ) {
       s.supply -= quantity;
    });

    sub_balance( st.issuer, quantity );
}

void mall_nft::transfer( const name& from, const name& to, const vector<nasset>& assets, const string& memo  )
{
   check( from != to, "cannot transfer to self" );
   require_auth( from );
   check( is_account( to ), "to account does not exist");
   check( memo.size() <= 256, "memo has more than 256 bytes" );

   auto payer = has_auth( to ) ? to : from;
   check(!assets.empty() && assets.size() <= 50, "transfer requires 1 to 50 assets");

   require_recipient( from );
   require_recipient( to );

   for( auto& quantity : assets) {
      auto sym = quantity.symbol;
      auto nstats = nstats_t::idx_t( get_self(), get_self().value );
      const auto& st = nstats.get( sym.nid );

      check(quantity.is_valid(), "invalid quantity");
      check( quantity.amount > 0, "must transfer positive quantity" );
      check( quantity.symbol == st.supply.symbol, "symbol precision mismatch" );

      // Every NFT has its own sender membership, even before setpolicy.
      // Neither receiver eligibility nor contract cosigning bypasses this check.
      xfer_table members(get_self(), sym.nid);
      check(members.find(from.value) != members.end(), "sender not in NFT transfer whitelist");

      sub_balance( from, quantity );
      add_balance( to, quantity, payer );
    }

}

void mall_nft::sub_balance( const name& owner, const nasset& value ) {
   auto from_acnts = account_t::idx_t( get_self(), owner.value );

   const auto& from = from_acnts.get( value.symbol.nid, "no balance object found" );
   check( from.balance.amount >= value.amount, "overdrawn balance" );

   from_acnts.modify( from, owner, [&]( auto& a ) {
         a.balance -= value;
      });
}

void mall_nft::add_balance( const name& owner, const nasset& value, const name& ram_payer )
{
   auto to_acnts = account_t::idx_t( get_self(), owner.value );
   auto to = to_acnts.find( value.symbol.nid );
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

void mall_nft::setpolicy(uint64_t nft_id, name manager) {
   nstats_t::idx_t stats(get_self(), get_self().value);
   const auto& st = stats.get(nft_id, "NFT not found");
   require_auth(st.issuer);
   check(is_account(manager), "invalid manager");
   policy_table policies(get_self(), get_self().value);
   check(policies.find(nft_id) == policies.end(), "policy is immutable");
   policies.emplace(st.issuer, [&](auto& row) { row.nft_id = nft_id; row.manager = manager; });
}

void mall_nft::setxferwl(uint64_t nft_id, name account, bool allowed) {
   nstats_t::idx_t stats(get_self(), get_self().value);
   const auto& st = stats.get(nft_id, "NFT not found");
   policy_table policies(get_self(), get_self().value);
   auto policy = policies.find(nft_id);
   bool issuer_auth = has_auth(st.issuer);
   check(issuer_auth || (policy != policies.end() && has_auth(policy->manager)),
         "requires issuer or policy manager");
   check(is_account(account), "invalid account");
   xfer_table members(get_self(), nft_id);
   auto it = members.find(account.value);
   if (allowed && it == members.end())
      members.emplace(issuer_auth ? st.issuer : policy->manager,
                      [&](auto& row) { row.account = account; });
   if (!allowed && it != members.end()) members.erase(it);
}

void mall_nft::setreserve(uint64_t nft_id, int64_t reserved) {
   nstats_t::idx_t stats(get_self(), get_self().value);
   const auto& st = stats.get(nft_id);
   require_auth(st.issuer);
   issuance_table history(get_self(), get_self().value);
   const auto& h = history.get(nft_id, "historical issuance not audited");
   check(reserved >= 0 && reserved <= st.max_supply.amount - h.minted, "invalid issuance reservation");
   history.modify(h, same_payer, [&](auto& r) { r.reserved = reserved; });
}

void mall_nft::setmaxsup(uint64_t nft_id, int64_t maximum) {
   nstats_t::idx_t stats(get_self(), get_self().value);
   const auto& st = stats.get(nft_id);
   require_auth(st.issuer);
   issuance_table history(get_self(), get_self().value);
   const auto& h = history.get(nft_id, "historical issuance not audited");
   check(maximum > 0 && maximum <= nasset::max_amount, "invalid maximum");
   check(h.minted <= maximum && h.reserved <= maximum - h.minted, "maximum below minted plus reserved");
   stats.modify(st, same_payer, [&](auto& r) { r.max_supply.amount = maximum; });
}

void mall_nft::setcreator(name creator, bool to_add) {
   require_auth(get_self());
   check(is_account(creator), "creator account does not exist");
   // Run setcreator once after upgrading from the global whitelist version.
   // Removing its persisted marker prevents the marketplace using the wrong protocol.
   flon::nft_transfer_modes mode(get_self(), get_self().value);
   if (mode.exists()) mode.remove();
   creators table(get_self(), get_self().value);
   auto it = table.find(creator.value);
   if (to_add && it == table.end())
      table.emplace(get_self(), [&](auto& row) { row.account = creator; });
   if (!to_add && it != table.end()) table.erase(it);
}

} // namespace mallnft
