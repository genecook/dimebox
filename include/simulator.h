#ifndef __SIMULATOR__

#define MAX_CPUS 8

class Simulator {
 public:
  Simulator() : clock(0) {};
  Simulator(SimConfig *_sim_cfg)
    : sim_cfg(_sim_cfg), clock(0), instr_count(0) { Init(); };
  virtual ~Simulator() {};
  virtual void Init();
  virtual int Go();
  int InstrCount() { return instr_count; };
 protected:
 private:
  SimConfig *sim_cfg;
  unsigned long long clock;
  State cpus[MAX_CPUS];
  Memory memory;
  int instr_count;
};

#endif
#define __SIMULATOR__ 1

