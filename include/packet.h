#ifndef __PACKET__

// state related to a single instruction's execution...

class Packet {
public:
  Packet() {};
  virtual ~Packet() {};
  virtual bool ForTest() { return false; };
  virtual int InstructionSize() { return 4; };
  virtual int InstrAccessSize() { return 4; };
  virtual bool InstrEndianness() { return false; }; // true for big-endian

  unsigned char mbuf[80];               // hold instruction bytes read from memory
  unsigned int Opcode;                  // instruction encoding read from memory
  unsigned int primary_opcode;          // instruction opcode bits
  char disassembly[256];                // record disassembly
  int execute_op;                       // which machine op to perform

  std::vector<MemoryAccess> mOpsMemory; // memory accesses from instruction 
private:
};


#endif
#define __PACKET__ 1
