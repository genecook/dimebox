HelloWorld example uses explicitely defined 'my_outbyte' function to
write character string to uart (and implicitely stdout). Riscv
implementation of exit from program employs ecall (return to system);
dimebox '--isa_test' cmdline option inturn causes dimebox simulation
to end on ecall.
