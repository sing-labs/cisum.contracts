#include <show.cisum.hpp>
#include <show.cisum.db.hpp>
#include <cvticket.nft.hpp>

using std::string;
using std::vector;

namespace flon {

static inline time_point nowtp() { return current_time_point(); }

// ===== 全局设置 =====
void show::init(const name& admin, const name& nft_bank) {
  require_auth(get_self());
  check(is_account(admin), "admin not exist");
  check(is_account(nft_bank), "nft_bank not exist");
  _gstate.admin    = admin;
  _gstate.nft_bank = nft_bank;
}

void show::setcvticket(const name& nft_bank) {
  require_auth(_gstate.admin);
  check(is_account(nft_bank), "bank not exist");
  _gstate.nft_bank = nft_bank;
}

void show::addshowadm(const name& account) {
  require_auth(_gstate.admin);
  check(is_account(account), "account not exist");
  _gstate.show_admin.insert(account);
}

void show::delshowadm(const name& account) {
  require_auth(_gstate.admin);
  auto it = _gstate.show_admin.find(account);
  check(it != _gstate.show_admin.end(), "account not in show_admin");
  _gstate.show_admin.erase(it);
}

// ===== 演出 =====
void show::newshow(const name&       category,
                   const bool&       ticket_transferable,
                   const bool&       ticket_refundable,
                   const time_point& sale_started_at,
                   const time_point& sale_ended_at,
                   const time_point& show_started_at,
                   const time_point& show_ended_at,
                   const name&       status)
{
  check_admin_or_showadm();

  check(sale_ended_at    >= sale_started_at, "sale_ended_at must be >= sale_started_at");
  check(show_started_at  >= sale_ended_at,   "show_started_at must be >= sale_ended_at");
  check(show_ended_at    >= show_started_at, "show_ended_at must be >= show_started_at");

  show_t::showidx shows(get_self(), get_self().value);
  uint64_t pk = shows.available_primary_key();
  if (pk == 0) pk = 1;

  const auto now = nowtp();
  shows.emplace(get_self(), [&](auto& r){
    r.show_id             = pk;
    r.category            = category;
    r.ticket_transferable = ticket_transferable;
    r.ticket_refundable   = ticket_refundable;
    r.sale_started_at     = sale_started_at;
    r.sale_ended_at       = sale_ended_at;
    r.show_started_at     = show_started_at;
    r.show_ended_at       = show_ended_at;
    r.status              = status;
    r.created_at          = now;
    r.updated_at          = now;
  });
}

void show::setshow(const uint64_t&   show_id,
                   const name&       category,
                   const bool&       ticket_transferable,
                   const bool&       ticket_refundable,
                   const time_point& sale_started_at,
                   const time_point& sale_ended_at,
                   const time_point& show_started_at,
                   const time_point& show_ended_at,
                   const name&       status)
{
  check_admin_or_showadm();

  show_t::showidx shows(get_self(), get_self().value);
  auto it = shows.find(show_id);
  check(it != shows.end(), "show not found");

  check(sale_ended_at    >= sale_started_at, "sale_ended_at must be >= sale_started_at");
  check(show_started_at  >= sale_ended_at,   "show_started_at must be >= sale_ended_at");
  check(show_ended_at    >= show_started_at, "show_ended_at must be >= show_started_at");

  const auto now = nowtp();
  shows.modify(it, same_payer, [&](auto& r){
    r.category            = category;
    r.ticket_transferable = ticket_transferable;
    r.ticket_refundable   = ticket_refundable;
    r.sale_started_at     = sale_started_at;
    r.sale_ended_at       = sale_ended_at;
    r.show_started_at     = show_started_at;
    r.show_ended_at       = show_ended_at;
    r.status              = status;
    r.updated_at          = now;
  });
}

void show::showstatus(const uint64_t& show_id,
                      const name&     status)
{
  check_admin_or_showadm();

  show_t::showidx shows(get_self(), get_self().value);
  auto it = shows.find(show_id);
  check(it != shows.end(), "show not found");

  shows.modify(it, same_payer, [&](auto& r){
    r.status     = status;
    r.updated_at = nowtp();
  });
}

// ===== 票档 =====
void show::newticket(const uint64_t& show_id,
                     const nsymbol&  ticket_nsym,
                     const nsymbol&  prerequisite_nsym,
                     const string&   ticket_type,
                     const asset&    price,
                     const uint32_t& total_count,
                     const name&     status)
{
  check_admin_or_showadm();

  check(_gstate.nft_bank.value != 0, "nft_bank not set");
  show_t::showidx shows(get_self(), get_self().value);
  check(shows.find(show_id) != shows.end(), "show not found");

  check(total_count > 0, "total_count must be positive");
  check(price.amount >= 0, "price must be non-negative");

  ticket_t::ticketidx tks(get_self(), show_id);
  const uint64_t tkid = ticket_nsym.raw();
  check(tks.find(tkid) == tks.end(), "ticket already exists in this show");

  const auto now = nowtp();
  tks.emplace(get_self(), [&](auto& r){
    r.ticket_id              = tkid;
    r.prerequisite_ticket_id = prerequisite_nsym.raw();
    r.ticket_type            = ticket_type;
    r.price                  = price;
    r.total_count            = total_count;
    r.sold_count             = 0;
    r.stock_count            = total_count;
    r.issued_count           = 0;
    r.status                 = status;
    r.created_at             = now;
    r.updated_at             = now;
  });
}

void show::setticket(const uint64_t& show_id,
                     const uint64_t& ticket_id,
                     const string&   ticket_type,
                     const asset&    price,
                     const uint32_t& total_count,
                     const name&     status)
{
  check_admin_or_showadm();

  ticket_t::ticketidx tks(get_self(), show_id);
  auto it = tks.find(ticket_id);
  check(it != tks.end(), "ticket not found");

  check(price.amount >= 0, "price must be non-negative");
  check(total_count >= it->sold_count, "total_count cannot be less than sold_count");

  const auto now = nowtp();
  tks.modify(it, same_payer, [&](auto& r){
    r.ticket_type  = ticket_type;
    r.price        = price;
    r.total_count  = total_count;
    r.stock_count  = r.total_count - r.sold_count;
    r.status       = status;
    r.updated_at   = now;
  });
}

void show::ticketstatus(const uint64_t& show_id,
                        const uint64_t& ticket_id,
                        const name&     status)
{
  check_admin_or_showadm();

  ticket_t::ticketidx tks(get_self(), show_id);
  auto it = tks.find(ticket_id);
  check(it != tks.end(), "ticket not found");

  tks.modify(it, same_payer, [&](auto& r){
    r.status     = status;
    r.updated_at = nowtp();
  });
}

// ===== 发放 =====
void show::issue(const name&     user,
                 const uint64_t& show_id,
                 const uint64_t& ticket_id,
                 const uint32_t& amount,
                 const string&   memo)
{
  check_admin_or_showadm();

  check(is_account(user), "user not exist");
  check(_gstate.nft_bank.value != 0, "nft_bank not set");
  check(amount > 0, "amount must be positive");
  check(memo.size() <= 256, "memo has more than 256 bytes");

  ticket_t::ticketidx tks(get_self(), show_id);
  auto it = tks.find(ticket_id);
  check(it != tks.end(), "ticket not found");

  // 库存检查
  check(it->stock_count >= amount, "insufficient ticket stock");

  // 转 NFT（从本合约账号 _self 发出）
  {
    vector<nasset> packs;
    nsymbol sym(ticket_id);
    packs.emplace_back(static_cast<int64_t>(amount), sym);

    flon::cvticket::transfer_action{
      _gstate.nft_bank,
      { permission_level{ _self, "active"_n } }
    }.send(_self, user, packs, memo);
  }

  // 更新计数
  const auto now = nowtp();
  tks.modify(it, same_payer, [&](auto& r){
    r.sold_count   += amount;
    check(r.sold_count <= r.total_count, "sold overflow");
    r.stock_count   = r.total_count - r.sold_count;
    r.issued_count += amount;
    r.updated_at    = now;
  });
}

} // namespace flon