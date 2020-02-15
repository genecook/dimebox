#ifndef __RISCV_CORE_STATE__

#include <stdlib.h>
#include <vector>

class RiscvState : public State {
public:
  RiscvState() : State() {
    for (auto i = 0; i < 32; i++) X[i].Value(0);
  };
  RiscvState(SimConfig *sim_cfg) : State(sim_cfg) {
    for (auto i = 0; i < 32; i++) X[i].Value(0);
    SetPC(sim_cfg->ResetAddress());
  };
  RiscvState(RiscvState *rhs,bool _show_updates = false) : State(rhs) {
    show_updates = _show_updates;
    for (auto i = 0; i < 32; i++) {
      if (X[i].Value() != rhs->X[i].Value())
        X[i].Value(rhs->X[i].Value());
    }
  };
  
  void Update(RiscvState *rhs) {
    State::Update((State *) rhs,show_updates);
    for (auto i = 0; i < 32; i++) {
      if (X[i].Value() != rhs->X[i].Value()) {
        X[i].Value(rhs->X[i].Value());
	if (show_updates) ShowRegisterAccess(RegAlias(i),i,X[i].Value(),true);
      }
    }
  };

  void ShowRegisterAccess(std::string rname,int rindex,unsigned long long rval, bool update=false) {
    char tbuf[256];
    sprintf(tbuf,"  # %c %s (x%d) = 0x%llx\n",(update ? 'W' : 'R'),rname.c_str(),rindex,rval);
    reg_updates.push_back(tbuf);
  };
  
  void ShowCSRAccess(std::string csr_name,int csr_index,unsigned long long rval, bool update=false) {
    char tbuf[256];
    sprintf(tbuf,"  # %c %s (csr 0x%03x) = 0x%llx\n",(update ? 'W' : 'R'),csr_name.c_str(),csr_index,rval); 
    reg_updates.push_back(tbuf);
  };

  void ShowRegisterReads() { for (auto us = reg_updates.begin(); us != reg_updates.end(); us++) std::cout << *us; };
  
  unsigned long long GP(unsigned int rindex) {
    if (rindex == 0) {
      if (show_updates) ShowRegisterAccess("zero",0,0);
      return 0;
    }
    if (show_updates) ShowRegisterAccess(RegAlias(rindex),rindex,X[rindex].Value());
    return X[rindex].Value();
  };
  void SetGP(unsigned int rindex,unsigned long long rval) {
    if (rindex > 0) {
      X[rindex].Value(rval);
      if (show_updates) ShowRegisterAccess(RegAlias(rindex),rindex,rval,true);
    }
  };
  unsigned long long SP() {
    return X[2].Value();
    if (show_updates) ShowRegisterAccess(RegAlias(2),2,X[2].Value());
  };
  void SetSP(unsigned long long rval) {
    X[2].Value(rval);
    if (show_updates) ShowRegisterAccess(RegAlias(2),2,X[2].Value(),true);
  };

  unsigned long long FP() {
    if (show_updates) ShowRegisterAccess(RegAlias(8),8,X[8].Value());
    return X[8].Value();
  };
  void SetFP(unsigned long long rval) {
    X[8].Value(rval);
    if (show_updates) ShowRegisterAccess(RegAlias(8),8,X[8].Value(),true);
  };

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
    if (show_updates) ShowCSRAccess(CSR_NAME(csr),csr,rval);
    return rval;
  };

  void SetCSR(int csr,unsigned long long rval) {
    switch(csr) {
      case 0x341: SetMEPC(rval);
	          break;  
      default:    //throw EXCEPTION;
	          break;
    }
    if (show_updates) ShowCSRAccess(CSR_NAME(csr),csr,rval,true);
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
  
  // alias riscv gp register indices to common riscv assembly language names...
  const char *RegAlias(int reg_index, bool frame_ptr=false) {
    if ( (reg_index == 8) && frame_ptr) return "fp";
    const char *reg_aliases[] = {
     "x0","ra","sp","gp","tp","t0","t1","t2","s0","s1",
     "a0","a1","a2","a3","a4","a5","a6","a7",
     "s2","s3","s4","s5","s6","s7","s8","s9","s10","s11",
     "t3","t4","t5","t6",NULL
    };
    return reg_aliases[reg_index];
  }
  const char *RegAliasFloat(int reg_index) {
    const char *reg_aliases_float[] = {
       "ft0","ft1","ft2","ft3","ft4","ft5","ft6","ft7",
       "fs0","fs1",
       "fa0","fa1","fa2","fa3","fa4","fa5","fa6","fa7",
       "fs2","fs3","fs4","fs5","fs6","fs7","fs8","fs9","fs10","fs11",
       "ft8","ft3","ft4","ft5","ft6","ft7","ft8","ft9","ft10","ft11",
       NULL
    };
    return reg_aliases_float[reg_index];
  };
  
private:
  GPRegister X[32];  // general purpose registers
  bool show_updates; // show register values
  std::vector<std::string> reg_updates;
};

#endif
#define __RISCV_CORE_STATE__ 1
