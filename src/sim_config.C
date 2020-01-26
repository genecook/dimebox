#include <dimebox.h>

void DimeboxSimConfig::AddSrcFile(std::string &_src_file) { load_files.push_back(_src_file); }

void DimeboxSimConfig::SetResetAddress(unsigned long long _reset_address) { reset_address = _reset_address; }
unsigned long long DimeboxSimConfig::ResetAddress() { return reset_address; };

void DimeboxSimConfig::SetCoreCount(unsigned int _num_cores) { num_cores = _num_cores; }
unsigned int DimeboxSimConfig::CoreCount() { return num_cores; } 

void DimeboxSimConfig::SetAddressRange(unsigned long long _address_lo,unsigned long long _address_hi) {
  dram.push_back(addr_range(_address_lo,_address_hi));
}
    
void DimeboxSimConfig::MapDevice(std::string _device,unsigned long long _device_base_address) {
    devices[_device_base_address] = _device;
}

void DimeboxSimConfig::SetShowProgress(bool _disasm_enable) { disasm_enable = _disasm_enable; }

void DimeboxSimConfig::SetMaxInstrs(int _max_instr_count) { max_instr_count = _max_instr_count;}
 
void DimeboxSimConfig::SetDebugServerParms(unsigned int _port,unsigned int _core_id) {
  int gdb_port = _port;
  int gdb_core_id = _core_id;
}
