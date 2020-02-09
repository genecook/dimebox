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
     load_store(false),jal(false),uaui(false) { OPCODE; };
  virtual ~RiscvInstruction() {};
 
  const int Size = 4;            // instruction size in bytes
  const int AccessSize = 4;      // memory access size
  const bool Endianness = false; // true for big-endian
  
  virtual void Decode() = 0;   // decode instruction encoding
  virtual void Step() = 0;     // execute the instruction
  
  // after instruction executes, update simulator state:
  virtual void Writeback(RiscvState *_state,Memory *_memory,Signals *_signals);

  virtual std::string Disassembly() = 0;
  virtual std::string InstrName() = 0;
  
  unsigned int RS2() { return state->GP(rs2); };
  unsigned int RS1() { return state->GP(rs1); };
  
  unsigned int RD() { return state->GP(rd); };
  void RD(unsigned int rval) { state->SetGP(rd,rval); };
  
  unsigned int PC() { return state->PC(); };
  void PC(unsigned int rval) { state->SetPC(rval); };

  void BumpPC() { state->SetPC(PC() + Size); };

  unsigned int FUNCT7() { return funct7; };
  unsigned int IMM() { return imm; };

  long long SIGNED_IMM(int sign_bit) { long long tl = SignExtend(imm,sign_bit); imm = tl; return tl; };
  
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

  const char *RegAlias(int reg_index, bool frame_ptr=false) {
    if ( (reg_index == 8) && frame_ptr) return "fp";
    const char *reg_aliases[] = {
     "0","ra","sp","gp","tp","t0","t1","t2","s0","s1",
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

  bool load_store;                      // used
  bool jal;                             //  in disassembly
  bool uaui;                            //
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
    sprintf(tbuf,"%s %s,%s,%s",InstrName().c_str(),RegAlias(rd),RegAlias(rs1),RegAlias(rs2));
    return std::string(tbuf);
  };
};

#define _ITYPE_IMM imm = (encoding >> 20) & 0xfff

class ItypeInstruction : public RiscvInstruction {
 public:
  ItypeInstruction(RiscvState *_state,Memory *_memory,Signals *_signals,unsigned int _encoding)
    : RiscvInstruction(_state,_memory,_signals,_encoding) { Decode(); };
  ~ItypeInstruction() {};
  virtual void Decode() { _ITYPE_IMM; _RS1; _FUNCT3; _RD; };
  virtual std::string Disassembly() {
    char tbuf[1024];
    if (load_store)
      sprintf(tbuf,"%s %s,%d(%s)",InstrName().c_str(),RegAlias(rd),(int) imm,RegAlias(rs1));
    else
      sprintf(tbuf,"%s %s,%s,%d",
	      InstrName().c_str(),RegAlias(rd),RegAlias(rs1),(int) imm);
    return std::string(tbuf);
  };
};

#define _STYPE_IMM imm = (((encoding >> 25) & 0x3f) << 5) | ((encoding >> 7) & 0x1f)

class StypeInstruction : public RiscvInstruction {
 public:
  StypeInstruction(RiscvState *_state,Memory *_memory,Signals *_signals,unsigned int _encoding)
    : RiscvInstruction(_state,_memory,_signals,_encoding) { Decode(); };
  ~StypeInstruction() {};  
  virtual void Decode() { _STYPE_IMM; _RS2; _RS1; _FUNCT3; };
  virtual std::string Disassembly() {
    char tbuf[1024];
    sprintf(tbuf,"%s %s,%s,0x%x",InstrName().c_str(),RegAlias(rs2),RegAlias(rs1),imm);
    return std::string(tbuf);
  };
};

#define _BTYPE_IMM imm = (((encoding >> 31) & 1) << 12) | (((encoding >> 25) & 0x3f) << 5) | (((encoding >> 8) & 0xf) << 1) | ((encoding >> 7) & 1)

class BtypeInstruction : public RiscvInstruction {
 public:
  BtypeInstruction(RiscvState *_state,Memory *_memory,Signals *_signals,unsigned int _encoding)
    : RiscvInstruction(_state,_memory,_signals,_encoding) { Decode(); };
  ~BtypeInstruction() {};
  virtual void Decode() { _BTYPE_IMM; _RS2; _RS1; _FUNCT3; };
  virtual std::string Disassembly() {
    char tbuf[1024];
    sprintf(tbuf,"%s %s,%s,0x%x",InstrName().c_str(),RegAlias(rs2),RegAlias(rs1),imm);
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
      sprintf(tbuf,"%s %s,0x%x",InstrName().c_str(),RegAlias(rd),imm>>12);
    else
      sprintf(tbuf,"%s %s,0x%x",InstrName().c_str(),RegAlias(rd),imm);
    return std::string(tbuf);
  };
};

#define _JTYPE_IMM imm = (((encoding>>31) & 1) << 20) | (((encoding>>11) & 0x3ff) << 1) | (((encoding>>10) & 1) << 11) | (((encoding>>12) & 0xff) << 12)

class JtypeInstruction : public RiscvInstruction {
 public:
  JtypeInstruction(RiscvState *_state,Memory *_memory,Signals *_signals,unsigned int _encoding)
    : RiscvInstruction(_state,_memory,_signals,_encoding) { Decode(); };
  ~JtypeInstruction() {};  
  virtual void Decode() { _JTYPE_IMM; _RD; };
  virtual std::string Disassembly() {
    char tbuf[1024];
    if (jal)
      sprintf(tbuf,"%s %s,0x%x",InstrName().c_str(),RegAlias(rd),imm * 2);
    else
      sprintf(tbuf,"%s %s,0x%x",InstrName().c_str(),RegAlias(rd),imm);
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
