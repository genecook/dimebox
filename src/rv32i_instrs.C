#include <dimebox.h>

void ADDI::Step() {
  RD( RS1() + SignExtend(IMM(),12) );
};
void SLTI::Step() {
  RD( (long) RS1() < (long) SignExtend(IMM(),12) ? 1 : 0 );
};
void SLTIU::Step() { 
  RD( RS1() < (unsigned int) SignExtend(IMM(),12) ? 1 : 0 );
};
void ANDI::Step() { 
  RD( RS1() & (unsigned int) SignExtend(IMM(),12) );
};
void ORI::Step() { 
  RD( RS1() | (unsigned int) SignExtend(IMM(),12) );
};
void XORI::Step() { 
  RD( RS1() ^ (unsigned int) SignExtend(IMM(),12) );
};
void SLLI::Step() {
  RD( RS1() << IMM() );
};
void SRLI::Step() { 
  RD( RS1() >> IMM() );
};
void SRAI::Step() {
  RD ( SignExtend(RS1() >> IMM(),32 - IMM()) );
};
void LUI::Step() {
  RD( IMM()<<12 );
};
void AUIPC::Step() {
  RD( PC() + (IMM()<<12) );
};
void ADD::Step() {
  RD( RS1() + RS2() );
};
void SLT::Step() {
  RD ( (long) RS1() < (long) RS2() ? 1 : 0 );
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
  RD ( SignExtend(RS1() >> RS2(),32 - RS2()) );
};
void JAL::Step() {
  RD( PC() + 4 );
  PC( PC() + SignExtend(IMM(),20) );
};
void JALR::Step() { 
  RD( PC() + 4 );
  PC( (RS1() + SignExtend(IMM(),20)) & 0xfffffffe );
};
void BEQ::Step() {
  PC( PC() + (RS1() == RS2() ? SignExtend(IMM(),12) : 4)); 
};
void BNE::Step() { 
  PC( PC() + (RS1() != RS2() ? SignExtend(IMM(),12) : 4)); 
};
void BLT::Step() { 
  PC( PC() + ((long)RS1() != (long)RS2() ? SignExtend(IMM(),12) : 4)); 
};
void BLTU::Step() { 
  PC( PC() + (RS1() != RS2() ? SignExtend(IMM(),12) : 4)); 
};
void BGE::Step() { 
  PC( PC() + ((long)RS1() >= (long)RS2() ? SignExtend(IMM(),12) : 4)); 
};
void BGEU::Step() { 
  PC( PC() + (RS1() >= RS2() ? SignExtend(IMM(),12) : 4)); 
};
void LB::Step() {
  RD( SignExtend(MEMORY_READ(RS1() + SignExtend(IMM(),12), 1),8) );
};
void LH::Step() { 
  RD( SignExtend(MEMORY_READ(RS1() + SignExtend(IMM(),12), 2),16) );
};
void LW::Step() {
  RD( MEMORY_READ(RS1() + SignExtend(IMM(),12), 4) );
};
void LBU::Step() { 
  RD( MEMORY_READ(RS1() + SignExtend(IMM(),12), 1) );
};
void LHU::Step() { 
  RD( MEMORY_READ(RS1() + SignExtend(IMM(),12), 2) );
};
void SB::Step() { 
  MEMORY_WRITE( RS1() + SignExtend(IMM(),12), 1, RS2() );
};
void SH::Step() { 
  MEMORY_WRITE( RS1() + SignExtend(IMM(),12), 2, RS2() );
};
void SW::Step() { 
  MEMORY_WRITE( RS1() + SignExtend(IMM(),12), 4, RS2() );
};
void FENCE::Step() {
  // ignored...
};
void FENCE_I::Step() { 
  // ignored...
};


void CSRRW::Step() { 
};
void CSRRS::Step() { 
};
void CSRRC::Step() { 
};
void CSRRWI::Step() { 
};
void CSRRSI::Step() { 
};
void CSRRCI::Step() { 
};


void ECALL::Step() { 
};
void EBREAK::Step() {
  // ignored...
};




