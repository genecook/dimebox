#ifndef __SIM_CONFIG__

#include <string>
#include <vector>
#include <unordered_map>

class SimConfig {
public:
  SimConfig() : reset_address(0), num_cores(1), disasm_enable(false), show_updates(false),
		max_instr_count(1000),gdb_port(0),gdb_core_id(0),
		signature_start_address(0),signature_end_address(0),isa_test(false) {};
  
  struct addr_range {
    addr_range(unsigned long long _address_lo,unsigned long long _address_hi) {
      address_lo = _address_lo;
      address_hi = _address_hi;
    };  
    unsigned long long address_lo;
    unsigned long long address_hi;
  };

  void AddSrcFile(std::string &_src_file);
  void SrcFiles(std::vector<std::string> &elf_files);

  void SetDumpFile(std::string &_dump_file) { dump_file = _dump_file; };
  std::string DumpFile() { return dump_file; };
  
  void SetResetAddress(unsigned long long _reset_address);
  unsigned long long ResetAddress();

  void SetSignatureAddressRange(unsigned long long _address_lo,unsigned long long _address_hi) {
    signature_start_address = _address_lo;
    signature_end_address = _address_hi;
    isa_test = true;
  };
  void SetISAtest() { isa_test = true; };
  bool ISAtest() { return isa_test; };
  unsigned long long SignatureStartAddress() { return signature_start_address; };
  unsigned long long SignatureEndAddress() { return signature_end_address; };
  
  void SetCoreCount(unsigned int _num_cores);
  unsigned int CoreCount();
  
  void SetAddressRange(unsigned long long _address_lo,unsigned long long _address_hi);
  bool DefaultAddressRange(unsigned long long &address_lo,unsigned long long &address_hi) {
    address_lo = 0;
    address_hi = 0xffffffff;
    return true;
  }
  void MapDevice(std::string _device,unsigned long long _device_base_address);
  
  void SetShowProgress(bool _sp);
  bool ShowDisassembly() { return disasm_enable; };

  void SetShowUpdates(bool _updates) { show_updates = _updates; };
  bool ShowUpdates() { return show_updates; };
  
  void SetMaxInstrs(int max_count);
  int MaxInstrs() { return max_instr_count; };

  void SetDebugServerParms(unsigned int port,unsigned int core_id);
  unsigned int DebugPort() { return gdb_port; };
  unsigned int DebugCoreID() { return gdb_core_id; };

  std::vector<std::string> load_files;
  std::string dump_file;
  unsigned long long reset_address;
  unsigned int num_cores;
  std::vector<struct addr_range> dram;
  std::unordered_map<unsigned long long,std::string> devices;
  bool disasm_enable;
  bool show_updates;
  int max_instr_count;
  unsigned int gdb_port;
  unsigned int gdb_core_id;
  unsigned long long signature_start_address;
  unsigned long long signature_end_address;
  bool isa_test;
};

#endif
#define __SIM_CONFIG__ 1

