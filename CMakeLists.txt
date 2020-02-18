cmake_minimum_required(VERSION 3.13)

project(dimebox VERSION 0.1)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_C_FLAGS "-g")
set(CMAKE_CXX_FLAGS "-g")

add_executable(dimebox src/main.C src/program_options.C)

add_library(riscv src/memory.C src/physical_memory.C src/elfio.C src/libelf_interface.c
               src/rsp.C src/tcp_server.C src/debug_server.C src/uart.C src/uart_pl011.C
	       src/rv32i_instrs.C src/riscv_instruction.C src/sim_config.C src/riscv_simulator.C
	       include/rv32i_instrs.h)

add_custom_command(OUTPUT include/rv32i_instrs.h
  COMMAND ${CMAKE_SOURCE_DIR}/utils/make_rv32i_iclasses.pl > include/rv32i_instrs.h)

include_directories(include build/include tools/boost_1_72_0 tools/elftoolchain-0.7.1/libelf
	       tools/elftoolchain-0.7.1/common )

add_library(libboost_program_options.a STATIC IMPORTED)

find_library(BOOST_PGM_OPTS libboost_program_options.a PATHS
             ${CMAKE_SOURCE_DIR}/tools/boost_1_72_0/stage/lib)

add_library(libelf.a STATIC IMPORTED)

find_library(LIBELF libelf.a PATHS ${CMAKE_SOURCE_DIR}/tools/elftoolchain-0.7.1/libelf)

target_link_libraries(dimebox riscv "${BOOST_PGM_OPTS}" "${LIBELF}")

configure_file(include/dimeboxVersion.h.in include/dimeboxVersion.h)

install(TARGETS dimebox DESTINATION ${CMAKE_SOURCE_DIR}/bin)
install(TARGETS riscv DESTINATION ${CMAKE_SOURCE_DIR}/lib)

