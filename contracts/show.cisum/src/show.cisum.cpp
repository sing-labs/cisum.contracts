#include <show.cisum.hpp>
#include <show.cisum.db.hpp>
#include "flon.swap/utils.hpp"
#include <eosio/check.hpp>
#include <string>
#include <vector>
using std::string;
using std::vector;
#include <grab.cisum.db.hpp>

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

void show::record_ticket_increase(uint64_t               show_id,
                                  uint64_t               ticket_id,
                                  uint64_t               ticket_count,
                                  uint64_t               prev_ticket_count,
                                  const name&            issuer,
                                  const string&          memo)
{
    ticket_increase_idx tbl(get_self(), get_self().value);

    // 获取自增主键（multi_index 的 available_primary_key 在空表时返回 0）
    uint64_t pk = tbl.available_primary_key();
    if (pk == 0) pk = 1;

    tbl.emplace(get_self(), [&](auto& row) {
        row.id                    = pk;
        row.show_id               = show_id;               // 由 scope 或 memo 解析得到
        row.ticket_id             = ticket_id;             // nsymbol(raw)
        row.ticket_count          = ticket_count;                // 与 nasset.amount 对齐
        row.prev_ticket_count     = prev_ticket_count;           // 修改前的票数
        row.memo                  = memo;
        row.issuer                = issuer;                // 实际操作者（admin / show_admin / 合约）
        row.created_at            = current_time_point();
    });
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
void show::nftcreate(
                    const int64_t& max_supply,
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

void show::nftissue(const name&   issuer,
                    const name&   to,
                    const nasset& quantity,
                    const string& memo)
{

  require_any_admin();

  check(_gstate.nft_bank.value != 0, "nft_bank not set");
  check(is_account(_gstate.nft_bank), "nft_bank not exist");
  check(quantity.amount > 0, "quantity must be positive");
  check(memo.size() <= 256, "memo too long");

  auto parts = split(memo, ":");
  check(parts.size() >= 2 && parts[0] == "issue", "memo must be 'issue:<show_id>'");
  uint64_t show_id = std::stoull(std::string(parts[1]));
  check(show_id > 0, "invalid show_id");

  // 铸造 NFT 到指定账户
  flon::cvticket::issue_action{
    _gstate.nft_bank,
    { permission_level{ get_self(), "active"_n } }
  }.send(to, quantity, memo);

  // 更新库存
  ticket_t::ticketidx tickets(get_self(), show_id);
  auto itr = tickets.find(quantity.symbol.raw());
  check(itr != tickets.end(), "ticket not found in nftissue");
  uint64_t prev_amount = static_cast<uint64_t>(itr->total_count);
  tickets.modify(itr, same_payer, [&](auto& row){
      row.total_count += quantity.amount;
      row.stock_count += quantity.amount;
      row.updated_at = current_time_point();
  });

  // 记录票量增加日志
  record_ticket_increase(
    show_id,                     // 演出ID
    quantity.symbol.raw(),       // 票的symbol
    static_cast<uint64_t>(quantity.amount),             // 增加数量
    prev_amount,
    issuer,                      // 实际操作者
    memo                         // 备注
  );

}

// ========== 演出 ==========
void show::newshow(const uint64_t&   show_id,
                   const name&       category,
                   const bool&       ticket_transferable,
                   const bool&       ticket_refundable,
                   const time_point& show_started_at,
                   const time_point& show_ended_at,
                   const string&       show_name,
                   const string&       show_address)
{
  require_admin_or_showadm();

  check(show_started_at != time_point{}, "show_started_at is required");
  if (show_ended_at != time_point{}) {
    check(show_ended_at >= show_started_at, "show_ended_at must be >= show_started_at");
  }

  show_t::showidx shows(get_self(), get_self().value);

  // 检查是否已存在
  auto it = shows.find(show_id);
  check(it == shows.end(), "show_id already exists");

  const auto t = nowtp();
  shows.emplace(get_self(), [&](auto& r){
    r.show_id               = show_id;
    r.category              = category;
    r.ticket_transferable   = ticket_transferable;
    r.ticket_refundable     = ticket_refundable;
    r.show_name             = show_name;
    r.show_address          = show_address;
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
                   const time_point& show_started_at,
                   const time_point& show_ended_at,
                   const string&       show_name,
                   const string&       show_address)
{
    require_admin_or_showadm();

    show_t::showidx shows(get_self(), get_self().value);
    auto it = shows.find(show_id);
    check(it != shows.end(), "show not found");

    // ===== 时间校验 =====
    const bool has_show_start = (show_started_at != time_point{});
    const bool has_show_end   = (show_ended_at   != time_point{});

    // show_started_at 必填
    check(has_show_start, "show_started_at is required");

    // 如果有 show_ended_at，则校验必须大于等于 show_started_at
    if (has_show_end) {
        check(show_ended_at >= show_started_at,
              "show_ended_at must be >= show_started_at");
    }

    const auto t = nowtp();

    // ===== 修改记录 =====
    shows.modify(it, same_payer, [&](auto& r){
        r.category            = category;
        r.ticket_transferable = ticket_transferable;
        r.ticket_refundable   = ticket_refundable;
        r.show_started_at     = show_started_at;
        r.show_ended_at       = show_ended_at;
        r.show_name           = show_name;
        r.show_address        = show_address;
        r.updated_at          = t;
    });
}


// ========== 票档（scope: show_id） ==========
void show::newticket(const uint64_t& show_id,
                     const nsymbol&  ticket_nsym,
                     const nsymbol&  prerequisite_nsym,
                     const string&   ticket_type,
                     const asset&    price,
                     const asset&    price_usd,
                     const time_point& sale_started_at,
                     const time_point& sale_ended_at)
{
  require_admin_or_showadm();

  check(is_account(_gstate.nft_bank), "nft_bank not exist");
  check(price.amount >= 0, "price must be >= 0");
  check(price_usd.amount >= 0, "price_usd must be >= 0");
  check(ticket_type.size() <= 64, "ticket_type too long");

  check(sale_started_at != time_point{}, "sale_started_at is required");
  check(sale_ended_at   != time_point{}, "sale_ended_at is required");
  check(sale_ended_at >= sale_started_at, "sale_ended_at must be >= sale_started_at");

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
    r.price_usd               = price_usd;
    r.total_count             = 0;
    r.sold_count              = 0;
    r.stock_count             = 0;
    r.issued_count            = 0;
    r.sale_started_at         = sale_started_at;
    r.sale_ended_at           = sale_ended_at;
    r.created_at              = t;
    r.updated_at              = t;
  });
}

void show::setticket(const uint64_t& show_id,
                     const uint64_t& ticket_id,
                     const string&   ticket_type,
                     const asset&    price,
                     const asset&    price_usd,
                     const time_point& sale_started_at,
                     const time_point& sale_ended_at)
{
  require_admin_or_showadm();

  ticket_t::ticketidx tks(get_self(), show_id);
  auto it = tks.find(ticket_id);
  check(it != tks.end(), "ticket not found");

  // 基本校验
  check(price.amount >= 0, "price must be >= 0");
  check(price_usd.amount >= 0, "price_usd must be >= 0");
  check(ticket_type.size() <= 64, "ticket_type too long");

  // 售卖窗口校验
  check(sale_started_at != time_point{}, "sale_started_at is required");
  check(sale_ended_at   != time_point{}, "sale_ended_at is required");
  check(sale_ended_at >= sale_started_at, "sale_ended_at must be >= sale_started_at");

  const auto t = nowtp();
  tks.modify(it, same_payer, [&](auto& r){
    r.ticket_type     = ticket_type;
    r.price           = price;
    r.price_usd       = price_usd;
    r.sale_started_at = sale_started_at;
    r.sale_ended_at   = sale_ended_at;
    r.updated_at      = t;
  });
}

// ========== 发放 ==========
void show::issue(const name&     user,
                 const uint64_t& show_id,
                 const uint64_t& ticket_id,
                 const uint32_t& ticket_count,
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
  check(ticket_count > 0, "ticket_count must be positive");
  check(memo.size() <= 256, "memo too long");

  ticket_t::ticketidx tks(get_self(), show_id);
  auto it = tks.find(ticket_id);
  check(it != tks.end(), "ticket not found");
  check(it->stock_count >= ticket_count, "insufficient stock");

  // === 售卖时间窗口校验（以票档为准） ===
  const auto now = nowtp();
  check(it->sale_started_at <= now, "ticket not started yet");
  check(now <= it->sale_ended_at,   "ticket already ended");


  nsymbol tk_sym(ticket_id);

  // 转 NFT（从本合约账号 _self 发出）
  {
    vector<nasset> packs;
    packs.emplace_back(static_cast<int64_t>(ticket_count), tk_sym);

    flon::cvticket::transfer_action{
      _gstate.nft_bank,
      { permission_level{ get_self(), "active"_n } }
    }.send(get_self(), user, packs, memo);
  }

  const auto t = nowtp();
  tks.modify(it, same_payer, [&](auto& r){
    r.sold_count   += ticket_count;
    check(r.sold_count <= r.total_count, "sold overflow");
    r.stock_count   = r.total_count - r.sold_count;
    r.issued_count += ticket_count;
    r.updated_at    = t;
  });
}

void show::issuetograb(const name& to, const nasset& quantity, const string& memo) {

  require_admin_or_showadm();
  check(_gstate.nft_bank.value != 0, "nft_bank not set");
  check(is_account(_gstate.nft_bank), "nft_bank not exist");
  check(is_account(to), "to not exist");
  check(quantity.amount > 0, "quantity must be positive");
  check(memo.size() <= 256, "memo too long");

  // memo --   add:<rush_sale_id>:<show_id>
  check(memo.rfind("add:", 0) == 0, "memo must start with 'add:'");
  auto parts = split(memo, ":");
  check(parts.size() == 3, "memo format invalid");
  uint64_t rush_sale_id = std::stoull(std::string(parts[1]));
  uint64_t show_id      = std::stoull(std::string(parts[2]));
  check(rush_sale_id > 0, "invalid rush_sale_id");
  check(show_id > 0, "invalid show_id");

  // 更新票档库存
  ticket_t::ticketidx tickets(get_self(), show_id);
  auto itr = tickets.find(quantity.symbol.raw());
  check(itr != tickets.end(), "ticket not found for this symbol");

  tickets.modify(itr, same_payer, [&](auto& row){
    check(row.stock_count >= static_cast<uint64_t>(quantity.amount), "insufficient stock");
    row.stock_count -= static_cast<uint64_t>(quantity.amount);
    row.updated_at = current_time_point();
  });

  // 执行转账（从 show 合约 -> grab 合约）
  flon::cvticket::transfer_action{
    _gstate.nft_bank,
    { permission_level{ get_self(), "active"_n } }
  }.send(get_self(), to, std::vector<nasset>{ quantity }, "add:"+std::to_string(rush_sale_id) );
}


} // namespace flon