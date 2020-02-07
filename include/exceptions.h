#ifndef __EXCEPTIONS__

enum SIM_EXCEPTIONS { NO_SIM_EXCEPTION,    // used in general processing of try blocks
		      EXCEPTION,           // core exception
		      INTERRUPT,           // device interrupt
                      WAIT_FOR_EVENT,      // core goes to sleep
		      WAIT_FOR_INTERRUPT,  // core waiting on interrupt
		      EVENT_NOTICE,        // wakeup core(s)
		      IRQ,                 // interrupt
		      FIQ,                 // fast interrupt
                      INTERNAL_ERROR,      // dont go there
		      GENERATION_ERROR     // could be useful for test 'generators'
};

enum EXCEPTION_TYPES { NO_EXCEPTION,
		       UNIMPLEMENTED_INSTRUCTION=1  // unimplemented or unknown instruction encoding
};

class Signals {
public:
  Signals() : flags(0) {};
  Signals(Signals &rhs) {};
  Signals(Signals *rhs) {};
  void Exception(unsigned int _exc_flag) { flags |= _exc_flag; };
private:
  unsigned int flags;
};

#endif
#define __EXCEPTIONS__ 1


