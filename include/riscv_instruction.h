#ifndef __RISCV_INSTRUCTION__

#include <iostream>

//*******************************************************************************
// common to all riscv instructions...
//*******************************************************************************

#define OPCODE opcode = encoding & 0x3f

class RiscvInstruction {
public:
  RiscvInstruction(RiscvState *_state,Memory *_memory,unsigned int _encoding, int _instr_size)
    : encoding(_encoding), instr_size(_instr_size), state(_state), memory(_memory),
     load_store(false),jal(false),uaui(false),unsigned_sign_extension(false),shift(false),
     csrs(false), csrr(false), csri(false), trap(false) {
   instr_pc = state->PC();
   next_pc = instr_pc + instr_size;
   OPCODE;
 };
  virtual ~RiscvInstruction() {};
 
  virtual void Execute(bool verbose) {
    Decode();
    Display(verbose);       
    try {
       Step();
       AdvancePC();
    }  catch(SIM_EXCEPTIONS sim_exception) {
       Exception(sim_exception);      
    }
  };

  void Display(bool verbose) {
    if (verbose) {
      char tbuf[128];
      if (instr_size == 2)
        sprintf(tbuf,"0x%08x 0x%04x %s",instr_pc,encoding,Disassembly().c_str());
      else
        sprintf(tbuf,"0x%08x 0x%08x %s",instr_pc,encoding,Disassembly().c_str());
      std::cout << tbuf << std::endl;
    }
  }
  
  virtual void Decode() = 0;   // decode instruction encoding
  virtual void Step() = 0;     // execute the instruction
  
  // after instruction executes, update simulator state:
  virtual void Writeback(RiscvState *_state,Memory *_memory,bool show_updates);

  void SetPC(unsigned long long branch_pc, int alignment = 4) {
    // allow no misaligned branch target...
    // if compressed (instr) extension align to two bytes, else specified alignment...
    unsigned long long alignment_mask = state->CompressedExtensionEnabled() ? 1 : alignment - 1;
    //printf("# branch pc: 0x%08llx alignment: %d mask: 0x%08llx\n",branch_pc,alignment,alignment_mask);
    if ( (branch_pc & alignment_mask) != 0) {
      state->RecordMisalignedAddress(branch_pc);
      throw INSTRUCTION_ADDRESS_MISALIGNED;
    }
    
    next_pc = branch_pc;
  };
  
  virtual void AdvancePC() { state->SetPC(next_pc); };

  virtual std::string Disassembly() = 0;
  virtual std::string InstrName() = 0;

  void Exception(SIM_EXCEPTIONS sim_exception) {
    // handle exception, or return from exception...
    state->ProcessException(sim_exception,encoding);
  };
  
  bool UserMode() { return state->CurrentPrivilegeLevel() == state->USER_MODE; };
  bool MachineMode() { return state->CurrentPrivilegeLevel() == state->MACHINE_MODE; };
  bool PrivilegedMode() { return state->CurrentPrivilegeLevel() != state->USER_MODE; };
  bool WFIEnabled() { return state->WFIEnabled(); };
  
  unsigned int RS2() { return state->GP(rs2); };
  unsigned int RS1() { return state->GP(rs1); };
  
  unsigned int RD() { return state->GP(rd); };
  void RD(unsigned int rval) { state->SetGP(rd,rval); };

  unsigned int rvc2reg(int indx) { return indx + 8; };
  
  unsigned int PC() { return state->PC(); };
  void PC(unsigned int rval) { state->SetPC(rval); };

  unsigned int MEPC() { return state->MEPC(); };

  unsigned long long CSR(int csr) { return state->CSR(csr,state->PrivilegeLevel()); };
  void SetCSR(int csr,unsigned long long rval) { state->SetCSR(csr,state->PrivilegeLevel(),rval); };
  std::string CSR_NAME(int csr) { return state->CSR_NAME(csr); };

  unsigned int FUNCT7() { return funct7; };
  unsigned int IMM() { return imm; };

  unsigned int IMM_FOR_SHIFT() { return imm & 0x1f; };

  // normally immediate sign-extended and result used in signed expressions...
  long long int SIGN_EXTEND_IMM(int sign_bit) { imm_sign_extended = SignExtend(imm,sign_bit); return imm_sign_extended; };
  long long IMM_SIGN_EXTENDED() { return imm_sign_extended; };

  // but for some unsigned cases, immediate is sign-extended, and result used in unsigned expressions...
  unsigned long long UNSIGNED_SIGN_EXTEND_IMM(int sign_bit) { unsigned_imm_sign_extended = SignExtend(imm,sign_bit); return unsigned_imm_sign_extended; };  
  unsigned long long UNSIGNED_IMM_SIGN_EXTENDED() { return unsigned_imm_sign_extended; };
  
  long long SignExtend(unsigned long long op,int field_width,int rwidth=64) {
    int sign_bit = field_width - 1;
    bool sbit_set = ( (op>>sign_bit) & 1 ) == 1;
    if (sbit_set) {
      for (int i = sign_bit; i < rwidth; i++) {
         op = op | (1ull<<i);
      }
    }
    return (long long int) op;
  };

  // define a few register aliases...
  unsigned int GP() { return state->GP(3);  };
  unsigned int SP() { return state->GP(2);  };
  unsigned int A7() { return state->GP(17); };
  unsigned int A0() { return state->GP(10); };

  unsigned int X(int idx) { return state->GP(idx); };

  void SetSP(unsigned long long rval) { state->SetGP(2,rval); };

  unsigned long long MEMORY_READ(unsigned long long address,int size);
  void MEMORY_WRITE(unsigned long long address,int size,unsigned long long rval);
  
protected:
  unsigned char mbuf[80];               // hold bytes read-from/written-to memory
  unsigned int encoding;                // instruction encoding read from memory
  int instr_size;                       // instruction size in bytes
  unsigned int opcode;                  //     "       main opcode
  char disassembly[256];                // record disassembly
  RiscvState *state;                    // register state
  Memory *memory;                       // memory
  std::vector<MemoryAccess> mOpsMemory; // memory accesses from execution of instruction
  unsigned int next_pc;                 // usually pc + instr-size
  unsigned int funct7;                  //
  unsigned int rs2;                     // instruction encoding fields
  unsigned int rs1;                     //   common to all instruction encodings
  unsigned int funct3;                  //
  unsigned int funct4;                  //
  unsigned int rd;                      //
  unsigned int imm;                     //
  unsigned int op;                      //

  bool load_store;                                // 
  bool jal;                                       // 
  bool uaui;                                      // used
  unsigned int instr_pc;                          //   in disassembly
  long long imm_sign_extended;                    //
  unsigned long long unsigned_imm_sign_extended;  // (wonky)
  bool unsigned_sign_extension;                   //
  bool shift;                                     //
  bool csrs;                                      //
  bool csrr;                                      //
  bool csri;                                      //
  bool trap;                                      //
};

//*******************************************************************************
// unknown instruction...
//*******************************************************************************

class UnknownInstruction : public RiscvInstruction {
 public:
  UnknownInstruction(RiscvState *_state,Memory *_memory,unsigned int _encoding)
    : RiscvInstruction(_state,_memory,_encoding,4) { };
  ~UnknownInstruction() {};  
  void Decode() {};
  std::string Disassembly() {
    char tbuf[1024];
    sprintf(tbuf,"???");
    return std::string(tbuf);
  };
  void Step() { throw ILLEGAL_INSTRUCTION_UNKNOWN_INSTR; };
  std::string InstrName() { return "?"; };
};


//*******************************************************************************
// one sub-class per riscv instruction encoding type...
//*******************************************************************************

#define _FUNCT7 funct7 = (encoding >> 25) & 0x3f
#define _RS2 rs2 = (encoding >> 20) & 0x1f
#define _RS1 rs1 = (encoding >> 15) & 0x1f
#define _FUNCT3 funct3 = (encoding >> 12) & 0x7
#define _RD rd = (encoding >> 7) & 0x1f

class RtypeInstruction : public RiscvInstruction {
 public:
  RtypeInstruction(RiscvState *_state,Memory *_memory,unsigned int _encoding)
    : RiscvInstruction(_state,_memory,_encoding,4) { };
  ~RtypeInstruction() {};  
  virtual void Decode() { _FUNCT7; _RS2; _RS1; _FUNCT3; _RD; };
  virtual std::string Disassembly() {
    char tbuf[1024];
    sprintf(tbuf,"%s %s,%s,%s",InstrName().c_str(),state->RegAlias(rd),state->RegAlias(rs1),state->RegAlias(rs2));
    return std::string(tbuf);
  };
};

#define _ITYPE_IMM imm = (encoding >> 20) & 0xfff
#define _ITYPE_IMM_SHIFT imm = (encoding >> 20) & 0x1f

class ItypeInstruction : public RiscvInstruction {
 public:
  ItypeInstruction(RiscvState *_state,Memory *_memory,unsigned int _encoding)
    : RiscvInstruction(_state,_memory,_encoding,4) { };
  ~ItypeInstruction() {};
  virtual void Decode() { _ITYPE_IMM; _RS1; _FUNCT3; _RD; };
  virtual std::string Disassembly() {
    char tbuf[1024];
    if (load_store)
      sprintf(tbuf,"%s %s,%d(%s)",InstrName().c_str(),state->RegAlias(rd),(int) IMM_SIGN_EXTENDED(),state->RegAlias(rs1));
    else if (unsigned_sign_extension)
      sprintf(tbuf,"%s %s,%s,%u",InstrName().c_str(),state->RegAlias(rd),state->RegAlias(rs1),(unsigned int) UNSIGNED_IMM_SIGN_EXTENDED());
    else if (shift)
      sprintf(tbuf,"%s %s,%s,%u",InstrName().c_str(),state->RegAlias(rd),state->RegAlias(rs1),(unsigned int) IMM_FOR_SHIFT());
    else if (csrs)
      sprintf(tbuf,"%s %s,%s",InstrName().c_str(),state->RegAlias(rd),CSR_NAME(imm).c_str());
    else if (csrr)
      sprintf(tbuf,"%s %s,%s,%s",InstrName().c_str(),state->RegAlias(rd),CSR_NAME(imm).c_str(),state->RegAlias(rs1));
    else if (csri)
      sprintf(tbuf,"%s %s,%s,%d",InstrName().c_str(),state->RegAlias(rd),CSR_NAME(imm).c_str(),rs1);
    else if (trap)
      sprintf(tbuf,"%s",InstrName().c_str());      
    else
      sprintf(tbuf,"%s %s,%s,%d",InstrName().c_str(),state->RegAlias(rd),state->RegAlias(rs1),(int) IMM_SIGN_EXTENDED());
    return std::string(tbuf);
  };
};

#define _STYPE_IMM imm = (((encoding >> 25) & 0x7f) << 5) | ((encoding >> 7) & 0x1f)

class StypeInstruction : public RiscvInstruction {
 public:
  StypeInstruction(RiscvState *_state,Memory *_memory,unsigned int _encoding)
    : RiscvInstruction(_state,_memory,_encoding,4) { };
  ~StypeInstruction() {};  
  virtual void Decode() { _STYPE_IMM; _RS2; _RS1; _FUNCT3; };
  virtual std::string Disassembly() {
    char tbuf[1024];
    sprintf(tbuf,"%s %s,%d(%s)",InstrName().c_str(),state->RegAlias(rs2),(int) IMM_SIGN_EXTENDED(),state->RegAlias(rs1));
    return std::string(tbuf);
  };
};

#define _BTYPE_IMM imm = (((encoding >> 31) & 1) << 12) | (((encoding >> 25) & 0x3f) << 5) | (((encoding >> 8) & 0xf) << 1) | (((encoding >> 7) & 1) << 11)

class BtypeInstruction : public RiscvInstruction {
 public:
  BtypeInstruction(RiscvState *_state,Memory *_memory,unsigned int _encoding)
    : RiscvInstruction(_state,_memory,_encoding,4) { };
  ~BtypeInstruction() {};
  virtual void Decode() { _BTYPE_IMM; _RS2; _RS1; _FUNCT3; };
  virtual std::string Disassembly() {
    char tbuf[1024];
    sprintf(tbuf,"%s %s,%s,%d",InstrName().c_str(),state->RegAlias(rs2),state->RegAlias(rs1),(int) IMM_SIGN_EXTENDED());
    return std::string(tbuf);
  };
};

#define _UTYPE_IMM imm = encoding & 0xfffff000

class UtypeInstruction : public RiscvInstruction {
 public:
  UtypeInstruction(RiscvState *_state,Memory *_memory,unsigned int _encoding)
    : RiscvInstruction(_state,_memory,_encoding,4) { };
  ~UtypeInstruction() {};  
  virtual void Decode() { _UTYPE_IMM; _RD; };
  virtual std::string Disassembly() {
    char tbuf[1024];
    if (uaui)
      sprintf(tbuf,"%s %s,0x%x",InstrName().c_str(),state->RegAlias(rd),(int) imm);
    else
      sprintf(tbuf,"%s %s,0x%x",InstrName().c_str(),state->RegAlias(rd),imm);
    return std::string(tbuf);
  };
};

#define _JTYPE_IMM imm = ( ((encoding>>12) & 0xff) << 12) | ( ((encoding>>20) & 1) << 11) | ( ((encoding>>25) & 0x3f) << 5) | ( ((encoding>>21) & 0xf) << 1)

class JtypeInstruction : public RiscvInstruction {
 public:
  JtypeInstruction(RiscvState *_state,Memory *_memory,unsigned int _encoding)
    : RiscvInstruction(_state,_memory,_encoding,4) { };
  ~JtypeInstruction() {};  
  virtual void Decode() { _JTYPE_IMM; _RD; };
  virtual std::string Disassembly() {
    char tbuf[1024];
    if (jal)
      sprintf(tbuf,"%s %s,0x%x",InstrName().c_str(),state->RegAlias(rd),(unsigned int) (instr_pc + IMM_SIGN_EXTENDED()) );
    else
      sprintf(tbuf,"%s %s,0x%x",InstrName().c_str(),state->RegAlias(rd),imm);
    return std::string(tbuf);
  };
};


//*******************************************************************************
// Compressed instruction classes...
//*******************************************************************************

#define _FUNCT4    funct4   = (encoding & 0xf000)>>12
#define _FUNCT3_C  funct3   = (encoding >> 12) & 0x7
#define _OP        op       = (encoding & 3)
#define _CB_OFFSET imm      = (((encoding & 0x1c00)>>10)<<5) | ((encoding & 0x7c)>>2)
#define _CJ_TARGET imm      = (encoding & 0x1ffc)>>2
#define _IMM_CI    imm      = (((encoding & 0x1000)>>12)<<5) | ((encoding & 0x7c)>>2)
#define _IMM_CIW   imm      = (encoding & 0x1fe0)>>5
#define _IMM_CL    imm      = (((encoding>>10) & 0x7)<<2) | ((encoding>>5) & 3)
#define _IMM_CSS   imm      = (encoding >> 7) & 0x3f
#define _RD3       rd       = rvc2reg( (encoding>>2) & 0x3 )
#define _RD_RS1    rd = rs1 = (encoding>>7) & 0x1f
#define _RS1_3     rs1      = rvc2reg( (encoding>>7) & 0x3 )
#define _RS2_3     rs2      = rvc2reg( (encoding>>2) & 0x3 )

class CRtypeInstruction : public RiscvInstruction {
 public:
  CRtypeInstruction(RiscvState *_state,Memory *_memory,unsigned int _encoding)
    : RiscvInstruction(_state,_memory,_encoding,2) { };
  ~CRtypeInstruction() {};  
  virtual void Decode() { _FUNCT4; _RD_RS1; _RS2; _OP; };
  virtual std::string Disassembly() {
    char tbuf[1024];
    sprintf(tbuf,"%s %s,%s",InstrName().c_str(),state->RegAlias(rd),state->RegAlias(rs2));
    return std::string(tbuf);
  };
};

class CItypeInstruction : public RiscvInstruction {
 public:
  CItypeInstruction(RiscvState *_state,Memory *_memory,unsigned int _encoding)
    : RiscvInstruction(_state,_memory,_encoding,2) { };
  ~CItypeInstruction() {};  
  virtual void Decode() { _FUNCT3; _IMM_CI; _RD_RS1; _OP; };
  virtual std::string Disassembly() {
    char tbuf[1024];
    sprintf(tbuf,"%s %s,0x%x",InstrName().c_str(),state->RegAlias(rd),imm);
    return std::string(tbuf);
  };
};

class CSStypeInstruction : public RiscvInstruction {
 public:
  CSStypeInstruction(RiscvState *_state,Memory *_memory,unsigned int _encoding)
    : RiscvInstruction(_state,_memory,_encoding,2) { };
  ~CSStypeInstruction() {};  
  virtual void Decode() { _FUNCT3; _IMM_CSS; _RS2; _OP; };
  virtual std::string Disassembly() {
    char tbuf[1024];
    sprintf(tbuf,"%s %s,0x%x",InstrName().c_str(),state->RegAlias(rs2),imm);
    return std::string(tbuf);
  };
};

class CIWtypeInstruction : public RiscvInstruction {
 public:
  CIWtypeInstruction(RiscvState *_state,Memory *_memory,unsigned int _encoding)
    : RiscvInstruction(_state,_memory,_encoding,2) { };
  ~CIWtypeInstruction() {};  
  virtual void Decode() { _FUNCT3; _IMM_CIW; _RD3; _OP; };
  virtual std::string Disassembly() {
    char tbuf[1024];
    sprintf(tbuf,"%s %s,0x%x",InstrName().c_str(),state->RegAlias(rd),imm);
    return std::string(tbuf);
  };
};

class CLtypeInstruction : public RiscvInstruction {
 public:
  CLtypeInstruction(RiscvState *_state,Memory *_memory,unsigned int _encoding)
    : RiscvInstruction(_state,_memory,_encoding,2) { };
  ~CLtypeInstruction() {};  
  virtual void Decode() { _FUNCT3; _IMM_CL; _RS1_3; _RD3; _OP; };
  virtual std::string Disassembly() {
    char tbuf[1024];
    sprintf(tbuf,"%s %s,%d(%s)",InstrName().c_str(),state->RegAlias(rd),imm,state->RegAlias(rs1));
    return std::string(tbuf);
  };
};

class CStypeInstruction : public RiscvInstruction {
 public:
  CStypeInstruction(RiscvState *_state,Memory *_memory,unsigned int _encoding)
    : RiscvInstruction(_state,_memory,_encoding,2) { };
  ~CStypeInstruction() {};  
  virtual void Decode() { _FUNCT3; _IMM_CL; _RS1_3; _RS2_3; _OP; };
  virtual std::string Disassembly() {
    char tbuf[1024];
    sprintf(tbuf,"%s %s,%d(%s)",InstrName().c_str(),state->RegAlias(rd),imm,state->RegAlias(rs1));
    return std::string(tbuf);
  };
};

class CBtypeInstruction : public RiscvInstruction {
 public:
  CBtypeInstruction(RiscvState *_state,Memory *_memory,unsigned int _encoding)
    : RiscvInstruction(_state,_memory,_encoding,2) { };
  ~CBtypeInstruction() {};  
  virtual void Decode() { _FUNCT3; _CB_OFFSET; _RS1_3; _OP; };
  virtual std::string Disassembly() {
    char tbuf[1024];
    sprintf(tbuf,"%s %s,%d(%s)",InstrName().c_str(),state->RegAlias(rd),imm,state->RegAlias(rs1));
    return std::string(tbuf);
  };
};

class CJtypeInstruction : public RiscvInstruction {
 public:
  CJtypeInstruction(RiscvState *_state,Memory *_memory,unsigned int _encoding)
    : RiscvInstruction(_state,_memory,_encoding,2) { };
  ~CJtypeInstruction() {};  
  virtual void Decode() { _FUNCT3; _CJ_TARGET;  _OP; };
  virtual std::string Disassembly() {
    char tbuf[1024];
    sprintf(tbuf,"%s 0x%x",InstrName().c_str(),imm);
    return std::string(tbuf);
  };
};


//*******************************************************************************
// riscv instruction factory...
//*******************************************************************************

class RiscvInstructionFactory {
 public:
  RiscvInstructionFactory() {};
  ~RiscvInstructionFactory() {};

  static RiscvInstruction * NewInstruction(RiscvState *_state,Memory *_memory,unsigned int _encoding,bool compressed=false);
  
 private:
  static RiscvInstruction * NewInstruction16(RiscvState *_state,Memory *_memory,unsigned int _encoding);
};

#endif
#define __RISCV_INSTRUCTION__ 1
