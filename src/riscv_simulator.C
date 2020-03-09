#include <dimebox.h>
#include <iostream>
#include <vector>
#include <algorithm>

#define RISCV_ISA_TESTING 1

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

//****************************************************************************
// run a simulation...
//****************************************************************************

int RiscvSimulator::Go() {
  rcode = 0; // 'master' error code
  
  rcode = LoadTestImage();

  if (rcode)
    return rcode;
  
  cores_are_running = true; // we'll ASSUME at least one core is alive...

  instr_count = 0; // total # of instructions simulated for all cores

  while( !rcode && (instr_count < sim_cfg->MaxInstrs()) && cores_are_running) {
    StepCores();
    AdvanceClock();
    ServiceDevices();
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

  for (auto ci = ready_cores.begin(); ci != ready_cores.end() && !rcode; ci++) {
     unsigned int pc = (*ci)->PC();

     hit_test_pass_region = sim_cfg->PassAddress(pc); // did test 'pass' memory region get accessed???

     SIM_EXCEPTIONS interrupt_to_service;
     if ( (*ci)->InterruptPending(interrupt_to_service) ) {
       // an interrupt is pending and ready for service...
       (*ci)->ProcessInterrupt(interrupt_to_service);
     }

     union {
       unsigned char buf[4];
       unsigned int encoding;
     } opcode;
     
     try {
        memory.ReadMemory(*ci,pc,false,false,4,true,opcode.buf);
        memory.ApplyEndianness(opcode.buf,opcode.buf,false,4,4);
     } catch(SIM_EXCEPTIONS sim_exception) {
       std::cerr << "Problems reading memory at (PC) 0x" << std::hex << pc << std::dec << "???" << std::endl;
       rcode = -1;
       continue;
     }
     
     try {
        if (!DebugPreStepChecks(*ci,&memory,pc)) {
          (*ci)->SetEndTest(true);
	  return;
        }
        RiscvState state_updates(*ci,sim_cfg->ShowUpdates());
        RiscvInstruction *instr = RiscvInstructionFactory::NewInstruction(&state_updates,&memory,opcode.encoding);
	instr->Execute(sim_cfg->ShowUpdates());
	instr->Writeback(*ci,&memory,sim_cfg->ShowUpdates());
#ifdef RISCV_ISA_TESTING
	if (instr->InstrName() == "ecall") {
          if ( ((*ci)->GP(3) == 1) && ((*ci)->GP(17) == 93) && ((*ci)->GP(10) == 0) ) {
            // at env-call, core register state indicates test has passed...
            std::cout << "TEST PASSES!!!" << std::endl;
            //throw TEST_PASSES;
	    // machine state seems to indicate test has passed...
	    if (hit_test_pass_region) {
	       // we expect the test to have 'passed' thru the 'pass' region...
	       printf("'Test harness' indicates success!\n");
	       (*ci)->SetEndTest(true);
	    } else {
	       fprintf(stderr,"'Test harness' indicates success but test did NOT pass through 'pass' memory region???\n");
	       rcode = -1;
	    }
          } else {
            std::cout << "TEST FAILS!!!" << std::endl;
            //throw TEST_FAILS;
	    rcode = -1;
          }
	}
#endif
        delete instr;
        instr_count++;
	(*ci)->AdvanceClock();
	
        if ((*ci)->EndTest()) {
	  // test has ended...
	} else {
	  // apparent jump to self instruction triggers end-test...
	  (*ci)->SetEndTest((*ci)->PC() == pc);  
	}
        if (!DebugPostStepChecks(*ci,&memory,pc)) {
	  // debug may end test...
          (*ci)->SetEndTest(true);
        }
     } catch(const std::runtime_error &msg) {
       // generally will get here if core cannot handle an exception or interrupt...
       std::cout << msg.what() << std::endl;
       rcode = -1;
       break;
     }
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
  if (timer.IsImplemented()) {
    timer.AdvanceTimer();
    if (timer.InterruptPending()) {
      // until interrupt controller is implemented, timer interrupt is tied to cpu0...
      cores[0]->Signal(MACHINE_TIMER_INT);
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




