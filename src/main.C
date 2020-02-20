#include <iostream>
#include <string>
#include <sys/time.h>

#include <dimebox.h>
#include <dimeboxVersion.h>

// used by main only:

int process_options(SimConfig &my_sim_cfg,int argc,char **argv);
int run_simulation(SimConfig *my_sim_cfg);

//*******************************************************************************
// main entry point...
//*******************************************************************************

int main(int argc,char **argv) {
  std::cout << "dimebox/risc-v simulator, version " << dimebox_VERSION_MAJOR << "."
	    << dimebox_VERSION_MINOR << ". Tuleta Software Co. Copyright 2020."
	    << " All rights reserved.\n" << std::endl;

  SimConfig my_sim_cfg;

  if (process_options(my_sim_cfg,argc,argv) ) {
    std::cerr << "One or more errors occurred, simulation aborted." << std::endl;
    return -1;
  }

  return run_simulation(&my_sim_cfg);
}


//*******************************************************************************
// given a simulator config, run a simulation...
//*******************************************************************************

int run_simulation(SimConfig *my_sim_cfg) {
  int rcode = 0;
  
  try {
     RiscvSimulator my_simulator(my_sim_cfg);
     
     std::cout << "\nStarting simulation..." << std::endl;

     struct timeval t1,t2;
     gettimeofday(&t1,NULL);

     rcode = my_simulator.Go();

     gettimeofday(&t2,NULL);

     std::cout << "Simulation ended, rcode: " << rcode << std::endl;

     double elapsed_time = ((t2.tv_sec - t1.tv_sec) * 1000.0)        // pick up milliseconds part 
                             + ((t2.tv_usec - t1.tv_usec) / 1000.0);   //  then microseconds...

     std::cout << "\nElapsed time: " << elapsed_time << " ms.\n";

     int total_instrs = my_simulator.InstrCount();

     if (total_instrs > 0) {
       long instrs_per_seconds = (long)((double) total_instrs * 1000.0 / elapsed_time);
       std::cout << "simulation rate: " << instrs_per_seconds << " instructions per second.\n";
     }

     rcode = my_simulator.WriteTestSignature();
     
  } catch(const std::runtime_error& ex) {
     std::cerr << ex.what() << std::endl;
     rcode = -1;
  }

  return rcode;
}


