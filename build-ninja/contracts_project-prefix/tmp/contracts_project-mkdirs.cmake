# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

# If CMAKE_DISABLE_SOURCE_CHANGES is set to true and the source directory is an
# existing directory in our source tree, calling file(MAKE_DIRECTORY) on it
# would cause a fatal error, even though it would be a no-op.
if(NOT EXISTS "/Users/aaron/Documents/flon/cisum/cisum.contracts/contracts")
  file(MAKE_DIRECTORY "/Users/aaron/Documents/flon/cisum/cisum.contracts/contracts")
endif()
file(MAKE_DIRECTORY
  "/Users/aaron/Documents/flon/cisum/cisum.contracts/build-ninja/contracts"
  "/Users/aaron/Documents/flon/cisum/cisum.contracts/build-ninja/install"
  "/Users/aaron/Documents/flon/cisum/cisum.contracts/build-ninja/contracts_project-prefix/tmp"
  "/Users/aaron/Documents/flon/cisum/cisum.contracts/build-ninja/contracts_project-prefix/src/contracts_project-stamp"
  "/Users/aaron/Documents/flon/cisum/cisum.contracts/build-ninja/contracts_project-prefix/src"
  "/Users/aaron/Documents/flon/cisum/cisum.contracts/build-ninja/contracts_project-prefix/src/contracts_project-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/Users/aaron/Documents/flon/cisum/cisum.contracts/build-ninja/contracts_project-prefix/src/contracts_project-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/Users/aaron/Documents/flon/cisum/cisum.contracts/build-ninja/contracts_project-prefix/src/contracts_project-stamp${cfgdir}") # cfgdir has leading slash
endif()
