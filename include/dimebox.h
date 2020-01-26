#include <string>
#include <vector>
#include <unordered_map>

class DimeboxSimConfig {
public:
  DimeboxSimConfig() : reset_address(0), num_cores(1), disasm_enable(true), max_instr_count(1000),gdb_port(0),gdb_core_id(0) {};
  
  struct addr_range {
    addr_range(unsigned long long _address_lo,unsigned long long _address_hi) {
      address_lo = _address_lo;
      address_hi = _address_hi;
    };  
    unsigned long long address_lo;
    unsigned long long address_hi;
  };

  void AddSrcFile(std::string &_src_file);
  void SetResetAddress(unsigned long long _reset_address);
  void SetCoreCount(unsigned int _num_cores);
  unsigned int CoreCount();
  void SetAddressRange(unsigned long long _address_lo,unsigned long long _address_hi);
  void MapDevice(std::string _device,unsigned long long _device_base_address);
  void SetShowProgress(bool _sp);
  void SetMaxInstrs(int max_count);
  void SetDebugServerParms(unsigned int port,unsigned int core_id);

private:
  std::vector<std::string> load_files;
  unsigned long long reset_address;
  unsigned int num_cores;
  std::vector<struct addr_range> dram;
  std::unordered_map<unsigned long long,std::string> devices;
  bool disasm_enable;
  int max_instr_count;
  unsigned int gdb_port;
  unsigned int gdb_core_id;
};

class DimeboxSimulator {
 public:
  DimeboxSimulator() {};
  DimeboxSimulator(struct DimeboxSimConfig &sim_cfg) {};
  ~DimeboxSimulator() {};
  int Go() { return 0; };
  int SimulatedCount() { return 0; };

 private:
};

enum SIM_EXCEPTION_CLASS { NO_SIM_EXCEPTION,    // used in general processing of try blocks
			   EXCEPTION,           // core exception
			   INTERRUPT,           // device interrupt
                           WAIT_FOR_EVENT,      // core goes to sleep
			   WAIT_FOR_INTERRUPT,  // core waiting on interrupt
			   EVENT_NOTICE,        // wakeup core(s)
                           INTERNAL_ERROR,      // dont go there
			   GENERATION_ERROR     // could be useful for test 'generators'
};

enum { FOR_READ, FOR_WRITE, FOR_PREFETCH };

// exclMonitor - maintains exclusive monitor state only. exclusive monitor policy defined elsewhere...

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

// MemoryAccess - used with data read/write operations only, ie, NOT for instruction access...

class MemoryAccess {
 public:
 MemoryAccess(unsigned long long _address, // logical address
              int  _size,                  // total number of bytes 
              int  _type,                  // memory type
              bool _exclusive,             // is exclusive access
              int  _direction,             // read vs write
              bool _big_endian,            // true if big endian
	      int  _word_size,             // element size in bytes
              bool _sign_extend,           // sign-extend after loading
              int  _rwidth,                // when sign-extending, # of bits to extend to - 32 or 64
              bool _paired,                // true if register pair access
              bool _privileged             // privileged
             )            
   : address(_address), size(_size), type(_type), exclusive(_exclusive), direction(_direction), big_endian(_big_endian),
    word_size(_word_size), sign_extend(_sign_extend), rwidth(_rwidth), paired(_paired), privileged(_privileged),
    exclusive_passed(false) {};

  friend std::ostream& operator<< (std::ostream &os, MemoryAccess &ma);

  // SignBit - when sign-extending, which bit is the (input) sign bit 
  int SignBit() { 
    if (word_size==1) return 8;
    if (word_size==2) return 16;
    if (word_size==4) return 32;
    return 64;
  }
  // SignExtendTo - when sign-extending, how far to extend the sign to (either to bit 32 or bit 64)
  int SignExtendTo() {
    return rwidth;
  }

  void ClearBuffer() { for (int i = 0; i < size; i++) { membuffer[i] = 0; } };

  unsigned long long Address() { return address; };
  int Size() { return size; };
  int WordSize() { return word_size; };

  bool Aligned() { return (word_size==1) || ( ((word_size - 1) & address) == 0 ); };
  
  unsigned long long address;     // memory address
  int  size;                      // size of access in bytes
  int  type;                      // see ACCESS_TYPE
  bool exclusive;                 // set if access to exclusive monitor/memory
  int  direction;                 // read (0) vs write (1)
  bool big_endian;                // true if big endianness
      // NOTE: for exclusive register-pair access and little endian need to swap data (word) values
  int  word_size;                 // word size to use for endianness
  bool sign_extend;               // whether or not to sign extend value read from memory
  int rwidth;                     // when sign-extending, # of bits to extend to - 32 or 64
  bool paired;                    // true if paired register access
  bool privileged;                // privileged vs unprivileged
  int  exclusive_passed;          // true if exclusive write succeeds
  unsigned char membuffer[1024];  // source or dest of access - matches memory (and byte order) at address
};

class Packet {
public:
  Packet() {};
  ~Packet() {};
  bool ForTest() { return false; };
  std::vector<MemoryAccess> mOpsMemory;    // memory accesses
private:
};

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


  
  
