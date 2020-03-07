#include <string>
#include <vector>
#include <string.h>
#include <stdlib.h>
#include <dimebox.h>
#include <iostream>
#include <stdlib.h>

//**************************************************************************
// simplified memory access methods - will Byte me later...
//**************************************************************************

void RiscvInstruction::MEMORY_WRITE(unsigned long long address,int size,unsigned long long rval) {
  // don't write memory directly. instead 'queue up' a memory access. This way, if top-level
  // simulator or test generator elects to abort instruction, memory is unaffected...

  // for now only accept 'aligned' data accesses of no more than 8 bytes...
  switch(size) {
  case 1: case 2: case 4: case 8: break;
  default: throw std::runtime_error("Internal error: MEMORY_WRITE invalid byte count"); break;
  }

  // put input (assumed register) value directly into write buffer...
  // (account for how the data is stored in the host register)
  if (memory->HostEndianness()/* == true if data stored to memory in big-endian */) {
    // big endian: swap register bytes during copy...
    int j = size - 1;
    for (int i = 0; i < size; i++) {
       mbuf[j] = (rval>>(i*8)) & 0xff;
       j = j - 1;
    }
  } else {
    // little endian: straight copy from register to memory...
    for (int i = 0; i < size; i++) {
      mbuf[i] = (rval>>(i*8)) & 0xff;
    }
  }
  
  mOpsMemory.push_back(MemoryAccess(address,
				    size,
				    DATA,
				    false,  // exclusive?
				    1,      // write
				    0,      // big-endian
				    size,   // word size
				    false,  // sign extend
				    32,     // word size for (unused) sign extend
				    false,  // paired
				    false,  // privileged
				    mbuf    // the data
				    )
		       );
}

unsigned long long RiscvInstruction::MEMORY_READ(unsigned long long address,int size) {
  // for now only accept 'aligned' data accesses of no more than 8 bytes...
  switch(size) {
  case 1: case 2: case 4: case 8: break;
  default: throw std::runtime_error("Internal error: MEMORY_READ invalid byte count"); break;
  }

  // for memory reads however, go ahead and access memory. will ignore side effects of the memory access
  // should an instruction need to be aborted...
  bool is_data    = true;
  bool privileged = false;
  bool aligned    = true;
  bool for_test   = false;
  bool initialize = true;
  int rcode = memory->ReadMemory(state,address,is_data,privileged,size,aligned,mbuf,for_test,initialize);
  unsigned long long rval = 0;
  if (memory->HostEndianness() /* == true if data stored to memory in big-endian */) {
    // big endian: swap memory bytes during copy...
    int j = size - 1;
    for (int i = 0; i < size; i++) {
       rval = rval | ((unsigned long long)mbuf[j])<<(i*8);
       j = j - 1;
    }
  } else {
    // little endian: straight copy from memory to register...
    for (int i = 0; i < size; i++)
       rval = rval | (mbuf[i]<<(i*8));
  }

  // record the memory access...
  mOpsMemory.push_back(MemoryAccess(address,
				    size,
				    DATA,
				    false,  // exclusive?
				    0,      // read
				    0,      // big-endian
				    size,   // word size
				    false,  // sign extend
				    32,     // word size for (unused) sign extend
				    false,  // paired
				    false,  // privileged
				    mbuf    // the data
				    )
		       );
  
  return rval;
}

void RiscvInstruction::Writeback(RiscvState *_state,Memory *_memory,bool show_updates) {
  // after (successfully) stepping an instruction, update (core) register state, memory...
  if (show_updates) state->ShowRegisterAccesses();
  _state->Update(state);
  for (auto mop = mOpsMemory.begin(); mop != mOpsMemory.end(); mop++) {
     if (mop->IsWrite()) {
       memory->WriteMemory(state,mop->Address(),mop->IsData(),mop->Privileged(),
			   mop->Size(),mop->Aligned(),mop->Buffer());       
     }
     if (show_updates) {
       printf("  # M%c 0x%llx ",(mop->IsWrite() ? 'W' : 'R'),mop->Address());
       for (auto i = 0; i < mop->Size(); i++)
	  printf(" %02x",mop->Buffer()[i]);
       printf("\n");
     }
  }
}

//**************************************************************************
//**************************************************************************

#define INSTR_INST(X) new X(state,memory,encoding)

RiscvInstruction * RiscvInstructionFactory::NewInstruction(RiscvState *state,Memory *memory,
				  unsigned int encoding) {

  // decode RV32I Base Instruction Set...
  
  int opcode = encoding & 0x7f;
  int funct3 = (encoding >> 12) & 0x7;
  int rs1    = (encoding >> 15) & 0x1f;
  int rs2    = (encoding >> 20) & 0x1f;
  int funct7 = (encoding >> 25) & 0x7f;
  int imm    = (encoding >> 20) & 0xfff;
  
  RiscvInstruction *instruction = NULL;
  
  //printf("# opcode: 0x%02x funct3: 0x%x rs1: 0x%x rs2: 0x%x funct7: 0x%02x imm: %0x03x\n",
  //	 (unsigned) opcode,(unsigned) funct3,(unsigned) rs1,(unsigned) rs2,(unsigned) funct7,(unsigned) imm);
             
  // this code was auto-generated by hand... ;)

  switch(opcode) {
  case 0x37: instruction = INSTR_INST(LUI); break;
  case 0x17: instruction = INSTR_INST(AUIPC); break;
  case 0x6f: instruction = INSTR_INST(JAL); break;
  case 0x67: if (funct3 == 0) instruction = INSTR_INST(JALR); break;

  case 0x63: switch(funct3) {
               case 0: instruction = INSTR_INST(BEQ); break;
               case 1: instruction = INSTR_INST(BNE); break;
               case 4: instruction = INSTR_INST(BLT); break;
               case 5: instruction = INSTR_INST(BGE); break;
               case 6: instruction = INSTR_INST(BLTU); break;
               case 7: instruction = INSTR_INST(BGEU); break;
               default: break;
             }
             break;
	       
  case 0x3:  switch(funct3) {
	       case 0: instruction = INSTR_INST(LB); break;
	       case 1: instruction = INSTR_INST(LH); break;
	       case 2: instruction = INSTR_INST(LW); break;
	       case 4: instruction = INSTR_INST(LBU); break;
	       case 5: instruction = INSTR_INST(LHU); break;
                 default: break;
             }
             break;
	       
  case 0x23: switch(funct3) {
	       case 0: instruction = INSTR_INST(SB); break;
	       case 1: instruction = INSTR_INST(SH); break;
	       case 2: instruction = INSTR_INST(SW); break;
                 default: break;
             }
             break;
	       
  case 0x13: switch(funct3) {
	       case 0: instruction = INSTR_INST(ADDI); break;
	       case 2: instruction = INSTR_INST(SLTI); break;
	       case 3: instruction = INSTR_INST(SLTIU); break;
	       case 4: instruction = INSTR_INST(XORI); break;
	       case 6: instruction = INSTR_INST(ORI); break;
	       case 7: instruction = INSTR_INST(ANDI); break;
	       case 1: if (funct7 == 0) instruction = INSTR_INST(SLLI); break;
	       case 5: if (funct7 == 0) instruction = INSTR_INST(SRLI);
		 if (funct7 == 0x20) instruction = INSTR_INST(SRAI);
                         break;
               default: break;
             }
             break;

  case 0x33: switch(funct7) {
	     case 0: switch(funct3) {
	               case 0: instruction = INSTR_INST(ADD);  break;
	               case 1: instruction = INSTR_INST(SLL);  break;
	               case 2: instruction = INSTR_INST(SLT);  break;
	               case 3: instruction = INSTR_INST(SLTU); break;
	               case 4: instruction = INSTR_INST(XOR);  break;
	               case 5: instruction = INSTR_INST(SRL);  break;
	               case 6: instruction = INSTR_INST(OR);   break;
	               case 7: instruction = INSTR_INST(AND);  break;
	               default: break;		 
	             }
	             break;
	       
	     case 1: switch(funct3) {
	               case 0: instruction = INSTR_INST(MUL);    break;
	               case 1: instruction = INSTR_INST(MULH);   break;
		       case 2: instruction = INSTR_INST(MULHSU); break;
		       case 3: instruction = INSTR_INST(MULHU);  break;
		       case 4: instruction = INSTR_INST(DIV);    break;
		       case 5: instruction = INSTR_INST(DIVU);   break;
		       case 6: instruction = INSTR_INST(REM);    break;
		       case 7: instruction = INSTR_INST(REMU);   break;
	               default: break;		 
	             }
	             break;
	       
	     case 0x20: switch(funct3) {
	                  case 0: instruction = INSTR_INST(SUB); break;
	                  case 1: break;
		          case 2: break;
		          case 3: break;
		          case 4: break;
		          case 5: instruction = INSTR_INST(SRA); break;
		          case 6: break;
		          case 7: break;
	                  default: break;		 
	                }
	                break;
	     default: break;
	     }
             break;
	     
  case 0x73: if (encoding == 0x73) {
	       instruction = INSTR_INST(ECALL);
	       break;
	     }
	     if (encoding == 0x100073) {
	       instruction = INSTR_INST(EBREAK);
	       break;
	     }
	     if ( (funct3 == 0) && (rs1 == 0) ) {
	       switch(imm) {
	         case 0x002: instruction = INSTR_INST(URET);  break;
	         case 0x102: instruction = INSTR_INST(SRET);  break;
	         case 0x202: instruction = INSTR_INST(HRET);  break;
	         case 0x302: instruction = INSTR_INST(MRET);  break;
	         case 0x105: instruction = INSTR_INST(WFI);   break;
	         case 0x104: instruction = INSTR_INST(SFENCE);break;
                 default: break;
	       }
	       break;
	     }
             switch(funct3) {
	       case 1: instruction = INSTR_INST(CSRRW);  break;
	       case 2: instruction = INSTR_INST(CSRRS);  break;
	       case 3: instruction = INSTR_INST(CSRRC);  break;
	       case 5: instruction = INSTR_INST(CSRRWI); break;
	       case 6: instruction = INSTR_INST(CSRRSI); break;
	       case 7: instruction = INSTR_INST(CSRRCI); break;
               default: break;
             }
             break;
    case 0xf:  if ((encoding & 0xf00fffff) == 0xf) instruction = INSTR_INST(FENCE);
               if ((encoding & 0xffffffff) == 0x100f) instruction = INSTR_INST(FENCE_I);
	       break;
    default: break;
  }

  if (instruction == NULL) {
    throw ILLEGAL_INSTRUCTION_UNKNOWN_INSTR;
  }
  
  return instruction;
}
