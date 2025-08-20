#pragma once
#include <cstdint>
#include <eosio/eosio.hpp>

namespace flon {


struct nsymbol {
    uint64_t value = 0;

    // consts
    static constexpr uint32_t U1E9  = 10'0000'0000UL;
    nsymbol() = default;

    static uint64_t to_raw_value(uint32_t i, uint32_t p) {
        eosio::check( p < U1E9, "pid must be below 10**9" );
        eosio::check( i < U1E9, "id must be below 10**9" );
        return (uint64_t)p * U1E9 + i;
    }

    explicit nsymbol(uint32_t i, uint32_t p = 0): value(to_raw_value(i, p)) {}

    explicit nsymbol(uint64_t raw): value(raw) {}

    friend bool operator==(const nsymbol& a, const nsymbol& b) {
        return( a.value == b.value );
    }

    inline uint64_t raw() const {
        return value;
    }

    inline uint32_t id() const {
        return value % U1E9;
    }

    inline uint32_t pid() const {
        return value / U1E9;
    }

    EOSLIB_SERIALIZE( nsymbol, (value) )
};

struct nasset {
    int64_t         amount  = 0;
    nsymbol         symbol;

    nasset() = default;
    explicit nasset(const uint32_t& id): symbol(id), amount(0) {}
    explicit nasset(uint32_t id, uint32_t pid, int64_t amount = 0): symbol(id, pid), amount(amount) {}
    explicit nasset(const int64_t& amount, const nsymbol& symb): amount(amount), symbol(symb) {}

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