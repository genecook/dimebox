#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include <iostream>

#include <boost/program_options.hpp>
#include "boost/property_tree/ptree.hpp"
#include "boost/property_tree/info_parser.hpp"
#include "boost/foreach.hpp"

#include <dimebox.h>
#include <dimeboxVersion.h>

namespace 
{
  const size_t COMMAND_LINE_ERROR = 1;
  const size_t SUCCESS = 0;
  const size_t ERROR_UNHANDLED_EXCEPTION = 2;
}

using namespace std;

int process_config_file(DimeboxSimConfig my_sim_cfg,std::string cfg_file);

int process_options(DimeboxSimConfig my_sim_cfg,int argc,char **argv) {
  try {
    namespace po = boost::program_options;
    po::options_description desc("Options");
    
    desc.add_options()
      ("help,h","Print help messages")
      ("version,V","Print tool version")

      ("sim_config_file,C",po::value< vector<string> >(),"Simulation config file")
      ("sim_load_file,L",po::value< vector<string> >(),"Simulation load file")
      ("num_cores,N",po::value<unsigned int>(),"Number of cores to enable")
      ("max_instrs,n",po::value<unsigned int>(),"Maximum number of instructions per core to simulate")
      ("show_disassembly,D","Print address/disassembly for each instruction during simulation")
      ("reset_address",po::value<string>(),"Reset address")
      ("dram_range",po::value<string>(),"DRAM address range")
      ("uart",po::value<string>(),"Instantiate uart, specify (physical) base address for memory mapped registers")
      ("counter",po::value<string>(),"Instantiate system counter module, specify (physical) base address for memory mapped registers")
      ("port,P",po::value<string>(),"Port simulation debug server will listen on")
      ("core,C",po::value<string>(),"ID of processor element simulation debug server will attach to");
    
    po::variables_map vm;
  
    try {
      po::store(po::parse_command_line(argc,argv,desc),vm);

      if (vm.count("version")) {
	 std::cout << "dimebox " << dimebox_VERSION_MAJOR << "." << dimebox_VERSION_MINOR << std::endl;
         return SUCCESS;
      }
      
      if (vm.count("help")) {
         printf("\n");
         printf("    command line options:\n");
	 
         printf("        --help (or -h)                          -- print this help message.\n");
         printf("        --version (or -V)                       -- print tool version information.\n");
	 printf("\n");	 
         printf("        --sim_config_file (or -C) <file path>   -- path to simulation configuration file - (default is none)\n");
	 printf("\n");	 
         printf("        --sim_load_file (or -L) <file path>     -- path to simulation source (ELF) file\n");
	 printf("\n");	 
         printf("        --num_cores (or -N) <count>             -- number of cores to enable (default is 1)\n");
         printf("        --max_instrs (or -n) <count>            -- Maximum number of instructions per core to simulate (default is 1000)\n");
	 printf("\n");
         printf("        --port (or -P) <port>                   -- (TCP) port simulation debug server will listen on\n");
         printf("        --core (or -C) <core>                   -- ID of processor element simulation debug server will attach to\n");
	 printf("\n");
         printf("        --show_disassembly (or -D)              -- Print address/disassembly for each instruction during simulation\n");
	 printf("\n");
	 printf("        --reset_address                         -- Start simulation at this address\n");
	 printf("        --dram_range <addressLo..addressHi>     -- Constrain simulation to this physical address range\n");
	 printf("        --uart <baseAddress>                    -- Instantiate uart, specify (physical) base address for memory mapped registers\n");
	 printf("        --counter <baseAddress>                 -- Instantiate system counter module, specify (physical) base address for memory mapped registers\n");
         return SUCCESS;
      }
    
      if (vm.count("sim_config_file")) {
	 string cfg_file = vm["sim_config_file"].as<string>();
	 process_config_file(my_sim_cfg,cfg_file);
      }

      if (vm.count("sim_load_file")) {
	 vector<string> obj_files = vm["sim_load_file"].as< vector<string> >();
         for (vector<string>::iterator i = obj_files.begin(); i != obj_files.end(); i++) {
	    my_sim_cfg.AddSrcFile(*i);
	 } 
      }

      if (vm.count("num_cores")) {
	unsigned int num_cores = vm["num_cores"].as<unsigned int>();
	if (num_cores > MAX_CPUS) {
	  fprintf(stderr,"The number of cores specified (%u) exceeds the current (hardcoded) maximum # of cores (%u). Program aborted.\n",num_cores,MAX_CPUS);
          return COMMAND_LINE_ERROR;      
	}	  
	my_sim_cfg.SetCoreCount(num_cores);
      }

      if (vm.count("reset_address")) {
	string rs = vm["reset_address"].as<string>();
        unsigned long long reset_address = 0;
	if (sscanf(rs.c_str(),"0x%llx",&reset_address) != 1) {
	    fprintf(stderr,"Invalid reset address specified (value must be specified in hexadecimal format). Program aborted.\n");
            return COMMAND_LINE_ERROR;      
	}
	printf("  Reset address specified: 0x%llx\n",reset_address);
	my_sim_cfg.SetResetAddress(reset_address);
      }

      if (vm.count("dram_range")) {
	string rs = vm["dram_range"].as<string>();
        std::size_t pos = rs.find("..");
        if (pos == std::string::npos) {
	    fprintf(stderr,"Invalid 'dram' address range specified. Program aborted.\n");
            return COMMAND_LINE_ERROR;      
	}
        string addrLo_str = rs.substr(0,pos);
	string addrHi_str = rs.substr(pos + 2);
	unsigned long long address_lo = 0, address_hi = 0;
	if (sscanf(addrLo_str.c_str(),"0x%llx",&address_lo) != 1) {
	    fprintf(stderr,"Invalid 'dram' address range (low address) specified. Program aborted.\n");
            return COMMAND_LINE_ERROR;      
	}
	if (sscanf(addrHi_str.c_str(),"0x%llx",&address_hi) != 1) {
	    fprintf(stderr,"Invalid 'dram' address range (high address) specified. Program aborted.\n");
            return COMMAND_LINE_ERROR;      
	}

	printf("  dram (simulator memory) address range specified: 0x%llx..0x%llx\n",address_lo,address_hi);
	my_sim_cfg.SetAddressRange(address_lo,address_hi);
      }

      if (vm.count("uart")) {
	string rs = vm["uart"].as<string>();
        unsigned long long uart_base_address = 0;
	if (sscanf(rs.c_str(),"0x%llx",&uart_base_address) != 1) {
	    fprintf(stderr,"Invalid uart base address specified (value must be specified in hexadecimal format). Program aborted.\n");
            return COMMAND_LINE_ERROR;      
	}
	printf("  Uart memory-mapped registers base address specified: 0x%llx\n",uart_base_address);
	my_sim_cfg.MapDevice("UART_PL011",uart_base_address);
      }
      
      if (vm.count("counter")) {
	string rs = vm["counter"].as<string>();
        unsigned long long counter_base_address = 0;
	if (sscanf(rs.c_str(),"0x%llx",&counter_base_address) != 1) {
	    fprintf(stderr,"Invalid system counter module base address specified (value must be specified in hexadecimal format). Program aborted.\n");
            return COMMAND_LINE_ERROR;      
	}
	printf("  System counter module memory-mapped registers base address specified: 0x%llx\n",counter_base_address);
	my_sim_cfg.MapDevice("SYSTEM_COUNTER",counter_base_address);
      }
      
      if (vm.count("show_disassembly")) {
	my_sim_cfg.SetShowProgress(true);
      }

      if (vm.count("max_instrs")) {
	int max_count = vm["max_instrs"].as<unsigned int>();
	if (max_count < 0) {
          printf("Note: max_instrs arguments is negative, thus maximum instructions count will NOT be enforced...\n");   
	} else if (max_count == 0) {
          printf("Note: zero specified for max_instrs (maximum # of instructions) argument?\n");
	}
	my_sim_cfg.SetMaxInstrs(max_count);
      }

      int port = -1;
      unsigned int core_id = 0;
      
      if (vm.count("port"))  {
 	 std::string ts = vm["port"].as<string>();
         if (sscanf(ts.c_str(),"%d",&port) != 1) {
 	   fprintf(stderr,"ERROR: for 'port' cmdline arg, was expecting port#. Instead saw: '%s'\n",ts.c_str());
           return COMMAND_LINE_ERROR;
 	 } 
      }

      if (vm.count("core"))  {
 	 std::string ts = vm["core"].as<string>();
         if (sscanf(ts.c_str(),"%d",&core_id) != 1) {
 	   fprintf(stderr,"ERROR: for 'core' cmdline arg, was expecting core ID. Instead saw: '%s'\n",ts.c_str());
           return COMMAND_LINE_ERROR;
 	 }
	 if (((int) core_id) >= my_sim_cfg.CoreCount()) {
 	   fprintf(stderr,"ERROR: for 'core' cmdline arg, core ID specified (%d) is invalid given the core count (%d).\n",
		  core_id,my_sim_cfg.CoreCount());
           return COMMAND_LINE_ERROR;
	 }
      }

      my_sim_cfg.SetDebugServerParms(port,core_id);

      po::notify(vm);
    }
    catch(po::error& e) {
      fprintf(stderr,"ERROR: %s\n",e.what());
      return COMMAND_LINE_ERROR;
    }
  }
  catch(std::exception& e) {
      fprintf(stderr,"Error(s) occurred when processing command line options\n");
      return ERROR_UNHANDLED_EXCEPTION;
  }

  return SUCCESS;
}

namespace pt = boost::property_tree;

int process_config_file(DimeboxSimConfig my_sim_cfg,std::string cfg_file) {
    // using boost ptree, info parser (simplified json format sort of)...

    pt::ptree tree;

    pt::read_info(cfg_file, tree); // parse the entire file contents into tree...

    pt::ptree config = tree.get_child("config"); // file describes simulator configuration...
    
    // pick off main sim cfg items...

    std::string        load_file     = config.get<std::string>("sim_load_file");
    unsigned int       num_cores     = config.get<unsigned int>("num_cores", 1);
    unsigned int       max_instrs    = config.get<unsigned int>("max_instrs", 1000);
    bool               show_disasm   = config.get<bool>("show_disassembly", true);
    unsigned long long reset_address = config.get<unsigned long long>("reset_address", 0);
    
    my_sim_cfg.AddSrcFile(load_file);
    my_sim_cfg.SetCoreCount(num_cores);
    my_sim_cfg.SetMaxInstrs(max_instrs);
    my_sim_cfg.SetShowProgress(show_disasm);
    my_sim_cfg.SetResetAddress(reset_address);

    // pick off dram address ranges, device/debug info...

    for (pt::ptree::iterator child = config.begin(); child != config.end(); child++) {
       if (child->first == "dram_range") {
         pt::ptree unit = child->second;
         unsigned long long addr_lo = unit.get<unsigned long long>("address_lo",0);
         unsigned long long addr_hi = unit.get<unsigned long long>("address_hi",0xffffffff);
         my_sim_cfg.SetAddressRange(addr_lo,addr_hi);
       }
       else if (child->first == "uart") {
         pt::ptree unit = child->second;
         unsigned long long uart_base_addr = unit.get<unsigned long long>("base_address",0);
         my_sim_cfg.MapDevice("uart",uart_base_addr);
       }
       else if (child->first == "counter") {
         pt::ptree unit = child->second;
         unsigned long long counter_base_addr = unit.get<unsigned long long>("base_address",0);
         my_sim_cfg.MapDevice("counter",counter_base_addr);
       }
       else if (child->first == "debug") {
         pt::ptree unit = child->second;
         int port = unit.get<int>("port",-1);
         unsigned int core_id = unit.get<int>("core",0);
         my_sim_cfg.SetDebugServerParms(port,core_id);
       }
    }
    
    return 0;
}

