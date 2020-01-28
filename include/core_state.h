#ifndef __CORE_STATE__

#include <sim_config.h>

class State {
public:
  State() : id(0), asid(0), is_secure(false), clock(0) {};
  State(SimConfig *sim_cfg) : id(0), asid(0), is_secure(false), clock(0) { Init(sim_cfg); };
  ~State() {};

  void Init(SimConfig *sim_cfg) { _PC.Value(0); };
  unsigned int GetID() { return id; };
  bool IsSecure() { return is_secure; };
  bool Ready() { return true; };
  bool Halted() { return false; };
  void *ITLB() { return NULL; };
  void *DTLB() { return NULL; };
  exclMonitor &LocalMonitor() { return local_monitor; };
  unsigned int Asid() { return asid; };
  void AdvanceClock() { clock++; };
  unsigned long long Clock() { return clock; };
  unsigned long long GP(unsigned int rindex) {
    if (rindex == 0)
      return 0;
    return X[rindex].Value();
  };
  void setGP(unsigned int rindex,unsigned long long rval) {
    if (rindex > 0)
      X[rindex].Value(rval);
  };
  unsigned long long SP() { return X[2].Value(); };
  void setSP(unsigned long long rval) { X[2].Value(rval); };
  
  unsigned long long PC() { return _PC.Value(); };
  void setPC(unsigned long long rval) { _PC.Value(rval); };

private:
  unsigned int id;
  unsigned int asid;
  bool is_secure;
  unsigned long long clock;
  GPRegister X[32];    // general purpose registers X1 thru X31
  ProgramCounter _PC;  // PC

  exclMonitor local_monitor; // each CPU has a local monitor (holdover from another simulator)
};

#endif
#define __CORE_STATE__ 1
