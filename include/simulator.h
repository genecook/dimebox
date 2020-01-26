#ifndef __SIMULATOR__

#define MAX_CPUS 8

class DimeboxSimulator {
 public:
  DimeboxSimulator() : clock(0) {};
  DimeboxSimulator(DimeboxSimConfig *_sim_cfg) : sim_cfg(_sim_cfg), clock(0) {};
  ~DimeboxSimulator() {};
  int Go() { return 0; };
  int SimulatedCount() { return 0; };

 private:
  DimeboxSimConfig *sim_cfg;
  unsigned long long clock;
  State cpus[MAX_CPUS];
  Memory memory;        
};

#endif
#define __SIMULATOR__ 1

