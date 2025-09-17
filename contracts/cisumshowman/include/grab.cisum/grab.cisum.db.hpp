#pragma once

#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/time.hpp>
#include <eosio/singleton.hpp>
#include <string>
#include <set>
#include <flon/nasset.hpp>

using std::set;
using std::string;
using namespace eosio;

namespace flon {

struct  glab_global_t {
   uint64_t       last_rush_sale_id;

   EOSLIB_SERIALIZE(glab_global_t, (last_rush_sale_id))
};

typedef eosio::singleton< "grabglobal"_n, glab_global_t > global1_singleton;



} // namespace flon