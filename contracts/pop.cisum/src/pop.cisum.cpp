#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/time.hpp>
#include <eosio/transaction.hpp>
#include <eosio/crypto.hpp>
#include "pop.cisum.hpp"
#include <string>

namespace flon {

using namespace eosio;
using std::string;

void pop_cisum::mine(   name        payer,
                        asset       pay_amount,
                        string      memo
){
    require_auth(get_self());
    CHECKC(is_account(payer), err::ACCOUNT_INVALID, "payer account not exists");
    CHECKC(pay_amount.symbol == USDT_SYMBOL, err::INVALID_FORMAT, "pay_amount symbol mismatch");
    CHECKC(pay_amount.amount > 0, err::INVALID_FORMAT, "pay_amount must be positive");
    CHECKC(memo.size() <= 256, err::INVALID_FORMAT, "memo has more than 256 bytes");


    CHECKC(is_account(_gstate.reward_contract), err::ACCOUNT_INVALID, "reward contract account not exists");
    CHECKC(is_account(_gstate.oracle_contract), err::ACCOUNT_INVALID, "oracle contract account not exists");
    // TODO: calc rewards by pay_amount and price
    // TODO: transfer rewards to payer
}


} /// namespace flon
