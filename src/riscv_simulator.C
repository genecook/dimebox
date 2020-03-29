#include <dimebox.h>
#include <iostream>
#include <vector>
#include <algorithm>

//****************************************************************************
// allocate/initialize cores, timers, devices, etc...
//****************************************************************************

void RiscvSimulator::Init() {
  // allocate state for each configured core...
  for (auto i = 0; i < sim_cfg->CoreCount(); i++) {
     cores.push_back(new RiscvState(sim_cfg));
  }
  // setup ram...
  unsigned long long address_lo,address_hi;
  if (sim_cfg->DefaultAddressRange(address_lo,address_hi)) {
    memory.SetPhysicalSize(32);
    memory.SetPhysicalAddressRange(address_lo,address_hi);
  } else {
    // no memory range specified???
  }
  // setup devices...
  for (auto md = sim_cfg->devices.begin(); md != sim_cfg->devices.end(); md++) {
    if (md->second == "UART_PL011") {
      uart1.MapDevice(md->first); // instance device, map to requested physical address range
      memory.AddDevice(&uart1);   // make memory aware of device
    } else if (md->second == "MACHINE_TIMER") {
      timer.MapDevice(md->first); // instance device, map to requested physical address range
      memory.AddDevice(&timer);   // make memory aware of device
    } else {
      // add other devices tbd...
    }
  }
  SetupDebugServer(sim_cfg->DebugPort(),sim_cfg->DebugCoreID());
}

void RiscvSimulator::Fini() {
  // release (cores) memory...
  for (auto ci = cores.begin(); ci != cores.end(); ci++) {
     delete *ci;
  }
}

//****************************************************************************
// load test image(s), ie, elf files...
//****************************************************************************

int RiscvSimulator::LoadTestImage() {
  int rcode = 0;

  std::vector<std::string> elf_files;

  sim_cfg->SrcFiles(elf_files);

  for (auto ef = elf_files.begin(); ef != elf_files.end(); ef++) {
     ELFIO elf_helper;
     rcode = elf_helper.Load(memory,*ef,/* verbose= */ false);
     if (rcode)
       break;
  }

  return rcode;
}

int RiscvSimulator::DumpTest(std::string out_file) {
  ELFIO elf_helper;
  return elf_helper.Save(memory,out_file,sim_cfg->ResetAddress(),false,false);
}

//****************************************************************************
// run a simulation...
//****************************************************************************

int RiscvSimulator::Go() {
  rcode = 0; // 'master' error code
  
  rcode = LoadTestImage();

  if (rcode)
    return rcode;
  
  std::vector<RiscvState *> ready_cores;
  
  bool cores_are_running = GetReadyCpus(ready_cores);

  instr_count = 0; // total # of instructions simulated for all cores

  while( !rcode && !InstructionCountExceeded() && (cores_are_running || SleepingCores()) ) {
    if (cores_are_running)
      StepCores(ready_cores);
    AdvanceClock();
    ServiceDevices();
    cores_are_running = GetReadyCpus(ready_cores);
  }

  // we expect all cores to be 'properly' halted at the end of simulation...
  
  for (auto ci = cores.begin(); ci != cores.end(); ci++) {
    if ( !(*ci)->Halted() )
       rcode = -1;
  }
  
  return rcode;
}

//****************************************************************************
// max number of instructions simulated exceeded?
//****************************************************************************

bool RiscvSimulator::InstructionCountExceeded() {
  if (sim_cfg->MaxInstrs() < 0) // if max-instrs is negative then simulation 
    return false;               //   runs forever...
  
  return (instr_count > sim_cfg->MaxInstrs()); // have we reached the max yet?
}

//****************************************************************************
// step all ready cores...
//****************************************************************************

void RiscvSimulator::StepCores(std::vector<RiscvState *> &ready_cores) {
  for (auto ci = ready_cores.begin(); ci != ready_cores.end() && !rcode; ci++) {
    try {
       StepCore(*ci);
    } catch(const std::runtime_error &msg) {
       // generally will get here if core cannot handle an exception or interrupt...
       std::cout << msg.what() << std::endl;
       rcode = -1;
       break;
    }
  }
}

//****************************************************************************
// step a single core...
//****************************************************************************

void RiscvSimulator::StepCore(RiscvState *core) {
  RiscvState state_updates(core,sim_cfg->ShowUpdates());
  
  SIM_EXCEPTIONS interrupt_to_service;
  
  if ( state_updates.InterruptPending(interrupt_to_service) ) {
    // an interrupt is pending and ready for service...
    state_updates.ProcessInterrupt(interrupt_to_service);
  }

  if ( state_updates.LowPowerMode() ) {
    return;
  }
     
  unsigned int pc = state_updates.PC();

  union {
    unsigned char buf[4];
    unsigned int encoding;
  } opcode;
     
  try {
     memory.ReadMemory(&state_updates,pc,false,false,4,true,opcode.buf,false,false);
     memory.ApplyEndianness(opcode.buf,opcode.buf,false,4,4);
  } catch(SIM_EXCEPTIONS sim_exception) {
     std::cerr << "Problems reading memory at (PC) 0x" << std::hex << pc << std::dec << "???" << std::endl;
     rcode = -1;
     return;
  }

  if (!DebugPreStepChecks(&state_updates,&memory,pc)) {
    state_updates.SetEndTest(true);
    return;
  }

  RiscvInstruction *instr = RiscvInstructionFactory::NewInstruction(&state_updates,&memory,opcode.encoding);
	
  instr->Execute(sim_cfg->ShowDisassembly());
  instr->Writeback(core,&memory,sim_cfg->ShowUpdates());
  
  if (sim_cfg->ISAtest() && (instr->InstrName() == "ecall")) {
    // for ISA tests ecall instruction ends test...
    core->SetEndTest(true);
  }
  
  delete instr;
  instr_count++;
	
  if (core->EndTest()) {
    // test has ended...
  } else if (core->LowPowerMode()) {
    // core in low power mode. hopefully some event (interrupt or wakeup from another core) will wake it up...
  } else {
    // apparent jump to self instruction triggers end-test...
    core->SetEndTest(core->PC() == pc);  
  }
  
  if (!DebugPostStepChecks(core,&memory,pc)) {
    // debug may end test...
    core->SetEndTest(true);
  }
}

//****************************************************************************
// return (randomized) list of 'ready' cores. A core is ready if it is not
//  halted or in a wait state...
//****************************************************************************

bool RiscvSimulator::GetReadyCpus(std::vector<RiscvState *> &ready_cores) {
  ready_cores.clear();
  
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

bool RiscvSimulator::SleepingCores() {
  for (auto ci = cores.begin(); ci != cores.end(); ci++) {
    if ((*ci)->LowPowerMode()) {
       return true;
    }
  }
  return false;
}

//****************************************************************************
// on each clock 'tick' service any devices...
//****************************************************************************

void RiscvSimulator::ServiceDevices() {
  if (timer.IsImplemented()) {
    timer.AdvanceTimer();
    if (timer.InterruptPending()) {
      // until interrupt controller is implemented, timer interrupt is tied to cpu0...
      cores[0]->Signal(MACHINE_TIMER_INT);
      //      cores[0]->ClearLowPowerMode();
    }
  }
  if (uart1.IsImplemented()) {
    uart1.advanceClock();
    uart1.ServiceIOs();
    int int_info;
    if (uart1.InterruptPending(int_info)) {
      // until interrupt controller is implemented, uart interrupts are tied to cpu0...
      cores[0]->Signal(MACHINE_EXTERNAL_INT);
    }
  }
}

//****************************************************************************
// output test signature file...
//****************************************************************************

int RiscvSimulator::WriteTestSignature() {
  if (sim_cfg->SignatureStartAddress() == 0)  // test signature is optional...
    return 0;
  
  int signature_size = sim_cfg->SignatureEndAddress() - sim_cfg->SignatureStartAddress();

  // write out signature as quad-words (16 bytes each)...
  
  if (signature_size % 16 != 0) {
    fprintf(stderr,"ERROR: Test signature address must fall on 16 byte boundaries.\n");
    return -1;
  }

  FILE *sig_file = fopen("test.signature","w");
  
  int rcode = 0;

  for (int i = 0; (i < signature_size) && !rcode; i += 16) {
     unsigned long long next_sig_addr = sim_cfg->SignatureStartAddress() + i;
     unsigned char tbuf[16];
     try {
        memory.ReadPhysicalMemory(next_sig_addr,16,tbuf,false);
	if (memory.HostEndianness() /* true for big-endian */) {
	  // big-endian...
	  for (auto bx = 0; bx < 16; bx++)
	    fprintf(sig_file,"%02x",tbuf[bx]);
	} else {
	  // little-endian...
	  for (auto bx = 15; bx >= 0; bx--)
	    fprintf(sig_file,"%02x",tbuf[bx]);
	}
	fprintf(sig_file,"\n");
     } catch(SIM_EXCEPTIONS sim_exception) {
       std::cerr << "Problems reading physical memory at 0x"
		 << std::hex << next_sig_addr << std::dec << "???" << std::endl;
       rcode = -1;
     }
  }

  fclose(sig_file);
  std::cout << "\nTest signature file: test.signature" << std::endl;

  return rcode;
}




