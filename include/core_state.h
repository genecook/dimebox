#ifndef __CORE_STATE__

#include <sim_config.h>
#include <iostream>
#include <stdlib.h>

class State {
public:
  State() : id(0), asid(0), is_secure(false), clock(0), end_test(false) {};
  State(SimConfig *sim_cfg) : id(0), asid(0), is_secure(false), clock(0), end_test(false),
			      instr_count(0), timer(0) { Init(sim_cfg); };
  State(State *rhs) { Update(rhs,false); };
  virtual ~State() {};

  virtual void Init(SimConfig *sim_cfg) { _PC.Value(0); };

  virtual void Update(State *rhs, bool show_updates) {
    id = rhs->id;
    asid = rhs->asid;
    is_secure = rhs->is_secure;
    clock = rhs->clock;
    end_test = rhs->end_test;
    SetPC(rhs->PC());
    instr_count = rhs->instr_count;
    timer = rhs->timer;
  };
  
  unsigned int GetID() { return id; };
  unsigned int Asid() { return asid; };
  void AdvanceClock() { clock++; };
  unsigned long long Clock() { return clock; };
  
  unsigned long long PC() { return _PC.Value(); };
  void SetPC(unsigned long long rval) { _PC.Value(rval); };
  
  exclMonitor &LocalMonitor() { return local_monitor; };

  void IncrementInstrCount() { instr_count++; };
  unsigned long long InstructionCount() { return instr_count; };

  void AdvanceTimer(unsigned int timer_increment) { timer += timer_increment; };
  unsigned long long TimerValue() { return timer; };
  
  virtual unsigned long long GP(unsigned int rindex) = 0;
  virtual void SetGP(unsigned int rindex,unsigned long long rval) = 0;

  virtual unsigned long long SP() = 0;
  virtual void SetSP(unsigned long long rval) = 0;

  virtual bool IsSecure() { return is_secure; };
  virtual bool Ready() { return !EndTest(); };
  virtual bool Halted() { return end_test; };  // later may check end-test + core actually halted
  virtual bool Privileged() { return false; };

  bool EndTest() { return end_test; };
  void SetEndTest(bool _end_test) {
    end_test = _end_test;
  };

  virtual void *ITLB() { return NULL; };
  virtual void *DTLB() { return NULL; };

protected:
  unsigned int id;
  unsigned int asid;
  bool is_secure;
  unsigned long long clock;
  bool end_test;
  ProgramCounter _PC;  // PC
  unsigned long long instr_count;  // # of 'retired' instructions
  unsigned long long timer;        // timer
  
  exclMonitor local_monitor; // each CPU has a local monitor (holdover from another simulator)
};

#endif
#define __CORE_STATE__ 1
