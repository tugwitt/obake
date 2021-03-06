#!/usr/bin/env bash

# Echo each command
set -x

# Exit on error.
set -e

# Core deps.
sudo apt-get install build-essential cmake libgmp-dev libmpfr-dev wget libboost-dev libboost-serialization-dev libtbb-dev

# Create the build dir and cd into it.
mkdir build
cd build

# Download and install mppp and abseil.
export MPPP_WITH_QUADMATH=YES
bash ../tools/circleci_install_mppp.sh
bash ../tools/circleci_install_abseil.sh

# GCC build.
cmake ../ -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=~/.local -DCMAKE_PREFIX_PATH=~/.local -DOBAKE_BUILD_TESTS=YES -DCMAKE_CXX_FLAGS="-march=native -fsanitize=address" -DOBAKE_WITH_LIBBACKTRACE=YES -DBoost_NO_BOOST_CMAKE=ON
make -j2 VERBOSE=1
make install
# Run the tests. Enable the custom suppression file for ASAN
# in order to suppress spurious warnings from TBB code.
LSAN_OPTIONS=suppressions=/home/circleci/project/tools/lsan.supp ctest -j4 -V

set +e
set +x
