#ifndef __INSTRUCTION__

#include <vector>

// state related to a single instruction's execution...

class Instruction {
public:
  Instruction(unsigned int _opcode, unsigned int _opcode_mask)
    : opcode(_opcode), opcode_mask(_opcode_mask) { Decode(); };
  virtual ~Instruction() {};
  
  virtual bool ForTest() { return false; };
  virtual int  Size() { return 4; };           // instruction size in bytes
  virtual int  AccessSize() { return 4; };     // memory access size
  virtual bool Endianness() { return false; }; // true for big-endian
  virtual void Decode() {};  // decode instruction opcode to yield operand indices
  virtual void Step(const State *_state,const Memory *_memory) {}; // execute the instruction
  // after instruction is stepped, update simulator state:
  virtual void Writeback(State *_state,Memory *_memory,Signals *_signals);
protected:
  unsigned char mbuf[80];               // hold instruction bytes read from memory
  unsigned int opcode;                  // instruction encoding read from memory
  unsigned int opcode_mask;             //     "           "    mask
  unsigned int primary_opcode;          // instruction opcode bits
  char disassembly[256];                // record disassembly
  int execute_op;                       // which machine op to perform
  std::vector<MemoryAccess> mOpsMemory; // memory accesses from execution of instruction
  Signals signals;                      // exceptions/interrupt flags
private:
};

#endif
#define __INSTRUCTION__ 1
