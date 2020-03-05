#ifndef __RISCV_CORE_STATE__

#include <stdlib.h>
#include <vector>

class RiscvState : public State {
public:
  RiscvState() : State() {
    for (auto i = 0; i < 32; i++) X[i].Value(0);
    SetPC(0);
    _machine_state = MACHINE_MODE; // defaults to machine mode
  };
  
  RiscvState(SimConfig *sim_cfg) : State(sim_cfg) {
    for (auto i = 0; i < 32; i++) X[i].Value(0);
    SetPC(sim_cfg->ResetAddress());
    _machine_state = MACHINE_MODE; // defaults to machine mode
  };
  
  RiscvState(RiscvState *rhs,bool _show_updates = false) : State(rhs) {
    show_updates = _show_updates;
    Update(rhs);    
  };
  
  void Update(RiscvState *rhs);

  void ShowRegisterAccess(std::string rname,int rindex,unsigned long long rval, bool update=false) {
    char tbuf[256];
    sprintf(tbuf,"  # %c %s (x%d) = 0x%llx\n",(update ? 'W' : 'R'),rname.c_str(),rindex,rval);
    reg_updates.push_back(tbuf);
  };
  
  void ShowCSRAccess(std::string csr_name,int csr_index,unsigned long long rval,bool update=false) {
    char tbuf[256];
    sprintf(tbuf,"  # %c %s (csr 0x%03x) = 0x%llx\n",(update ? 'W' : 'R'),csr_name.c_str(),csr_index,rval); 
    reg_updates.push_back(tbuf);
  };

  void ShowComment(std::string comment) { reg_updates.push_back(comment); };
    
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
    if (show_updates) ShowCSRAccess("ustatus",0,rval,true);
  };
  unsigned long long FFLAGS() { return _FFLAGS.Value(); };
  void SetFFLAGS(unsigned long long rval) {
    _FFLAGS.Value(rval);
    if (show_updates) ShowCSRAccess("fflags",1,rval,true);
  };
  unsigned long long FRM() { return _FRM.Value(); };
  void SetFRM(unsigned long long rval) {
    _FRM.Value(rval);
    if (show_updates) ShowCSRAccess("frm",2,rval,true);
  };
  unsigned long long FCSR() { return _FCSR.Value(); };
  void SetFCSR(unsigned long long rval) {
    _FCSR.Value(rval);
    if (show_updates) ShowCSRAccess("fcsr",3,rval,true);
  };
  unsigned long long SATP() { return _SATP.Value(); };
  void SetSATP(unsigned long long rval) {
    _SATP.Value(rval); 
    if (show_updates) ShowCSRAccess("satp",0x180,rval,true);
  };
  
  unsigned long long MSTATUS() {
    return _MSTATUS.Value() & 0x807fffff; // bits 23..30 = WPRI
  };
  void SetMSTATUS(unsigned long long rval) {
    rval &= rval & ~0x805de162;  // make sure these bits are clear: SIE,SPP,SPIE,MXR,SUM,UBE,TVM,TSR,FP,XS,SD
    if ( ((rval & 0xc00) == 0x400) || ((rval & 0xc00) == 0x800) ) {
      rval &= ~0xc00; // don't allow MPP to be 01 (S) or 02 (reserved)
    }
    _MSTATUS.Value(rval);
    if (show_updates) ShowCSRAccess("mstatus",0x300,rval,true);
  };
  unsigned long long MSTATUSH() {
    return _MSTATUSH.Value() & 0x30; // bits 6..31,0 = WPRI
  };
  void SetMSTATUSH(unsigned long long rval) {
    rval &= rval & ~0x30; // make sure these bits are clear: MBE,SBE
    _MSTATUSH.Value(rval);
    if (show_updates) ShowCSRAccess("mstatush",0x310,rval,true);
  };
  bool GlobalInterruptsEnabled() { return (MSTATUS() & 0x8) != 0; }; // machine mode global interrupt-enable
  bool WFIEnabled() {
    // MSTATUS.TW is WFI 'enable' when not in machine mode...
    return (CurrentPrivilegeLevel() == 3) || (MSTATUS() & 0x200000);
  };

  static const unsigned int USER_MODE=0;
  static const unsigned int SUPERVISOR_MODE=1;
  static const unsigned int HYPERVISOR_MODE=2;
  static const unsigned int MACHINE_MODE=3;

  unsigned CurrentPrivilegeLevel() { return _machine_state & 0x3; };
  void SetPrivilegeLevel(unsigned int _new_privilege_level) {
    _machine_state = (_machine_state & ~0x3) | _new_privilege_level;
  };
  bool LowPowerMode() { return (_machine_state & 0x10) != 0; };
  void ClearLowPowerMode() { _machine_state &= ~0x10; };
  void SetLowPowerMode() { _machine_state |= 0x10; };

  void PushPrivilegeLevel();
  void PopPrivilegeLevel();

  // used to signal interrupt from device such as uart: 
  void Signal(SIM_EXCEPTIONS sim_interrupt) {
    switch((int) sim_interrupt) {
      case MACHINE_TIMER_INT:    SetMIP( MIP() | 1<<7);  break; // MIP.MTIP = 1
      case MACHINE_EXTERNAL_INT: SetMIP( MIP() | 1<<11); break; // MIP.MEIP = 1
      default: break;
    }
  };

  void ProcessException(SIM_EXCEPTIONS sim_exception,unsigned int opcode=0);
  void DecodeException(char *tbuf,SIM_EXCEPTIONS sim_exception, unsigned long long pc,unsigned int opcode);

  bool InterruptPending(SIM_EXCEPTIONS &sim_interrupt);
  void ProcessInterrupt(SIM_EXCEPTIONS sim_exception) { ProcessException(sim_exception); };

  unsigned long long MEDELEG() { return _MEDELEG.Value(); };
  void SetMEDELEG(unsigned long long rval) {
    _MEDELEG.Value(rval);
    if (show_updates) ShowCSRAccess("medeleg",0x302,rval,true);
  };
  unsigned long long MIDELEG() { return _MIDELEG.Value(); };
  void SetMIDELEG(unsigned long long rval) {
    _MIDELEG.Value(rval);
    if (show_updates) ShowCSRAccess("mideleg",0x303,rval,true);
  };
  unsigned long long MIE() { return _MIE.Value(); };
  void SetMIE(unsigned long long rval) {
    _MIE.Value(rval);
    if (show_updates) ShowCSRAccess("mie",0x304,rval,true);
  };
  unsigned long long MTVEC() { return _MTVEC.Value(); };
  void SetMTVEC(unsigned long long rval) {
    _MTVEC.Value(rval);
    if (show_updates) ShowCSRAccess("mtvec",0x305,rval,true);
  };
  unsigned long long MCOUNTEREN() { return _MCOUNTEREN.Value(); };
  void SetMCOUNTEREN(unsigned long long rval) {
    _MCOUNTEREN.Value(rval);
    if (show_updates) ShowCSRAccess("mcounteren",0x306,rval,true);
  };
  unsigned long long MSCRATCH() { return _MSCRATCH.Value(); };
  void SetMSCRATCH(unsigned long long rval) {
    _MSCRATCH.Value(rval);
    if (show_updates) ShowCSRAccess("mscratch",0x340,rval,true);
  };
  unsigned long long MEPC() { return _MEPC.Value(); };
  void SetMEPC(unsigned long long rval) {
    _MEPC.Value(rval);
    if (show_updates) ShowCSRAccess("mepc",0x341,rval,true);
  };
  unsigned long long MCAUSE() { return _MCAUSE.Value(); };
  void SetMCAUSE(unsigned long long rval) {
    _MCAUSE.Value(rval);
    if (show_updates) ShowCSRAccess("mcause",0x342,rval,true);
  };
  unsigned long long MTVAL() { return _MTVAL.Value(); };
  void SetMTVAL(unsigned long long rval) {
    _MTVAL.Value(rval);
    if (show_updates) ShowCSRAccess("mtval",0x343,rval,true);
  };
  unsigned long long MIP() { return _MIP.Value(); };
  void SetMIP(unsigned long long rval) {
    _MIP.Value(rval);
    if (show_updates) ShowCSRAccess("mip",0x344,rval,true);
  };
  unsigned long long MTINST() { return _MTINST.Value(); };
  void SetMTINST(unsigned long long rval) {
    _MTINST.Value(rval);
    if (show_updates) ShowCSRAccess("mtinst",0x34a,rval,true);
  };  
  unsigned long long PMPCFG0() { return _PMPCFG0.Value(); };
  void SetPMPCFG0(unsigned long long rval) {
    _PMPCFG0.Value(rval);
    if (show_updates) ShowCSRAccess("pmpcfg0",0x3a0,rval,true);
  };
  unsigned long long PMPADDR0() { return _PMPADDR0.Value(); };
  void SetPMPADDR0(unsigned long long rval) {
    _PMPADDR0.Value(rval);
    if (show_updates) ShowCSRAccess("pmpaddr0",0x3b0,rval,true);
  };
  unsigned long long MCYCLE() { return _MCYCLE.Value(); };
  void SetMCYCLE(unsigned long long rval) {
    _MCYCLE.Value(rval);
    if (show_updates) ShowCSRAccess("mcycle",0xb00,rval,true);
  };
  unsigned long long MTINSTRET() { return _MTINSTRET.Value(); };
  void SetMTINSTRET(unsigned long long rval) {
    _MTINSTRET.Value(rval);
    if (show_updates) ShowCSRAccess("mtinstret",0xb02,rval,true);
  };
  unsigned long long MCYCLEH() { return _MCYCLEH.Value(); };
  void SetMCYCLEH(unsigned long long rval) {
    _MCYCLEH.Value(rval);
    if (show_updates) ShowCSRAccess("mcycleh",0xb80,rval,true);
  };
  unsigned long long MTINSTRETH() { return _MTINSTRETH.Value(); };
  void SetMTINSTRETH(unsigned long long rval) {
    _MTINSTRETH.Value(rval);
    if (show_updates) ShowCSRAccess("mtinstreth",0xb82,rval,true);
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
                           //   bits           used for
                           // ---------   -----------------------
  unsigned _machine_state; //    4        low power mode (set to 1 if waiting for interrupt, ie, in low power mode
                           //    1:0      current privilege mode (User=0, Super=1, Hyper=2, Machine=3
  
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
