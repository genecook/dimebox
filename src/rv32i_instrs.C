#include <dimebox.h>
#include <iostream>

// Itype:
void ADDI::Step() {
  RD( RS1() + SIGN_EXTEND_IMM(12) ); BumpPC();
};
void SLTI::Step() {
  RD( (long) RS1() < (long) SIGN_EXTEND_IMM(12) ? 1 : 0 ); BumpPC();
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
  RD( RS1() << IMM() ); BumpPC();
};
void SRLI::Step() { 
  RD( RS1() >> IMM() ); BumpPC();
};
void SRAI::Step() {
  RD ( SignExtend(RS1() >> IMM(),32 - IMM()) ); BumpPC();
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
  RD ( (long) RS1() < (long) RS2() ? 1 : 0 ); BumpPC();
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
  RD ( SignExtend(RS1() >> RS2(),32 - RS2()) ); BumpPC();
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
  PC( PC() + (RS1() == RS2() ? SIGN_EXTEND_IMM(12) : 4)); 
};
void BNE::Step() { 
  PC( PC() + (RS1() != RS2() ? SIGN_EXTEND_IMM(12) : 4)); 
};
void BLT::Step() { 
  PC( PC() + ((long)RS1() != (long)RS2() ? SIGN_EXTEND_IMM(12) : 4)); 
};
void BLTU::Step() { 
  PC( PC() + (RS1() != RS2() ? SIGN_EXTEND_IMM(12) : 4)); 
};
void BGE::Step() { 
  PC( PC() + ((long)RS1() >= (long)RS2() ? SIGN_EXTEND_IMM(12) : 4)); 
};
void BGEU::Step() { 
  PC( PC() + (RS1() >= RS2() ? SIGN_EXTEND_IMM(12) : 4)); 
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
  signals.Exception(UNIMPLEMENTED_INSTRUCTION); throw EXCEPTION;
};
void CSRRS::Step() { 
  signals.Exception(UNIMPLEMENTED_INSTRUCTION); throw EXCEPTION;
};
void CSRRC::Step() { 
  signals.Exception(UNIMPLEMENTED_INSTRUCTION); throw EXCEPTION;
};
void CSRRWI::Step() { 
  signals.Exception(UNIMPLEMENTED_INSTRUCTION); throw EXCEPTION;
};
void CSRRSI::Step() { 
  signals.Exception(UNIMPLEMENTED_INSTRUCTION); throw EXCEPTION;
};
void CSRRCI::Step() { 
  signals.Exception(UNIMPLEMENTED_INSTRUCTION); throw EXCEPTION;
};


void ECALL::Step() { 
  signals.Exception(UNIMPLEMENTED_INSTRUCTION); throw EXCEPTION;
};
void EBREAK::Step() {
  BumpPC(); // ignored...
};




