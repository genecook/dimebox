void my_outbyte(char xc) {
  // uart access here...
  asm("li a1, 0x9000000");
  asm("sw a0, 0(a1)");
}

int main(int argc,char **argv) {
  my_outbyte('H');
  my_outbyte('e');
  my_outbyte('l');
  my_outbyte('l');
  my_outbyte('o');
  my_outbyte(' ');
  my_outbyte('W');
  my_outbyte('o');
  my_outbyte('r');
  my_outbyte('l');
  my_outbyte('d');
  my_outbyte('!');
  my_outbyte('\n');
  
  return 0;
}
