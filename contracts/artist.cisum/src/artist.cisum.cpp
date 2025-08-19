#include "artist.cisum.hpp"
#include <eosio/time.hpp>

namespace flon {

// ========== 管理动作 ==========

void artists::init(const name& admin, const std::vector<name>& auditors) {
  require_auth(get_self());
  CHECKC(_gstate.admin.value == 0, err::STATUS_MISMATCH, "already initialized");
  CHECKC(is_account(admin), err::ACCOUNT_INVALID, "admin not exist");
  _gstate.admin = admin;

  for (const auto& m : auditors) {
    CHECKC(is_account(m), err::ACCOUNT_INVALID, ("auditor not exist: " + m.to_string()));
    _gstate.auditors.insert(m);
  }
}

void artists::setadmin(name admin) {
  require_auth(get_self());
  CHECKC(is_account(admin), err::ACCOUNT_INVALID, "admin not exist");
  _gstate.admin = admin;
}

void artists::addauditor(name auditor) {
  CHECKC(has_auth(get_self()) || has_auth(_gstate.admin), err::DID_NOT_AUTH, "self or admin only");
  CHECKC(is_account(auditor), err::ACCOUNT_INVALID, "auditor not exist");
  _gstate.auditors.insert(auditor);
}

void artists::delauditor(name auditor) {
  CHECKC(has_auth(get_self()) || has_auth(_gstate.admin), err::DID_NOT_AUTH, "self or admin only");
  auto itr = _gstate.auditors.find(auditor);
  CHECKC(itr != _gstate.auditors.end(), err::RECORD_NO_FOUND, "auditor not found");
  _gstate.auditors.erase(auditor);
}


void artists::addartist(name account,
                        string display_name,
                        string avatar_url,
                        string banner_url,
                        string bio,
                        string country,
                        string language,
                        string links,
                        name  status  )
{
    // 只允许管理员或审核员
    bool is_admin_or_auditor = has_auth(get_self()) || has_auth(_gstate.admin);
    if (!is_admin_or_auditor) {
        for (const auto& m : _gstate.auditors) {
            if (has_auth(m)) { is_admin_or_auditor = true; break; }
        }
    }
    CHECKC(is_admin_or_auditor, err::DID_NOT_AUTH, "auditor/admin required to add");

    CHECKC(is_account(account), err::ACCOUNT_INVALID, "artist account not exist");

    artist_t::idx_t atbl(get_self(), get_self().value);
    auto itr = atbl.find(account.value);
    CHECKC(itr == atbl.end(), err::REDPACK_EXIST, "artist already exists");

    // 基础长度校验
    auto check_len = [&](const string& s, size_t max, const char* msg){
        CHECKC(s.size() <= max, err::INVALID_FORMAT, msg);
    };
    check_len(display_name, 64,   "display_name too long");
    check_len(avatar_url,   256,  "avatar_url too long");
    check_len(banner_url,   256,  "banner_url too long");
    check_len(bio,          512,  "bio too long");
    check_len(country,      32,   "country too long");
    check_len(language,     32,   "language too long");
    check_len(links,        1024, "links too long");


    atbl.emplace(get_self(), [&](auto& a){
        a.account      = account;
        a.display_name = display_name;
        a.avatar_url   = avatar_url;
        a.banner_url   = banner_url;
        a.bio          = bio;
        a.country      = country;
        a.language     = language;
        a.links        = links;
        a.status       = status;
        a.verified     = false;
        a.created_at   = current_time_point();
        a.updated_at   = a.created_at;
    });
}

void artists::updateartist(name account,
                           string display_name,
                           string avatar_url,
                           string banner_url,
                           string bio,
                           string country,
                           string language,
                           string links,
                           name status,
                           name level  )
{
    // 权限：管理员/审核员 或 本人
    bool is_admin_or_auditor = has_auth(get_self()) || has_auth(_gstate.admin);
    if (!is_admin_or_auditor) {
        for (const auto& m : _gstate.auditors) {
            if (has_auth(m)) { is_admin_or_auditor = true; break; }
        }
    }
    bool self = has_auth(account);
    CHECKC(is_admin_or_auditor || self, err::DID_NOT_AUTH, "auditor/admin or self required to update");

    CHECKC(is_account(account), err::ACCOUNT_INVALID, "artist account not exist");

    artist_t::idx_t atbl(get_self(), get_self().value);
    auto itr = atbl.find(account.value);
    CHECKC(itr != atbl.end(), err::RECORD_NO_FOUND, "artist not found");

    // 基础长度校验
    auto check_len = [&](const string& s, size_t max, const char* msg){
        CHECKC(s.size() <= max, err::INVALID_FORMAT, msg);
    };
    check_len(display_name, 64,   "display_name too long");
    check_len(avatar_url,   256,  "avatar_url too long");
    check_len(banner_url,   256,  "banner_url too long");
    check_len(bio,          512,  "bio too long");
    check_len(country,      32,   "country too long");
    check_len(language,     32,   "language too long");
    check_len(links,        1024, "links too long");

    atbl.modify(itr, same_payer, [&](auto& a){
        // 公开资料字段，管理员/审核员 & 本人都可以更新
        a.display_name = display_name;
        a.avatar_url   = avatar_url;
        a.banner_url   = banner_url;
        a.bio          = bio;
        a.country      = country;
        a.language     = language;
        a.links        = links;
        a.level        = level;
        a.updated_at   = current_time_point();
    });
}

void artists::setstatus(name account, name status, bool verified) {
    bool is_admin_or_auditor = has_auth(get_self()) || has_auth(_gstate.admin);
    if (!is_admin_or_auditor) {
        for (const auto& m : _gstate.auditors) {
            if (has_auth(m)) { is_admin_or_auditor = true; break; }
        }
    }
    CHECKC(is_admin_or_auditor, err::DID_NOT_AUTH, "auditor/admin only");

    CHECKC(is_account(account), err::ACCOUNT_INVALID, "account not exist");

    artist_t::idx_t atbl(get_self(), get_self().value);
    auto itr = atbl.find(account.value);
    CHECKC(itr != atbl.end(), err::RECORD_NO_FOUND, "artist not found");

    atbl.modify(itr, same_payer, [&](auto& a){
        a.status        = status;
        a.verified      = verified;
        a.updated_at    = current_time_point();
    });
}

void artists::setmeta(name account, string key, string value) {
  // ---- 权限判定：管理员/审核员 或 本人 ----
  bool is_auditor = false;
  if (has_auth(get_self()) || has_auth(_gstate.admin)) {
    is_auditor = true;
  } else {
    for (const auto& m : _gstate.auditors) {
      if (has_auth(m)) { is_auditor = true; break; }
    }
  }
  const bool is_self = has_auth(account);
  CHECKC(is_auditor || is_self, err::DID_NOT_AUTH, "no permission");

  CHECKC(is_account(account), err::ACCOUNT_INVALID, "artist account not exist");

  artist_t::idx_t atbl(get_self(), get_self().value);
  auto itr = atbl.find(account.value);
  CHECKC(itr != atbl.end(), err::RECORD_NO_FOUND, "artist not found");

  // 本人可改字段白名单
  static const std::set<string> self_keys = {
    "display_name", "avatar_url", "banner_url", "bio", "links"
  };

  // 如果不是管理员/审核员，则只能改白名单字段
  if (!is_auditor) {
    CHECKC(self_keys.count(key) > 0, err::DID_NOT_AUTH, "field not editable by self");
  }

  auto check_len = [&](size_t n, size_t max, const char* msg){
    CHECKC(n <= max, err::INVALID_FORMAT, msg);
  };

  atbl.modify(itr, same_payer, [&](auto& a){
    if (key == "display_name") { check_len(value.size(), 64,   "display_name too long"); a.display_name = value; }
    else if (key == "avatar_url") { check_len(value.size(), 256, "avatar_url too long"); a.avatar_url = value; }
    else if (key == "banner_url") { check_len(value.size(), 256, "banner_url too long"); a.banner_url = value; }
    else if (key == "bio")        { check_len(value.size(), 512, "bio too long");        a.bio = value; }
    else if (key == "country")    { check_len(value.size(), 32,  "country too long");    a.country = value; }
    else if (key == "language")   { check_len(value.size(), 32,  "language too long");   a.language = value; }
    else if (key == "links")      { check_len(value.size(), 1024,"links too long");      a.links = value; }
    else {
      CHECKC(false, err::INVALID_FORMAT, "unsupported meta key");
    }
    a.updated_at = current_time_point();
  });
}

void artists::delartist(name account) {
  // 仅合约账号或全局管理员可删
  const bool is_admin = has_auth(get_self()) || has_auth(_gstate.admin);
  CHECKC(is_admin, err::DID_NOT_AUTH, "admin only");

  artist_t::idx_t atbl(get_self(), get_self().value);
  auto itr = atbl.find(account.value);
  CHECKC(itr != atbl.end(), err::RECORD_NO_FOUND, "artist not found");

  atbl.erase(itr);
}

} // namespace flon