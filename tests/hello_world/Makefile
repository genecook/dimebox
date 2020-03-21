CC = /opt/riscv32/bin/riscv32-unknown-elf-gcc

DIMEBOX = ../../build/dimebox

all:
	make clean
	make HelloWorld
	make test

HelloWorld: main.c
	${CC} ${CFLAGS} -o $@ $^

#       dimebox option                           usage
# --------------------------  -----------------------------------------------
#   -L                         load HelloWorld, format must be ELF executable
#   --reset_address 0x10090    specify address of 1st instruction to simulate
#   --uart 0x9000000           instantiate uart; memory mapped uart registers
#                              start at 0x9000000
#   --isa_test                 cause simulation to end on ecall instruction

test: HelloWorld
	${DIMEBOX} -L HelloWorld --reset_address 0x10090 --uart 0x9000000 --isa_test 1>tlog 2>&1
	grep 'Hello World!' tlog 1>/dev/null 2>&1

clean:
	rm -f HelloWorld tlog


