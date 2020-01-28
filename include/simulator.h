#ifndef __SIMULATOR__

class Signals {
public:
  Signals() {};
private:
};

class Simulator {
 public:
  Simulator() : clock(0) {};
  Simulator(SimConfig *_sim_cfg)
    : sim_cfg(_sim_cfg), clock(0), instr_count(0), rcode(0),
      cores_are_running(false) { Init(); };
  virtual ~Simulator() { Fini(); };
  virtual int Go();
  int InstrCount() { return instr_count; };

 protected:
  virtual void Init();
  virtual void Fini();
  virtual void ServiceDevices();
  virtual bool GetReadyCpus(vector<State *> &ready_cores);
  virtual void StepCores();
  virtual void Step(State *core);
  virtual void AdvanceClock() { clock++; };
  
private:
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

