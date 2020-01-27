cmake_minimum_required(VERSION 3.13)

project(dimebox VERSION 0.1)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable(dimebox src/main.C src/program_options.C src/sim_config.C src/memory.C
               src/physical_memory.C src/elfio.C src/libelf_interface.c
               src/rsp.C src/tcp_server.C src/debug_server.C)

include_directories(include build/include tools/boost_1_72_0 tools/elftoolchain-0.7.1/libelf tools/elftoolchain-0.7.1/common )

add_library(libboost_program_options.a STATIC IMPORTED)

find_library(BOOST_PGM_OPTS libboost_program_options.a PATHS ${CMAKE_SOURCE_DIR}/tools/boost_1_72_0/stage/lib)

add_library(libelf.a STATIC IMPORTED)

find_library(LIBELF libelf.a PATHS ${CMAKE_SOURCE_DIR}/tools/elftoolchain-0.7.1/libelf)

target_link_libraries(dimebox "${BOOST_PGM_OPTS}" "${LIBELF}" )

configure_file(include/dimeboxVersion.h.in include/dimeboxVersion.h)

install(TARGETS dimebox DESTINATION ${CMAKE_SOURCE_DIR}/bin)

