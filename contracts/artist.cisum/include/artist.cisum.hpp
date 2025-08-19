#pragma once
#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/action.hpp>
#include <eosio/time.hpp>
#include <wasm_db.hpp>
#include <string>
#include "artist.cisum.db.hpp"


namespace flon
{

using namespace eosio;
using std::string;
using namespace wasm::db;

#define CHECKC(exp, code, msg) \
   { if (!(exp)) eosio::check(false, string("[[") + std::to_string((int)code) + string("]] ") + msg); }

enum class err: uint8_t {
   INVALID_FORMAT       = 0,
   TYPE_INVALID         = 1,
   FEE_NOT_FOUND        = 2,
   INSUFFICIENT_QUANTITY  = 3,
   NOT_POSITIVE         = 4,
   SYMBOL_MISMATCH      = 5,
   EXPIRED              = 6,
   PWHASH_INVALID       = 7,
   RECORD_NO_FOUND      = 8,
   NOT_REPEAT_RECEIVE   = 9,
   NOT_EXPIRED          = 10,
   ACCOUNT_INVALID      = 11,
   FEE_NOT_POSITIVE     = 12,
   VAILD_TIME_INVALID   = 13,
   MIN_UNIT_INVALID     = 14,
   REDPACK_EXIST       = 15,
   DID_NOT_AUTH         = 16,
   UNDER_MAINTENANCE    = 17,
   NONE_DELETED         = 19,
   IN_THE_WHITELIST     = 20,
   NON_RENEWAL          = 21,
   AMOUNT_TOO_SMALL     = 22,
   AMOUNT_TOO_LARGE     = 23,
   FEE_NOT_REQUIRED     = 24,
   DID_NOT_SUPPORTED    = 25,
   DID_PACK_SYMBOL_ERR  = 26,
   STATUS_MISMATCH       = 27
};



class [[eosio::contract("artist.cisum")]] artists : public contract {
  private:
    dbc                 _dbc;
    global_singleton    _global;
    global_t            _gstate;

  public:
    using contract::contract;

    artists(eosio::name receiver, eosio::name code, datastream<const char*> ds): contract(receiver, code, ds),
          _dbc(get_self()),
          _global(get_self(), get_self().value)
      {
          _gstate = _global.exists() ? _global.get() : global_t{};
      }
      ~artists() { _global.set( _gstate, get_self() ); }

    // ===== 管理动作 =====
    [[eosio::action]] void init(const name& admin, const std::vector<name>& auditors);                   
    [[eosio::action]] void setadmin(name admin);                    // 变更管理员（admin 或合约）
    [[eosio::action]] void addauditor(name auditor);                // 添加审核员
    [[eosio::action]] void delauditor(name auditor);                // 移除审核员

    // ===== 艺人主数据维护 =====
    [[eosio::action]]
    void addartist(name account,
                  string display_name,
                  string avatar_url,
                  string banner_url,
                  string bio,
                  string country,
                  string language,
                  string links,
                  name  status  );

    [[eosio::action]]
    void updateartist(name account,
                      string display_name,
                      string avatar_url,
                      string banner_url,
                      string bio,
                      string country,
                      string language,
                      string links,
                      name status,
                      name level); 


    [[eosio::action]] void setstatus(name account, name status, bool verified);   

    [[eosio::action]] void setmeta(name account, string key, string value);       // 细粒度修改（如 avatar_url 等）
    [[eosio::action]] void delartist(name account);                               // 彻底删除（仅管理员）


  };

}