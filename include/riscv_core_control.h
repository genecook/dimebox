#ifndef __RISCV_CORE_CONTROL__

class RiscvCoreControl : public CoreControl {
 public:
  RiscvCoreControl(RiscvPacket *_updates,State *_state,Memory *_memory,Signals *_signals)
    : updates(_updates),CoreControl(_updates,_state,_memory,_signals) {};

  void DecodeInstruction();
  void FetchOperands();
  void ExecuteInstruction();
  void WritebackOperands();
protected:
  RiscvPacket *updates; // riscv updated register state, memory accesses
};

#endif
#define __RISCV_CORE_CONTROL__ 1
