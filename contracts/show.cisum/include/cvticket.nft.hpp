#pragma once
#include <eosio/eosio.hpp>
#include <string>
#include <vector>

namespace flon {
using namespace eosio;

static constexpr uint32_t U1E9 = 10'0000'0000UL;

struct nsymbol {
    uint32_t id;
    uint32_t pid;

    nsymbol() {}
    nsymbol(const uint32_t& i): id(i),pid(0) {}
    nsymbol(const uint32_t& i, const uint32_t& p): id(i),pid(p) {
        check( pid < U1E9, "pid must be below 10**9" );
        check( id < U1E9, "id must be below 10**10" );
    }

    nsymbol(const uint64_t& raw) {
        check( pid < U1E9, "pid must be below 10**9" );
        check( id < U1E9, "id must be below 10**10" );

        pid = raw / U1E9;
        id  = raw - pid * U1E9;
    }

    friend bool operator==(const nsymbol&, const nsymbol&);
    // bool is_valid()const { return( id > pid ); }
    uint64_t raw()const { return( (uint64_t) pid * U1E9 + id ); }

    EOSLIB_SERIALIZE( nsymbol, (id)(pid) )
};

bool operator==(const nsymbol& symb1, const nsymbol& symb2) {
    return( symb1.id == symb2.id && symb1.pid == symb2.pid );
}


struct nasset {
    int64_t         amount;
    nsymbol         symbol;

    nasset() {}
    nasset(const uint32_t& id): symbol(id), amount(0) {}
    nasset(const uint32_t& id, const uint32_t& pid): symbol(id, pid), amount(0) {}
    nasset(const uint32_t& id, const uint32_t& pid, const int64_t& am): symbol(id, pid), amount(am) {}
    nasset(const int64_t& amt, const nsymbol& symb): amount(amt), symbol(symb) {}

    nasset& operator+=(const nasset& quantity) {
        check( quantity.symbol.raw() == this->symbol.raw(), "nsymbol mismatch");
        this->amount += quantity.amount; return *this;
    }
    nasset& operator-=(const nasset& quantity) {
        check( quantity.symbol.raw() == this->symbol.raw(), "nsymbol mismatch");
        this->amount -= quantity.amount; return *this;
    }

    // bool is_valid()const { return symbol.is_valid(); }

    EOSLIB_SERIALIZE( nasset, (amount)(symbol) )
};


class [[eosio::contract("cvticket.nft")]] cvticket : public contract {
public:
  using contract::contract;

  // 仅声明 transfer 动作及其 action_wrapper
  [[eosio::action]]
  void transfer(const name& from, const name& to, const std::vector<nasset>& assets, const std::string& memo);

  using transfer_action = action_wrapper<"transfer"_n, &cvticket::transfer>;
};

} // namespace flon