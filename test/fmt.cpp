// Copyright 2019 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the piranha library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <iostream>

#include <piranha/detail/fmt.hpp>

#include "catch.hpp"

using namespace piranha;

TEST_CASE("fmt_test")
{
    fmt::print("{}\n", __uint128_t(12));
    fmt::print("{}\n", -__int128_t(12));
}
