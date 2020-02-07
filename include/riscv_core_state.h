#ifndef __RISCV_CORE_STATE__

class RiscvState : public State {
public:
  RiscvState() : State() {
    for (auto i = 0; i < 32; i++) X[i].Value(0);
  };
  RiscvState(SimConfig *sim_cfg) : State(sim_cfg) {
    for (auto i = 0; i < 32; i++) X[i].Value(0);
    SetPC(sim_cfg->ResetAddress());
  };
  RiscvState(RiscvState *rhs) {
    for (auto i = 0; i < 32; i++) {
      if (X[i].Value() != rhs->X[i].Value())
        X[i] = rhs->X[i];
    }
  };
  
  unsigned long long GP(unsigned int rindex) {
    if (rindex == 0)
      return 0;
    return X[rindex].Value();
  };
  void SetGP(unsigned int rindex,unsigned long long rval) {
    if (rindex > 0)
      X[rindex].Value(rval);
  };
  unsigned long long SP() { return X[2].Value(); };
  void SetSP(unsigned long long rval) { X[2].Value(rval); };

private:
  GPRegister X[32]; // general purpose registers
};

#endif
#define __RISCV_CORE_STATE__ 1
