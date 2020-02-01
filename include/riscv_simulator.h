#ifndef __RISCV_SIMULATOR__

class RiscvSimulator {
 public:
  RiscvSimulator() : sim_cfg(NULL), clock(0), instr_count(0), rcode(0),
		cores_are_running(false) {
    Init();
  };
  RiscvSimulator(SimConfig *_sim_cfg)
    : sim_cfg(_sim_cfg), clock(0),instr_count(0), rcode(0),
      cores_are_running(false) {
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
};


#endif
#define __RISCV_SIMULATOR__ 1

