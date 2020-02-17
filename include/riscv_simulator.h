#ifndef __RISCV_SIMULATOR__

class RiscvSimulator {
 public:
  RiscvSimulator() : sim_cfg(NULL), clock(0), instr_count(0), rcode(0),
		     cores_are_running(false), debug_server(NULL),
		     hit_test_pass_region(false) {
    Init();
  };
  RiscvSimulator(SimConfig *_sim_cfg)
    : sim_cfg(_sim_cfg), clock(0),instr_count(0), rcode(0),
      cores_are_running(false), debug_server(NULL), hit_test_pass_region(false) {
    sim_cfg = _sim_cfg;
    Init();
  };
  ~RiscvSimulator() { Fini(); };
  
  void Init(); // allocate riscv-specific resources
  void Fini(); // free       "      "        "

  int LoadTestImage();
  
  int Go();
  
  int InstrCount() { return instr_count; };

  bool GetReadyCpus(vector<RiscvState *> &ready_cores);
  void StepCores();
  
  void AdvanceClock() { clock++; };

  void ServiceDevices(); // service uart, other devices tbd

  bool DebugServerEnabled() { return debug_server != NULL; };
  void ShutdownDebugServer() { if (debug_server != NULL) delete debug_server; };
  void SetupDebugServer(int debug_port,unsigned int debug_core_id) {
    if (debug_port > 0) {
      printf("configuring debug server on port %d, attached to core %u...\n",
	     debug_port,debug_core_id);
      debug_server = new RiscvDebugServer(debug_port,debug_core_id);
    } else {
      printf("debug server NOT configured.\n");
    }
  };
  bool DebugPreStepChecks(RiscvState *my_cpu, Memory *my_memory, unsigned long long PC) {
    // only return false if debug server aborts or exits...
    return DebugServerEnabled() ? debug_server->RunPreStepChecks(my_cpu,my_memory,PC) : true;
  }
  bool DebugPostStepChecks(RiscvState *my_cpu, Memory *my_memory, unsigned long long PC) {
    // only return false if debug server aborts or exits...
    return DebugServerEnabled() ? debug_server->RunPostStepChecks(my_cpu,my_memory,PC) : true;
  }

protected:
  SimConfig *sim_cfg;
  unsigned long long clock;
  std::vector<RiscvState *> cores;
  Memory memory;
  int instr_count;
  int rcode;
  bool cores_are_running;
  Signals signals;   // timers, interrupts update signals
  UART_pl011 uart1;
  RiscvDebugServer *debug_server;

  bool hit_test_pass_region;
};


#endif
#define __RISCV_SIMULATOR__ 1

