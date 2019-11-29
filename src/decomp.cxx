#include <boost/program_options.hpp>
#include <iostream>
#include <chrono>
#include "TOFdecomp.h"

int main(int argc, char **argv)
{

  bool verbose = false, rewind = false;
  std::string inFileName, outFileName;
  int drmid = -1;
  
  /** define arguments **/
  namespace po = boost::program_options;
  po::options_description desc("Options");
  
  try {
    
    desc.add_options()
      ("help", "Print help messages")
#if defined(DECODER_VERBOSE) || defined(ENCODER_VERBOSE) || defined(CHECKER_VERBOSE)
      ("verbose,v", po::bool_switch(&verbose), "Verbose flag")
#endif
      ("verbose,v", po::bool_switch(&verbose), "Verbose flag")
      ("input,i", po::value<std::string>(&inFileName), "Input data file")
      ("output,o", po::value<std::string>(&outFileName), "Output data file")
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
  
  tof::data::TOFdecomp decomp;
#ifdef DECODER_VERBOSE
  decomp.setDecoderVerbose(verbose);
#endif
#ifdef ENCODER_VERBOSE
  decomp.setEncoderVerbose(verbose);
#endif
#ifdef CHECKER_VERBOSE
  decomp.setCheckerVerbose(verbose);
#endif
  decomp.init();
  if (decomp.open(inFileName, outFileName)) return 1;

  /** chrono **/
  std::chrono::time_point<std::chrono::high_resolution_clock> start, finish;
  std::chrono::duration<double> elapsed;
  double integratedTime;
 
  /** loop over pages **/
  while (!decomp.read()) {

    /** get start chrono **/
    start = std::chrono::high_resolution_clock::now();	
    
    /** decode RDH open **/
    decomp.decodeRDH();
    
    /** decode loop **/
    for (int i = 0; i < 3 && !decomp.decode(); ++i) {
      decomp.write();
    }
    /** end of decode loop **/

    /** pause chrono for IO operation **/
    finish = std::chrono::high_resolution_clock::now();
    elapsed = finish - start;
    integratedTime += elapsed.count();
    
    /** read page **/
    decomp.read();

    /** restart chrono after IO operation **/
    start = std::chrono::high_resolution_clock::now();
    
    /** decode RDH close **/
    decomp.decodeRDH();
    
    /** get finish chrono and increment **/
    finish = std::chrono::high_resolution_clock::now();
    elapsed = finish - start;
    integratedTime += elapsed.count();
    
  } /** end of loop over pages **/
  
  decomp.close();
  decomp.checkSummary();
  
  std::cout << " local benchmark: " << integratedTime << " s" << std::endl;

  return 0;
}

