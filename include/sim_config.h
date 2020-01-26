#ifndef __SIM_CONFIG__

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
  unsigned long long ResetAddress();
  
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

#endif
#define __SIM_CONFIG__ 1

