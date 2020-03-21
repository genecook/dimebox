#ifndef __MACHINE_TIMER__

#include <string>
#include <iostream>
#include "memmap_device.h"

class MachineTimer : public memmap_device {
 public:
  MachineTimer() : timer(0),timer_advance(1),timer_compare(0) { };
  MachineTimer(unsigned long long _timer_advance) : timer(0),timer_compare(0) { timer_advance = _timer_advance; };
  
  std::string Name() { return std::string("Machine Timer"); };
  std::string RegisterName(unsigned long long PA);
  
  void Reset() { timer = 0; timer_compare = 0; };
  void AdvanceTimer() { timer += timer_advance; };
  
  bool InterruptPending() { return (timer_compare != 0) && (timer >= timer_compare); };

  int Read(unsigned long long PA,unsigned char *buffer,int access_size);
  int Write(unsigned long long PA,unsigned char *buffer,int access_size);

  // (memory-mapped timer) register offsets from timer memory base address:

  enum { TIMER_LO=0x0, TIMER_HI=0x4, TIMER_COMPARE_LO=0x10, TIMER_COMPARE_HI=0x14, TIMER_RESERVED=-1 };
  
  int register_index(unsigned long long PA);

 private:
  int Read(unsigned long long PA,unsigned int *rval);
  int Write(unsigned long long PA,unsigned int rval);

  unsigned long long timer;
  unsigned long long timer_advance;
  unsigned long long timer_compare;  
};

#define __MACHINE_TIMER__
#endif
