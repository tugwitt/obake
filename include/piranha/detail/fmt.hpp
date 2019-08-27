// Copyright 2019 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the piranha library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef PIRANHA_DETAIL_FMT_HPP
#define PIRANHA_DETAIL_FMT_HPP

#include <piranha/config.hpp>

#include <fmt/format.h>

#if defined(PIRANHA_HAVE_GCC_INT128)

#include <type_traits>

#include <piranha/detail/to_string.hpp>

namespace fmt
{

template <typename T>
struct formatter<
    T, ::std::enable_if_t<::std::disjunction_v<::std::is_same<T, __uint128_t>, ::std::is_same<T, __int128_t>>, char>> {
    template <typename ParseContext>
    constexpr auto parse(ParseContext &ctx)
    {
        return ctx.begin();
    }
    template <typename FormatContext>
    auto format(const T &n, FormatContext &ctx)
    {
        return ::fmt::format_to(ctx.out(), "{}", ::piranha::detail::to_string(n));
    }
};

} // namespace fmt

#endif

#endif
