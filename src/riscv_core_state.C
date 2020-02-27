#include <stdlib.h>
#include <dimebox.h>

//#define DEBUG_CSR 1

//***********************************************************************
// update one RiscvState object from another, possibly displaying
// register values that have changed...
//***********************************************************************

void RiscvState::Update(RiscvState *rhs) {
    State::Update((State *) rhs,show_updates);
    
    if (USTATUS() != rhs->USTATUS()) SetUSTATUS(rhs->USTATUS());
    if (FFLAGS() != rhs->FFLAGS()) SetFFLAGS(rhs->FFLAGS());
    if (FRM() != rhs->FRM()) SetFRM(rhs->FRM());
    if (FCSR() != rhs->FCSR()) SetFCSR(rhs->FCSR());
    if (SATP() != rhs->SATP()) SetSATP(rhs->SATP());
    if (MSTATUS() != rhs->MSTATUS()) SetMSTATUS(rhs->MSTATUS());  
    if (MEDELEG() != rhs->MEDELEG()) SetMEDELEG(rhs->MEDELEG());
    if (MIDELEG() != rhs->MIDELEG()) SetMIDELEG(rhs->MIDELEG());
    if (MIE() != rhs->MIE()) SetMIE(rhs->MIE());
    if (MTVEC() != rhs->MTVEC()) SetMTVEC(rhs->MTVEC());
    if (MCOUNTEREN() != rhs->MCOUNTEREN()) SetMCOUNTEREN(rhs->MCOUNTEREN());
    if (MSTATUSH() != rhs->MSTATUSH()) SetMSTATUSH(rhs->MSTATUSH());
    if (MSCRATCH() != rhs->MSCRATCH()) SetMSCRATCH(rhs->MSCRATCH());
    if (MEPC() != rhs->MEPC()) SetMEPC(rhs->MEPC());
    if (MCAUSE() != rhs->MCAUSE()) SetMCAUSE(rhs->MCAUSE());
    if (MTVAL() != rhs->MTVAL()) SetMTVAL(rhs->MTVAL());
    if (MIP() != rhs->MIP()) SetMIP(rhs->MIP());	
    if (MTINST() != rhs->MTINST()) SetMTINST(rhs->MTINST());
    if (PMPCFG0() != rhs->PMPCFG0()) SetPMPCFG0(rhs->PMPCFG0());
    if (PMPADDR0() != rhs->PMPADDR0()) SetPMPADDR0(rhs->PMPADDR0());
    if (MCYCLE() != rhs->MCYCLE()) SetMCYCLE(rhs->MCYCLE());
    if (MTINSTRET() != rhs->MTINSTRET()) SetMTINSTRET(rhs->MTINSTRET());
    if (MCYCLEH() != rhs->MCYCLEH()) SetMCYCLEH(rhs->MCYCLEH());
    if (MTINSTRETH() != rhs->MTINSTRETH()) SetMTINSTRETH(rhs->MTINSTRETH());

    for (auto i = 0; i < 32; i++) {
      if (X[i].Value() != rhs->X[i].Value()) {
        X[i].Value(rhs->X[i].Value());
	if (show_updates) ShowRegisterAccess(RegAlias(i),i,X[i].Value(),true);
      }
    }
  }

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
    case 0x000: rval = USTATUS(); break;

    case 0x001: rval = FFLAGS(); break;
    case 0x002: rval = FRM(); break;
    case 0x003: rval = FCSR(); break;

    case 0x180: rval = SATP(); break;

    case 0x300: rval = MSTATUS(); break;
    case 0x301: rval = MISA; break;
    case 0x302: rval = MEDELEG(); break;
    case 0x303: rval = MIDELEG(); break;
    case 0x304: rval = MIE(); break;
    case 0x305: rval = MTVEC(); break;
    case 0x306: rval = MCOUNTEREN(); break;
      
    case 0x310: rval = MSTATUSH(); break;
      
    case 0x340: rval = MSCRATCH(); break;
    case 0x341: rval = MEPC(); break;
    case 0x342: rval = MCAUSE(); break;
    case 0x343: rval = MTVAL(); break;
    case 0x344: rval = MIP(); break;
      
    case 0x34A: rval = MTINST(); break;

    case 0x3a0: rval = PMPCFG0(); break;
      
    case 0x3b0: rval = PMPADDR0(); break;

    case 0xb00: rval = MCYCLE(); break;
    case 0xb02: rval = MTINSTRET(); break;

    case 0xb80: rval = MCYCLEH(); break;
    case 0xb82: rval = MTINSTRETH(); break;

    case 0xc00: rval = Clock() & 0xffffffff; break;
    case 0xc01: rval = TimerValue() & 0xffffffff; break;
    case 0xc02: rval = InstructionCount() & 0xffffffff; break;
      
    case 0xc80: rval = Clock() >> 32; break;
    case 0xc81: rval = TimerValue() >> 32; break;
    case 0xc82: rval = InstructionCount() & 0xffffffff; break;
      
    case 0xf11: rval = MVENDORID; break;
    case 0xf12: rval = MARCHID; break;
    case 0xf13: rval = MIMPID; break;
    case 0xf14: rval = GetID(); break;
  
    default: throw ILLEGAL_INSTRUCTION;
             break;
  }
  
  if (show_updates) ShowCSRAccess(CSR_NAME(csr),csr,rval);
  return rval;
}

void RiscvState::SetCSR(unsigned int csr,unsigned int privilege_level, unsigned long long rval) {
  ValidateCSRAccess(csr,privilege_level,true);
  
  switch(csr) {
    case 0x000: SetUSTATUS(rval); break;

    case 0x001: SetFFLAGS(rval); break;
    case 0x002: SetFRM(rval); break;
    case 0x003: SetFCSR(rval); break;
      
    case 0x180: SetSATP(rval); break;
		
    case 0x300: SetMSTATUS(rval); break;
      
    case 0x302: SetMEDELEG(rval); break;
    case 0x303: SetMIDELEG(rval); break;
    case 0x304: SetMIE(rval); break;
    case 0x305: SetMTVEC(rval); break;
    case 0x306: SetMCOUNTEREN(rval); break;
		
    case 0x310: SetMSTATUSH(rval); break;

    case 0x340: SetMSCRATCH(rval); break;
    case 0x341: SetMEPC(rval); break;
    case 0x342: SetMCAUSE(rval); break;
    case 0x343: SetMTVAL(rval); break;
    case 0x344: SetMIP(rval); break;
		
    case 0x34A: SetMTINST(rval); break;

    case 0x3a0: SetPMPCFG0(rval); break;
		
    case 0x3b0: SetPMPADDR0(rval); break;
		
    case 0xb00: SetMCYCLE(rval); break;
    case 0xb02: SetMTINSTRET(rval); break;

    case 0xb80: SetMCYCLEH(rval); break;
    case 0xb82: SetMTINSTRETH(rval); break;

    default:    throw ILLEGAL_INSTRUCTION;
                break;
  }
    
    if (show_updates) ShowCSRAccess(CSR_NAME(csr),csr,rval,true);
}

std::string RiscvState::CSR_NAME(int csr) {
    std::string rname;
    char tbuf[16];
    switch(csr) {	
      case 0x000: rname = "ustatus";    break;

      case 0x001: rname = "fflags";     break;
      case 0x002: rname = "frm";        break;
      case 0x003: rname = "fcsr";       break;
      
      case 0x180: rname = "satp";       break;
	
      case 0x300: rname = "mstatus";    break;

      case 0x301: rname = "misa";       break;
      case 0x302: rname = "medeleg";    break;
      case 0x303: rname = "mideleg";    break;
      case 0x304: rname = "mie";        break;
      case 0x305: rname = "mtvec";      break;
      case 0x306: rname = "mcounteren"; break;

      case 0x310: rname = "mstatush";   break;

      case 0x340: rname = "mscratch";   break;
      case 0x341: rname = "mepc";       break;
      case 0x342: rname = "mcause";     break;
      case 0x343: rname = "mtval";      break;
      case 0x344: rname = "mip";        break;
		
      case 0x34A: rname = "mtinst";     break;

      case 0x3a0: rname = "pmpcfg0";    break;
      case 0x3b0: rname = "pmpaddr0";   break;
	
      case 0xb00: rname = "mycycle";    break;
      case 0xb02: rname = "mtinstret";  break;

      case 0xb80: rname = "mycycleh";   break;
      case 0xb82: rname = "mtinstreth"; break;
      
      case 0xc00: rname = "rdcycle";    break;
      case 0xc01: rname = "rdtime";     break;
      case 0xc02: rname = "rdinstret";  break;
	
      case 0xc80: rname = "rdcycleh";   break;
      case 0xc81: rname = "rdtimeh";    break;
      case 0xc82: rname = "rdinstreth"; break;
	
      case 0xf14: rname = "mhartid";    break;
	
      default: //throw EXCEPTION;
               sprintf(tbuf,"<0x%03x>",csr);
               rname = tbuf;
               break;
    }
    return rname;
}

