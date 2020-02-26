#ifndef __RISCV_CORE_STATE__

#include <stdlib.h>
#include <vector>

class RiscvState : public State {
public:
  RiscvState() : State() {
    for (auto i = 0; i < 32; i++) X[i].Value(0);
    SetPC(0);
  };
  
  RiscvState(SimConfig *sim_cfg) : State(sim_cfg) {
    for (auto i = 0; i < 32; i++) X[i].Value(0);
    SetPC(sim_cfg->ResetAddress());
  };
  
  RiscvState(RiscvState *rhs,bool _show_updates = false) : State(rhs) {
    show_updates = _show_updates;
    
    SetMEPC(rhs->MEPC());
    SetMTVEC(rhs->MTVEC());
    SetSATP(rhs->SATP());
    SetPMPADDR0(rhs->PMPADDR0());
    SetPMPCFG0(rhs->PMPCFG0());
    SetMEDELEG(rhs->MEDELEG());
    SetMIDELEG(rhs->MIDELEG());
    SetMIE(rhs->MIE());
    SetMSTATUS(rhs->MSTATUS());
    
    for (auto i = 0; i < 32; i++) {
       X[i].Value(rhs->X[i].Value());
    }
  };
  
  void Update(RiscvState *rhs) {
    State::Update((State *) rhs,show_updates);
    
    if (MEPC() != rhs->MEPC()) { SetMEPC(rhs->MEPC());   }
    if (MTVEC() != rhs->MTVEC()) { SetMTVEC(rhs->MTVEC()); }
    if (SATP() != rhs->SATP()) { SetSATP(rhs->SATP()); }
    if (PMPADDR0() != rhs->PMPADDR0()) { SetPMPADDR0(rhs->PMPADDR0()); }
    if (PMPCFG0() != rhs->PMPCFG0()) { SetPMPCFG0(rhs->PMPCFG0()); }
    if (MEDELEG() != rhs->MEDELEG()) { SetMEDELEG(rhs->MEDELEG()); }
    if (MIDELEG() != rhs->MIDELEG()) { SetMIDELEG(rhs->MIDELEG()); }
    if (MIE() != rhs->MIE())   { SetMIE(rhs->MIE()); }
    if (MSTATUS() != rhs->MSTATUS())   { SetMSTATUS(rhs->MSTATUS()); }

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

  void ValidateCSRAccess(unsigned int csr_address, unsigned int privilege_level, bool for_write);
  
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

  unsigned int PrivilegeLevel() { return 3; }; // fudge to machine level for now...

  unsigned long long CSR(unsigned int csr, unsigned int privilege_level);

  void SetCSR(unsigned int csr,unsigned int privilege_level, unsigned long long rval);
  
  std::string CSR_NAME(int csr);

  unsigned long long MEPC() { return _MEPC.Value(); };
  void SetMEPC(unsigned long long rval) {
    _MEPC.Value(rval);
    if (show_updates) printf("  # mepc:\t0x%llx\n",_MEPC.Value());
  };
  unsigned long long MTVEC() { return _MTVEC.Value(); };
  void SetMTVEC(unsigned long long rval) {
    _MTVEC.Value(rval);
    if (show_updates) printf("  # mtvec:\t0x%llx\n",_MTVEC.Value());
  };
  unsigned long long SATP() { return _SATP.Value(); };
  void SetSATP(unsigned long long rval) {
    _SATP.Value(rval); 
    if (show_updates) printf("  # satp:\t0x%llx\n",_SATP.Value());
  };
  unsigned long long PMPADDR0() { return _PMPADDR0.Value(); };
  void SetPMPADDR0(unsigned long long rval) {
    _PMPADDR0.Value(rval);
  };
  unsigned long long PMPCFG0() { return _PMPCFG0.Value(); };
  void SetPMPCFG0(unsigned long long rval) {
    _PMPCFG0.Value(rval);
  };
  unsigned long long MEDELEG() { return _MEDELEG.Value(); };
  void SetMEDELEG(unsigned long long rval) {
    _MEDELEG.Value(rval);
  };
  unsigned long long MIDELEG() { return _MIDELEG.Value(); };
  void SetMIDELEG(unsigned long long rval) {
    _MIDELEG.Value(rval);
  };
  unsigned long long MIE() { return _MIE.Value(); };
  void SetMIE(unsigned long long rval) {
    _MIE.Value(rval);
  };
  unsigned long long MSTATUS() { return _MSTATUS.Value(); };
  void SetMSTATUS(unsigned long long rval) {
    _MSTATUS.Value(rval);
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

  AddressRegister _MEPC;
  AddressRegister _MTVEC;
  ControlRegister _SATP;
  AddressRegister _PMPADDR0;
  AddressRegister _PMPCFG0;
  ControlRegister _MEDELEG;
  ControlRegister _MIDELEG;
  ControlRegister _MIE;
  ControlRegister _MSTATUS;
  
  bool show_updates; // show register values
  std::vector<std::string> reg_updates;
};

#endif
#define __RISCV_CORE_STATE__ 1
