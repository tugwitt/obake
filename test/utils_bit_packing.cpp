// Copyright 2019 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the piranha library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <piranha/utils/bit_packing.hpp>

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include <random>
#include <stdexcept>
#include <tuple>
#include <type_traits>
#include <vector>

#include <piranha/config.hpp>
#include <piranha/detail/to_string.hpp>
#include <piranha/detail/tuple_for_each.hpp>
#include <piranha/type_traits.hpp>

using namespace piranha;

using int_types = std::tuple<int, unsigned, long, unsigned long, long long, unsigned long long
#if defined(PIRANHA_HAVE_GCC_INT128)
                             ,
                             __int128_t, __uint128_t
#endif
                             >;

static std::mt19937 rng;

constexpr auto ntrials = 100;

TEST_CASE("bit_packer_unpacker")
{
    detail::tuple_for_each(int_types{}, [](const auto &n) {
        using int_t = remove_cvref_t<decltype(n)>;
        using bp_t = bit_packer<int_t>;
        using bu_t = bit_unpacker<int_t>;
        using value_t = typename bp_t::value_type;

        using Catch::Matchers::Contains;

        const auto [lim_min, lim_max] = detail::limits_minmax<int_t>;
        constexpr auto nbits = static_cast<unsigned>(detail::limits_digits<value_t>);

        // Start with an empty packer.
        bp_t bp0(0);
        REQUIRE(bp0.get() == value_t(0));

        // Check that adding a value to the packer throws.
        REQUIRE_THROWS_WITH(
            bp0 << int_t{0},
            Contains("Cannot push any more values to this bit packer: the number of "
                     "values already pushed to the packer is equal to the size used for construction (0)"));
        REQUIRE_THROWS_AS(bp0 << int_t{0}, std::out_of_range);

        // Empty unpacker.
        bu_t bu0(0, 0);
        int_t out;
        REQUIRE_THROWS_WITH(bu0 >> out,
                            Contains("Cannot unpack any more values from this bit unpacker: the number of "
                                     "values already unpacked is equal to the size used for construction (0)"));
        REQUIRE_THROWS_AS(bu0 >> out, std::out_of_range);

        // Unitary packing/unpacking.
        bp_t bp1(1);
        REQUIRE_THROWS_WITH(
            bp1.get(), Contains("Cannot fetch the packed value from this bit packer: the number of "
                                "values pushed to the packer (0) is less than the size used for construction (1)"));
        REQUIRE_THROWS_AS(bp1.get(), std::out_of_range);

        // Try the limits.
        bp1 << lim_min;
        bu_t bu1(bp1.get(), 1);
        bu1 >> out;
        REQUIRE(out == lim_min);

        bp1 = bp_t(1);
        bp1 << lim_max;
        bu1 = bu_t(bp1.get(), 1);
        bu1 >> out;
        REQUIRE(out == lim_max);

        // Random testing.
#if defined(PIRANHA_HAVE_GCC_INT128)
        if constexpr (!std::is_same_v<int_t, __int128_t> && !std::is_same_v<int_t, __uint128_t>)
#endif
        {
            std::uniform_int_distribution<int_t> idist(lim_min, lim_max);
            for (auto i = 0; i < ntrials; ++i) {
                const auto tmp = idist(rng);
                bp1 = bp_t(1);
                bp1 << tmp;
                bu1 = bu_t(bp1.get(), 1);
                bu1 >> out;
                REQUIRE(tmp == out);
            }
        }

        // Random testing with variable sizes.
#if defined(PIRANHA_HAVE_GCC_INT128)
        if constexpr (!std::is_same_v<int_t, __int128_t> && !std::is_same_v<int_t, __uint128_t>)
#endif
        {
            for (auto i = 2u; i < nbits; ++i) {
                const auto [cur_min, cur_max] = [pbits = nbits / i]() {
                    if constexpr (is_signed_v<int_t>) {
                        return std::tuple{-(int_t(1) << (pbits - 1u)), (int_t(1) << (pbits - 1u)) - int_t(1)};
                    } else {
                        return std::tuple{int_t(0), (int_t(1) << pbits) - int_t(1)};
                    }
                }();
                std::uniform_int_distribution<int_t> idist(cur_min, cur_max);
                std::vector<int_t> v(i);
                for (auto k = 0; k < ntrials; ++k) {
                    bp1 = bp_t(i);
                    for (auto &n : v) {
                        n = idist(rng);
                        bp1 << n;
                    }
                    bu1 = bu_t(bp1.get(), i);
                    for (const auto &n : v) {
                        bu1 >> out;
                        REQUIRE(out == n);
                    }
                }

                // Check out of range packing.
                bp1 = bp_t(i);
                if constexpr (is_signed_v<int_t>) {
                    REQUIRE_THROWS_WITH(bp1 << (cur_min - int_t(1)),
                                        Contains("The signed value being pushed to this bit packer ("
                                                 + detail::to_string(cur_min - int_t(1))
                                                 + ") is outside the allowed range [" + detail::to_string(cur_min)
                                                 + ", " + detail::to_string(cur_max) + "]"));
                    REQUIRE_THROWS_AS(bp1 << (cur_min - int_t(1)), std::overflow_error);
                    REQUIRE_THROWS_WITH(bp1 << (cur_max + int_t(1)),
                                        Contains("The signed value being pushed to this bit packer ("
                                                 + detail::to_string(cur_max + int_t(1))
                                                 + ") is outside the allowed range [" + detail::to_string(cur_min)
                                                 + ", " + detail::to_string(cur_max) + "]"));
                    REQUIRE_THROWS_AS(bp1 << (cur_max + int_t(1)), std::overflow_error);
                } else {
                }
            }
        }

        // Error checking.
        REQUIRE_THROWS_WITH(bp_t(nbits + 1u),
                            Contains("The number of values to be pushed to this bit packer ("
                                     + detail::to_string(nbits + 1u) + ") is larger than the bit width ("
                                     + detail::to_string(nbits) + ") of the value type of the packer"));
        REQUIRE_THROWS_AS(bp_t(nbits + 1u), std::overflow_error);

        bp1 = bp_t(3);
        bp1 << int_t(0) << int_t(0) << int_t(0);
        REQUIRE_THROWS_WITH(
            bp1 << int_t(0),
            Contains("Cannot push any more values to this bit packer: the number of "
                     "values already pushed to the packer is equal to the size used for construction (3)"));
        REQUIRE_THROWS_AS(bp1 << int_t(0), std::out_of_range);
    });
}
