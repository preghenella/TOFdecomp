#include <boost/program_options.hpp>
#include <iostream>
#include <chrono>
#include <csignal>
#include "TOFCompression/Compressor.h"

bool doPrintChecks = false;

void tick(int signal)
{
  doPrintChecks = true;
  std::signal(SIGUSR1, tick);
}

int main(int argc, char **argv)
{

  bool verbose = false, rewind = false;
  std::string inFileName, outFileName;
  int drmid = -1;
  int timer = 0;
   
  /** define arguments **/
  namespace po = boost::program_options;
  po::options_description desc("Options");
  
  try {
    
    desc.add_options()
      ("help", "Print help messages")
      ("verbose,v", po::bool_switch(&verbose)->default_value(false), "Verbose flag")
      ("input,i", po::value<std::string>(&inFileName), "Input data file")
      ("output,o", po::value<std::string>(&outFileName)->default_value("/dev/null"), "Output data file")
      ("timer,t", po::value<int>(&timer)->default_value(0), "Timer for summary of checks")
      ;
    
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    
    /** process arguments **/

    /** help **/
    if (vm.count("help")) {
      std::cout << desc << std::endl;
      return 1;
    }
    po::notify(vm);
    
  }
  catch(std::exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    std::cout << desc << std::endl;
    return 1;
  }
  
  if (inFileName.empty() || outFileName.empty()) {
    std::cout << desc << std::endl;
    return 1;
  }

  /** fork for summary timer **/
  if (timer > 0) {
    int pid = fork();
    if (pid < 0) return 1;
    if (pid > 0) { // send signals
      while (true) {
	sleep(timer);
	kill(pid, SIGUSR1);
      }
    }
    std::signal(SIGUSR1, tick);
  }
    
  /** initialise **/
  o2::tof::Compressor compressor;
  compressor.setDecoderVerbose(verbose);
  compressor.setEncoderVerbose(verbose);
  compressor.setCheckerVerbose(verbose);
  compressor.init();
  if (compressor.open(inFileName, outFileName)) return 1;

  /** chrono **/
  std::chrono::time_point<std::chrono::high_resolution_clock> start, finish;
  std::chrono::duration<double> elapsed;
  double integratedTime;

  /** loop over pages **/
  while (true) {

    /** read page **/
    if (compressor.read())
      break;

    /** get start chrono **/
    start = std::chrono::high_resolution_clock::now();	
    
    /** run **/
    compressor.run();
    
    /** get finish chrono and increment **/
    finish = std::chrono::high_resolution_clock::now();
    elapsed = finish - start;
    integratedTime += elapsed.count();

    /** write page **/
    compressor.write();
    
    if (doPrintChecks) {
      system("clear");
      compressor.checkSummary();
      compressor.resetCounters();
      doPrintChecks = false;
    }
    
  } /** end of loop over pages **/
  
  compressor.checkSummary();
  compressor.close();  
  std::cout << " local benchmark: " << integratedTime << " s" << std::endl;

  return 0;
}

