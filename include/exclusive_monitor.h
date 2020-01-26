#ifndef __EXCL_MONITOR__

// exclMonitor - maintains exclusive monitor state only.
//               exclusive monitor policy defined elsewhere...

class exclMonitor {
 public:
 exclMonitor() : la(0), pa(0), cpuID(-1), elem_size(0), exclusive(false) {};
  void Set(unsigned long long _la, unsigned long long _pa, int _elem_size, int _cpuID) {
    la = _la; pa = _pa; elem_size = _elem_size; exclusive = true; cpuID = _cpuID;
  };
  void Clear() { exclusive = false; };
  bool IsOpen() { return !exclusive; };
  bool IsExclusive() { return exclusive; };

  bool IsExclusive(unsigned long long _la, unsigned long long _pa, int _elem_size, int _cpuID) {
    return exclusive && (_la==la) && (_pa==pa) && (_elem_size==elem_size) && (_cpuID==cpuID) ;
  };

  unsigned long long LA() { return la; };
  unsigned long long PA() { return pa; };
  int CPUID() { return cpuID; };
  int ElementSize() { return elem_size; };

 private:
  unsigned long long la;         // la,
  unsigned long long pa;         //   pa,
  int                cpuID;      //     cpuID (if applicable) 
  int                elem_size;  //       size of access, (see WORD_SIZES enum)
  bool               exclusive;  //         state of monitor: false if open, true if exclusive
};

#endif
#define __EXCL_MONITOR__ 1

