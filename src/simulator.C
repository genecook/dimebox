#include <dimebox.h>
#include <vector>
#include <algorithm>

//****************************************************************************
// run a simulation...
//****************************************************************************

int Simulator::Go() {
  cores_are_running = true; // we'll ASSUME at least one core is alive...

  rcode = 0; // 'master' error code
  
  instr_count = 0; // total # of instructions simulated for all cores
  
  while( !rcode && (instr_count < sim_cfg->MaxInstrs()) && cores_are_running) {
    ServiceDevices();
    StepCores();
    AdvanceClock();
  }

  // we expect all cores to be 'properly' halted at the end of simulation...
  
  for (auto ci = cores.begin(); ci != cores.end(); ci++) {
    if ( !(*ci)->Halted() )
       rcode = -1;
  }
  
  return rcode;
}

//****************************************************************************
// step all ready cores...
//****************************************************************************

void Simulator::StepCores() {
  std::vector<State *> ready_cores;
  
  cores_are_running = GetReadyCpus(ready_cores);
  
  for (auto ci = ready_cores.begin(); ci != ready_cores.end(); ci++) {
     unsigned int opcode;
     unsigned int pc = state->PC();
     memory->ReadMemory(pc,4,(unsigned char) &opcode);
     memory->ApplyEndianness(opcode);
     RiscvInstruction *instr = RiscvInstructionFactory::NewInstruction(state,memory,signals,opcode);
     instr.Step();
     instr.Writeback();
     char tbuf[128];
     sprintf(tbuf,"0x%08x 0x%08x %s",pc,opcode,instr.Disassembly());
     std::cout << tbuf << std::endl;
     instr_count++;
     (*ci).SetEndTest(state->PC() == pc);  // apparent jump to self instruction triggers end-test     
  }
}

//****************************************************************************
// return (randomized) list of 'ready' cores. A core is ready if it is not
//  halted or in a wait state...
//****************************************************************************

bool Simulator::GetReadyCpus(std::vector<RiscvState *> &ready_cores) {
  int ready_count = 0;
  
  for (auto ci = cores.begin(); ci != cores.end(); ci++) {
    if ((*ci)->Ready()) {
       ready_cores.push_back(*ci);
       ready_count++;
     }
  }

  if (ready_count > 0)
    random_shuffle(ready_core_indices.begin(), ready_core_indices.end());

  return (ready_count > 0);
}

