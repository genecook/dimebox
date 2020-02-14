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
  rcode = LoadTestImage();

  if (rcode)
    return rcode;
  
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

  for (auto ci = ready_cores.begin(); ci != ready_cores.end() && !rcode; ci++) {
     unsigned int pc = (*ci)->PC();
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
        RiscvState state_updates(*ci);
        RiscvInstruction *instr = RiscvInstructionFactory::NewInstruction(&state_updates,&memory,&signals,opcode.encoding);
        instr->Step();
        instr->Writeback(*ci,&memory,&signals);
	if (sim_cfg->ShowDisassembly()) {
          char tbuf[128];
          sprintf(tbuf,"0x%08x 0x%08x %s",pc,opcode.encoding,instr->Disassembly().c_str());
          std::cout << tbuf << std::endl;
	}
        delete instr;
        instr_count++;
	(*ci)->AdvanceClock();
        (*ci)->SetEndTest((*ci)->PC() == pc);  // apparent jump to self instruction triggers end-test
        if (!DebugPostStepChecks(*ci,&memory,pc)) {
          (*ci)->SetEndTest(true);
        }
     } catch(SIM_EXCEPTIONS sim_exception) {
       switch((int) sim_exception) {
         case UNIMPLEMENTED_INSTRUCTION:
	   fprintf(stderr,"Unimplemented or unknown instruction encoding! PC: 0x%08x, encoded instruction: 0x%08x\n",pc,opcode.encoding);
           rcode = -1;
	   break;
         case CSR_ACCESS:
	   fprintf(stderr,"Unknown csr or privileged csr access! PC: 0x%08x, encoded instruction: 0x%08x\n",pc,opcode.encoding);
           rcode = -1;
	   break;
         case TEST_PASSES:
	   printf("'Test harness' indicates success!\n");
	   rcode = 0;
	   (*ci)->SetEndTest(true);
	   break;
         case TEST_FAILS:
	   fprintf(stderr,"'Test harness' indicates FAILURE!\n");
	   rcode = -1;
	   break;	 
         default:
           fprintf(stderr,"Problems with instruction??? PC: 0x%08x, encoded instruction: 0x%08x\n",pc,opcode.encoding);
           rcode = -1;
	   break;
       }
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
  // add code to uart to track clock...
  if (uart1.IsImplemented()) {
    uart1.advanceClock();
    uart1.ServiceIOs();
    /* uart interrupt not supported yet...
    int int_info;
    if (uart.InterruptPending(int_info)) {
      // until GIC is implemented, uart interrupt is tied to cpu0 IRQ...
      cpus[0].SignalIRQ();
    }
    */
}
}



