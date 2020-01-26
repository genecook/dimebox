#ifndef __SIMULATOR__

class DimeboxSimulator {
 public:
  DimeboxSimulator() {};
  DimeboxSimulator(struct DimeboxSimConfig &sim_cfg) {};
  ~DimeboxSimulator() {};
  int Go() { return 0; };
  int SimulatedCount() { return 0; };

 private:
};

#endif
#define __SIMULATOR__ 1

