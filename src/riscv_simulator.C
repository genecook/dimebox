#include <dimebox.h>
#include <vector>
#include <algorithm>

//****************************************************************************
// allocate/initialize cores, timers, devices, etc...
//****************************************************************************

void RiscvSimulator::Init() {
  // allocate state for each configured core...
  if (sim_cfg != NULL) {
    for (auto i = 0; i < sim_cfg->CoreCount(); i++) {
       cores.push_back(new RiscvState(sim_cfg));
    }
  }
}

void RiscvSimulator::Fini() {
  // release (cores) memory...
  for (auto ci = cores.begin(); ci != cores.end(); ci++) {
     delete *ci;
  }
}

//****************************************************************************
// run a simulation...
//****************************************************************************

int RiscvSimulator::Go() {
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

void RiscvSimulator::StepCores() {
  std::vector<RiscvState *> ready_cores;
  
  cores_are_running = GetReadyCpus(ready_cores);
  
  for (auto ci = ready_cores.begin(); ci != ready_cores.end(); ci++) {
     RiscvPacket updates(*ci);
     RiscvCoreControl ctrl(&updates,*ci,&memory,&signals);
     ctrl.Step();
     instr_count++;
  }
}

//****************************************************************************
// return (randomized) list of 'ready' cores. A core is ready if it is not
//  halted or in a wait state...
//****************************************************************************

bool RiscvSimulator::GetReadyCpus(std::vector<RiscvState *> &ready_cores) {
  int ready_count = 0;
  
  for (auto ci = cores.begin(); ci != cores.end(); ci++) {
    if ((*ci)->Ready()) {
       ready_cores.push_back(*ci);
       ready_count++;
     }
  }

  if (ready_count > 0)
    random_shuffle(ready_cores.begin(), ready_cores.end());

  return (ready_count > 0);
}

//****************************************************************************
// on each clock 'tick' service any devices...
//****************************************************************************

void RiscvSimulator::ServiceDevices() {
  // add code to uart to track clock...
}



