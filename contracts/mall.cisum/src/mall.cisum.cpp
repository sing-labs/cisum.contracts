#include "mall.cisum.hpp"
#include <ticket.cvnft/ticket.cvnft.db.hpp>
#include "nft.policy.hpp"
#include <limits>
#include <flon.auth/flon.auth.hpp>
#include <flon/consts.hpp>
#include <tuple>

namespace mall {
namespace {
uint64_t number(const string& s) {
    check(!s.empty() && s.size() <= 20, "MALL_MEMO: invalid integer");
    uint64_t n = 0;
    for (char c : s) {
        check(c >= '0' && c <= '9', "MALL_MEMO: invalid integer");
        check(n <= (std::numeric_limits<uint64_t>::max() - (c-'0')) / 10, "MALL_MEMO: integer overflow");
        n = n * 10 + c-'0';
    }
    check(n > 0, "MALL_MEMO: zero integer");
    return n;
}
vector<string> split(const string& s) {
    check(s.size() <= 256, "MALL_MEMO: too long");
    vector<string> out;
    size_t start = 0;
    for (size_t i = 0; i <= s.size(); ++i) {
        if (i == s.size() || s[i] == ':') {
            out.push_back(s.substr(start, i-start)); start = i+1;
        }
    }
    return out;
}
void request_ok(const string& s) {
    check(!s.empty() && s.size() <= 64, "MALL_REQUEST: invalid length");
    for (char c : s)
        check((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
              (c >= '0' && c <= '9') || c == '-' || c == '_', "MALL_REQUEST: invalid character");
}
void amount_ok(int64_t n) { check(n >= 0 && n <= nasset::max_amount, "MALL_AMOUNT: out of range"); }
}
config_row mall_contract::config() const {
    config_table table(get_self(), get_self().value);
    check(table.exists(), "MALL_CONFIG: not initialized");
    return table.get();
}
name mall_contract::require_perm(name submitter, const string& perm) const {
    if (has_auth(get_self())) return get_self();
    auth_table auth(get_self(), get_self().value);
    check(auth.exists(), "MALL_AUTH: initialize unified authorization first");
    if (submitter == auth.get().admin && has_auth(submitter)) return submitter;
    require_auth(submitter);
    flon::flonauth::checkrole_action(flon::CISUMAUTH_CONTRACT,
        {get_self(), "active"_n}).send(get_self(), submitter, perm);
    return submitter;
}
void mall_contract::bank_ok(name bank, bool nft) const {
    banks table(get_self(), get_self().value);
    const auto& b = table.get(bank.value, "MALL_BANK: unsupported contract");
    check(b.nft == nft, "MALL_BANK: wrong asset kind");
}
void mall_contract::terms_ok(name bank, const sale_terms& t) const {
    bank_ok(bank, false);
    check(t.price.is_valid() && t.price.amount > 0, "MALL_PRICE: invalid");
    funds table(get_self(), bank.value);
    table.get(t.price.symbol.raw(), "MALL_BANK: unsupported symbol/precision");
    check(t.starts.sec_since_epoch() > 0, "MALL_TIME: start required");
    check(t.per_purchase > 0 && t.per_purchase <= t.per_account, "MALL_LIMIT: invalid limits");
}
int64_t mall_contract::balance(const product& p) const {
    flon::account_t::idx_t table(p.nft_contract, get_self().value);
    auto it = table.find(p.nft_id);
    return it == table.end() || it->paused ? 0 : it->balance.amount;
}
void mall_contract::quota_ok(const product& p, int64_t quota) const {
    amount_ok(quota);
    check(quota >= p.sold, "MALL_QUOTA: below sold");
    if (p.mint) {
        flon::nstats_t::idx_t stats(p.nft_contract, p.nft_contract.value);
        const auto& st = stats.get(p.nft_id);
        flon::issuance_table history(p.nft_contract, p.nft_contract.value);
        // Extended backends enforce lifetime issuance; original ticket.cvnft uses current supply.
        auto h = history.find(p.nft_id);
        const auto issued = h == history.end() ? st.supply.amount : h->minted;
        check(issued >= 0 && issued <= st.max_supply.amount, "MALL_SUPPLY: invalid issuance");
        check(quota-p.sold <= st.max_supply.amount-issued, "MALL_QUOTA: exceeds issuance space");
    } else check(quota-p.sold <= balance(p), "MALL_STOCK: insufficient NFT balance");
}
void mall_contract::log(uint64_t id, name actor, name action_name, const string& reason,
                        vector<char> before, vector<char> after) {
    check(id > 0 && !reason.empty() && reason.size() <= 256, "MALL_OPERATION: id and reason required");
    operations table(get_self(), get_self().value);
    check(table.find(id) == table.end(), "MALL_OPERATION: duplicate operation");
    table.emplace(get_self(), [&](auto& r) {
        r.id=id; r.actor=actor; r.action=action_name; r.reason=reason;
        r.before=before; r.after=after; r.at=time_point_sec(current_time_point());
    });
}
void mall_contract::init(name admin) {
    require_auth(get_self());
    check(is_account(admin), "MALL_CONFIG: admin does not exist");
    config_table table(get_self(), get_self().value);
    table.set(table.get_or_default(), get_self());
    auth_table(get_self(), get_self().value).set(auth_config{admin}, get_self());
}
void mall_contract::addbank(name submitter, name bank, bool nft, symbol sym) {
    require_perm(submitter, "productMgmt");
    check(is_account(bank) && bank != get_self(), "MALL_BANK: invalid contract");
    banks table(get_self(), get_self().value);
    auto it=table.find(bank.value);
    if(it == table.end()) table.emplace(get_self(), [&](auto& r){r.bank=bank; r.nft=nft;});
    else check(it->nft == nft, "MALL_BANK: cannot change asset kind");
    if (!nft) {
        check(sym.is_valid(), "MALL_BANK: invalid symbol");
        funds f(get_self(), bank.value);
        check(f.find(sym.raw()) == f.end(), "MALL_BANK: symbol already registered");
        f.emplace(get_self(), [&](auto& r){r.sym=sym;});
    }
}
void mall_contract::advancenft(name submitter, uint64_t next_id, uint64_t op_id, string reason) {
    auto actor=require_perm(submitter, "productMgmt"); auto c=config(); auto before=pack(c);
    check(next_id > c.next_nft && next_id < std::numeric_limits<uint64_t>::max(), "MALL_ID: must advance");
    c.next_nft=next_id;
    log(op_id, actor, "advancenft"_n, reason, before, pack(c));
    config_table(get_self(), get_self().value).set(c,get_self());
}
void mall_contract::createnft(name submitter, name bank, string uri, name ipowner, int64_t maximum, int64_t quota,
                             name paybank, sale_terms terms, uint64_t op_id, string reason, string product_code) {
    auto actor=require_perm(submitter, "productMgmt"); bank_ok(bank,true); terms_ok(paybank,terms);
    check(!product_code.empty() && product_code.size() <= 256, "MALL_PRODUCT: product_code requires 1 to 256 bytes");
    amount_ok(maximum); amount_ok(quota);
    check(maximum > 0 && quota <= maximum, "MALL_SUPPLY: invalid maximum/quota");
    auto c=config();
    flon::nstats_t::idx_t stats(bank,bank.value);
    uint32_t scanned=0;
    while(stats.find(c.next_nft)!=stats.end() && scanned<100) {
        check(c.next_nft < std::numeric_limits<uint64_t>::max(), "MALL_ID: exhausted");
        ++c.next_nft; ++scanned;
    }
    check(scanned<100 && c.next_nft<std::numeric_limits<uint64_t>::max(), "MALL_ID: scan exhausted; audit and advance cursor");
    products table(get_self(),get_self().value);
    product p; p.id=table.available_primary_key(); if(p.id==0)p.id=1;
    p.product_code=product_code;
    p.nft_contract=bank; p.nft_id=c.next_nft++; p.mint=true; p.pay_contract=paybank; p.terms=terms; p.quota=quota;
    log(op_id,actor,"createnft"_n,reason,{},pack(p));
    config_table(get_self(),get_self().value).set(c,get_self());
    table.emplace(get_self(),[&](auto& r){r=p;});
    action(permission_level{get_self(),"active"_n},bank,"create"_n,
           std::make_tuple(get_self(),maximum,flon::nsymbol(p.nft_id),uri,ipowner)).send();
    // Only create the NFT; policies, whitelists and external reservations are not changed.
}
void mall_contract::regstock(name submitter, name bank, uint64_t nft_id, int64_t quota, name paybank,
                            sale_terms terms, uint64_t op_id, string reason) {
    auto actor=require_perm(submitter, "productMgmt"); bank_ok(bank,true); terms_ok(paybank,terms);
    flon::nstats_t::idx_t stats(bank,bank.value); stats.get(nft_id,"MALL_NFT: unknown NFT");
    products table(get_self(),get_self().value); auto idx=table.get_index<"byasset"_n>();
    check(idx.find((uint128_t(bank.value)<<64)|nft_id)==idx.end(),"MALL_NFT: already registered");
    product p; p.id=table.available_primary_key(); if(p.id==0)p.id=1;
    p.product_code=string{};
    p.nft_contract=bank; p.nft_id=nft_id; p.mint=false; p.pay_contract=paybank; p.terms=terms; p.quota=quota;
    bank_ok(p.nft_contract,true); quota_ok(p,quota);
    log(op_id,actor,"regstock"_n,reason,{},pack(p));
    table.emplace(get_self(),[&](auto& r){r=p;});
}
void mall_contract::setsale(name submitter, uint64_t product_id, sale_terms terms, uint64_t op_id, string reason) {
    auto actor=require_perm(submitter, "productMgmt"); products table(get_self(),get_self().value); auto p=table.get(product_id);
    check(p.status==0,"MALL_STATUS: must be offline"); terms_ok(p.pay_contract,terms);
    auto before=pack(p); p.terms=terms;
    log(op_id,actor,"setsale"_n,reason,before,pack(p)); table.modify(table.get(product_id),same_payer,[&](auto& r){r=p;});
}
void mall_contract::setquota(name submitter, uint64_t product_id, int64_t quota, uint64_t op_id, string reason) {
    auto actor=require_perm(submitter, "productMgmt"); products table(get_self(),get_self().value); auto p=table.get(product_id);
    check(p.status!=2,"MALL_STATUS: archived"); quota_ok(p,quota); auto before=pack(p); p.quota=quota;
    log(op_id,actor,"setquota"_n,reason,before,pack(p)); table.modify(table.get(product_id),same_payer,[&](auto& r){r=p;});
}
void mall_contract::setsupply(name submitter, uint64_t product_id, int64_t maximum, int64_t quota, uint64_t op_id, string reason) {
    auto actor=require_perm(submitter, "productMgmt"); products table(get_self(),get_self().value); auto p=table.get(product_id);
    check(p.status==0,"MALL_STATUS: must be offline"); amount_ok(maximum); amount_ok(quota);
    flon::nstats_t::idx_t stats(p.nft_contract,p.nft_contract.value); const auto& st=stats.get(p.nft_id);
    check(st.issuer==get_self(),"MALL_SUPPLY: mall must be issuer");
    flon::issuance_table history(p.nft_contract,p.nft_contract.value); const auto& h=history.get(p.nft_id,"MALL_SUPPLY: history not audited");
    check(maximum>0 && maximum>=h.minted && quota>=p.sold,"MALL_SUPPLY: invalid limits");
    if(p.mint) check(quota-p.sold<=maximum-h.minted,"MALL_SUPPLY: reservation exceeds maximum");
    else quota_ok(p,quota);
    auto before=pack(std::make_tuple(p,st.max_supply.amount)); p.quota=quota;
    log(op_id,actor,"setsupply"_n,reason,before,pack(std::make_tuple(p,maximum)));
    table.modify(table.get(product_id),same_payer,[&](auto& r){r=p;});
    action(permission_level{get_self(),"active"_n},p.nft_contract,"setmaxsup"_n,std::make_tuple(p.nft_id,maximum)).send();
}
void mall_contract::setmeta(name submitter, uint64_t product_id, string uri, uint64_t op_id, string reason) {
    auto actor=require_perm(submitter, "productMgmt"); products table(get_self(),get_self().value); const auto& p=table.get(product_id);
    check(p.status==0,"MALL_STATUS: must be offline");
    flon::nstats_t::idx_t stats(p.nft_contract,p.nft_contract.value); const auto& st=stats.get(p.nft_id);
    check(st.issuer==get_self(),"MALL_META: mall must be issuer");
    log(op_id,actor,"setmeta"_n,reason,pack(std::make_tuple(product_id,st.token_uri)),pack(std::make_tuple(product_id,uri)));
    action(permission_level{get_self(),"active"_n},p.nft_contract,"settokenuri"_n,std::make_tuple(p.nft_id,uri)).send();
}
void mall_contract::setstatus(name submitter, uint64_t product_id, uint8_t status, uint64_t op_id, string reason) {
    auto actor=require_perm(submitter, "productMgmt"); products table(get_self(),get_self().value); auto p=table.get(product_id);
    check(status<=2 && p.status!=2,"MALL_STATUS: invalid transition");
    if(status==1){bank_ok(p.nft_contract,true);quota_ok(p,p.quota);}
    if(status==2) check(p.quota==p.sold,"MALL_STATUS: release quota before archival");
    auto before=pack(p);p.status=status;log(op_id,actor,"setstatus"_n,reason,before,pack(p));
    table.modify(table.get(product_id),same_payer,[&](auto& r){r=p;});
}
void mall_contract::sendnft(const product& p, name to, int64_t amount, const string& memo) {
    action(permission_level{get_self(),"active"_n},p.nft_contract,"transfer"_n,
           std::make_tuple(get_self(),to,vector<nasset>{nasset(amount,flon::nsymbol(p.nft_id))},memo)).send();
}
void mall_contract::withdrawnft(name submitter, uint64_t product_id, name to, int64_t quantity, uint64_t op_id, string reason) {
    auto actor=require_perm(submitter, "productMgmt");products table(get_self(),get_self().value);const auto& p=table.get(product_id);
    check(p.status==0 && !p.mint,"MALL_STOCK: offline inventory product required");
    check(quantity>0 && quantity<=balance(p)-(p.quota-p.sold),"MALL_STOCK: inventory reserved");bank_ok(p.nft_contract,true);
    log(op_id,actor,"withdrawnft"_n,reason,pack(std::make_tuple(product_id,balance(p))),pack(std::make_tuple(product_id,to,quantity)));
    sendnft(p,to,quantity,"mall inventory withdrawal");
}
void mall_contract::allocfund(name submitter, name bank, asset quantity, uint64_t op_id, string reason) {
    auto c=config();auto actor=require_perm(submitter, "mallTreasury");bank_ok(bank,false);
    check(quantity.is_valid() && quantity.amount>0,"MALL_AMOUNT: invalid allocation");
    funds table(get_self(),bank.value);auto f=table.get(quantity.symbol.raw());auto before=pack(std::make_tuple(bank,f));
    check(quantity.amount<=f.sales && quantity.amount<=asset::max_amount-f.drops,"MALL_FUNDS: insufficient sales/overflow");
    f.sales-=quantity.amount;f.drops+=quantity.amount;
    log(op_id,actor,"allocfund"_n,reason,before,pack(std::make_tuple(bank,f)));
    table.modify(table.get(quantity.symbol.raw()),same_payer,[&](auto& r){r=f;});
}
void mall_contract::airdrop(name submitter, uint64_t batch_id, name bank, vector<payment> entries) {
    auto c=config();require_perm(submitter, "mallAirdrop");check(!c.drop_paused,"MALL_PAUSED: airdrops");bank_ok(bank,false);
    check(batch_id>0 && !entries.empty() && entries.size()<=50,"MALL_BATCH: invalid size/id");
    batches table(get_self(),get_self().value);check(table.find(batch_id)==table.end(),"MALL_BATCH: duplicate");
    asset total(0,entries.front().quantity.symbol);
    for(const auto& e:entries){
        check(e.to!=get_self() && is_account(e.to),"MALL_BATCH: invalid recipient");
        check(e.quantity.is_valid() && e.quantity.amount>0 && e.quantity.symbol==total.symbol,"MALL_BATCH: invalid amount/symbol");
        total+=e.quantity;
    }
    funds f(get_self(),bank.value);const auto& budget=f.get(total.symbol.raw());
    check(total.amount<=budget.drops,"MALL_FUNDS: insufficient airdrop budget");
    f.modify(budget,same_payer,[&](auto& r){r.drops-=total.amount;});
    auto bytes=pack(entries);
    table.emplace(get_self(),[&](auto& r){r.id=batch_id;r.bank=bank;r.total=total;r.digest=sha256(bytes.data(),bytes.size());r.at=time_point_sec(current_time_point());});
    for(const auto& e:entries) action(permission_level{get_self(),"active"_n},bank,"transfer"_n,
        std::make_tuple(get_self(),e.to,e.quantity,string("mall airdrop:")+std::to_string(batch_id))).send();
}
void mall_contract::pause(name submitter, bool buying, bool dropping, uint64_t op_id, string reason) {
    auto c=config();auto actor=require_perm(submitter, "mallPause");auto before=pack(c);c.buy_paused=buying;c.drop_paused=dropping;
    log(op_id,actor,"pause"_n,reason,before,pack(c));config_table(get_self(),get_self().value).set(c,get_self());
}
void mall_contract::notifybuy(name buyer, string request_id, uint64_t product_id, name bank,
                             uint64_t nft_id, bool mint, int64_t quantity, asset paid) { require_auth(get_self()); }
void mall_contract::ontransfer() {
    banks table(get_self(),get_self().value);auto bank=table.find(get_first_receiver().value);
    // Unknown contracts cannot affect any mall ledger. Their notifications may use arbitrary ABIs.
    if(bank==table.end())return;
    if(bank->nft){
        auto args=unpack_action_data<std::tuple<name,name,vector<nasset>,string>>();
        receivenft(std::get<0>(args),std::get<1>(args),std::get<2>(args),std::get<3>(args));
    }else{
        auto args=unpack_action_data<std::tuple<name,name,asset,string>>();
        receivepay(std::get<0>(args),std::get<1>(args),std::get<2>(args),std::get<3>(args));
    }
}
void mall_contract::receivenft(name from, name to, vector<nasset> assets, string memo) {
    if(from==get_self() || to!=get_self())return;
    check(memo=="mallstock:v1" && !assets.empty() && assets.size()<=50,"MALL_STOCK: invalid deposit");
    products table(get_self(),get_self().value);auto idx=table.get_index<"byasset"_n>();
    for(const auto& a:assets){
        check(a.is_valid() && a.amount>0,"MALL_STOCK: invalid asset");
        auto p=idx.find((uint128_t(get_first_receiver().value)<<64)|a.symbol.nid);
        check(p!=idx.end() && !p->mint && p->status!=2,"MALL_STOCK: register inventory product first");
        bank_ok(p->nft_contract,true);
    }
}
void mall_contract::receivepay(name from, name to, asset quantity, string memo) {
    if(from==get_self() || to!=get_self())return;
    require_auth(from);
    check(quantity.is_valid() && quantity.amount>0,"MALL_AMOUNT: invalid payment");
    funds f(get_self(),get_first_receiver().value);const auto& budget=f.get(quantity.symbol.raw(),"MALL_BANK: unsupported symbol/precision");
    if(memo=="mallfund:v1"){
        check(quantity.amount<=asset::max_amount-budget.drops,"MALL_FUNDS: overflow");
        f.modify(budget,same_payer,[&](auto& r){r.drops+=quantity.amount;});return;
    }
    auto c=config();check(!c.buy_paused,"MALL_PAUSED: purchases");
    auto parts=split(memo);check(parts.size()==5 && parts[0]=="mall" && parts[1]=="v1","MALL_MEMO: expected mall:v1:product:quantity:request");
    auto product_id=number(parts[2]);auto expected=number(parts[3]);request_ok(parts[4]);
    products table(get_self(),get_self().value);auto p=table.get(product_id,"MALL_NFT: product not found");
    check(p.status==1,"MALL_STATUS: not on sale");
    check(time_point_sec(current_time_point())>=p.terms.starts,"MALL_TIME: not started");
    check(p.pay_contract==get_first_receiver() && p.terms.price.symbol==quantity.symbol,"MALL_PAYMENT: wrong contract/symbol");
    check(quantity.amount%p.terms.price.amount==0,"MALL_PAYMENT: amount must be exact multiple");
    int64_t count=quantity.amount/p.terms.price.amount;
    check(count>0 && uint64_t(count)==expected,"MALL_PAYMENT: unexpected quantity");
    check(count<=p.terms.per_purchase,"MALL_LIMIT: per purchase");
    purchases bought(get_self(),from.value);auto b=bought.find(product_id);int64_t used=b==bought.end()?0:b->quantity;
    check(used<=p.terms.per_account && count<=p.terms.per_account-used,"MALL_LIMIT: per account");
    check(count<=p.quota-p.sold,"MALL_QUOTA: sold out");bank_ok(p.nft_contract,true);quota_ok(p,p.quota);
    const string code = p.product_code.has_value() ? p.product_code.value() : string{};
    if(b==bought.end())bought.emplace(get_self(),[&](auto& r){r.product_id=p.id;r.quantity=count;r.product_code=code;});
    else bought.modify(b,same_payer,[&](auto& r){r.quantity+=count;r.product_code=code;});
    table.modify(table.get(p.id),same_payer,[&](auto& r){r.sold+=count;});
    check(quantity.amount<=asset::max_amount-budget.sales,"MALL_FUNDS: overflow");
    f.modify(budget,same_payer,[&](auto& r){r.sales+=quantity.amount;});
    if(p.mint)action(permission_level{get_self(),"active"_n},p.nft_contract,"issue"_n,
                     std::make_tuple(get_self(),nasset(count,flon::nsymbol(p.nft_id)),string("mall purchase"))).send();
    sendnft(p,from,count,"mall:"+parts[4]);
    action(permission_level{get_self(),"active"_n},get_self(),"notifybuy"_n,
           std::make_tuple(from,parts[4],p.id,p.nft_contract,p.nft_id,p.mint,count,quantity)).send();
}
}
