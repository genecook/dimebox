#ifndef __MEMORY__

#include <unordered_set>

using namespace std;

enum { FOR_READ, FOR_WRITE, FOR_PREFETCH };

// memory makes use of mmu and tlb...

enum WORD_SIZES { BYTE=0, HWORD=1, WORD=2, DWORD=3, QWORD=4 };
enum ACCESS_TYPE { INSTRUCTION=0, DATA=1, NORMAL=2, ATOMIC=3, ORDERED=4, UNPRIVILEGED=5, STREAM=6, VECTOR=7 };

class Memory {
 public:
  Memory();
  ~Memory();

  int SetPhysicalSize(int _size_in_bits) { size_in_bits = _size_in_bits; return size_in_bits; };
  unsigned long long pa_mask() { return (1ull<<size_in_bits) - 1ull; };

  void SetPhysicalAddressRange(unsigned long long address_lo, unsigned long long address_hi);
  bool PhysicalAddressRange(unsigned long long &address_lo, unsigned long long &address_hi);

  void AddDevice(memmap_device *memory_mapped_device) { devices.push_back(memory_mapped_device); };
  
  int ReadMemory(State *state,unsigned long long address,bool is_data,bool privileged,
		 int number_of_bytes,bool is_aligned,unsigned char *buffer,bool for_test=false,bool init_if_free=true);
  
  int WriteMemory(State *state,unsigned long long address,bool is_data,bool privileged,
		  int number_of_bytes,bool is_aligned,unsigned char *buffer,bool for_test=false);
  
  // endianness is applied when transferring data to/from registers/memory or when reading the next instruction...

  bool HostEndianness() { return host_endianness; };

  // endianness applied after bytes read/written to memory...

  void ApplyEndianness(unsigned char *dest,unsigned char *src,bool big_endian,int access_size,int number_of_bytes);

  bool AccessGlobalMonitor(MMU *mmu,Translation *trans);
  void ClearExclusiveMonitor(State *cpu);
  bool AccessExclusiveMonitor(MMU *mmu,Translation *trans);
  void SetExclusiveMonitor(State *cpu,unsigned long long logical_address,int num_bytes,bool privileged);
  bool IsExclusive(State *cpu,unsigned long long logical_address,int num_bytes,bool privileged);
  bool ExclusiveMonitorsPass(State *cpu,unsigned long long logical_address,int num_bytes,
			     bool is_aligned,bool privileged, bool for_test);

  void TestBlockAddresses(vector<unsigned long long> &test_block_addresses,bool verbose);
  void SetFileBlockAddresses(vector<unsigned long long> &file_block_addresses,vector<unsigned long long> &test_block_addresses,
			     unsigned long long address_mask,bool verbose);

  int AccessType(unsigned long long address);
  int InvalidRegionAccess(unsigned long long address,int instr_vs_data,int read_vs_write,int number_of_bytes,bool init_if_free);

  void ReadInitialState(unsigned long long address,unsigned char *buffer, bool is_data);
  int WritePhysicalMemory(unsigned long long address,int number_of_bytes,unsigned char *buffer, bool for_test, bool initial_values=false);
  int ReadPhysicalMemory(unsigned long long address,int number_of_bytes,unsigned char *buffer, bool for_test, bool init_if_free=true);
 
  unsigned long long ValidatePhysicalMemoryAddress(unsigned long long address, bool for_test);

  bool WriteDeviceMemory(int &rcode,unsigned long long address_in,int number_of_bytes,unsigned char *buffer,bool for_test);
  bool ReadDeviceMemory(int &rcode,unsigned long long address_in,int number_of_bytes,unsigned char *buffer,bool for_test);

protected:
  int ReadMemoryBlock(State *state,unsigned long long address,bool is_data,bool privileged,
		      int number_of_bytes,bool is_aligned,unsigned char *buffer,bool for_test=false,bool init_if_free=true);
  int WriteMemoryBlock(State *state,unsigned long long address,bool is_data,bool privileged,
		       int number_of_bytes,bool is_aligned,unsigned char *buffer,bool for_test);
  
 private:
  int RegionAccess(unsigned char *buffer,unsigned int cpu_id,unsigned long long LA,unsigned long long PA,bool instr_vs_data,
		   int read_vs_write,int byte_count,bool for_test,bool init_if_free=false);

  void BlockSort(int block_size=PM_BLOCK_SIZE);
  void LogRegionAccess(unsigned long long address,int instr_vs_data,int number_of_bytes);
  void ClearBlockStorage();

  int size_in_bits;

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
  bool host_endianness = false; //<---simulator host endianness is little endian
#else
  bool host_endianness = true;  //<---simulator host endianness is big endian
#endif

  exclMonitor global_monitor;          // in this implementation there is only one global monitor 
                                       //   (we assume a single cpu cluster with a single corresponding global monitor)

  PhysicalMemory phys_mem; //<---for now a single contiguous block of physical memory

  int block_size;
  unordered_set<unsigned long long> instr_blocks; // used when writing 
  unordered_set<unsigned long long> data_blocks;  //  to elf files

  unordered_map<unsigned long long,unsigned char *> physical_memory_blocks; //<---used only by Save method

  unsigned long long physical_address_lo, physical_address_hi; //<---"master" physical address range

  vector<memmap_device *> devices;  // any memory mapped devices
};


#endif
#define __MEMORY__ 1
