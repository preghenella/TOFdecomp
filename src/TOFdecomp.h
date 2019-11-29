#ifndef _TOF_DECO_H_
#define _TOF_DECO_H_

#include <fstream>
#include <string>
#include <cstdint>
#include "dataFormat.h"

namespace tof {
namespace data {

class TOFdecomp {
  
public:
  
  TOFdecomp();
  ~TOFdecomp();
  
  bool init();
  void rewind() { decoderRewind(); encoderRewind(); };
  bool open(std::string inFileName, std::string outFileName);
  bool close();
  inline bool read()  { return decoderRead(); };
  inline bool write() { return encoderWrite(); };
  
  bool decodeRDH();
  bool decode();
  void checkSummary();
  
#ifdef DECODER_VERBOSE
  void setDecoderVerbose(bool val) { mDecoderVerbose = val; };
#endif
#ifdef ENCODER_VERBOSE
  void setEncoderVerbose(bool val) { mEncoderVerbose = val; };
#endif
#ifdef CHECKER_VERBOSE
  void setCheckerVerbose(bool val) { mCheckerVerbose = val; };
#endif
  
  void setDecoderBufferSize(long val) { mDecoderBufferSize = val; };
  void setEncoderBufferSize(long val) { mEncoderBufferSize = val; };

  void setDRM(int val) {mDRM = val;};
  summary::RawSummary_t &getRawSummary() {return mRawSummary;};
  
  // benchmarks
  double mIntegratedBytes = 0.;
  double mIntegratedTime = 0.;
  
protected:

  /** decoder stuff **/
  
  bool decoderInit();
  bool decoderOpen(std::string name);
  bool decoderRead();
  bool decoderClose();
  inline void decoderRewind() { mDecoderPointer = (uint32_t *)mDecoderBuffer; mDecoderByteCounter = 0; };
  inline void decoderClear();
  inline void decoderNext128();
  inline void decoderNext32();

  std::ifstream mDecoderFile;
  char         *mDecoderBuffer      = nullptr;
  long          mDecoderBufferSize  = 8192;
  uint32_t     *mDecoderPointer     = nullptr;
#ifdef DECODER_VERBOSE
  bool          mDecoderVerbose     = false;
#endif
  uint32_t      mDecoderNextWord    = 1;
  uint32_t      mDecoderByteCounter = 0;

  /** encoder stuff **/
  
  bool encoderInit();
  bool encoderOpen(std::string name);
  bool encoderWrite();
  inline bool encoderClose();
  inline void encoderRewind() { mEncoderPointer = (uint32_t *)mEncoderBuffer; mEncoderByteCounter = 0; };
  inline void encoderNext32();

  std::ofstream mEncoderFile;
  char         *mEncoderBuffer      = nullptr;
  long          mEncoderBufferSize  = 8192;
  uint32_t     *mEncoderPointer     = nullptr;
#ifdef ENCODER_VERBOSE
  bool          mEncoderVerbose     = false;
#endif
  uint32_t      mEncoderNextWord    = 1;
  uint32_t      mEncoderByteCounter = 0;
  
  /** checker stuff **/
  
#ifdef CHECKER_VERBOSE
  bool          mCheckerVerbose     = false;
#endif
  uint32_t                     mCounter;
  counters::DRMCounters_t      mDRMCounters;
  counters::TRMCounters_t      mTRMCounters[10];
  counters::TRMChainCounters_t mTRMChainCounters[10][2];

  
  /** common stuff **/

  void spider();
  bool check();
  
  raw::RDH_t *mRDH;  
  summary::RawSummary_t mRawSummary = {0};    
  uint32_t mDRM = -1;
    
};

}}

#endif /** _TOF_RAW_DATA_DECODER_H **/
