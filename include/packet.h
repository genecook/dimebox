#ifndef __PACKET__

// state related to a single instruction's execution...

class Packet {
public:
  Packet() {};
  ~Packet() {};
  bool ForTest() { return false; };
  std::vector<MemoryAccess> mOpsMemory;    // memory accesses
private:
};


#endif
#define __PACKET__ 1
