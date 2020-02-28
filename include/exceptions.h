#ifndef __EXCEPTIONS__

//   bits     indicates...
//   0..3     interrupt or exception type
//   7..4     1==interrupt, 0==exception
//  11..8     interrupt or exception sub-type (used by simulator for debug purposes)

enum SIM_EXCEPTIONS {
		     SUPERVISOR_SWI=0x11,                      // interrupts...
		     MACHINE_SWI=0x13,                         //
		     SUPERVISOR_TIMER_INT=0x15,                //
		     MACHINE_TIMER_INT=0x17,                   //
		     SUPERVISOR_EXTERNAL_INT=0x19,             //
		     MACHINE_EXTERNAL_INT=0x1b,                //
		     MACHINE_EXTERNAL_INT_UART=0x11b,          //
		      
		     INSTRUCTION_ADDRESS_MISALIGNED=0,         // exceptions...
		     INSTRUCTION_ACCESS_FAULT=1,               //
		     ILLEGAL_INSTRUCTION=2,                    //
		     ILLEGAL_INSTRUCTION_UNKNOWN_CSR=0x102,    // unknown csr or 
		     ILLEGAL_INSTRUCTION_PRIVILEGED_CSR=0x202, // access to privileged csr 
		     ILLEGAL_INSTRUCTION_UNKNOWN_INSTR=0x302,  // unknown instruction
		     ILLEGAL_INSTRUCTION_UNIMPL_INSTR=0x402,   // unimplemented instruction
		     BREAKPOINT=3,                             //
		     LOAD_ADDRESS_MISALIGNED=4,                //
		     LOAD_ACCESS_FAULT=5,                      //
		     STORE_ADDRESS_MISALIGNED=6,               //
		     STORE_ACCESS_FAULT=7,                     //
		     ENV_CALL_UMODE=8,                         //
		     ENV_CALL_SMODE=9,                         //
		     ENV_CALL_MMODE=11,                        //
		     INSTRUCTION_PAGE_FAULT=12,                //
		     LOAD_PAGE_FAULT=13,                       //
		     STORE_PAGE_FAULT=15,                      //

		     NO_SIM_EXCEPTION=0,                       //
		     INTERNAL_ERROR=0xf00,                     //
		     GENERATION_ERROR=0xf01,                   //
		     TEST_PASSES=0xf02,                        //
		     TEST_FAILS=0xf03                          //
};

#endif
#define __EXCEPTIONS__ 1


