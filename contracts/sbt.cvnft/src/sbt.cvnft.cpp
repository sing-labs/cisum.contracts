#include <sbt.cvnft/sbt.cvnft.hpp>
#include <sbt.cvnft/sbt.cvnft.db.hpp>
#include "sbt.cvnft.hpp"
namespace flon {


// ===== 空投白名单 =====
void cvnft::setairdrop(name from, bool allow) {
   require_auth(_self ); // 只有合约自身能改白名单
   CHECKC(is_account(from), err::ACCOUNT_INVALID, "airdropper not exist");
   if (allow) {
      _gstate.airdroppers.insert(from);
   } else {
      _gstate.airdroppers.erase(from);
   }
}

void cvnft::setcreator( const name& creator, const bool& to_add){
   require_auth( _self );

   CHECKC(is_account(creator), err::ACCOUNT_INVALID, "creator not exist");

   if ( to_add ){
      _gstate.creators.insert( creator );

   } else {
      check( _gstate.creators.find( creator ) != _gstate.creators.end(), "creator not found:" + creator.to_string() );
      _gstate.creators.erase( creator );
   }
}

void cvnft::setipowner(const uint64_t& symbid, const name& ip_owner) {
   require_auth(_self );
   
   nstats_t::idx_t nstats(_self, _self.value);
   auto itr = nstats.find(symbid);
   CHECKC(itr != nstats.end(), err::RECORD_NO_FOUND, "nft not found");

   nstats.modify( itr, same_payer, [&](auto& row){
      row.ipowner        = ip_owner;
   });
}

void cvnft::settokenuri(const uint64_t& symbid, const string& url) {
   require_auth(_self );

   nstats_t::idx_t nstats(_self, _self.value);
   auto itr = nstats.find(symbid);
   CHECKC(itr != nstats.end(), err::RECORD_NO_FOUND, "nft not found");

   nstats.modify( itr, same_payer, [&](auto& row){
      row.token_uri     = url;
   });
}
void cvnft::setnotary(const name& notary, const bool& to_add) {
   require_auth( _self );
   CHECKC(is_account(notary), err::ACCOUNT_INVALID, "notary not exist");
   if (to_add){
      _gstate.notaries.insert(notary);
   }
   else{
      _gstate.notaries.erase(notary);
   }

}

void cvnft::notarize(const name& notary, const uint32_t& token_id) {
   require_auth( notary );
   CHECKC( _gstate.notaries.find(notary) != _gstate.notaries.end(),err::DID_NOT_AUTH, "not authorized notary" );

   nstats_t::idx_t nstats(get_self(), get_self().value);
   auto itr = nstats.find( token_id );
   CHECKC(itr != nstats.end(), err::RECORD_NO_FOUND, "token not found");
   nstats.modify( itr, same_payer, [&]( auto& row ) {
      row.notary = notary;
      row.notarized_at = time_point( current_time_point()  );
    });
}

void cvnft::create(const name& issuer,
                   const int64_t& maximum_supply,
                   const nsymbol& symbol,
                   const string& token_uri,
                   const name& ipowner)
{
   require_auth(issuer);

   CHECKC(is_account(issuer),                err::ACCOUNT_INVALID, "issuer not exist");
   CHECKC(ipowner.value == 0 || is_account(ipowner),err::ACCOUNT_INVALID, "ipowner not exist");
   CHECKC(maximum_supply > 0,                err::NOT_POSITIVE,     "max-supply must be positive");
   CHECKC(token_uri.size() < 1024,           err::INVALID_FORMAT,   "token uri too long");

   _creator_auth_check(issuer);

   nstats_t::idx_t nstats(get_self(), get_self().value);
   auto uri_idx                     = nstats.get_index<"tokenuriidx"_n>();
   auto raw_idx                     = nstats.get_index<"symrawidx"_n>();

   if (!token_uri.empty()) {
      auto uri_hash                    = HASH256(token_uri);
      CHECKC(uri_idx.find(uri_hash) == uri_idx.end(),err::REDPACK_EXIST, "token_uri exists");
   }
   nsymbol nsymb = symbol;

   if (nsymb.id == 0) {
      nsymb.id = nstats.available_primary_key();
      if (nsymb.id == 0) nsymb.id = 1; // 防止极端情况下返回 0
   }

   CHECKC(nsymb.id != nsymb.pid, err::TYPE_INVALID, "parent id shall not equal id");
   CHECKC(raw_idx.find(nsymb.raw()) == raw_idx.end(),err::REDPACK_EXIST, "symbol (pid,id) already exists");

   nstats.emplace(issuer, [&](auto& s){
      s.supply.symbol               = nsymb;
      s.max_supply                  = nasset(maximum_supply, nsymb);
      s.token_uri                   = token_uri;
      s.ipowner                     = ipowner;
      s.issuer                      = issuer;
      s.issued_at                   = eosio::current_time_point();
      s.paused                       = false;
   });
}

void cvnft::issue(const name& to, const nasset& quantity, const string& memo)
{
   require_auth(to);
   CHECKC(memo.size() <= 256,                 err::INVALID_FORMAT,      "memo too long");
   CHECKC(quantity.amount > 0,                err::INSUFFICIENT_QUANTITY,"must issue positive quantity");

   nstats_t::idx_t nstats(get_self(), get_self().value);
   auto raw_idx = nstats.get_index<"symrawidx"_n>();
   auto itr = raw_idx.find(quantity.symbol.raw());
   CHECKC(itr != raw_idx.end(),               err::SYMBOL_MISMATCH,     "token symbol not found");

   const auto& st = *itr;
   CHECKC(to == st.issuer,                    err::ACCOUNT_INVALID,      "tokens can only be issued to issuer account");
   CHECKC(!st.paused,                         err::STATUS_MISMATCH,      "token paused");
   CHECKC(quantity.symbol.raw() == st.supply.symbol.raw(),
                                             err::SYMBOL_MISMATCH,      "symbol mismatch");
   CHECKC(quantity.amount <= (st.max_supply.amount - st.supply.amount),
                                             err::AMOUNT_TOO_LARGE,     "quantity exceeds available supply");

   raw_idx.modify(itr, same_payer, [&](auto& s){
      s.supply    += quantity;
      s.issued_at  = eosio::current_time_point();
   });
   add_balance(st.issuer, quantity, st.issuer);
}

void cvnft::notifyreward(const name& predator, const name& victim, const asset& reward_quantity) {
    require_auth( _self );

    require_recipient( predator );
}

void cvnft::transfer(const name& from,
                     const name& to,
                     const nasset& quantity,
                     const string& memo)
{
    // 基础校验
    CHECKC(from != to, err::INVALID_FORMAT, "cannot transfer to self");
    require_auth(from);                                      // 谁转谁签
    CHECKC(is_account(to), err::ACCOUNT_INVALID, "to account not exist");
    CHECKC(memo.size() <= 256, err::INVALID_FORMAT, "memo too long");
    CHECKC(quantity.amount > 0, err::INSUFFICIENT_QUANTITY, "must transfer positive quantity");

    // 精确按 (pid,id) 查询目标 SBT
    nstats_t::idx_t nstats(get_self(), get_self().value);
    auto raw_idx = nstats.get_index<"symrawidx"_n>();
    auto itr = raw_idx.find(quantity.symbol.raw());
    CHECKC(itr != raw_idx.end(), err::SYMBOL_MISMATCH, "token symbol not found");

    const auto& st = *itr;

    // SBT 状态校验
    CHECKC(!st.paused, err::STATUS_MISMATCH, "token paused");

    // 符号一致
    CHECKC(quantity.symbol.raw() == st.supply.symbol.raw(), err::SYMBOL_MISMATCH, "symbol mismatch");

    // 权限：只允许发行方发起（或合约自身内联代发）
    CHECKC(from == st.issuer || has_auth(get_self()),
           err::DID_NOT_AUTH, "only issuer (or contract self) can transfer this SBT");

    // 从发行方余额扣减 -> 给接收方增加
    sub_balance(from, quantity);
    add_balance(to, quantity, has_auth(to) ? to : from);
}


void cvnft::sub_balance( const name& owner, const nasset& value ) {

    account_t::idx_t acnts(get_self(), owner.value);
    const auto& cur = acnts.get(value.symbol.raw(), "no balance object found");
    CHECKC(cur.balance.amount >= value.amount, err::INSUFFICIENT_QUANTITY, "overdrawn balance");
    acnts.modify(cur, same_payer, [&](auto& a){ a.balance -= value; });

}

void cvnft::add_balance( const name& owner, const nasset& value, const name& ram_payer )
{
   account_t::idx_t acnts(get_self(), owner.value);
   auto it = acnts.find(value.symbol.raw());
   if( it == acnts.end() ) {
      acnts.emplace( ram_payer, [&]( auto& a ){
        a.balance = value;
      });
   } else {
      CHECKC(!it->paused, err::STATUS_MISMATCH, "account paused");
      acnts.modify( it, same_payer, [&]( auto& a ) {
        a.balance += value;
      });
   }
}

void cvnft::_creator_auth_check( const name& creator){
      if (_gstate.creators.empty()) return;

      CHECKC(_gstate.creators.count(creator) > 0, err::DID_NOT_AUTH, ("creator not authorized: " + creator.to_string())); 

      auto is_auth = false;
      account_t::idx_t did(DID_CONTRACT, creator.value);
      for (auto it = did.begin(); it != did.end(); ++it) {
        if (it->balance.amount > 0) { is_auth = true; break; }
      }
      CHECKC(is_auth, err::DID_NOT_AUTH, ("creator has no DID: " + creator.to_string()));
}




} //namespace flon