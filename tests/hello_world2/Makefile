CC = /opt/riscv32/bin/riscv32-unknown-elf-gcc

DIMEBOX = ../../build/dimebox

all:
	make clean
	make HelloWorld
	make test
	make test_echo

HelloWorld: main.c my_stubs.o
	${CC} ${CFLAGS} -o $@ $^

EchoChars: echo.c my_stubs.o
	${CC} ${CFLAGS} -o $@ $^

# our system calls...

my_stubs.o: my_stubs.c
	${CC} ${CFLAGS} -c -o $@ $^

#       dimebox option                           usage
# --------------------------  -----------------------------------------------
#   -L                         load HelloWorld, format must be ELF executable
#   --reset_address 0x10090    specify address of 1st instruction to simulate
#   --uart 0x9000000           instantiate uart; memory mapped uart registers
#                              start at 0x9000000
#   --isa_test                 cause simulation to end on ecall instruction
#   -n 2000                    specify max of 2000 instructions to simulate

test: HelloWorld
	${DIMEBOX} -L HelloWorld --reset_address 0x10090 --uart 0x9000000 --isa_test -n 2000 1>tlog 2>&1
	grep 'Hello World!' tlog 1>/dev/null 2>&1

# EchoChars - echo characters typed from stdin. Note -n option with value -1 to specify that simulation
#             has no max-instruction count...

test_echo: EchoChars
	echo 'Hello World!' | ${DIMEBOX} -L EchoChars --reset_address 0x10090 --uart 0x9000000 --isa_test -n -1 

clean:
	rm -f HelloWorld EchoChars my_stubs.o tlog


