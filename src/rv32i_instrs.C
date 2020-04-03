#include <dimebox.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>

void ADDI::Step() {
  RD( RS1() + SIGN_EXTEND_IMM(12) );
};
void SLTI::Step() {
  RD( (int) RS1() < (int) SIGN_EXTEND_IMM(12) ? 1 : 0 );
};
void SLTIU::Step() { 
  RD( (unsigned int) RS1() < (unsigned int) UNSIGNED_SIGN_EXTEND_IMM(12) ? 1 : 0 );
};
void ANDI::Step() { 
  RD( RS1() & (unsigned int) SIGN_EXTEND_IMM(12) );
};
void ORI::Step() { 
  RD( RS1() | (unsigned int) SIGN_EXTEND_IMM(12) );
};
void XORI::Step() { 
  RD( RS1() ^ (unsigned int) SIGN_EXTEND_IMM(12) );
};

void SLLI::Step() {
  RD( RS1() << IMM_FOR_SHIFT() );
};
void SRLI::Step() { 
  RD( RS1() >> IMM_FOR_SHIFT() );
};
void SRAI::Step() {
  RD ( (int) SignExtend(RS1() >> IMM_FOR_SHIFT(),32 - IMM_FOR_SHIFT()) );
};

void LUI::Step() {
  RD( IMM() );
};
void AUIPC::Step() {
  RD( PC() + (int) IMM() );
};

void ADD::Step() {
  RD( RS1() + RS2() );
};
void SLT::Step() {
  RD ( (int) RS1() < (int) RS2() ? 1 : 0 );
};
void SLTU::Step() { 
  RD ( RS1() < RS2() ? 1 : 0 );
};
void AND::Step() { 
  RD ( RS1() & RS2() );
};
void OR::Step() { 
  RD ( RS1() | RS2() );
};
void XOR::Step() { 
  RD ( RS1() ^ RS2() );
};
void SLL::Step() {
  RD( RS1() << RS2() );
};
void SRL::Step() { 
  RD( RS1() >> RS2() );
};
void SUB::Step() { 
  RD( RS1() - RS2() );
};
void SRA::Step() { 
  RD ( (int) SignExtend(RS1() >> RS2(),32 - (RS2() & 0x1f)) );
};

void JAL::Step() {
  SetPC( PC() + SIGN_EXTEND_IMM(20) );
  RD( PC() + 4 );
};
void JALR::Step() { 
  SetPC( (RS1() + SIGN_EXTEND_IMM(12)) & 0xfffffffe );
  RD( PC() + 4 );
};

void BEQ::Step() {
  SetPC( PC() + (RS1() == RS2() ? SIGN_EXTEND_IMM(13) : 4)); 
};
void BNE::Step() {
  SetPC( PC() + (RS1() != RS2() ? SIGN_EXTEND_IMM(13) : 4)); 
};
void BLT::Step() { 
  SetPC( PC() + ((int)RS1() < (int)RS2() ? SIGN_EXTEND_IMM(13) : 4)); 
};
void BLTU::Step() { 
  SetPC( PC() + (RS1() < RS2() ? SIGN_EXTEND_IMM(13) : 4)); 
};
void BGE::Step() {
  SetPC( PC() + ((int)RS1() >= (int)RS2() ? SIGN_EXTEND_IMM(13) : 4)); 
};
void BGEU::Step() { 
  SetPC( PC() + (RS1() >= RS2() ? SIGN_EXTEND_IMM(13) : 4)); 
};

void LB::Step() {
  RD( SignExtend(MEMORY_READ(RS1() + SIGN_EXTEND_IMM(12), 1),8) );
};
void LH::Step() { 
  RD( SignExtend(MEMORY_READ(RS1() + SIGN_EXTEND_IMM(12), 2),16) );
};
void LW::Step() {
  RD( MEMORY_READ(RS1() + SIGN_EXTEND_IMM(12), 4) );
};
void LBU::Step() { 
  RD( MEMORY_READ(RS1() + SIGN_EXTEND_IMM(12), 1) );
};
void LHU::Step() { 
  RD( MEMORY_READ(RS1() + SIGN_EXTEND_IMM(12), 2) );
};

void SB::Step() { 
  MEMORY_WRITE( RS1() + SIGN_EXTEND_IMM(12), 1, RS2() );
};
void SH::Step() { 
  MEMORY_WRITE( RS1() + SIGN_EXTEND_IMM(12), 2, RS2() );
};
void SW::Step() { 
  MEMORY_WRITE( RS1() + SIGN_EXTEND_IMM(12), 4, RS2() );
};

void FENCE::Step() {
  // ignored...
};
void FENCE_I::Step() { 
  // ignored...
};
void SFENCE::Step() {
  // ignored...
};

void CSRRW::Step() {
  unsigned int tmp = RS1();
  RD(CSR(imm));
  SetCSR(imm, tmp);
};

void CSRRS::Step() {
  unsigned int tmp = RS1();
  RD(CSR(imm));
  if (rs1 != 0)
    SetCSR(imm, CSR(imm) | tmp);
};
void CSRRC::Step() { 
  unsigned int tmp = RS1();
  RD(CSR(imm)); 
  if (rs1 != 0)
    SetCSR(imm,CSR(imm) & (~tmp));
};
void CSRRWI::Step() { 
  RD(CSR(imm));
  SetCSR(imm,rs1);
};
void CSRRSI::Step() { 
  RD(CSR(imm));
  if (rs1 != 0)
    SetCSR(imm,CSR(imm) + rs1);
};
void CSRRCI::Step() { 
  RD(CSR(imm));
  if (rs1 != 0)
    SetCSR(imm,CSR(imm) - rs1);
};

void ECALL::Step() {
  if (UserMode())
    throw ENV_CALL_UMODE;
  else
    throw ENV_CALL_MMODE;
};

void EBREAK::Step() {
  // ignored...
};

void URET::Step() {
  throw ILLEGAL_INSTRUCTION_UNIMPL_INSTR;
};
void SRET::Step() {
  throw ILLEGAL_INSTRUCTION_UNIMPL_INSTR;
};
void HRET::Step() {
  throw ILLEGAL_INSTRUCTION_UNIMPL_INSTR;
};
void MRET::Step() { 
  if (MachineMode())
    throw PROCESS_MRET;
  else
    throw ILLEGAL_INSTRUCTION_PRIVILEGED_INSTR;
};

void WFI::Step() {
  if (UserMode()) {
    throw ILLEGAL_INSTRUCTION_PRIVILEGED_INSTR;
  } else if (WFIEnabled()) {
    throw PROCESS_WFI;
  }
  else
    throw ILLEGAL_INSTRUCTION_UNIMPL_INSTR;
};

void MUL::Step() {
  RD( RS1() * RS2() );
};
void MULHU::Step() {
  RD( ((unsigned long long) RS1() * (unsigned long long) RS2()) >> 32 );
};
void MULH::Step() {
  RD( ((long long) SignExtend(RS1(),32) * (long long) SignExtend(RS2(),32) ) >> 32 );
};
void MULHSU::Step() {
  RD( ((long long) SignExtend(RS1(),32) * (unsigned long long) RS2()) >> 32 );
};
void DIV::Step() {
  if ( RS2() == 0) // divide by zero
    RD( -1 );
  else if ( (RS1() == 0x80000000) && (RS2() == -1) ) // overflow
    RD(0x80000000);
  else
    RD( (int) RS1() / (int) RS2() );
};
void DIVU::Step() {
  if (RS2() == 0) // divide by zero
    RD( (unsigned) -1 );
  else
    RD( RS1() / RS2() );
};
void REM::Step() {
  if (RS2() == 0) { // division by zero
    RD( RS1() ); // dividend is result
  } else if ( (RS1() == 0x80000000) && (RS2() == -1) ) { // overflow
    RD(0);
  } else {
    int rdq = (int) RS1() / (int) RS2();
    int rem = (int) RS1() - (rdq * (int) RS2());
    RD( rem );
  }
};
void REMU::Step() {
  unsigned long long rdq = (unsigned long long) RS1() / (unsigned long long) RS2();
  unsigned int rem = rdq * (unsigned long long) RS2() - RS2();
  RD( rem );  
};

// compressed instructions...

void C_LWSP::Step() {
  RD( MEMORY_READ(X(2) + IMM() * 4, 4) );
}
void C_SWSP::Step() {
  MEMORY_WRITE( X(2) + IMM() * 4, 4, RS2() );
}
void C_LW::Step() {
  RD( MEMORY_READ(RS1() + IMM() * 4, 4) );
}
void C_SW::Step() {
  MEMORY_WRITE( RS1() + IMM() * 4, 4, RS2() );
}
void C_J::Step() {
  SetPC( PC() + SIGN_EXTEND_IMM(11) );
}
void C_JAL::Step() {
  SetPC( PC() + SIGN_EXTEND_IMM(11) );
  RD( PC() + 2 );
}
void C_JR::Step() {
  SetPC( RS1() );
}
void C_JALR::Step() {
  SetPC( RS1() );
  RD( PC() + 2 );
}
void C_BEQZ::Step() {
  SetPC( PC() + (RS1() == 0 ? SIGN_EXTEND_IMM(8) : 2)); 
}
void C_BNEZ::Step() {
  SetPC( PC() + (RS1() != 0 ? SIGN_EXTEND_IMM(8) : 2)); 
}
void C_LI::Step() {
  if (rd == 0)
    throw ILLEGAL_INSTRUCTION_UNIMPL_INSTR;
  RD( SIGN_EXTEND_IMM(6) );
}
void C_LUI::Step() {
  if ( (rd == 0) || (rd == 2) )
    throw ILLEGAL_INSTRUCTION_UNIMPL_INSTR;
  RD( SignExtend(IMM()<<12,17) );
}
void C_ADDI::Step() {
  if (imm == 0)
    throw ILLEGAL_INSTRUCTION_UNIMPL_INSTR;
  RD( RS1() + SIGN_EXTEND_IMM(6) );
}
void C_ADDI16SP::Step() {
  if (imm == 0)
    throw ILLEGAL_INSTRUCTION_UNIMPL_INSTR;
  SetSP( SP() + SIGN_EXTEND_IMM(6)<<4 );
}
void C_ADDI4SPN::Step() {
  if (imm == 0)
    throw ILLEGAL_INSTRUCTION_UNIMPL_INSTR;
  RD( SP() + IMM()<<4 );
}
void C_SLLI::Step() {
  if (( imm & 0x20) != 0)
    throw ILLEGAL_INSTRUCTION_UNIMPL_INSTR;
  RD( RS1() << IMM_FOR_SHIFT() );
}
void C_SRLI::Step() {
  if (( imm & 0x20) != 0)
    throw ILLEGAL_INSTRUCTION_UNIMPL_INSTR;
  RD( RS1() >> IMM_FOR_SHIFT() );
}
void C_SRAI::Step() {
  if (( imm & 0x20) != 0)
    throw ILLEGAL_INSTRUCTION_UNIMPL_INSTR;
  RD ( (int) SignExtend(RS1() >> IMM_FOR_SHIFT(),32 - IMM_FOR_SHIFT()) );
}
void C_ANDI::Step() {
  RD( RD() & (unsigned int) SIGN_EXTEND_IMM(6) );
}
void C_MV::Step() {
  RD( RS2() );
}
void C_ADD::Step() {
  RD( RD() + RS2() );
}
void C_AND::Step() {
  RD ( RD() & RS2() );
}
void C_OR::Step() {
  RD ( RD() | RS2() );
}
void C_XOR::Step() {
  RD ( RD() ^ RS2() );
}
void C_SUB::Step() {
  RD( RD() - RS2() );
}
void C_NOP::Step() {
}
void C_EBREAK::Step() {
  // ignored...
}









