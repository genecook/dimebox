#include <dimebox.h>

//****************************************************************************
// execute next instruction...
//****************************************************************************

void CoreControl::Step() {
  // step an instruction...

  try {
     FetchInstruction();
     DecodeInstruction();
     FetchOperands();
     ExecuteInstruction();
     AccessMemory();
     WritebackOperands();
  } catch(SIM_EXCEPTIONS eclass) {
    throw std::runtime_error("Sure do need to handle simulator exceptions dude!");
  }
  
  state->AdvanceClock();
}

void CoreControl::FetchInstruction() {
  //packet->Pstate = cpu->Pstate;
  //packet->NextPstate = packet->Pstate;

  //packet->FPSR = cpu->FPSR;
  //packet->Next_FPSR = packet->FPSR;

   memory->ReadMemory(state,updates,state->PC(),false,state->Privileged(),
		      updates->InstructionSize(),true,updates->mbuf,false,false);

   memory->ApplyEndianness((unsigned char *) &updates->Opcode,updates->mbuf,
			   updates->InstrEndianness(),updates->InstrAccessSize(),
			   updates->InstructionSize());
}

void CoreControl::AccessMemory() {
}

void CoreControl::RetireInstruction() {
  state->SetPC(state->PC() + updates->InstrAccessSize()); // ASSUME no wrap-around
}

