#ifndef __DEBUG_SERVER__

#include <string>
#include <vector>

using namespace std;

//***************************************************************************************
//***************************************************************************************

#define __MAX_MSG_LEN__ 8096

class DebugServer : public RSP {
 public:
   DebugServer(int _portno, unsigned int _core);
   virtual ~DebugServer();
  
   bool Enabled() { return server_socket != NULL; };
   
   bool PreStepChecks();
   bool PostStepChecks();

   virtual unsigned long long GP(int reg_index) = 0; // value - gp register
   virtual unsigned long long SP()              = 0; //  "      stack pointer
   virtual unsigned long long FP()              = 0; //  "      frame pointer
   virtual unsigned long long PC()              = 0; //  "      PC
   virtual unsigned           CoreID()          = 0; // current core ID
   virtual unsigned long long Clock()           = 0; // current clock value

   virtual void SetGP(int rindex,unsigned long long rval) = 0;
   virtual void SetSP(unsigned long long rval) = 0;
   virtual void SetPC(unsigned long long rval) = 0;

   virtual bool ReadMemory(unsigned long long address,int size,unsigned char *mbuf) = 0;
   virtual bool WriteMemory(unsigned long long address,int size,unsigned char *mbuf) = 0;

   enum REG_TYPES { GPREG, SPREG, QREG, PCREG, CPSRREG, FPSRREG, FPCRREG, BADREG };

   virtual int RegType(unsigned int rindex) = 0;
  
 protected:
   void Init();
   void Fini();

   bool Poll();

   void AcknowledgeBreakpoint(unsigned long long PC);
   void AcknowledgeWatchpoint(unsigned long long data_address);
   std::string ReadGPRegisters();
   std::string ReadRegister();
   std::string WriteRegister();
     
   std::string Continue();
   std::string Step();
   std::string KillTarget();
   
   std::string ReadMemory();
   std::string WriteMemory();

   std::string SetBreakpoints();
   std::string ClearBreakpoints();

   static const int MAX_MSG_LEN  = __MAX_MSG_LEN__;
   static const int MAX_BUF_SIZE = __MAX_MSG_LEN__ + 1024;
   
 private:
   struct breakpoint_range {
     unsigned long long address_lo;
     unsigned long long address_hi;
   };
   
   void reset_breakpoints();

   bool core_match(unsigned int _core) { return core == _core; };

   int breakpoint_count() { return (int) instr_breakpoints.size(); };
   int watchpoint_count() { return (int) data_breakpoints.size();  };
   
   bool breakpoint_hit(unsigned long long PC);
   bool watchpoint_hit(unsigned long long &matched_address, unsigned long long start_address,
		       int access_size);

   void _add_breakpoint(vector<struct breakpoint_range> &_breakpoints,
			unsigned long long _address_lo, unsigned long long _address_hi);

   bool _breakpoint_match(vector<struct breakpoint_range> &_breakpoints,
			  unsigned long long _input_address);

   bool deferResponse() { bool rcode = defer_response; defer_response = false; return rcode; };


   void setSingleStep() { do_step = true; };
   void clearSingleStep() { do_step = false; };
   bool SingleStep() { return do_step; };
   
   // endian-ness related utilities...
   
   void le8(char *encoded_rv,unsigned long long rv);
   bool decoded_le8(const char *encoded_rv,unsigned long long &rv);
   void le4(char *encoded_rv,unsigned int rv);
   bool decoded_le4(const char *encoded_rv,unsigned int &rv);

   int portno;                 // parms input
   unsigned int core;          //   via constructor
   
   TCPServer *server_socket;   // created at init time
   
   bool verbose;               // for debug prints
   bool at_start;              // true until after 1st access
   bool listening;             // mostly true, but clear during 'continue'
   bool defer_response;        // set by step or continue
   bool do_shutdown;           // set when time to shutdown server

   bool do_step;               // set by Step method. cleared in post-step
   
   vector<struct breakpoint_range> instr_breakpoints;
   vector<struct breakpoint_range> data_breakpoints;
};

#endif
#define __DEBUG_SERVER__ 1
