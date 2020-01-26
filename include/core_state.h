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
  exclMonitor local_monitor; // each CPU has a local monitor (holdover from another simulator)
  unsigned long long clock;

  GPRegister X[32];    // general purpose registers X1 thru X31
  ProgramCounter _PC;  // PC
};

#endif
#define __CORE_STATE__ 1
