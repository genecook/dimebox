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

#endif
#define __EXCEPTIONS__ 1


