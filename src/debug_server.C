#include <iostream>
#include <string>
#include <stdexcept>
#include <vector>
#include <string.h>

#include <dimebox.h>

//#define DEBUG_GDB_SERVER 1

//***************************************************************************************
// constructor, destructor...
//***************************************************************************************

DebugServer::DebugServer(int _portno, unsigned int _core)
  : portno(_portno), core(_core), server_socket(NULL), verbose(false), at_start(false),
    listening(false), defer_response(false), do_shutdown(false), do_step(false) {
  Init();
}

DebugServer::~DebugServer() {
  Fini();
}

//***************************************************************************************
// initialize debug server...
//***************************************************************************************

void DebugServer::Init() {
  verbose = false;
  
  // we are a server for the gdb client...
  
  std::cout << "\n\nwaiting for client...";
   
  fflush(stdout);
  server_socket = new TCPServer(portno,true); // NOTE: blocks on read!
   
  std::cout << "connected." << std::endl;

  at_start = true;
  
  std::cout << "\n\nlistening on port " << portno << "..." << std::endl;
}

//***************************************************************************************
// shutdown the debug server...
//***************************************************************************************

void DebugServer::Fini() {
  // TCPServer shuts itself down. Thus nothing to do just yet...

#ifdef DEBUG_GDB_SERVER
  printf("[DebugServer::Fini] do_shutdown? %d\n",do_shutdown);
#endif
  
  if (do_shutdown) {
    // already processed shutdown (kill-target) request, so no need to acknowledge...
  } else {
    // notify debug client simulation has ended...
    std::string process_terminated_packet = "X00";
    std::string my_response = Encode(process_terminated_packet);
    try {
       server_socket->Write((unsigned char *) my_response.c_str(),my_response.size());
    } catch (const std::runtime_error& ex) {
       // oops! error on write to server socket???
       std::cerr << ex.what() << std::endl;
       listening = false;
    }
  }
}

//***************************************************************************************
// PreStepChecks - called before each instruction step. Return true as long as
//                 debug session lasts...
//
//  NOTE: We check for a breakpoint at the start of instruction cycle.
//        We poll user for debug commands at the start of instruction cycle.
//***************************************************************************************

bool DebugServer::PreStepChecks() {
#ifdef DEBUG_GDB_SERVER
  printf("[DebugServer::RunPreStepChecks] PC: 0x%llx\n",PC());
#endif
  
  bool do_poll = false;

  if (!Enabled()) {
    // debug not enabled so of course no need to poll...
  } else if (at_start) {
    // start of simulation, so we do need to poll...
    at_start = false;    
    do_poll = true;
  } else if (!core_match(core)) {
    // not core of interest, so do NOT poll...
  } else if (SingleStep()) {
    // clear single-stepping flag, do poll. do NOT acknowledge as instruction has yet to be executed...
    clearSingleStep();
    do_poll = true;
  } else if (breakpoint_hit(PC())) {
    // we ASSUME at least one instruction has executed when checking for breakpoint...
    // a breakpoint was reached, so we do need to poll...
    AcknowledgeBreakpoint(PC());
    do_poll = true; 
  }

#ifdef DEBUG_GDB_SERVER
  if (do_poll)
    printf("\tpolling at the start of instruction execution cycle - PC has NOT advanced...\n");
#endif
  
  bool rcode = (do_poll) ? Poll(): true;

#ifdef DEBUG_GDB_SERVER
  printf("[DebugServer::RunPreStepChecks] exited.\n");
#endif
  
  return rcode;
}

//***************************************************************************************
// PostStepChecks - called after each instruction step, to acknowledge stepped
//                  instruction or breakpoint/watchpoint. Always returns true...
//
// NOTE: we acknowledge the completion of a single-stepped instruction at the end of
//       an instruction execution.
//       we check for and acknowledge a watchpoint at the end of an instruction execution.
//***************************************************************************************

bool DebugServer::PostStepChecks() {
#ifdef DEBUG_GDB_SERVER
  printf("[DebugServer::RunPostStepChecks] PC: 0x%llx\n",PC());
#endif
  
  if (!Enabled()) {
    // debug not enabled...
  } else if (at_start) {
    // start of simulation...
    at_start = false;    
  } else if (!core_match(core)) {
    // not core of interest...
  } else if (SingleStep()) {
    // single-step 'breakpoint' reached. acknowledge...
    AcknowledgeBreakpoint(PC());
  } else {
    // go thru packet memory accesses looking for match...
    /*
    unsigned long long matched_address = 0;
    bool have_match = false;
    for (vector<MemoryAccess>::iterator mem_acc = ipkt->mOpsMemory.begin(); (mem_acc != ipkt->mOpsMemory.end()) && !have_match; mem_acc++) {
       have_match = watchpoint_hit(matched_address,mem_acc->Address(),mem_acc->Size());
    }
    if (have_match) {
      // a watchpoint was reached. acknowledge. set 'single-step' flag to alert pre-step of the event...
      AcknowledgeWatchpoint(my_cpu,matched_address);
      setSingleStep();
    }
    */
  }

#ifdef DEBUG_GDB_SERVER
  printf("[DebugServer::RunPostStepChecks] exited.\n");
#endif
  
  return true;
}

//***************************************************************************************
// format/send breakpoint response string...
//***************************************************************************************

void DebugServer::AcknowledgeBreakpoint(unsigned long long PC) {
#ifdef DEBUG_GDB_SERVER
  printf("[DebugServer::AcknowledgeBreakpoint] entered, PC: 0x%llx\n",PC);
#endif
  
  if (SingleStep()) {
    // we do need to send acknowledge to the debug client, but since single-stepping
    // no need for excess verbage to console...
  } else {
    std::cout << "\tBreakpoint on core: " << CoreID() << ", clock: " << Clock()
	      << ", PC: 0x" << std::hex << PC << std::dec << std::endl;
  }
  
    // form server response...
    
    std::string my_response = "T02"; // response: signal 2 (SIGINT)...

    // add updated FP (frame pointer), SP, PC value to response...
    // (see RegType method for list of gdb-riscv register indices)
    
    my_response += "1d:";   
    my_response += EncodeRegisterValue(FP());
    my_response += ";";
  
    my_response += "1f:";
    my_response += EncodeRegisterValue(SP());
    my_response += ";";
  
    my_response += "20:";
    my_response += EncodeRegisterValue(PC);
    my_response += ";";

    // since this response is not handled by RSP::Dispatch, we need to encode it here...
    
    my_response = Encode(my_response);
    
    // send server response...

    //std::cout << "Breakpoint response (encoded): '" << my_response << "'" << std::endl;

    try {
       server_socket->Write((unsigned char *) my_response.c_str(),my_response.size());
    } catch (const std::runtime_error& ex) {
       // oops! error on write to server socket???
       std::cerr << ex.what() << std::endl;
       listening = false;
    }
}

//***************************************************************************************
//***************************************************************************************

void DebugServer::AcknowledgeWatchpoint(unsigned long long data_address) {
    std::cout << "\tWatchpoint at address: 0x" << std::hex << data_address << std::dec << std::endl;

    // form server response...
    
    std::string my_response = "T02"; // response: signal 2 (SIGINT)...

    // add updated FP (frame pointer), SP, PC value to response...
    // (see RegType method for list of gdb-riscv register indices)
    
    my_response += "1d:";   
    my_response += EncodeRegisterValue(FP());
    my_response += ";";
  
    my_response += "1f:";
    my_response += EncodeRegisterValue(SP());
    my_response += ";";
  
    my_response += "20:";
    my_response += EncodeRegisterValue(PC());
    my_response += ";";

    // since this response is not handled by RSP::Dispatch, we need to encode it here...
    
    my_response = Encode(my_response);
    
    // send server response...
    
#ifdef DEBUG_GDB_SERVER
    std::cout << "Watchpoint response (encoded): '" << my_response << "'" << std::endl;
#endif
    
    try {
       server_socket->Write((unsigned char *) my_response.c_str(),my_response.size());
    } catch (const std::runtime_error& ex) {
       // oops! error on write to server socket???
       std::cerr << ex.what() << std::endl;
       listening = false;
    }
}

//***************************************************************************************
// Poll - field/process user commands...
//***************************************************************************************

bool DebugServer::Poll() {
#ifdef DEBUG_GDB_SERVER
  verbose = true;
#endif
  
  if (verbose)
    printf("[DebugServer::Poll] polling, PC: 0x%llx...\n",PC());
  
  listening = true; // will 'listen' until step or continue initialiated, or 'kill server'
                    //   exception occurs...

  if (verbose)
    printf("[DebugServer::Poll] listening...\n");

  while(listening) {
    if (verbose)
      printf("get next client message...\n");

    char mbuf[MAX_BUF_SIZE];
    memset(mbuf,'\0',MAX_BUF_SIZE);

    int num_read = 0;

    try {
       num_read = server_socket->Read((unsigned char *) mbuf,MAX_MSG_LEN);
    } catch(const std::runtime_error& ex) {
       // oops! error on read from server socket???
       std::cerr << ex.what() << std::endl;
       listening = false;
       continue;
    }

    if (verbose) std::cout << "next packet: '" << mbuf << "'" << std::endl;
    
    // decode client message...

    std::string client_request;

    try {
       client_request = Decode(mbuf,num_read,verbose);   
    }
    catch(...) {
       // bail on server-shutdown request or any decode error...
      if (num_read == 0)
      	 std::cerr << "Empty message received from debug client. Will assume its a shutdown request..." << std::endl;
       else
         std::cerr << "Decode error occurred, client message: '" << mbuf << "'" << std::endl; 
       listening = false;
       continue;
    }

    if (verbose) std::cout << "message decoded..." << std::endl;
    
    // message decode may require acknowledge before proceeding...

    std::string acknowledge = Acknowledge(client_request);

    if (acknowledge.size() > 0) {
      try {
         if (verbose) std::cout << "sending acknowledge..." << std::endl;	
         server_socket->Write((unsigned char *) acknowledge.c_str(),acknowledge.size());
      } catch (const std::runtime_error& ex) {
         // oops! error on write to server socket???
         std::cerr << ex.what() << std::endl;
         listening = false;
         continue;
      }
    }
    
    // get gdb server response to client request... 
    if (verbose) std::cout << "get gdb server response to client request..." << std::endl;	

    std::string server_response;
    
    try {
       server_response = Dispatch(client_request);	 
    } catch (const std::runtime_error& ex) {
       // (we assume) its time to shutdown...
       std::cerr << ex.what() << std::endl;
       listening = false;
       continue;
    }

    if (verbose) std::cout << "response: '" << server_response << "'" << std::endl;
    
    // some client requests may be ignored...

    if (server_response.size() == 0) {
       if (verbose) std::cout << "Empty response IGNORED" << std::endl;
       continue;
    }
    
    if (deferResponse()) {
       if (verbose) std::cout << "Response deferred..." << std::endl;
       listening = false;
       break;
    }
    
    // send server response...
    
    try {
       if (verbose) std::cout << "Sending response..." << std::endl;
       server_socket->Write((unsigned char *) server_response.c_str(),server_response.size());
    } catch (const std::runtime_error& ex) {
       // oops! error on write to server socket???
       std::cerr << ex.what() << std::endl;
       listening = false;
       continue;
    }
  }

  if (verbose)
    printf("[DebugServer::Poll] break from listening...\n");

  // return false when its time to shutdown...
  
  return !do_shutdown;
}

//***************************************************************************************
// clear 'listening' state, ie, polling ends, allow simulation to continue until
// next breakpoint or watchpoint, or simulation ends...
//***************************************************************************************

std::string DebugServer::Continue() {
  listening = false;

  defer_response = true;

  std::string my_response = "";
  return my_response;
}


//***************************************************************************************
// Step - step request. add breakpoint for this instruction...
//***************************************************************************************

std::string DebugServer::Step() {
#ifdef DEBUG_GDB_SERVER
  verbose = true;
#endif
  
  if (verbose)
    printf("[DebugServer::Step] PC: 0x%llx, (assumed) next PC: 0x%llx\n",PC(), PC() + 4);
  
  defer_response = true;

  setSingleStep(); // entering 'step' state...
  
  std::string my_response = "";
  return my_response;
}


//***************************************************************************************
// signal its time to end simulation and shut down...
//***************************************************************************************

std::string DebugServer::KillTarget() {
#ifdef DEBUG_GDB_SERVER
  printf("[DebugServer::KillTarget] entered...\n");
#endif
  
  // time to shutdown...
  do_shutdown = true; 

  std::string my_response = "X00";
  return my_response;
}


//***************************************************************************************
//***************************************************************************************

std::string DebugServer::ReadGPRegisters() {
#ifdef DEBUG_GDB_SERVER
  std::cout << "[DebugServer::ReadGPRegisters] entered..." << std::endl;
#endif
  
  std::string my_response = "";

  // for arm64, registers are:
  //    X0 thru X30, SP, PC, PSTATE, Q0 thru Q31, FPSR, FPCR
  //
  // for risc-V, registers are(?):
  //    X1 thru X31, SP?, PC?...WHAT ARE THE OTHERS?

  char tbuf[128];

  for (int i = 1; i < 31; i++) {
     my_response += EncodeRegisterValue(GP(i));
  }

  my_response += EncodeRegisterValue(SP());
  my_response += EncodeRegisterValue(PC());

/*
  le4(tbuf,my_cpu->Pstate.Value());
  my_response += tbuf;

  // Q0 thru Q31...
  
  for (int i = 0; i < 31; i++) {
     le8(tbuf,my_cpu->V[i].ValueHi());  // hi order 64 bits
     my_response += tbuf;
     le8(tbuf,my_cpu->V[i].Value());    // lo order 64 bits
     my_response += tbuf;
  }

  // FPSR, FPCR...
  
  le4(tbuf,my_cpu->FPSR.Value());
  my_response += tbuf;
  
  le4(tbuf,my_cpu->FPCR.Value());
  my_response += tbuf;
*/

#ifdef DEBUG_GDB_SERVER
  std::cout << "[DebugServer::ReadGPRegisters] response: " << my_response << std::endl;
#endif
  
  return my_response;
}


//***************************************************************************************
// ReadRegister - read simulator register value. Return '?' for uninitialized
//                register...
//
//   register indices (observed) from gdb client:
//
//     register      index
//      X0..X30      0..30
//      SP           31
//      Q0..Q31      32..63
//      PC           64
//      CPSR         65
//      FPSR         66
//      FPCR         67
//***************************************************************************************

std::string DebugServer::ReadRegister() {
#ifdef DEBUG_GDB_SERVER
  std::cout << "[DebugServer::ReadRegister] reg: " << pdata << std::endl;
#endif
  
  std::string my_response = "";

  unsigned int rindex = 0;

  if (sscanf(pdata.substr(1).c_str(),"%x",&rindex) != 1) {
    my_response = "E1"; // malformed read register packet...
  } else {
    // read simulator register value, encode and return...

    char tbuf[128];

    switch(RegType(rindex)) {
      case GPREG:     // return GP register...
                      my_response = EncodeRegisterValue(GP(rindex));
                      break;
      case SPREG:     // return (current) SP...
                      my_response = EncodeRegisterValue(SP());
                      break;
/*
      case QREG:      // return Q reg...
                      {
                       int fp_index = rindex - QREG_START_INDEX;
                       le8(tbuf,my_cpu->V[fp_index].Value());      //
                       //my_cpu.V[rindex].ValueHi()             // how is quad word encoded
                       my_response += "0000000000000000";  //   in little endian ala ARMland?
                       my_response += tbuf;
                      }
                      break;
*/
      case PCREG:     // return PC...
                      my_response += EncodeRegisterValue(PC());;
                      break;
/*
      case CPSRREG:   // return 'CPSR' - just the flags, in low order 4 bits...
                      le4(tbuf,my_cpu->Pstate.Value());
                      my_response += tbuf;
                      break;
      case FPSRREG:   // return FPSR...
                      le4(tbuf,my_cpu->FPSR.Value());
                      my_response += tbuf;
                      break;
      case FPCRREG:   // return FPCR...
                      le4(tbuf,my_cpu->FPCR.Value());
                      my_response += tbuf;
                      break;
*/
      default:        // assume malformed read register packet...
                      my_response = "E2"; 
                      break;
    }
  }
  
#ifdef DEBUG_GDB_SERVER
  std::cout << "[DebugServer::ReadRegister] response: " << my_response << std::endl;
#endif
  
  return my_response;
}


//***************************************************************************************
// WriteRegister - write simulator register value...
//***************************************************************************************

std::string DebugServer::WriteRegister() {
#ifdef DEBUG_GDB_SERVER
  std::cout << "[DebugServer::WriteRegister] reg: " << pdata << std::endl;
#endif
  
  std::string my_response = "OK";

  unsigned int rindex = 0;

  bool go = true;
  
  if (sscanf(pdata.substr(1).c_str(),"%x",&rindex) != 1) {
    my_response = "E1"; // malformed write register packet...
    go = false;
  }

  std::size_t pos = pdata.find("=");

  if (go && (pos == std::string::npos)) {
      my_response = "E 1"; // malformed write-register packet...
      go = false;
  }

  if (go) {
    // scan register value from packet based on register type...

    unsigned long long rval = 0;

    switch(RegType(rindex)) {
       case GPREG:  // set GP register...
	            if (DecodeRegisterValue(pdata.substr(pos + 1).c_str(),rval))
                      SetGP(rindex,rval);
		    else
                      my_response = "E2"; // malformed write-register packet...
                    break;
		    
      case SPREG:   // set current SP...
	            if (DecodeRegisterValue(pdata.substr(pos + 1).c_str(),rval)) {
                      SetSP(rval); //<---updates current SP
	            } else
                      my_response = "E2"; // malformed write-register packet...
                    break;
/*		    
      case QREG:    // set Q reg...
                    // in this example we will parse only lo order 64 bits for a Quad reg
	            if (decoded_le8(pdata.substr(pos + 1).c_str(),rval)) {
                      // save 64 bit (presumably encoded) floating point value... 
                      int fp_index = rindex - QREG_START_INDEX;
                      my_cpu->V[fp_index].UnsignedLong(rval);
		    } else
                      my_response = "E2"; // malformed write-register packet...
                    break;
*/
      case PCREG:   // set PC...
	            if (DecodeRegisterValue(pdata.substr(pos + 1).c_str(),rval)) 
                      SetPC(rval);
		    else
                      my_response = "E2"; // malformed write-register packet...
                    break;
/*		    
      case FPSRREG: // set FPSR...
	            {
		      unsigned int rv = 0;
	              if (decoded_le4(pdata.substr(pos + 1).c_str(),rv))
                        my_cpu->FPSR.Value(rval);
		      else
                        my_response = "E2"; // malformed write-register packet... 
                    }
                    break;
		    
      case FPCRREG: // set FPCR...
	            {
		      unsigned int rv = 0;
	              if (decoded_le4(pdata.substr(pos + 1).c_str(),rv))
                        my_cpu->FPCR.Value(rv);
		      else
                        my_response = "E2"; // malformed write-register packet...
	            }
                    break;
		    
      case CPSRREG: // set CPSR - not going to allow this one (in example anyway)
                    my_response = "E2"; 
                    break;
*/
      default:      // malformed read register packet...
                    my_response = "E2"; 
                    break;
    }
  }
  
  return my_response;
}


//***************************************************************************************
// ReadMemory - read simulator (physical) memory. Byte values returned are in
//              order by ascending address.
//              Return '?' for any uninitialized memory bytes. 
//***************************************************************************************

std::string DebugServer::ReadMemory() {
#ifdef DEBUG_GDB_SERVER
  std::cout << "[DebugServer::ReadMemory] address,size: " << pdata << std::endl;
#endif

  std::string my_response = "";

  unsigned long long maddr;
  int msize;

  if (sscanf(pdata.substr(1).c_str(),"%llx,%d",&maddr,&msize) != 2) {
    my_response = "E1"; // malformed read/write-memory packet...
    return my_response;
  }

  unsigned char mbuf[msize];

  int rcode = 0;
  
  try {
     rcode = ReadMemory(maddr,msize,mbuf);
  } catch(...) {
     my_response = "E0"; // memory address did not validate
     return my_response;
  }
  
  if (rcode != ALL_BYTES_DEFINED) {
    my_response = "E0"; // one or more bytes were uninitialized
    return my_response;
  }
     
  for (int i = 0; i < msize; i++) {
     char tbuf[128];
     sprintf(tbuf,"%02x",mbuf[i]);
     my_response += tbuf;
  }
  
#ifdef DEBUG_GDB_SERVER
  std::cout << "[DebugServer::ReadMemory] response: " << my_response << std::endl;
#endif
  
  return my_response;
}


//***************************************************************************************
// WriteMemory - write simulator (physical) memory. Input buffer is sequence
//               of bytes to be written to memory in order by ascending address.
//***************************************************************************************

std::string DebugServer::WriteMemory() {
#ifdef DEBUG_GDB_SERVER
  std::cout << "[DebugServer::ReadMemory] address,size: " << pdata << std::endl;
#endif
  
  std::string my_response = "OK"; // will assume success...

  unsigned long long maddr;
  int msize;

  if (sscanf(pdata.substr(1).c_str(),"%llx,%d",&maddr,&msize) != 2) {
    my_response = "E1"; // malformed read/write-memory packet...
    return my_response;
  }

  std::size_t pos = pdata.find(":");

  if (pos == std::string::npos) {
    my_response = "E1"; // malformed read/write-memory packet...
    return my_response;
  }
  
  // scan supplied memory value, ASSUMING 2 hex digits per byte...

  unsigned char mbytes[msize];

  for (int i = 0; i < msize; i++) {
    unsigned int bval;
    if (sscanf(pdata.substr(pos + 1).c_str(),"%2x",&bval) != 1) {
        my_response = "E2"; // malformed write-memory packet...
        return my_response;
    }
    mbytes[i] = (unsigned char) bval; 
    pos += 2; // advance to next encoded byte...
  }

  try {
    WriteMemory(maddr,msize,mbytes);
  } catch(SIM_EXCEPTIONS eclass) {
     my_response = "E2"; // problems writing memory???    
  }

  return my_response;
}


//***************************************************************************************
//***************************************************************************************

std::string DebugServer::SetBreakpoints() {
#ifdef DEBUG_GDB_SERVER
  std::cout << "[DebugServer::SetBreakpoints] type,address,size: " << pdata << std::endl;
#endif
  
  std::string my_response = "";

  int btype; // type: 0 == breakpoint, 2==watchpoint...
  unsigned long long maddr;
  int msize;

  if (sscanf(pdata.substr(1).c_str(),"%d,%llx,%d",&btype,&maddr,&msize) != 3) {
    my_response = "E1"; // malformed breakpoint packet...
    return my_response;
  }

  if (btype == 0) {
    std::cout << "\tBreakpoint set at instruction address: 0x" << std::hex << maddr << std::dec << std::endl;
    unsigned long long maddr_hi = maddr + ((msize > 4) ? msize : 3); // instruction covers at least four bytes...
    _add_breakpoint(instr_breakpoints,maddr,maddr_hi);
  } else if (btype == 2) {
    std::cout << "\tWatchpoint set at data address: 0x" << std::hex << maddr << std::dec << ",size: "
              << msize << std::endl;
    unsigned long long maddr_hi = maddr + msize; // address + size establishes a range of data addresses... 
    _add_breakpoint(data_breakpoints,maddr,maddr_hi);
  }

  my_response = "OK"; // if no errors...

  return my_response;
}

//***************************************************************************************
// process request to clear all breakpoints and watchpoints...
//***************************************************************************************

std::string DebugServer::ClearBreakpoints() {
  reset_breakpoints();

  std::string my_response = "OK";

  return my_response;
}

//***************************************************************************************
// clear all breakpoints and watchpoints...
//***************************************************************************************

void DebugServer::reset_breakpoints() {
  instr_breakpoints.erase(instr_breakpoints.begin(),instr_breakpoints.end());
  data_breakpoints.erase(data_breakpoints.begin(),data_breakpoints.end());  
}

//***************************************************************************************
// has a breakpoint been hit?...
//***************************************************************************************

bool DebugServer::breakpoint_hit(unsigned long long PC) {
  bool have_match = false;

  // grossly assuming instructions are always four bytes...
  
  for (unsigned int i = 0; (i < 4ull) && !have_match; i++) {
     have_match = _breakpoint_match(instr_breakpoints, PC + i);
  }

  return have_match;
}


//***************************************************************************************
// has a watchpoint been hit?...
//***************************************************************************************

bool DebugServer::watchpoint_hit(unsigned long long &matched_address, unsigned long long mem_address, int access_size) {
  bool have_match = false;

  for (unsigned long long byte_address = mem_address; (byte_address != mem_address + access_size) && !have_match; byte_address += 1) {
     have_match = _breakpoint_match(data_breakpoints,byte_address);
  }

  return have_match;
}

//***************************************************************************************
// add new breakpoint (or watchpoint)...
//
// NOTE: even though all a64 instructions must be aligned, we record each breakpoint
//       as an address range even when setting a breakpoint on the next instruction,
//       just to use the same code for instructions and data when checking for
//       breakpoint/watchpoint match. As a result, there is the annoyance that when
//       you call this method, the 1st address is the instruction (of course) and the
//       last address is the (unaligned byte) address of the last byte of the last
//       instruction in the instruction range of interest...
//***************************************************************************************

void DebugServer::_add_breakpoint(vector<struct breakpoint_range> &_breakpoints,
				     unsigned long long _address_lo, unsigned long long _address_hi) {

    struct breakpoint_range addr_range;
    
    addr_range.address_lo = _address_lo;
    addr_range.address_hi = _address_hi;

    _breakpoints.push_back(addr_range);
}

//***************************************************************************************
// look for a breakpoint match on address and set of breakpoint address ranges...
//***************************************************************************************

bool DebugServer::_breakpoint_match(vector<struct breakpoint_range> &_breakpoints, unsigned long long _input_address) {
  bool have_match = false;
  
  for (vector<struct breakpoint_range>::iterator i = _breakpoints.begin(); (i != _breakpoints.end()) && !have_match; i++) {
     have_match = (_input_address >= i->address_lo) && (_input_address <= i->address_hi);
  }
  
  return have_match;
}

//*******************************************************************************
// le4, le8, decoded_le4, decoded_le8 - encode/decode unsigned 32/64 bit values
//   text little-endian. for use with ReadRegister(s)/WriteRegister(s)...
//*******************************************************************************

void DebugServer::le8(char *encoded_rv,unsigned long long rv) {
  encoded_rv[0] = '\0';
  
  for (int i = 0; i < 8; i++) {
     unsigned int tval = (rv>>(i*8)) & 0xff;
     char tbuf[3];
     sprintf(tbuf,"%02x",tval);
     strcat(encoded_rv,tbuf);
  }
}
bool DebugServer::decoded_le8(const char *encoded_rv,unsigned long long &rv) {
  bool success = true;
  
  rv = 0;
  
  for (int i = 0; i < 8; i++) {
     unsigned int td = 0;
     if (sscanf(&encoded_rv[i*2],"%2x",&td) == 1)
       rv |= td << (i * 8);
     else
       success = false;
  }

  return success;
}
void DebugServer::le4(char *encoded_rv,unsigned int rv) {
  encoded_rv[0] = '\0';
  
  for (int i = 0; i < 4; i++) {
     unsigned int tval = (rv>>(i*8)) & 0xff;
     char tbuf[3];
     sprintf(tbuf,"%02x",tval);
     strcat(encoded_rv,tbuf);
  }
}
bool DebugServer::decoded_le4(const char *encoded_rv,unsigned int &rv) {
  bool success = true;
  
  rv = 0;
  
  for (int i = 0; i < 4; i++) {
     unsigned int td = 0;
     if (sscanf(&encoded_rv[i*2],"%2x",&td) == 1)
       rv |= td << (i * 8);
     else 
       success = false;
  }

  return success;
}

