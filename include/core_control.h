#ifndef __CORE_CONTROL__

class Signals {
public:
  Signals() {};
private:
};

class CoreControl {
public:
  CoreControl(Packet *_updates,State *_state,Memory *_memory,Signals *_signals)
    : updates(_updates), state(_state), memory(_memory), signals(_signals) {
    instr_size_in_bytes = 4;    // instruction size
    access_size_in_bytes = 4;   //   is usually four bytes
    big_endian = false;         // instruction usually encoded in little endian
  };
  virtual ~CoreControl() {};
  
  void Step();
protected:
  virtual void FetchInstruction();
  virtual void DecodeInstruction() {};
  virtual void FetchOperands() {};
  virtual void ExecuteInstruction() {};
  virtual void AccessMemory();
  virtual void WritebackOperands() {};
  virtual void RetireInstruction();
  
  virtual void AdvanceClock(unsigned long long _clock) { clock = _clock; };

  int instr_size_in_bytes;
  int access_size_in_bytes;
  bool big_endian;
  
private:
  Packet  *updates;
  
  State   *state;
  Memory  *memory;
  Signals *signals;
  
  unsigned long long clock;
};

#endif
#define __CORE_CONTROL__ 1
