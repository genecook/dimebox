#ifndef __MMU__

class Translation {
public:
  Translation() {};
  Translation(unsigned long long _la,unsigned int _asid,bool _privileged, bool _ns,bool _is_data,
	      bool _aligned, int _size, bool _is_write, bool _big_endian)
    : la(_la), pa(_la), asid(_asid), privileged(_privileged), ns(_ns), is_data(_is_data),
      aligned(_aligned), size(_size), is_write(_is_write), big_endian(_big_endian) {};
 Translation(Translation &src_trans,int byte_count);
  ~Translation() {};
  unsigned long long LA() { return la; };
  unsigned long long PA() { return pa; };
  bool PageCrossed(int _byte_count) { return false; };
  bool Shareable() { return true; };
  bool Cacheable() { return true; };
private:
  unsigned long long la;
  unsigned long long pa;
  unsigned int asid;
  bool privileged;
  bool ns;
  bool is_data;
  bool aligned;
  int size;
  bool is_write;
  bool big_endian;
};

class Memory;

class MMU {
public:
  MMU(State *_cpu, Memory *_memory, Packet *_packet, void *_itlb, void *_dtlb)
    :cpu(_cpu),memory(_memory),packet(_packet),itlb(itlb),dtlb(_dtlb) {};
  ~MMU() {};
  Translation *LA2PA(unsigned long long _la,bool _ns,bool _privileged, bool _is_data, bool _aligned, int _size, bool _is_write) {
    bool trans_endian = _is_data ? BigEndian() : false; // endianness insofar as a translation goes is only relevent for data...
    return new Translation(_la,cpu->Asid(),_privileged,_ns,_is_data,_aligned,_size,_is_write,trans_endian);
  };
  bool BigEndian() { return false; }; // endianness may apply to data accesses for risc-V?
  bool Enabled() { return false; };
  bool Cacheable(bool is_data) { return true; };
private:
  State  *cpu;
  Memory *memory;
  Packet *packet;
  void   *itlb;
  void   *dtlb;
};

#endif
#define __MMU__ 1
