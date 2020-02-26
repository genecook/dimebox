#include <stdlib.h>
#include <dimebox.h>

//#define DEBUG_CSR 1

//***********************************************************************
// validate CSR access:
//   1. within valid range?
//   2. accessed from correct privilege level?
//   3. read/write access correct?
//***********************************************************************

void RiscvState::ValidateCSRAccess(unsigned int csr_address,
				   unsigned int privilege_level, bool for_write) {

#ifdef DEBUG_CSR
  printf("[ValidateCSRAccess] CSR: 0x%03x privilege-level: %d write? %s\n",
	 csr_address,privilege_level,(for_write ? "yes" : "no"));
#endif
  
  bool access_okay = false;

  if (csr_address > 0xfff) {
    // if csr address is not in correct range then assume instruction was
    // decoded incorrectly...
    char tbuf[1024];
    sprintf(tbuf,"Invalid CSR address: 0x%03x?",csr_address);
    throw std::runtime_error(tbuf);
  }

  bool machine_level_access    = privilege_level == 3;
  bool hypervisor_level_access = privilege_level >= 2;
  bool supervisor_level_access = privilege_level >= 1;

  int csr_group = (int)( (csr_address>>8) & 0xf);

#ifdef DEBUG_CSR
  printf("[ValidateCSRAccess] CSR group: 0x%x\n",csr_group);
#endif
  
  switch(csr_group) {
    // accessible from all privilege levels...
    case 0:
    case 4:
    case 8:   access_okay = true; break;
    case 0xc: access_okay = !for_write; break; // read-only
      
    // accessible from supervisor and above levels...
    case 1:
    case 5:
    case 9:   access_okay = supervisor_level_access; break;
    case 0xd: access_okay = supervisor_level_access && !for_write; break; // read-only
      
    // accessible fromm hypervisor and above levels...
    case 2:
    case 6:
    case 0xa: access_okay = hypervisor_level_access; break;
    case 0xe: access_okay = hypervisor_level_access && !for_write; break; // read-only
      
    // accessible from machine level only...
    case 3:
    case 7:
    case 0xb: access_okay = machine_level_access; break;
    case 0xf: access_okay = machine_level_access && !for_write; break; // read-only
    default: break;
  }

  if (!access_okay) {
    throw ILLEGAL_INSTRUCTION;
  }
}

//***********************************************************************
//***********************************************************************

 // CSR      description
  // 0xc00    RDCYCLE (cycle counter - lo)
  // 0xc01    RDTIME (timer - lo)
  // 0xc02    RDINSTRET (instrs retired counter - lo)
  // 0xc80    RDCYCLE (cycle counter -hi)
  // 0xc81    RDTIME (timer - hi)
  // 0xc82    RDINSTRET (instrs retired counter - hi)

unsigned long long RiscvState::CSR(unsigned int csr,unsigned int privilege_level) {
  ValidateCSRAccess(csr,privilege_level,false);
  
  unsigned long long rval = 0;
  
  switch(csr) {
    case 0xc00: rval = Clock() & 0xffffffff; break;
    case 0xc01: rval = TimerValue() & 0xffffffff; break;
    case 0xc02: rval = InstructionCount() & 0xffffffff; break;
    case 0xc80: rval = Clock() >> 32; break;
    case 0xc81: rval = TimerValue() >> 32; break;
    case 0xc82: rval = InstructionCount() & 0xffffffff; break;
    case 0x341: rval = MEPC(); break;
    case 0xf14: rval = GetID(); break;
    case 0x305: rval = MTVEC(); break;
    case 0x180: rval = SATP();  break;
    case 0x3b0: rval = PMPADDR0(); break;
    case 0x3a0: rval = PMPCFG0(); break;
    case 0x302: rval = MEDELEG(); break;
    case 0x303: rval = MIDELEG(); break;
    case 0x304: rval = MIE(); break;
    case 0x300: rval = MSTATUS(); break;
      
    default: throw ILLEGAL_INSTRUCTION;
             break;
  }
  
  if (show_updates) ShowCSRAccess(CSR_NAME(csr),csr,rval);
  return rval;
}

void RiscvState::SetCSR(unsigned int csr,unsigned int privilege_level, unsigned long long rval) {
  ValidateCSRAccess(csr,privilege_level,true);
  
  switch(csr) {
    case 0x341: SetMEPC(rval);
                break;
    case 0x305: SetMTVEC(rval);
                break;
    case 0x180: SetSATP(rval);
                break;
    case 0x3b0: SetPMPADDR0(rval);
                break;
    case 0x3a0: SetPMPCFG0(rval);
                break;
    case 0x302: SetMEDELEG(rval);
		break;
    case 0x303: SetMIDELEG(rval);
		break;
    case 0x304: SetMIE(rval);
                break;
    case 0x300: SetMSTATUS(rval);
                break;
    default:    throw ILLEGAL_INSTRUCTION;
                break;
  }
    
    if (show_updates) ShowCSRAccess(CSR_NAME(csr),csr,rval,true);
}

std::string RiscvState::CSR_NAME(int csr) {
    std::string rname;
    char tbuf[16];
    switch(csr) {
      case 0xc00: rname = "rdcycle";    break;
      case 0xc01: rname = "rdtime";     break;
      case 0xc02: rname = "rdinstret";  break;
      case 0xc80: rname = "rdcycleh";   break;
      case 0xc81: rname = "rdtimeh";    break;
      case 0xc82: rname = "rdinstreth"; break;
      case 0xf14: rname = "mhartid";    break;
      case 0x305: rname = "mtvec";      break;
      case 0x180: rname = "satp";       break;
      case 0x3b0: rname = "pmpaddr0";   break;
      case 0x3a0: rname = "pmpcfg0";    break;
      case 0x302: rname = "medeleg";    break;
      case 0x303: rname = "mideleg";    break;
      case 0x304: rname = "mie";        break;
      case 0x300: rname = "mstatus";    break;
      case 0x341: rname = "mepc";       break;

      default: //throw EXCEPTION;
               sprintf(tbuf,"<0x%03x>",csr);
               rname = tbuf;
               break;
    }
    return rname;
}

