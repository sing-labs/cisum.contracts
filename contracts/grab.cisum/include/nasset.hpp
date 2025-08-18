#pragma once
#include <cstdint>
#include <eosio/eosio.hpp>

namespace flon {

static constexpr uint32_t U1E9  = 10'0000'0000UL;

struct nsymbol {
    uint32_t id;
    uint32_t pid;

    nsymbol() {}
    nsymbol(const uint32_t& i): id(i),pid(0) {}
    nsymbol(const uint32_t& i, const uint32_t& p): id(i),pid(p) {
        eosio::check( pid < U1E9, "pid must be below 10**9" );
        eosio::check( id < U1E9, "id must be below 10**10" );
    }

    nsymbol(const uint64_t& raw) {
        eosio::check( pid < U1E9, "pid must be below 10**9" );
        eosio::check( id < U1E9, "id must be below 10**10" );

        pid = raw / U1E9;
        id  = raw - pid * U1E9;
    }

    friend bool operator==(const nsymbol& a, const nsymbol& b) {
        return( a.id == b.id && a.pid == b.pid );
    }
    // bool is_valid()const { return( id > pid ); }
    uint64_t raw()const { return( (uint64_t) pid * U1E9 + id ); }

    EOSLIB_SERIALIZE( nsymbol, (id)(pid) )
};

struct nasset {
    int64_t         amount;
    nsymbol         symbol;

    nasset() {}
    nasset(const uint32_t& id): symbol(id), amount(0) {}
    nasset(const uint32_t& id, const uint32_t& pid): symbol(id, pid), amount(0) {}
    nasset(const uint32_t& id, const uint32_t& pid, const int64_t& am): symbol(id, pid), amount(am) {}
    nasset(const int64_t& amt, const nsymbol& symb): amount(amt), symbol(symb) {}

    nasset& operator+=(const nasset& quantity) {
        eosio::check( quantity.symbol.raw() == this->symbol.raw(), "nsymbol mismatch");
        this->amount += quantity.amount; return *this;
    }
    nasset& operator-=(const nasset& quantity) {
        eosio::check( quantity.symbol.raw() == this->symbol.raw(), "nsymbol mismatch");
        this->amount -= quantity.amount; return *this;
    }

    // bool is_valid()const { return symbol.is_valid(); }
    friend bool operator==(const nasset& a, const nasset& b) {
        return( a.amount == b.amount && a.symbol == b.symbol );
    }

    EOSLIB_SERIALIZE( nasset, (amount)(symbol) )
};
}