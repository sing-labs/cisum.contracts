#include <show.cisum.hpp>
#include <show.cisum.db.hpp>
#include "flon.swap/utils.hpp"
#include <eosio/check.hpp>
#include <string>
#include <vector>
using std::string;
using std::vector;
#include <grab.cisum.db.hpp>
#include <limits>  // for std::numeric_limits
#include <eosio/crypto.hpp>
#include <pop.cisum.hpp>
#include "flon.auth/flon.auth.hpp"
#include "flon/consts.hpp"
namespace flon {

static std::string to_hex(const checksum256& c) {
    auto bytes = c.extract_as_byte_array();
    static const char* lut = "0123456789abcdef";
    std::string out;
    out.resize(64);
    for (size_t i = 0; i < 32; ++i) {
        unsigned char b = bytes[i];
        out[2*i]     = lut[(b >> 4) & 0xF];
        out[2*i + 1] = lut[b & 0xF];
    }
    return out;
}

// 生成“批次哈希”：oper + 当前秒级时间戳 -> sha256 -> hex（取前16位，缩短 memo）
static std::string make_batch_hash(const eosio::name& oper) {
    uint64_t ts = eosio::current_time_point().sec_since_epoch();
    std::string input = oper.to_string();
    input += std::to_string(ts);
    checksum256 d = sha256(input.c_str(), input.size());
    std::string hex = to_hex(d);
    return hex.substr(0, 32); // 控制长度，避免 memo 超 256
}

// ---------- 小工具 ----------
static inline time_point nowtp() { return current_time_point(); }

void show::require_role(const name& submitter,
                        const std::vector<std::string>& roles) const {
    require_auth(submitter);

    flonauth::checkrole_action(
        FLONAUTH_CONTRACT,
        { get_self(), "active"_n }        // 本合约自己授权
    ).send(
        get_self(),                       // submitter = 本合约
        get_self(),                       // contract = 本合约作用域
        submitter,                        // 要校验的用户
        roles
    );
}


void show::tkincrease(const uint64_t&  show_id,
                      const uint64_t&        ticket_id,
                      const uint64_t&        ticket_count,
                      const uint64_t&        prev_ticket_count,
                      const name&            issuer,
                      const string&          memo,
                      const uint64_t&        created_at)
{
     require_auth(get_self());
}


void show::init(const name& admin,const name& nft_bank) {
  require_auth(get_self());
  check(is_account(admin), "admin not exist");
  _gstate.admin = admin;
  _gstate.nft_bank =nft_bank;

}


void show::nftcreate(const name& submitter,
                    const int64_t& max_supply,
                    const nsymbol& symbol,
                    const string&  token_uri)
{
  require_role(submitter, {"admin","showadmin"});

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

void show::nftissue(const name& submitter,
                    const name&   issuer,
                    const nasset& quantity,
                    const string& memo)
{

  require_role(submitter, {"admin","showadmin"});

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
  }.send(_self, quantity, memo);

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

  tkincrease_action{
    get_self(),
    { permission_level{ get_self(), "active"_n } }
  }.send(    show_id,                     // 演出ID
    quantity.symbol.raw(),       // 票的symbol
    quantity.amount,             // 增加数量
    prev_amount,
    issuer,                      // 实际操作者
    memo,                         // 备注
    current_time_point().time_since_epoch().count() / 1'000'000);

}

// ========== 演出 ==========
void show::newshow(const name& submitter,
                   const uint64_t&   show_id,
                   const name&       category,
                   const bool&       ticket_transferable,
                   const bool&       ticket_refundable,
                   const time_point& show_started_at,
                   const time_point& show_ended_at,
                   const string&       show_name,
                   const string&       show_address)
{
  require_role(submitter, {"admin","showadmin"});

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

void show::setshow(const name& submitter,
                   const uint64_t&   show_id,
                   const name&       category,
                   const bool&       ticket_transferable,
                   const bool&       ticket_refundable,
                   const time_point& show_started_at,
                   const time_point& show_ended_at,
                   const string&       show_name,
                   const string&       show_address)
{
    require_role(submitter, {"admin","showadmin"});

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
void show::newticket(const name& submitter,
                     const uint64_t& show_id,
                     const nsymbol&  ticket_nsym,
                     const nsymbol&  prerequisite_nsym,
                     const string&   ticket_type,
                     const asset&    price,
                     const asset&    price_usdt,
                     const time_point& sale_started_at,
                     const time_point& sale_ended_at)
{
    require_role(submitter, {"admin","showadmin"});

    check(is_account(_gstate.nft_bank), "nft_bank not exist");
    check(price.amount >= 0, "price must be >= 0");
    check(price_usdt.amount >= 0, "price_usdt must be >= 0");
    check(price_usdt.symbol.code() == USDT_SYM.code(),"price_usdt symbol mismatch, must be USDT");
    check(price_usdt.symbol.precision() <= 6,"price_usdt precision must be <= 6");
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
      r.price_usdt               = price_usdt;
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

void show::setticket(const name& submitter,
                     const uint64_t& show_id,
                     const uint64_t& ticket_id,
                     const string&   ticket_type,
                     const asset&    price,
                     const asset&    price_usdt,
                     const time_point& sale_started_at,
                     const time_point& sale_ended_at)
{
  require_role(submitter, {"admin","showadmin"});

  ticket_t::ticketidx tks(get_self(), show_id);
  auto it = tks.find(ticket_id);
  check(it != tks.end(), "ticket not found");

  // 基本校验
  check(price.amount >= 0, "price must be >= 0");
  check(price_usdt.amount >= 0, "price_usdt must be >= 0");
  check(price_usdt.symbol.code() == USDT_SYM.code(),"price_usdt symbol mismatch, must be USDT");
  check(price_usdt.symbol.precision() <= 6,"price_usdt precision must be <= 6");
  check(ticket_type.size() <= 64, "ticket_type too long");

  // 售卖窗口校验
  check(sale_started_at != time_point{}, "sale_started_at is required");
  check(sale_ended_at   != time_point{}, "sale_ended_at is required");
  check(sale_ended_at >= sale_started_at, "sale_ended_at must be >= sale_started_at");

  const auto t = nowtp();
  tks.modify(it, same_payer, [&](auto& r){
    r.ticket_type     = ticket_type;
    r.price           = price;
    r.price_usdt       = price_usdt;
    r.sale_started_at = sale_started_at;
    r.sale_ended_at   = sale_ended_at;
    r.updated_at      = t;
  });
}

// ========== 发放 ==========
void show::issue(const name&     submitter,
                 const name&     user,
                 const uint64_t& show_id,
                 const uint64_t& ticket_id,
                 const uint32_t& ticket_count,
                 const string&   memo)
{
  require_role(submitter, {"admin","showadmin"});
  show_t::showidx shows(get_self(), get_self().value);
  auto sit = shows.find(show_id);
  check(sit != shows.end(), "show not found");

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

void show::giftbatch(const name&          oper,
                     const uint64_t&      show_id,
                     const uint64_t&      ticket_id,
                     const uint32_t&      ticket_count,
                     const vector<name>&  recipients,
                     const string&        memo)
{

    require_role(oper, {"admin","showadmin"});

    CHECKC(!recipients.empty(), err::INVALID_FORMAT, "recipients is empty");

    uint64_t cnt = static_cast<uint64_t>(recipients.size());
    CHECKC(cnt <= 500, err::INVALID_FORMAT, "too many recipients in one batch (max 500)");

    // 找票种
    ticket_t::ticketidx tickets(get_self(), show_id);
    auto it_ticket = tickets.find(ticket_id);
    CHECKC(it_ticket != tickets.end(), err::RECORD_NO_FOUND, "ticket_id not exists under show_id");

    uint64_t stock_avail = it_ticket->stock_count;

    // 计算总需求
    CHECKC(ticket_count <= std::numeric_limits<uint64_t>::max() / cnt,
           err::AMOUNT_TOO_LARGE, "total quantity overflow");
    uint64_t total_need = static_cast<uint64_t>(ticket_count) * cnt;

    CHECKC(total_need <= stock_avail, err::INSUFFICIENT_QUANTITY, "insufficient ticket stock");

    // 内联 issue
    // giftissue:$<show_id>:$<ticket_id>:$<md5>:$<oper>
    auto inline_issue = [&](const name& to, uint32_t qty) {
        std::string batch_hash = make_batch_hash(to);
        std::string full_memo = "giftissue:"
                          + std::to_string(show_id)
                          + ":"
                          + std::to_string(ticket_id)
                          + ":"
                          + batch_hash
                           + ":"+oper.to_string()+":"+memo;
        issue_action issue{ get_self(), { get_self(), "active"_n } };
        issue.send(oper,to, show_id, ticket_id, qty,full_memo);
    };

    // 发票
    for (const auto& to : recipients) {
        CHECKC(to.value && is_account(to), err::ACCOUNT_INVALID, "invalid recipient");
        inline_issue(to, ticket_count);
    }
}

void show::issuetograb(const name&  submitter,const name& to, const nasset& quantity, const string& memo) {

  require_role(submitter, {"admin","showadmin"});

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

void show::buyticket(const name&  submitter,
                     const name&          payer,
                     const asset&         pay_amount,
                     const uint64_t&      show_id,
                     const uint64_t&      ticket_id,
                     const uint32_t&      ticket_count,
                     const string&        memo)
{
    require_role(submitter, {"admin","showadmin","platform_admin"});

    check(is_account(payer), "invalid payer");
    check(pay_amount.is_valid(), "invalid pay_amount");

    // 只要求符号代码为 USDT，精度不超过 6
    check(pay_amount.symbol.code() == USDT_SYM.code(),
          "[[5]] pay_amount symbol must be USDT");
    check(pay_amount.symbol.precision() <= 6,
          "[[6]] pay_amount precision must be <= 6");

    check(pay_amount.amount > 0, "pay_amount must be positive");
    check(ticket_count > 0, "ticket_count must be positive");

    // ===== 查票价 =====
    ticket_t::ticketidx tickets(get_self(), show_id);
    auto it = tickets.find(ticket_id);
    check(it != tickets.end(), "ticket not found");

    const auto now = current_time_point();
    check(it->sale_started_at <= now,
          std::string("ticket not started yet, now=") + now.to_string() +
          ", sale_started_at=" + it->sale_started_at.to_string());
    check(now <= it->sale_ended_at,
          std::string("ticket already ended, now=") + now.to_string() +
          ", sale_ended_at=" + it->sale_ended_at.to_string());

    // ===== 归一化金额比较（统一到 USDT_SYM 的精度）=====
    auto normalize_amount = [&](const asset& a, const symbol& target) -> __int128 {
        check(a.symbol.code() == target.code(), "symbol code mismatch");
        int sp = a.symbol.precision();
        int dp = target.precision();
        __int128 v = (__int128)a.amount;
        if (sp < dp) {
            for (int i = 0; i < dp - sp; i++) v *= 10;
        } else if (sp > dp) {
            for (int i = 0; i < sp - dp; i++) v /= 10; // 截断到目标精度
        }
        return v;
    };

    __int128 due_units = normalize_amount(it->price_usdt, USDT_SYM) * (__int128)ticket_count;
    __int128 pay_units = normalize_amount(pay_amount,    USDT_SYM);

    check(pay_units <= due_units,
          "overpayment not allowed: require "
          + asset((int64_t)due_units, USDT_SYM).to_string()
          + ", got " + pay_amount.to_string());

    // 把归一化后的最小单位装回 asset（6 位 USDT）
    asset normalized_pay{ (int64_t)pay_units, USDT_SYM };

    // ===== pop 奖励（传 6 位 USDT 的 normalized_pay）=====
    pop_cisum::mine_action mine{ POP_CONTRACT, { get_self(), "active"_n } };
    mine.send(payer, normalized_pay, memo);

    // ===== 发票 =====
    issue_action issue{ get_self(), { get_self(), "active"_n } };
    issue.send(submitter,payer, show_id, ticket_id, ticket_count, memo);
}


} // namespace flon