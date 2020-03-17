#!/bin/sh -f

export RISCV_AS=/opt/riscv32/bin/riscv32-unknown-elf-as

$RISCV_AS -o wfi_timer.o wfi_timer.S
