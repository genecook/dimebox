#include <stdlib.h>
#include <dimebox.h>

//#define DEBUG_CSR 1

//***********************************************************************
// update one RiscvState object from another, possibly displaying
// register values that have changed...
//***********************************************************************

void RiscvState::Update(RiscvState *rhs) {
    State::Update((State *) rhs,show_updates);
   
    _machine_state = rhs->_machine_state;

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
    throw ILLEGAL_INSTRUCTION_PRIVILEGED_CSR;
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
  
    default: throw ILLEGAL_INSTRUCTION_UNKNOWN_CSR;
             break;
  }
  
  if (show_updates) ShowCSRAccess(CSR_NAME(csr),csr,rval);
  return rval;
}

//***********************************************************************
//***********************************************************************

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

    default:    throw ILLEGAL_INSTRUCTION_UNKNOWN_CSR;
                break;
  }
    
    if (show_updates) ShowCSRAccess(CSR_NAME(csr),csr,rval,true);
}

//***********************************************************************
//***********************************************************************

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

//***********************************************************************
// set/restore privilege level entering/exiting exception...
//***********************************************************************

void RiscvState::PushPrivilegeLevel() {
  // get mstatus, isolate mie bit:
  unsigned int mstatus = MSTATUS();        
  unsigned int mie = (mstatus >> 3) & 1; // mie is bit 3 of mstatus
  // clear mstatus mpp, mpie, mie fields:
  mstatus &= ~0xc88; // clear bits 12,11,7,3 of mstatus
  // save current privilege level in mpp (bits 12,11), save mie state in mpie:
  SetMSTATUS( mstatus | (CurrentPrivilegeLevel() << 11) | (mie << 7) );
  // goto privilege level used to service exception:
  SetPrivilegeLevel( MACHINE_MODE );   
}

void RiscvState::PopPrivilegeLevel() {
  // get mstatus, isolate mpp, mpie fields:
  unsigned int mstatus = MSTATUS();        
  unsigned int mpie    = (mstatus >> 7) & 1;  // mpie is bit 3 of mstatus
  unsigned int mpp     = (mstatus >> 11) & 3; // mpp is bits 12,11
  // clear mstatus mpp, mpie, mie fields:
  mstatus &= ~0xc88; // clear bits 12,11,7,3 of mstatus
  // restore privilege level from mpp
  SetPrivilegeLevel( mpp );
  // save current (new) privilege level in mpp (bits 12,11), restore mie from mpie:
  SetMSTATUS( mstatus | (CurrentPrivilegeLevel() << 11) | (mpie << 3) );
}

//***********************************************************************
// is there an interrupt ready to be serviced?
//***********************************************************************

bool RiscvState::InterruptPending(SIM_EXCEPTIONS &sim_interrupt) {
  if ( !GlobalInterruptsEnabled() || !MIP() || !MIE() )
    return false;
  
  // global interrupts are enabled, there is at least one interrupt pending,
  // and there is at least one interrupt enabled...
  
  bool external_interrupt_pending = (MIP()>>11) & 1;
  bool external_interrupt_enabled = (MIE()>>11) & 1;
  
  if (external_interrupt_pending && external_interrupt_enabled) {
    sim_interrupt = MACHINE_EXTERNAL_INT;
    if (show_updates) ShowComment("  # machine external interrupt...");
    return true;
  }
  
  bool timer_interrupt_pending = (MIP()>>7) & 1;
  bool timer_interrupt_enabled = (MIE()>>7) & 1;
  
  if (timer_interrupt_pending && timer_interrupt_enabled) {
    sim_interrupt = MACHINE_TIMER_INT;
    if (show_updates) ShowComment("  # timer interrupt...");
   return true;
  }
  
  return false; // no interrupts are pending and/or no interrupts are enabled
}

//***********************************************************************
// process exception or interrupt...
//
//   --process any (supported) architecture related exception or
//     platform interrupt here...
//***********************************************************************

// take exception if pending AND has priority...
void RiscvState::ProcessException(SIM_EXCEPTIONS sim_exception, unsigned int opcode) {
  int cause_bit = -1;
  
  switch((int) sim_exception) {
    // timer and uart are only hardware interrupts currently supported...
    case MACHINE_TIMER_INT:
      cause_bit = 7;
    case MACHINE_EXTERNAL_INT:
      if (cause_bit == -1) cause_bit = 11;
      // validate interrupt enabled -- OR INTERNAL ERROR!
      // interrupt enabled?
/*
      MEIP = machine external interrupt pending - MIP.MEIP (bit 11)
      MEIE =    "       "        "      enable  - MIE.MEIE    "
      MTIP = machine timer interrupt pending - MIP.MTIP (bit 7)
      MTIE =    "       "        "   enable  - MIE.MTIE    "

      if ( !GlobalInterruptsEnabled() || ( (MIE()>>cause_bit) != 1) ) {
        // interrupts NOT enabled. set interrupt pending bit but otherwise ignore it...
	SetMIP( MIP() | (1<<cause_bit) );
	break;
      }
*/
      SetMIP( MIP() & ~(1<<cause_bit) ); // clear interrupt pending bit...
      ClearLowPowerMode();
      PushPrivilegeLevel();
      SetMCAUSE( (sim_exception & 0xf) | 0x80000000 ); // set Interrupt bit
      SetMTVAL(0);
      SetMEPC( PC() );
      SetPC( MTVEC() + ((MCAUSE() & 0xfffffff) * 4) ); // strip interrupt bit
      break;

    // WFI causes processor into low power mode... 
    case PROCESS_WFI:
      SetMCAUSE( sim_exception & 0xf );
      SetMTVAL(0);
      SetMEPC( PC() + 4 );
      SetLowPowerMode();
      break;

    // return from interrupt or exception...
    case PROCESS_MRET:
      PopPrivilegeLevel();
      SetPC ( MEPC() );
      break;

    // env call from user or machine mode...
    case ENV_CALL_UMODE:
    case ENV_CALL_MMODE:
      PushPrivilegeLevel();
      SetMCAUSE( sim_exception & 0xf );
      SetMTVAL(0);
      SetMEPC( PC() );
      SetPC( MTVEC() );
      break;

    // illegal instruction exception. broken down into simulator specific cases
    // for debug purposes...
    case ILLEGAL_INSTRUCTION_UNKNOWN_INSTR:
    case ILLEGAL_INSTRUCTION_UNIMPL_INSTR:
    case ILLEGAL_INSTRUCTION_PRIVILEGED_INSTR:
    case ILLEGAL_INSTRUCTION_UNKNOWN_CSR:
    case ILLEGAL_INSTRUCTION_PRIVILEGED_CSR:
      SetMCAUSE( sim_exception & 0xf );
      SetMTVAL(0);
      PushPrivilegeLevel();
      SetMEPC( PC() );
      SetPC( MTVEC() );
      break;

    // any other trap or interrupt implies logic error in simulator...
    default:
      // some unimplemented interrupt/exception???
      {
       char tbuf[1024];
       DecodeException(tbuf,sim_exception,PC(),opcode);
       throw std::runtime_error(tbuf);
      }
      break;
  }
}

//***********************************************************************
// decode exception info into print string...
//***********************************************************************

void RiscvState::DecodeException(char *tbuf,SIM_EXCEPTIONS sim_exception, unsigned long long pc,unsigned int opcode) {
  switch((int) sim_exception) {
    case MACHINE_SWI:
      sprintf(tbuf,"SWI issued in Machine Mode. PC: 0x%08llx, encoded instruction: 0x%08x\n",pc,opcode);
      break;
    case MACHINE_TIMER_INT:
      sprintf(tbuf,"Timer interrupt, Machine Mode. On or around PC: 0x%08llx\n",pc);
      break;
    case MACHINE_EXTERNAL_INT_UART:
      sprintf(tbuf,"Interrupt from Uart, Machine Mode. On or around PC: 0x%08llx.\n",pc);
      break;
    case PROCESS_WFI:
      sprintf(tbuf,"WFI issued in Machine Mode. PC: 0x%08llx, encoded (assumed WFI) instruction: 0x%08x\n",pc,opcode);
      break;     
    case PROCESS_MRET:
      sprintf(tbuf,"MRET issued. PC: 0x%08llx, encoded instruction: 0x%08x\n",pc,opcode);
      break;     
    case ENV_CALL_UMODE:
      sprintf(tbuf,"ECALL issued from User Mode. PC: 0x%08llx, encoded instruction: 0x%08x\n",pc,opcode);
      break;     
    case ENV_CALL_MMODE:  
      sprintf(tbuf,"ECALL issued from Machine Mode. PC: 0x%08llx, encoded instruction: 0x%08x\n",pc,opcode);
      break;     
    case ILLEGAL_INSTRUCTION_UNKNOWN_INSTR:
      sprintf(tbuf,"Unknown instruction. PC: 0x%08llx, encoded instruction: 0x%08x\n",pc,opcode);
      break;
    case ILLEGAL_INSTRUCTION_UNIMPL_INSTR:
      sprintf(tbuf,"Unimplemented instruction. PC: 0x%08llx, encoded instruction: 0x%08x\n",pc,opcode);
      break;
    case ILLEGAL_INSTRUCTION_PRIVILEGED_INSTR:
      sprintf(tbuf,"Attempt to execute privileged instruction. PC: 0x%08llx, encoded instruction: 0x%08x\n",pc,opcode);
      break;
    case ILLEGAL_INSTRUCTION_UNKNOWN_CSR:
      sprintf(tbuf,"Access to unknown CSR. PC: 0x%08llx, encoded instruction: 0x%08x\n",pc,opcode);
      break;
    case ILLEGAL_INSTRUCTION_PRIVILEGED_CSR:
      sprintf(tbuf,"Privileged csr access. PC: 0x%08llx, encoded instruction: 0x%08x\n",pc,opcode);
      break;
      
    default:
      // some unimplemented interrupt/exception???
      sprintf(tbuf,"Unknown or unsupported exception type: 0x%03x",(int) sim_exception);
      break;
  }
}

