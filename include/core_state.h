#ifndef __CORE_STATE__

class State {
public:
  State() {};
  ~State() {};
  unsigned int GetID() { return id; };
  bool IsSecure() { return is_secure; };
  void *ITLB() { return NULL; };
  void *DTLB() { return NULL; };
  exclMonitor &LocalMonitor() { return local_monitor; };
  unsigned int Asid() { return asid; };
  unsigned long long Clock() { return clock; };
  unsigned long long GP(unsigned int rindex) { return 0; /* GP[rindex].Value(); */ };
  unsigned long long PC() { return 0; /* PC.Value(); */ };
  unsigned long long SP() { return 0; };
  void setGP(unsigned int rindex,unsigned long long rval) { /* GP[rindex].Value(rval); */ };
  void setPC(unsigned long long rval) { /* PC.Value(rval); */ };
  void setSP(unsigned long long rval) { /* Register SP(rval); UpdateSP(SP); */ };

private:
  unsigned int id;
  unsigned int asid;
  bool is_secure;
  exclMonitor local_monitor; // each CPU has a local monitor (holdover from another simulator)
  unsigned long long clock;
};

#endif
#define __CORE_STATE__ 1
