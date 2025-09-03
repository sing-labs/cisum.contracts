#include <eosio/asset.hpp>
#include <eosio/eosio.hpp>

#include <string>
#include "cisumsummary.hpp"
#include "cisumsummary.db.hpp"
#include <cstdint>

using std::string;
using namespace eosio;
using namespace flon;


asset cisum_summary::get_balance(const name& bank, const symbol& symb, const name& account) {
   tbl_accounts tmp(bank, account.value);
   auto itr = tmp.find(symb.code().raw());

   if (itr != tmp.end())
      return itr->balance;
   else
      return asset(0, symb);
}


std::string cisum_summary::format_amount(int64_t amount, uint8_t precision) {
    bool neg = amount < 0;
    if (neg) amount = -amount;
    std::string s = std::to_string(amount);
    if (precision > 0) {
        if (s.size() <= precision) s.insert(0, precision - s.size() + 1, '0');
        s.insert(s.size() - precision, ".");
    }
    if (neg) s.insert(s.begin(), '-');
    return s;
}

CisumSummary cisum_summary::view(const name& account) {
    std::vector<extended_asset> result;

    std::vector<std::pair<name, symbol>> tokenlist = {
        { CISUM_BANK,   CISUM },
        { NESTAR_BANK,  NESTAR },
        { CISUM_BANK,   MUSIC  }
    };

    for (const auto& item : tokenlist) {
        auto bank = item.first;
        auto symb = item.second;

        asset bal = get_balance( bank, symb, account);
        if (bal.amount <= 0) continue;

        result.emplace_back( extended_asset( bal, bank ) );
    }

    return CisumSummary(result);
}


