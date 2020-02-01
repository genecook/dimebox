#ifndef __RISC_PACKET__

// state related to a single instruction's execution...

class RiscvPacket : public Packet {
public:
  RiscvPacket() : state(NULL) {
  };
  RiscvPacket(RiscvState *_state) {
    state = new RiscvState(_state);
  };
  ~RiscvPacket() {
    if (state != NULL) delete state;
  };
  
  RiscvState *state; // register state
private:
};


#endif
#define __RISC_PACKET__ 1
