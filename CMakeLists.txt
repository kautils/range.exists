cmake_minimum_required(VERSION 3.20)
set(__version 0.0.1)
project(KautilRangeExists.${__version} LANGUAGES C CXX VERSION ${__version})
unset(__version)

include(${CMAKE_CURRENT_LIST_DIR}/exists/exists.cmake)