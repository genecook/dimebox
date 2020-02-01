#ifndef __REGISTERS__

class REGISTER_BASE {
 protected:
  bool is_set;
  bool is_reserved;
 public:
  REGISTER_BASE() : is_set(false), is_reserved(false) {};
  
  bool IsSet() { return is_set; };
  void Set() { is_set = true; };
  
  bool IsReserved() { return is_reserved; };
  void SetReserved(bool do_reserve = true) { is_reserved = do_reserve; };
};

// a cpu 'general purpose'...

class GPRegister : public REGISTER_BASE {
 public:
  GPRegister() {};
  
  unsigned long long Value() { return rval; };
  unsigned long long Value(unsigned long long nval) { rval = nval; Set(); return rval; };

  GPRegister operator = (GPRegister &src) {
    GPRegister dd;
    dd.Value(src.Value());
    return dd;
  };
  GPRegister operator = (unsigned long long &src) {
    GPRegister dd;
    dd.Value(src);
    return dd;
  };
  
 private:  
  unsigned long long rval;
};

// a cpu 'register' - floating pt or perhaps simd

class Register : public REGISTER_BASE {
 public:
  Register() {};
  
  Register(unsigned long long nval) : REGISTER_BASE(), rval(nval) {};
  Register(unsigned long long nval_hi,unsigned long long nval_lo) : REGISTER_BASE(), rval_hi(nval_hi),rval(nval_lo) {};

  unsigned long long Value() { return rval; };
  unsigned long long ValueHi() { return rval_hi; };
  
  unsigned long long Value(unsigned long long nval) { rval = nval; Set(); return rval; };
  unsigned long long ValueHi(unsigned long long nval_hi) { rval_hi = nval_hi; Set(); return rval_hi; };
  
 protected:
  unsigned long long rval_hi;  // extended value when asimd
  unsigned long long rval;     // value
};

enum float_type { FL_UNKNOWN, FL_BYTE, FL_HALFWORD, FL_SINGLE, FL_DOUBLE, FL_QUAD };

class FloatRegister : public Register {
 public:
 FloatRegister() : Register(), ftype(FL_UNKNOWN) {};

  int Type() { return ftype; };
  int SetType(float_type _ftype) { ftype = _ftype; return ftype; };
  
  union flpack {
        float fl_foo[2];
        double db_foo;      
        unsigned long long ull_foo;
  };
 
  float Single(float fval) { 
    flpack uval;
    uval.ull_foo = 0ull;
    uval.fl_foo[0] = fval; // value (when accessed) will be low order 32 bits of rval
    rval = uval.ull_foo;
    rval_hi = 0ull;
    ftype = FL_SINGLE; 

    return Single();
  };

  unsigned int Unsigned(unsigned int fval) { 
    rval = (unsigned long long) fval;
    rval_hi = 0ull;

    return rval;
  };

  unsigned int Half(unsigned long long hval) { 
    rval = hval;
    rval_hi = 0ull;

    return rval;
  };

  double Double(double fval) {
    flpack uval;
    uval.db_foo = fval;
    rval = uval.ull_foo;
    rval_hi = 0ull;
    ftype = FL_DOUBLE;

    return Double();
  };

  unsigned long long UnsignedLong(unsigned long long fval) {
    rval = fval;
    rval_hi = 0ull;

    return rval;
  };

  float Single() {
    flpack uval;
    uval.ull_foo = rval;
    return uval.fl_foo[0];
  }
  
  double Double() {
    flpack uval;
    uval.ull_foo = rval;
    return uval.db_foo;
  }
  
  double DoubleHi() {
    flpack uval;
    uval.ull_foo = rval_hi;
    return uval.db_foo;
  }

 private:
  float_type ftype;
};

class ProgramCounter : public REGISTER_BASE {
 public:
  ProgramCounter() {};

  unsigned long long Value() { validate(); return rval; };
  unsigned long long Value(unsigned long long nval) { rval = nval; Set(); return rval; };

  ProgramCounter& operator=(unsigned long long rhs ) { this->Value(rhs); Set(); return *this; };
  
 private:
  void validate() { if (!IsSet()) throw INTERNAL_ERROR; };
  unsigned long long rval; // value
};

#endif
#define __REGISTERS__ 1

