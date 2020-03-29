#include <stdio.h>

int uart_status();
void show_uart_status(int us);

int main(int argc,char **argv) {
  puts("\nHello World!\n");

  int working = 1;
  
  while(working) {
    
    fputs(">>>",stdout);
    
    int ic = 0;
   
    // loop 'til we have input...
    
    while(1) {
      ic = getchar();
      if (ic > 0)
	break;
    }

    printf("ic: '%c' 0x%x\n",ic,ic);
    show_uart_status(uart_status());
    
    if (ic == '!')
      working = 0;
  }
  
  return 0;
}

int uart_status() {
  register int FR asm("t0");
  asm("li a1, 0x9000000");
  asm("lw t0, 0x18(a1)");
  return FR;
}

void show_uart_status(int us) {
  int RI  = (us>>8) & 1;
  int TXE = (us>>7) & 1;
  int RXF = (us>>6) & 1;
  
  int TXF = (us>>5) & 1;
  int RXE = (us>>4) & 1;
  
  int UB  = (us>>3) & 1;
  int DCD = (us>>2) & 1;
  int DSR = (us>>1) & 1;
  int CTS = (us>>0) & 1;

  printf("[UART STATUS] US: 0x%03x (Tx empty/full: %d/%d Rcv empty/full: %d/%d busy? %d RI/DCD/DSR/CTS: %d/%d/%d/%d \n",
         us,TXE,TXF,RXE,RXF,UB,RI,DCD,DSR,CTS);
}

