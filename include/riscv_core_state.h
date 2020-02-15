#ifndef __RISCV_CORE_STATE__

#include <stdlib.h>

class RiscvState : public State {
public:
  RiscvState() : State() {
    for (auto i = 0; i < 32; i++) X[i].Value(0);
  };
  RiscvState(SimConfig *sim_cfg) : State(sim_cfg) {
    for (auto i = 0; i < 32; i++) X[i].Value(0);
    SetPC(sim_cfg->ResetAddress());
  };
  RiscvState(RiscvState *rhs) : State(rhs) {
    for (auto i = 0; i < 32; i++) {
      if (X[i].Value() != rhs->X[i].Value())
        X[i].Value(rhs->X[i].Value());
    }
  };
  
  void Update(RiscvState *rhs, bool show_updates) {
    State::Update((State *) rhs);
    for (auto i = 0; i < 32; i++) {
      if (X[i].Value() != rhs->X[i].Value()) {
        X[i].Value(rhs->X[i].Value());
	if (show_updates) printf("  # x[%d] = 0x%llx\n",i,X[i].Value());
      }
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

  unsigned long long FP() { return X[8].Value(); };
  void SetFP(unsigned long long rval) { X[8].Value(rval); };

  // CSR      description
  // 0xc00    RDCYCLE (cycle counter - lo)
  // 0xc01    RDTIME (timer - lo)
  // 0xc02    RDINSTRET (instrs retired counter - lo)
  // 0xc80    RDCYCLE (cycle counter -hi)
  // 0xc81    RDTIME (timer - hi)
  // 0xc82    RDINSTRET (instrs retired counter - hi)
	       
  unsigned long long CSR(int csr) {
    unsigned long long rval = 0;
    switch(csr) {
      case 0xc00: rval = Clock() & 0xffffffff; break; 
      case 0xc01: rval = TimerValue() & 0xffffffff; break;
      case 0xc02: rval = InstructionCount() & 0xffffffff; break;
      case 0xc80: rval = Clock() >> 32; break;
      case 0xc81: rval = TimerValue() >> 32; break;
      case 0xc82: rval = InstructionCount() & 0xffffffff; break;
      case 0x341: rval = MEPC(); break;  
      default: //throw EXCEPTION;
	       break;
    }
    return rval;
  };

  void SetCSR(int csr,unsigned long long rval) {
    switch(csr) {
      case 0x341: SetMEPC(rval);
	          break;  
      default:    //throw EXCEPTION;
	          break;
    }
  }
  
  std::string CSR_NAME(int csr) {
    std::string rname;
    char tbuf[16];
    switch(csr) {
      case 0xc00: rname = "rdcycle";    break; 
      case 0xc01: rname = "rdtime";     break;
      case 0xc02: rname = "rdinstret";  break;
      case 0xc80: rname = "rdcycleh";   break;
      case 0xc81: rname = "rdtimeh";    break;
      case 0xc82: rname = "rdinstreth"; break;
      case 0xf14: rname = "mhartid";    break; 
      case 0x305: rname = "mtvec";      break;  
      case 0x180: rname = "satp";       break;
      case 0x3b0: rname = "pmpaddr0";   break;
      case 0x3a0: rname = "pmpcfg0";    break;  
      case 0x302: rname = "medeleg";    break;  
      case 0x303: rname = "mideleg";    break;  
      case 0x304: rname = "mie";        break;  
      case 0x300: rname = "mstatus";    break;  
      case 0x341: rname = "mepc";       break;  
	  
      default: //throw EXCEPTION;
	       sprintf(tbuf,"<0x%03x>",csr);
	       rname = tbuf;
               break;
    }
    return rname;    
  };
  
private:
  GPRegister X[32]; // general purpose registers
};

#endif
#define __RISCV_CORE_STATE__ 1
