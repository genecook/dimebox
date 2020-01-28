#include <dimebox.h>

void SimConfig::AddSrcFile(std::string &_src_file) { load_files.push_back(_src_file); }

void SimConfig::SetResetAddress(unsigned long long _reset_address) { reset_address = _reset_address; }
unsigned long long SimConfig::ResetAddress() { return reset_address; };

void SimConfig::SetCoreCount(unsigned int _num_cores) { num_cores = _num_cores; }
unsigned int SimConfig::CoreCount() { return num_cores; } 

void SimConfig::SetAddressRange(unsigned long long _address_lo,unsigned long long _address_hi) {
  dram.push_back(addr_range(_address_lo,_address_hi));
}
    
void SimConfig::MapDevice(std::string _device,unsigned long long _device_base_address) {
    devices[_device_base_address] = _device;
}

void SimConfig::SetShowProgress(bool _disasm_enable) { disasm_enable = _disasm_enable; }

void SimConfig::SetMaxInstrs(int _max_instr_count) { max_instr_count = _max_instr_count;}
 
void SimConfig::SetDebugServerParms(unsigned int _port,unsigned int _core_id) {
  int gdb_port = _port;
  int gdb_core_id = _core_id;
}
