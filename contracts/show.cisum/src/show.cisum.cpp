#include <show.cisum.hpp>
#include <show.cisum.db.hpp>

#include <eosio/check.hpp>
#include <string>
#include <vector>
using std::string;
using std::vector;

namespace flon {

// ---------- 小工具 ----------
static inline time_point nowtp() { return current_time_point(); }

static bool has_any_auth_in(const std::set<name>& whitelist) {
  for (const auto& acct : whitelist) {
    if (has_auth(acct)) return true;
  }
  return false;
}

// ---------- 私有权限检查 ----------

void show::require_platform_admin() const {
  check(!_gstate.platform_admin.empty(), "platform_admin empty");
  bool ok = false;
  for (auto& a : _gstate.platform_admin) {
    if (has_auth(a)) { ok = true; break; }
  }
  check(ok, "missing platform_admin auth");
}

void show::require_show_admin() const {
  check(!_gstate.show_admin.empty(), "show_admin empty");
  bool ok = false;
  for (auto& a : _gstate.show_admin) {
    if (has_auth(a)) { ok = true; break; }
  }
  check(ok, "missing show_admin auth");
}

void show::require_admin_or_showadm() const {
  if (_gstate.admin.value && has_auth(_gstate.admin)) return;
  if (has_any_auth_in(_gstate.show_admin)) return;
  check(false, "requires admin or show_admin");
}

void show::require_admin_or_platadm() const {
  if (_gstate.admin.value && has_auth(_gstate.admin)) return;
  if (has_any_auth_in(_gstate.platform_admin)) return;
  check(false, "requires admin or platform_admin");
}

void show::require_any_admin() const {
  if (_gstate.admin.value && has_auth(_gstate.admin)) return;
  if (has_any_auth_in(_gstate.platform_admin)) return;
  if (has_any_auth_in(_gstate.show_admin)) return;
  check(false, "requires admin/platform_admin/show_admin");
}


static bool has_show_checker_auth(const show_t& s) {
  for (const auto& a : s.ticket_check_admins) {
    if (has_auth(a)) return true;
  }
  return false;
}

// ========== 全局设置 ==========
void show::init(const name& admin) {
  require_auth(get_self());
  check(is_account(admin), "admin not exist");
  _gstate.admin = admin;

}


void show::addshowadm(const name& account) {
  require_auth(_gstate.admin);
  check(is_account(account), "account not exist");
  _gstate.show_admin.insert(account);
}
void show::delshowadm(const name& account) {
  require_auth(_gstate.admin);
  _gstate.show_admin.erase(account);
}

// platform_admin 管理：仅 admin 可改
void show::addplatadm(const name& account) {
  require_auth(_gstate.admin);
  check(is_account(account), "account not exist");
  _gstate.platform_admin.insert(account);
}
void show::delplatadm(const name& account) {
  require_auth(_gstate.admin);
  _gstate.platform_admin.erase(account);
}

// per-show 核销员：admin 或 show_admin 可改
void show::addchecker(const uint64_t& show_id, const name& account) {
  require_admin_or_showadm();
  check(is_account(account), "account not exist");

  show_t::showidx shows(get_self(), get_self().value);
  auto it = shows.find(show_id);
  check(it != shows.end(), "show not found");

  shows.modify(it, same_payer, [&](auto& r){
    r.ticket_check_admins.insert(account);
    r.updated_at = nowtp();
  });
}
void show::delchecker(const uint64_t& show_id, const name& account) {
  require_admin_or_showadm();

  show_t::showidx shows(get_self(), get_self().value);
  auto it = shows.find(show_id);
  check(it != shows.end(), "show not found");

  shows.modify(it, same_payer, [&](auto& r){
    r.ticket_check_admins.erase(account);
    r.updated_at = nowtp();
  });
}

// ========== cvticket.nft: 票种创建 / 发放 ==========
void show::nftcreate(const int64_t& max_supply,
                    const nsymbol& symbol,
                    const string&  token_uri)
{
  require_any_admin();

  check(_gstate.nft_bank.value != 0, "nft_bank not set");
  check(is_account(_gstate.nft_bank), "nft_bank not exist");
  check(max_supply > 0, "max_supply must be positive");
  check(token_uri.size() <= 256, "token_uri too long");

  // issuer/ipowner 都设为本合约账号
  flon::cvticket::create_action{
    _gstate.nft_bank,
    { permission_level{ get_self(), "active"_n } }
  }.send(get_self(), max_supply, symbol, token_uri, get_self());
}

void show::nftissue(const nasset& quantity,
                   const string& memo)
{
  // 发放到合约自身账号
  require_any_admin();

  check(_gstate.nft_bank.value != 0, "nft_bank not set");
  check(is_account(_gstate.nft_bank), "nft_bank not exist");
  check(quantity.amount > 0, "quantity must be positive");
  check(memo.size() <= 256, "memo too long");

  flon::cvticket::issue_action{
    _gstate.nft_bank,
    { permission_level{ get_self(), "active"_n } }
  }.send(get_self(), quantity, memo);
}

// ========== 演出 ==========
void show::newshow(const name&       category,
                   const bool&       ticket_transferable,
                   const bool&       ticket_refundable,
                   const time_point& sale_started_at,
                   const time_point& sale_ended_at,
                   const time_point& show_started_at,
                   const time_point& show_ended_at,
                   const name&       status)
{
  require_admin_or_showadm();

  check(sale_ended_at   >= sale_started_at, "sale_ended_at must be >= sale_started_at");
  check(show_started_at >= sale_ended_at,   "show_started_at must be >= sale_ended_at");
  check(show_ended_at   >= show_started_at, "show_ended_at must be >= show_started_at");

  show_t::showidx shows(get_self(), get_self().value);
  uint64_t pk = shows.available_primary_key();
  if (pk == 0) pk = 1;

  const auto t = nowtp();
  shows.emplace(get_self(), [&](auto& r){
    r.show_id               = pk;
    r.category              = category;
    r.ticket_transferable   = ticket_transferable;
    r.ticket_refundable     = ticket_refundable;
    r.status                = status;
    r.sale_started_at       = sale_started_at;
    r.sale_ended_at         = sale_ended_at;
    r.show_started_at       = show_started_at;
    r.show_ended_at         = show_ended_at;
    r.created_at            = t;
    r.updated_at            = t;
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
  require_admin_or_showadm();

  show_t::showidx shows(get_self(), get_self().value);
  auto it = shows.find(show_id);
  check(it != shows.end(), "show not found");

  check(sale_ended_at   >= sale_started_at, "sale_ended_at must be >= sale_started_at");
  check(show_started_at >= sale_ended_at,   "show_started_at must be >= sale_ended_at");
  check(show_ended_at   >= show_started_at, "show_ended_at must be >= show_started_at");

  const auto t = nowtp();
  shows.modify(it, same_payer, [&](auto& r){
    r.category              = category;
    r.ticket_transferable   = ticket_transferable;
    r.ticket_refundable     = ticket_refundable;
    r.status                = status;
    r.sale_started_at       = sale_started_at;
    r.sale_ended_at         = sale_ended_at;
    r.show_started_at       = show_started_at;
    r.show_ended_at         = show_ended_at;
    r.updated_at            = t;
  });
}

void show::showstatus(const uint64_t& show_id,
                      const name&     status)
{
  require_admin_or_showadm();

  show_t::showidx shows(get_self(), get_self().value);
  auto it = shows.find(show_id);
  check(it != shows.end(), "show not found");

  shows.modify(it, same_payer, [&](auto& r){
    r.status     = status;
    r.updated_at = nowtp();
  });
}

// ========== 票档（scope: show_id） ==========
void show::newticket(const uint64_t& show_id,
                     const nsymbol&  ticket_nsym,
                     const nsymbol&  prerequisite_nsym,
                     const string&   ticket_type,
                     const asset&    price,
                     const uint32_t& total_count,
                     const name&     status)
{
  require_admin_or_showadm();

  check(is_account(_gstate.nft_bank), "nft_bank not exist");
  check(total_count > 0, "total_count must be positive");
  check(price.amount >= 0, "price must be >= 0");
  check(ticket_type.size() <= 64, "ticket_type too long");

  // show 存在性
  show_t::showidx shows(get_self(), get_self().value);
  auto sit = shows.find(show_id);
  check(sit != shows.end(), "show not found");

  ticket_t::ticketidx tks(get_self(), show_id);
  auto it = tks.find(ticket_nsym.raw());
  check(it == tks.end(), "ticket already exists in this show");

  const auto t = nowtp();
  tks.emplace(get_self(), [&](auto& r){
    r.ticket_id               = ticket_nsym.raw();
    r.prerequisite_ticket_id  = prerequisite_nsym.raw();
    r.ticket_type             = ticket_type;
    r.price                   = price;
    r.total_count             = total_count;
    r.sold_count              = 0;
    r.stock_count             = total_count;
    r.issued_count            = 0;
    r.status                  = status;
    r.created_at              = t;
    r.updated_at              = t;
  });
}

void show::setticket(const uint64_t& show_id,
                     const uint64_t& ticket_id,
                     const string&   ticket_type,
                     const asset&    price,
                     const uint32_t& total_count,
                     const name&     status)
{
  require_admin_or_showadm();

  ticket_t::ticketidx tks(get_self(), show_id);
  auto it = tks.find(ticket_id);
  check(it != tks.end(), "ticket not found");

  check(price.amount >= 0, "price must be >= 0");
  check(total_count >= it->sold_count, "total_count cannot be less than sold_count");
  check(ticket_type.size() <= 64, "ticket_type too long");

  const auto t = nowtp();
  tks.modify(it, same_payer, [&](auto& r){
    r.ticket_type = ticket_type;
    r.price       = price;
    r.total_count = total_count;
    r.stock_count = r.total_count - r.sold_count;
    r.status      = status;
    r.updated_at  = t;
  });
}

void show::ticketstatus(const uint64_t& show_id,
                        const uint64_t& ticket_id,
                        const name&     status)
{
  require_admin_or_showadm();

  ticket_t::ticketidx tks(get_self(), show_id);
  auto it = tks.find(ticket_id);
  check(it != tks.end(), "ticket not found");

  tks.modify(it, same_payer, [&](auto& r){
    r.status     = status;
    r.updated_at = nowtp();
  });
}

// ========== 发放 ==========
void show::issue(const name&     user,
                 const uint64_t& show_id,
                 const uint64_t& ticket_id,
                 const uint32_t& amount,
                 const string&   memo)
{
  // 允许：admin / platform_admin / show_admin / 该场次的 ticket_check_admins
  show_t::showidx shows(get_self(), get_self().value);
  auto sit = shows.find(show_id);
  check(sit != shows.end(), "show not found");

  bool allowed = false;
  if (_gstate.admin.value && has_auth(_gstate.admin)) allowed = true;
  if (!allowed && has_any_auth_in(_gstate.platform_admin)) allowed = true;
  if (!allowed && has_any_auth_in(_gstate.show_admin)) allowed = true;
  if (!allowed && has_show_checker_auth(*sit)) allowed = true;
  check(allowed, "missing issue permission");

  check(is_account(user), "user not exist");
  check(_gstate.nft_bank.value != 0, "nft_bank not set");
  check(is_account(_gstate.nft_bank), "nft_bank not exist");
  check(amount > 0, "amount must be positive");
  check(memo.size() <= 256, "memo too long");

  ticket_t::ticketidx tks(get_self(), show_id);
  auto it = tks.find(ticket_id);
  check(it != tks.end(), "ticket not found");
  check(it->status == TicketStatus::running, "ticket not running");
  check(it->stock_count >= amount, "insufficient stock");

  // === 销售时间窗口校验 ===
  const auto now = nowtp();
  check(sit->sale_started_at <= now, "not started yet");   // 未到开售时间
  check(now <= sit->sale_ended_at,   "already ended");     // 已过截止时间

  // 组装 nsymbol（ticket_id 为 nsymbol.raw()）
  static constexpr uint64_t U1E9 = 1000000000ULL;
  uint64_t p64 = ticket_id / U1E9;
  uint64_t i64 = ticket_id % U1E9;
  check(p64 < U1E9, "bad pid");
  check(i64 < U1E9, "bad id");
  nsymbol tk_sym{ static_cast<uint32_t>(i64), static_cast<uint32_t>(p64) };

  // 转 NFT（从本合约账号 _self 发出）
  {
    vector<nasset> packs;
    packs.emplace_back(static_cast<int64_t>(amount), tk_sym);

    flon::cvticket::transfer_action{
      _gstate.nft_bank,
      { permission_level{ get_self(), "active"_n } }
    }.send(get_self(), user, packs, memo);
  }

  // 更新计数
  const auto t = nowtp();
  tks.modify(it, same_payer, [&](auto& r){
    r.sold_count   += amount;
    check(r.sold_count <= r.total_count, "sold overflow");
    r.stock_count   = r.total_count - r.sold_count;
    r.issued_count += amount;
    r.updated_at    = t;
  });
}

} // namespace flon