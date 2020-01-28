#include <dimebox.h>
#include <vector>
#include <algorithm>

//****************************************************************************
// allocate/initialize cores, timers, devices, etc...
//****************************************************************************

void Simulator::Init() {
  for (auto i = 0; i < sim_cfg->CoreCount(); i++) {
     cores.push_back(new State(sim_cfg));
  }
}

// release (cores) memory...

void Simulator::Fini() {
  for (auto i = 0; i < sim_cfg->CoreCount(); i++) {
     delete cores[i];
  }
}

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
// on each clock 'tick' service any devices...
//****************************************************************************

void Simulator::ServiceDevices() {
}

//****************************************************************************
// step all ready cores...
//****************************************************************************

void Simulator::StepCores() {
  std::vector<State *> ready_cores;
  
  cores_are_running = GetReadyCpus(ready_cores);
  
  for (auto ci = ready_cores.begin(); ci != ready_cores.end(); ci++) {
     Step(*ci);
     instr_count++;
  }
}

//****************************************************************************
// return (randomized) list of 'ready' cores. A core is ready if it is not
//  halted or in a wait state...
//****************************************************************************

bool Simulator::GetReadyCpus(std::vector<State *> &ready_core_indices) {
  int ready_count = 0;
  
  for (auto ci = cores.begin(); ci != cores.end(); ci++) {
    if ((*ci)->Ready()) {
       ready_core_indices.push_back(*ci);
       ready_count++;
     }
  }

  if (ready_count > 0)
    random_shuffle(ready_core_indices.begin(), ready_core_indices.end());

  return (ready_count > 0);
}

//****************************************************************************
// execute next instruction on indexed core...
//****************************************************************************

void Simulator::Step(State *core) {
  // step an instruction...

  AdvanceClock();
}
