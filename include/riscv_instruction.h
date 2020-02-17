#ifndef __RISCV_INSTRUCTION__

#include <iostream>

//*******************************************************************************
// common to all riscv instructions...
//*******************************************************************************

#define OPCODE opcode = encoding & 0x3f

class RiscvInstruction {
public:
  RiscvInstruction(RiscvState *_state,Memory *_memory,Signals *_signals,unsigned int _encoding)
    : encoding(_encoding), state(_state), memory(_memory), signals(_signals),
     load_store(false),jal(false),uaui(false),unsigned_sign_extension(false),shift(false),
     csrs(false), csrr(false), csri(false), trap(false) {
   instr_pc = state->PC();
   OPCODE;
 };
  virtual ~RiscvInstruction() {};
 
  const int Size = 4;            // instruction size in bytes
  const int AccessSize = 4;      // memory access size
  const bool Endianness = false; // true for big-endian
  
  virtual void Decode() = 0;   // decode instruction encoding
  virtual void Step() = 0;     // execute the instruction
  
  // after instruction executes, update simulator state:
  virtual void Writeback(RiscvState *_state,Memory *_memory,Signals *_signals,bool show_updates);

  virtual std::string Disassembly() = 0;
  virtual std::string InstrName() = 0;
  
  unsigned int RS2() { return state->GP(rs2); };
  unsigned int RS1() { return state->GP(rs1); };
  
  unsigned int RD() { return state->GP(rd); };
  void RD(unsigned int rval) { state->SetGP(rd,rval); };
  
  unsigned int PC() { return state->PC(); };
  void PC(unsigned int rval) { state->SetPC(rval); };

  unsigned int MEPC() { return state->MEPC(); };

  void BumpPC() { state->SetPC(PC() + Size); };

  unsigned long long CSR(int csr) { return state->CSR(csr); };
  void SetCSR(int csr,unsigned long long rval) { state->SetCSR(csr,rval); };
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

  // define a few register aliases to use with 'test harness' accesses...
  unsigned int GP() { return state->GP(3);  };
  unsigned int A7() { return state->GP(17); };
  unsigned int A0() { return state->GP(10); };

  unsigned long long MEMORY_READ(unsigned long long address,int size);
  void MEMORY_WRITE(unsigned long long address,int size,unsigned long long rval);
  
protected:
  unsigned char mbuf[80];               // hold bytes read-from/written-to memory
  unsigned int encoding;                // instruction encoding read from memory
  unsigned int opcode;                  //     "       main opcode
  char disassembly[256];                // record disassembly
  RiscvState *state;                    // register state
  Memory *memory;                       // memory
  Signals signals;                      // exceptions/interrupt flags
  std::vector<MemoryAccess> mOpsMemory; // memory accesses from execution of instruction
  unsigned int funct7;                  //
  unsigned int rs2;                     // instruction encoding fields
  unsigned int rs1;                     //   common to all instruction encodings
  unsigned int funct3;                  //
  unsigned int rd;                      //
  unsigned int imm;                     //

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
// one sub-class per riscv instruction encoding type...
//*******************************************************************************

#define _FUNCT7 funct7 = (encoding >> 25) & 0x3f
#define _RS2 rs2 = (encoding >> 20) & 0x1f
#define _RS1 rs1 = (encoding >> 15) & 0x1f
#define _FUNCT3 funct3 = (encoding >> 12) & 0x7
#define _RD rd = (encoding >> 7) & 0x1f

class RtypeInstruction : public RiscvInstruction {
 public:
  RtypeInstruction(RiscvState *_state,Memory *_memory,Signals *_signals,unsigned int _encoding)
    : RiscvInstruction(_state,_memory,_signals,_encoding) { Decode(); };
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
  ItypeInstruction(RiscvState *_state,Memory *_memory,Signals *_signals,unsigned int _encoding)
    : RiscvInstruction(_state,_memory,_signals,_encoding) { Decode(); };
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
  StypeInstruction(RiscvState *_state,Memory *_memory,Signals *_signals,unsigned int _encoding)
    : RiscvInstruction(_state,_memory,_signals,_encoding) { Decode(); };
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
  BtypeInstruction(RiscvState *_state,Memory *_memory,Signals *_signals,unsigned int _encoding)
    : RiscvInstruction(_state,_memory,_signals,_encoding) { Decode(); };
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
  UtypeInstruction(RiscvState *_state,Memory *_memory,Signals *_signals,unsigned int _encoding)
    : RiscvInstruction(_state,_memory,_signals,_encoding) { Decode(); };
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
  JtypeInstruction(RiscvState *_state,Memory *_memory,Signals *_signals,unsigned int _encoding)
    : RiscvInstruction(_state,_memory,_signals,_encoding) { Decode(); };
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
// riscv instruction factory...
//*******************************************************************************

class RiscvInstructionFactory {
 public:
  RiscvInstructionFactory() {};
  ~RiscvInstructionFactory() {};

  static RiscvInstruction * NewInstruction(RiscvState *_state,Memory *_memory,
					   Signals *_signals,unsigned int _encoding);
};

#endif
#define __RISCV_INSTRUCTION__ 1
