#ifndef __RISCV_DEBUG_SERVER__

#include <string>
#include <vector>

using namespace std;

//*****************************************************************************
//*****************************************************************************

class RiscvDebugServer : public DebugServer {
 public:
   RiscvDebugServer(int portno, unsigned int core) : DebugServer(portno,core) {};
   virtual ~RiscvDebugServer() {};

   bool RunPreStepChecks(RiscvState *_cpu, Memory *_memory, unsigned long long _PC) {
     cpu = _cpu;
     memory = _memory;
     pc = _PC;
     return PreStepChecks();
   };
   bool RunPostStepChecks(RiscvState *_cpu, Memory *_memory, unsigned long long _PC) {
     cpu = _cpu;
     memory = _memory;
     pc = _PC;
     return PostStepChecks();
   };

   int RegisterSize() { return 32; };  // register size is 32 bits
   int AddressSize() { return 32; };   // memory address size is 32 bits

   std::string EncodeRegisterValue(unsigned long long rval) {
     char tbuf[16];
     le4(tbuf,rval);
     return std::string(tbuf);
   };

   bool DecodeRegisterValue(const char *encoded_rv,unsigned long long &rval) {
     unsigned int _rval = 0;
     bool rcode = decoded_le4(encoded_rv,_rval);
     rval = _rval;
     return rcode;
   };
  
   unsigned long long GP(int reg_index) { return cpu->GP(reg_index); }; // value - gp register
   unsigned long long SP()              { return cpu->SP(); };          //  "      stack pointer
   unsigned long long FP()              { return cpu->FP(); };          //  "      frame pointer
   unsigned long long PC()              { return pc; };                 //  "      PC
   unsigned           CoreID()          { return cpu->GetID(); };       // current core ID
   unsigned long long Clock()           { return cpu->Clock(); };       // current clock value

   void SetGP(int rindex,unsigned long long rval) { cpu->SetGP(rindex,rval); };
   void SetSP(unsigned long long rval) { cpu->SetSP(rval); };
   void SetPC(unsigned long long rval) { cpu->SetPC(rval); };

   bool ReadMemory(unsigned long long address,int size,unsigned char *mbuf) {
     return memory->ReadPhysicalMemory(address,size,mbuf,false);
   };
   bool WriteMemory(unsigned long long address,int size,unsigned char *mbuf) {
     memory->WritePhysicalMemory(address,size,mbuf,false);
     return true;
  };

  // RegType - convenience method used by ReadRegister/WriteRegister - return
  //           reg type from reg index...

  //   (not sure yet what the register indices are for risc-V)

  int RegType(unsigned int rindex) {
    int reg_type = BADREG;

    //                                           // range       size
    if (rindex < 31)       reg_type = GPREG;     // 0..30        64
    else if (rindex == 31) reg_type = SPREG;     // 31           64
    else if (rindex == 32) reg_type = PCREG;     // 32           64
    else if (rindex == 33) reg_type = CPSRREG;   // 33           32
    else if (rindex < 66)  reg_type = QREG;      // 34..65       128
    else if (rindex == 66) reg_type = FPSRREG;   // 66           32
    else if (rindex == 67) reg_type = FPCRREG;   // 67           32

    return reg_type;
  };

 protected:
  RiscvState *cpu;
  Memory *memory;
  unsigned long long pc;
 private:
};

#endif
#define __RISCV_DEBUG_SERVER__ 1
