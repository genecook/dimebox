#ifndef __SIMULATOR__

class Simulator {
 public:
  Simulator() : sim_cfg(NULL), clock(0), instr_count(0), rcode(0),
		cores_are_running(false) {
    Init();
  };
  Simulator(SimConfig *_sim_cfg) : clock(0), instr_count(0), rcode(0),
		cores_are_running(false) {
    sim_cfg = _sim_cfg;
    Init();
  };
  virtual ~Simulator() { Fini(); };
  
  virtual void Init() {};
  virtual void Fini() {};
  virtual int Go();
  
  int InstrCount() { return instr_count; };

  virtual void ServiceDevices() {};
  
  bool GetReadyCpus(vector<State *> &ready_cores);
  void StepCores();
  virtual void Step() {};
  
  void AdvanceClock() { clock++; };
protected:
  SimConfig *sim_cfg;
  unsigned long long clock;
  std::vector<State *> cores;
  Memory memory;
  int instr_count;
  int rcode;
  bool cores_are_running;
  Signals signals;   // timers, interrupts update signals
};


#endif
#define __SIMULATOR__ 1

