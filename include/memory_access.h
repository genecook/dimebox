#ifndef __MEMORY_ACCESS__

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

#endif
#define __MEMORY_ACCESS__ 1
