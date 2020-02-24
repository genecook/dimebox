#include <dimebox.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>

#define RISCV_ISA_TESTING 1

void ADDI::Step() {
  RD( RS1() + SIGN_EXTEND_IMM(12) ); BumpPC();
};
void SLTI::Step() {
  RD( (int) RS1() < (int) SIGN_EXTEND_IMM(12) ? 1 : 0 ); BumpPC();
};
void SLTIU::Step() { 
  RD( (unsigned int) RS1() < (unsigned int) UNSIGNED_SIGN_EXTEND_IMM(12) ? 1 : 0 ); BumpPC();
};
void ANDI::Step() { 
  RD( RS1() & (unsigned int) SIGN_EXTEND_IMM(12) ); BumpPC();
};
void ORI::Step() { 
  RD( RS1() | (unsigned int) SIGN_EXTEND_IMM(12) ); BumpPC();
};
void XORI::Step() { 
  RD( RS1() ^ (unsigned int) SIGN_EXTEND_IMM(12) ); BumpPC();
};

void SLLI::Step() {
  RD( RS1() << IMM_FOR_SHIFT() ); BumpPC();
};
void SRLI::Step() { 
  RD( RS1() >> IMM_FOR_SHIFT() ); BumpPC();
};
void SRAI::Step() {
  RD ( (int) SignExtend(RS1() >> IMM_FOR_SHIFT(),32 - IMM_FOR_SHIFT()) ); BumpPC();
};

void LUI::Step() {
  RD( IMM() ); BumpPC();
};
void AUIPC::Step() {
  RD( PC() + (int) IMM() ); BumpPC();
};

void ADD::Step() {
  RD( RS1() + RS2() ); BumpPC();
};
void SLT::Step() {
  RD ( (int) RS1() < (int) RS2() ? 1 : 0 ); BumpPC();
};
void SLTU::Step() { 
  RD ( RS1() < RS2() ? 1 : 0 ); BumpPC();
};
void AND::Step() { 
  RD ( RS1() & RS2() ); BumpPC();
};
void OR::Step() { 
  RD ( RS1() | RS2() ); BumpPC();
};
void XOR::Step() { 
  RD ( RS1() ^ RS2() ); BumpPC();
};
void SLL::Step() {
  RD( RS1() << RS2() ); BumpPC();
};
void SRL::Step() { 
  RD( RS1() >> RS2() ); BumpPC();
};
void SUB::Step() { 
  RD( RS1() - RS2() ); BumpPC();
};
void SRA::Step() { 
  RD ( (int) SignExtend(RS1() >> RS2(),32 - (RS2() & 0x1f)) ); BumpPC();
};

void JAL::Step() {
  RD( PC() + 4 );
  PC( PC() + SIGN_EXTEND_IMM(20) );
};
void JALR::Step() { 
  RD( PC() + 4 );
  PC( (RS1() + SIGN_EXTEND_IMM(12)) & 0xfffffffe );
};

void BEQ::Step() {
  PC( PC() + (RS1() == RS2() ? SIGN_EXTEND_IMM(13) : 4)); 
};
void BNE::Step() {
  PC( PC() + (RS1() != RS2() ? SIGN_EXTEND_IMM(13) : 4)); 
};
void BLT::Step() { 
  PC( PC() + ((int)RS1() < (int)RS2() ? SIGN_EXTEND_IMM(13) : 4)); 
};
void BLTU::Step() { 
  PC( PC() + (RS1() < RS2() ? SIGN_EXTEND_IMM(13) : 4)); 
};
void BGE::Step() {
  PC( PC() + ((int)RS1() >= (int)RS2() ? SIGN_EXTEND_IMM(13) : 4)); 
};
void BGEU::Step() { 
  PC( PC() + (RS1() >= RS2() ? SIGN_EXTEND_IMM(13) : 4)); 
};

void LB::Step() {
  RD( SignExtend(MEMORY_READ(RS1() + SIGN_EXTEND_IMM(12), 1),8) ); BumpPC();
};
void LH::Step() { 
  RD( SignExtend(MEMORY_READ(RS1() + SIGN_EXTEND_IMM(12), 2),16) ); BumpPC();
};
void LW::Step() {
  RD( MEMORY_READ(RS1() + SIGN_EXTEND_IMM(12), 4) ); BumpPC();
};
void LBU::Step() { 
  RD( MEMORY_READ(RS1() + SIGN_EXTEND_IMM(12), 1) ); BumpPC();
};
void LHU::Step() { 
  RD( MEMORY_READ(RS1() + SIGN_EXTEND_IMM(12), 2) ); BumpPC();
};

void SB::Step() { 
  MEMORY_WRITE( RS1() + SIGN_EXTEND_IMM(12), 1, RS2() ); BumpPC();
};
void SH::Step() { 
  MEMORY_WRITE( RS1() + SIGN_EXTEND_IMM(12), 2, RS2() ); BumpPC();
};
void SW::Step() { 
  MEMORY_WRITE( RS1() + SIGN_EXTEND_IMM(12), 4, RS2() ); BumpPC();
};
void FENCE::Step() {
  BumpPC(); // ignored...
};
void FENCE_I::Step() { 
  BumpPC(); // ignored...
};


void CSRRW::Step() {
  RD( CSR(imm) ); SetCSR(imm, RS1() ); BumpPC();
};
void CSRRS::Step() { 
  RD( CSR(imm) ); if (rs1 != 0) SetCSR( imm, CSR(imm) | RS1() ); BumpPC();
};
void CSRRC::Step() { 
  RD( CSR(imm) ); if (rs1 != 0) SetCSR( imm, CSR(imm) | !RS1() ); BumpPC();
};
void CSRRWI::Step() { 
  RD( CSR(imm) ); SetCSR( imm, rs1 ); BumpPC();
};
void CSRRSI::Step() { 
  RD( CSR(imm) ); if (rs1 != 0) SetCSR( imm, CSR(imm) | rs1 ); BumpPC();
};
void CSRRCI::Step() { 
  RD( CSR(imm) ); if (rs1 != 0) SetCSR( imm, CSR(imm) | !rs1 ); BumpPC();
};


void ECALL::Step() {
#ifdef RISCV_ISA_TESTING
  if ( (GP() == 1) && (A7() == 93) && (A0() == 0) ) {
    std::cout << "TEST PASSES!!!" << std::endl;
    throw TEST_PASSES;
  } else {
    std::cout << "TEST FAILS!!!" << std::endl;
    throw TEST_FAILS;
  }    
#endif
  throw UNIMPLEMENTED_INSTRUCTION;
};
void EBREAK::Step() {
  BumpPC(); // ignored...
};

void URET::Step() {
  throw UNIMPLEMENTED_INSTRUCTION;
};
void SRET::Step() {
  throw UNIMPLEMENTED_INSTRUCTION;
};
void HRET::Step() {
  throw UNIMPLEMENTED_INSTRUCTION;
};
void MRET::Step() { PC ( MEPC() ); };

void WFI::Step() {
  throw WAIT_FOR_INTERRUPT;
};
void SFENCE::Step() {
  BumpPC(); // ignored...
};

void MUL::Step() {
  RD( RS1() * RS2() );
  BumpPC();
};
void MULHU::Step() {
  RD( ((unsigned long long) RS1() * (unsigned long long) RS2()) >> 32 );
  BumpPC();
};
void MULH::Step() {
  RD( ((long long) SignExtend(RS1(),32) * (long long) SignExtend(RS2(),32) ) >> 32 );
  BumpPC();
};
void MULHSU::Step() {
  RD( ((long long) SignExtend(RS1(),32) * (unsigned long long) RS2()) >> 32 );
  BumpPC();
};
void DIV::Step() {
  if ( RS2() == 0) // divide by zero
    RD( -1 );
  else if ( (RS1() == 0x80000000) && (RS2() == -1) ) // overflow
    RD(0x80000000);
  else
    RD( (int) RS1() / (int) RS2() );
  BumpPC();
};
void DIVU::Step() {
  if (RS2() == 0) // divide by zero
    RD( (unsigned) -1 );
  else
    RD( RS1() / RS2() );
  BumpPC();
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
  BumpPC();
};
void REMU::Step() {
  unsigned long long rdq = (unsigned long long) RS1() / (unsigned long long) RS2();
  unsigned int rem = rdq * (unsigned long long) RS2() - RS2();
  RD( rem );  
  BumpPC();
};




