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
    
    SetUSTATUS(rhs->USTATUS());
    SetFFLAGS(rhs->FFLAGS());
    SetFRM(rhs->FRM());
    SetFCSR(rhs->FCSR());
    SetSATP(rhs->SATP());
    SetMSTATUS(rhs->MSTATUS());  
    SetMEDELEG(rhs->MEDELEG());
    SetMIDELEG(rhs->MIDELEG());
    SetMIE(rhs->MIE());
    SetMTVEC(rhs->MTVEC());
    SetMCOUNTEREN(rhs->MCOUNTEREN());		
    SetMSTATUSH(rhs->MSTATUSH());
    SetMSCRATCH(rhs->MSCRATCH());
    SetMEPC(rhs->MEPC());
    SetMCAUSE(rhs->MCAUSE());
    SetMTVAL(rhs->MTVAL());
    SetMIP(rhs->MIP());
    SetMTINST(rhs->MTINST());
    SetPMPCFG0(rhs->PMPCFG0());
    SetPMPADDR0(rhs->PMPADDR0());
    SetMCYCLE(rhs->MCYCLE());
    SetMTINSTRET(rhs->MTINSTRET());
    SetMCYCLEH(rhs->MCYCLEH());
    SetMTINSTRETH(rhs->MTINSTRETH());

    for (auto i = 0; i < 32; i++) {
       X[i].Value(rhs->X[i].Value());
    }
  };
  
  void Update(RiscvState *rhs) {
    State::Update((State *) rhs,show_updates);
    
    if (USTATUS() != rhs->USTATUS()) SetUSTATUS(rhs->USTATUS());
    if (FFLAGS() != rhs->FFLAGS()) SetFFLAGS(rhs->FFLAGS());
    if (FRM() != rhs->FRM()) SetFRM(rhs->FRM());
    if (FCSR() != rhs->FCSR()) SetFCSR(rhs->FCSR());
    if (SATP() != rhs->SATP()) SetSATP(rhs->SATP());
    if (MSTATUS() != rhs->MSTATUS()) SetMSTATUS(rhs->MSTATUS());  
    if (MEDELEG() != rhs->MEDELEG()) SetMEDELEG(rhs->MEDELEG());
    if (MIDELEG() != rhs->MIDELEG()) SetMIDELEG(rhs->MIDELEG());
    if (MIE() != rhs->MIE()) SetMIE(rhs->MIE());
    if (MTVEC() != rhs->MTVEC()) SetMTVEC(rhs->MTVEC());
    if (MCOUNTEREN() != rhs->MCOUNTEREN()) SetMCOUNTEREN(rhs->MCOUNTEREN());
    if (MSTATUSH() != rhs->MSTATUSH()) SetMSTATUSH(rhs->MSTATUSH());
    if (MSCRATCH() != rhs->MSCRATCH()) SetMSCRATCH(rhs->MSCRATCH());
    if (MEPC() != rhs->MEPC()) SetMEPC(rhs->MEPC());
    if (MCAUSE() != rhs->MCAUSE()) SetMCAUSE(rhs->MCAUSE());
    if (MTVAL() != rhs->MTVAL()) SetMTVAL(rhs->MTVAL());
    if (MIP() != rhs->MIP()) SetMIP(rhs->MIP());	
    if (MTINST() != rhs->MTINST()) SetMTINST(rhs->MTINST());
    if (PMPCFG0() != rhs->PMPCFG0()) SetPMPCFG0(rhs->PMPCFG0());
    if (PMPADDR0() != rhs->PMPADDR0()) SetPMPADDR0(rhs->PMPADDR0());
    if (MCYCLE() != rhs->MCYCLE()) SetMCYCLE(rhs->MCYCLE());
    if (MTINSTRET() != rhs->MTINSTRET()) SetMTINSTRET(rhs->MTINSTRET());
    if (MCYCLEH() != rhs->MCYCLEH()) SetMCYCLEH(rhs->MCYCLEH());
    if (MTINSTRETH() != rhs->MTINSTRETH()) SetMTINSTRETH(rhs->MTINSTRETH());

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

  // misa - 32 bits
  //     31:30 - MXL - 01 (XLEN is 32 bits)
  //     bit  0 - A atomic extension
  //          3 - D double-precision floating pt
  //          5 - F single-precision     "
  //	    8 - I RV32I base ISA
  //	   12 - M Integer multiply/divide extension
  //	   20 - U user mode

  static const unsigned int MISA      = 0x40101100; // 32 bits, user mode supported, rv32i + rv32m
  static const unsigned int MVENDORID = 0;          // non-commercial so no vendor ID,
  static const unsigned int MARCHID   = 0;          //       "              machine arch. ID,
  static const unsigned int MIMPID    = 0;          //                        machine implementation

  unsigned long long USTATUS() { return _USTATUS.Value(); };
  void SetUSTATUS(unsigned long long rval) {
    _USTATUS.Value(rval);
  };
  unsigned long long FFLAGS() { return _FFLAGS.Value(); };
  void SetFFLAGS(unsigned long long rval) {
    _FFLAGS.Value(rval);
  };
  unsigned long long FRM() { return _FRM.Value(); };
  void SetFRM(unsigned long long rval) {
    _FRM.Value(rval);
  };
  unsigned long long FCSR() { return _FCSR.Value(); };
  void SetFCSR(unsigned long long rval) {
    _FCSR.Value(rval);
  };
  unsigned long long SATP() { return _SATP.Value(); };
  void SetSATP(unsigned long long rval) {
    _SATP.Value(rval); 
    if (show_updates) printf("  # satp:\t0x%llx\n",_SATP.Value());
  };
  unsigned long long MSTATUS() { return _MSTATUS.Value(); };
  void SetMSTATUS(unsigned long long rval) {
    _MSTATUS.Value(rval);
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
  unsigned long long MTVEC() { return _MTVEC.Value(); };
  void SetMTVEC(unsigned long long rval) {
    _MTVEC.Value(rval);
    if (show_updates) printf("  # mtvec:\t0x%llx\n",_MTVEC.Value());
  };
  unsigned long long MCOUNTEREN() { return _MCOUNTEREN.Value(); };
  void SetMCOUNTEREN(unsigned long long rval) {
    _MCOUNTEREN.Value(rval);
  };
  unsigned long long MSTATUSH() { return _MSTATUSH.Value(); };
  void SetMSTATUSH(unsigned long long rval) {
    _MSTATUSH.Value(rval);
  };
  unsigned long long MSCRATCH() { return _MSCRATCH.Value(); };
  void SetMSCRATCH(unsigned long long rval) {
    _MSCRATCH.Value(rval);
  };
  unsigned long long MEPC() { return _MEPC.Value(); };
  void SetMEPC(unsigned long long rval) {
    _MEPC.Value(rval);
    if (show_updates) printf("  # mepc:\t0x%llx\n",_MEPC.Value());
  };
  unsigned long long MCAUSE() { return _MCAUSE.Value(); };
  void SetMCAUSE(unsigned long long rval) {
    _MCAUSE.Value(rval);
  };
  unsigned long long MTVAL() { return _MTVAL.Value(); };
  void SetMTVAL(unsigned long long rval) {
    _MTVAL.Value(rval);
  };
  unsigned long long MIP() { return _MIP.Value(); };
  void SetMIP(unsigned long long rval) {
    _MIP.Value(rval);
  };
  unsigned long long MTINST() { return _MTINST.Value(); };
  void SetMTINST(unsigned long long rval) {
    _MTINST.Value(rval);
  };  
  unsigned long long PMPCFG0() { return _PMPCFG0.Value(); };
  void SetPMPCFG0(unsigned long long rval) {
    _PMPCFG0.Value(rval);
  };
  unsigned long long PMPADDR0() { return _PMPADDR0.Value(); };
  void SetPMPADDR0(unsigned long long rval) {
    _PMPADDR0.Value(rval);
  };
  unsigned long long MCYCLE() { return _MCYCLE.Value(); };
  void SetMCYCLE(unsigned long long rval) {
    _MCYCLE.Value(rval);
  };
  unsigned long long MTINSTRET() { return _MTINSTRET.Value(); };
  void SetMTINSTRET(unsigned long long rval) {
    _MTINSTRET.Value(rval);
  };
  unsigned long long MCYCLEH() { return _MCYCLEH.Value(); };
  void SetMCYCLEH(unsigned long long rval) {
    _MCYCLEH.Value(rval);
  };
  unsigned long long MTINSTRETH() { return _MTINSTRETH.Value(); };
  void SetMTINSTRETH(unsigned long long rval) {
    _MTINSTRETH.Value(rval);
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
  ControlRegister _MSTATUSH;
  
  ControlRegister _MCOUNTEREN;    // machine counter enable
  ControlRegister _MCOUNTINHIBIT; //    "      "     inhibit
  
  ControlRegister _MSCRATCH;
  ControlRegister _MCAUSE;
  ControlRegister _MTVAL;
  ControlRegister _MIP;
  ControlRegister _MTINST;
  
  ControlRegister _MCYCLE;     // use master clock?
  ControlRegister _MCYCLEH;    //      "
  ControlRegister _MTINSTRET;  // use # of instrs simulated?
  ControlRegister _MTINSTRETH; //       "

  ControlRegister _USTATUS;    // user status register
  ControlRegister _FFLAGS;     // floating pt flags
  ControlRegister _FRM;        //      "      rounding mode
  ControlRegister _FCSR;       //      "      control/status

  bool show_updates; // show register values
  std::vector<std::string> reg_updates;
};

#endif
#define __RISCV_CORE_STATE__ 1
