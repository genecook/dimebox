#include "timer.h"

//#define TIMER_DEBUG 1

//-------------------------------------------------------------------------------------------
// return index to valid timer register...
//-------------------------------------------------------------------------------------------
int MachineTimer::register_index(unsigned long long PA) {
  int reg_index = TIMER_RESERVED;
  
  switch( (int) (PA & 0xfff) ) {
    case 0x000: reg_index = TIMER_LO;         break;
    case 0x004: reg_index = TIMER_HI;         break;
    case 0x010: reg_index = TIMER_COMPARE_LO; break;
    case 0x014: reg_index = TIMER_COMPARE_HI; break;

    default: // either access to reserved timer address, or misaligned address...
             break;
  }

#ifdef TIMER_DEBUG
  printf("[MachineTimer::register_index] index: 0x%x\n",reg_index);
#endif
  
  return reg_index;
}

std::string MachineTimer::RegisterName(unsigned long long PA) {
  std::string rn = "?";

  switch(register_index(PA)) {
    case TIMER_LO:         rn = "MTIME";       break;
    case TIMER_HI:         rn = "MTIME_HI";    break;
    case TIMER_COMPARE_LO: rn = "MTIMECMP";    break;
    case TIMER_COMPARE_HI: rn = "MTIMECMP_HI"; break;
    default: // either access to reserved uart address, or misaligned address...
             break;
  }

  return rn;
}

//-------------------------------------------------------------------------------------------
// read from memory-mapped timer register...
//-------------------------------------------------------------------------------------------

// for now, only handle word (32 bit) aligned access...

int MachineTimer::Read(unsigned long long PA,unsigned char *buffer,int access_size) {
#ifdef TIMER_DEBUG
  printf("[MachineTimer::Read] entered, PA: 0x%llx, size: %d\n",PA,access_size);
#endif
  
  if (access_size != 4)
    return -1;

  int rcode = Read(PA,(unsigned int *) buffer);
#ifdef TIMER_DEBUG
  printf("[MachineTimer::Read] exited, rcode: 0x%x\n",rcode);
#endif

  return rcode;
}

int MachineTimer::Read(unsigned long long PA,unsigned int *rval) {
  int rcode = 0;

  *rval = 0;

  switch( register_index(PA) ) {
    case TIMER_LO:         *rval = timer & 0xffffffff;
                           break;
    case TIMER_HI:         *rval = timer >> 32;
                           break;
		   
    case TIMER_COMPARE_LO: *rval = timer & 0xffffffff;
                           break;
    case TIMER_COMPARE_HI: *rval = timer >> 32;
                           break;
	       
    default: // either access to reserved uart address, or misaligned address...
             rcode = -1;
             break;
  }
  
  return rcode;
}

//-------------------------------------------------------------------------------------------
// write to memory-mapped timer register...
//-------------------------------------------------------------------------------------------

int MachineTimer::Write(unsigned long long PA,unsigned char *buffer,int access_size) {
#ifdef TIMER_DEBUG
  printf("[MachineTimer::Write] entered, PA: 0x%llx, size: %d\n",PA,access_size);
#endif
  
  if (access_size != 4)
    return -1;

  int rcode = Write(PA,*( (unsigned int *) buffer));

#ifdef TIMER_DEBUG
  printf("[MachineTimer::Write] exited, rcode: 0x%x\n",rcode);
#endif

  return rcode;
}

int MachineTimer::Write(unsigned long long PA,unsigned int rval) {  
  int rcode = 0;

  switch( register_index(PA) ) {
    case TIMER_LO:         // write to timer 'lo' writes entire 64-bit timer value...
                           timer = rval; 
                           break;
		       
    case TIMER_HI:         // write to timer 'hi' overwrites upper 32 bits of timer...
                           timer |= (unsigned long long) rval<<32;
                           break;
		       
    case TIMER_COMPARE_LO: // write to timer 'lo' writes entire 64-bit timer value...
                           timer_compare = rval; 
#ifdef TIMER_DEBUG
                           printf("[MachineTimer::Write] timer-compare value written: 0x%llx\n",
				  timer_compare);
#endif
                           break;
		       
    case TIMER_COMPARE_HI: // write to timer 'hi' overwrites upper 32 bits of timer...
                           timer_compare |= (unsigned long long) rval<<32;
                           break;
		       
    default: // either access to reserved uart address, or misaligned address...
             rcode = -1;
             break;
  }
 
#ifdef TIMER_DEBUG
  printf("[MachineTimer::Write] exited, rcode: %d\n",rcode);
#endif

  return rcode;
}
