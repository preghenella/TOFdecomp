#include "TOFdecomp.h"
#include <iostream>
#include <chrono>

#ifdef DECODER_VERBOSE
#warning "Building code with DecoderVerbose option. This may limit the speed."
#endif
#ifdef ENCODER_VERBOSE
#warning "Building code with EncoderVerbose option. This may limit the speed."
#endif
#ifdef CHECKER_VERBOSE
#warning "Building code with CheckerVerbose option. This may limit the speed."
#endif

#define colorRed     "\033[1;31m"
#define colorGreen   "\033[1;32m"
#define colorYellow  "\033[1;33m"
#define colorBlue    "\033[1;34m"

namespace tof {
namespace data {

TOFdecomp::TOFdecomp()
{
}

TOFdecomp::~TOFdecomp()
{
  if (mDecoderBuffer) delete [] mDecoderBuffer;
  if (mEncoderBuffer) delete [] mEncoderBuffer;
}

bool
TOFdecomp::init()
{
  if (decoderInit()) return true;
  if (encoderInit()) return true;
  return false;
}

bool
TOFdecomp::open(std::string inFileName, std::string outFileName)
{
  if (decoderOpen(inFileName)) return true;
  if (encoderOpen(outFileName)) return true;
  return false;
}

bool
TOFdecomp::close()
{
  if (decoderClose()) return true;
  if (encoderClose()) return true;
  return false;
}

bool
TOFdecomp::decoderInit()
{
#ifdef DECODER_VERBOSE
  if (mDecoderVerbose) {
    std::cout << colorBlue
	      << "--- INITIALISE DECODER BUFFER: " << mDecoderBufferSize << " bytes"
	      << std::endl;
  }
#endif
  if (mDecoderBuffer) {
    std::cout << colorYellow
	      << "-W- a buffer was already allocated, cleaning\033[0m"
	      << std::endl;
    delete [] mDecoderBuffer;
  }
  mDecoderBuffer = new char[mDecoderBufferSize];
  return false;
}

bool
TOFdecomp::encoderInit()
{
#ifdef ENCODER_VERBOSE
  if (mEncoderVerbose) {
    std::cout << colorBlue
	      << "--- INITIALISE ENCODER BUFFER: " << mEncoderBufferSize << " bytes"
	      << std::endl;
  }
#endif
  if (mEncoderBuffer) {
    std::cout << colorYellow
	      << "-W- a buffer was already allocated, cleaning"
	      << std::endl;
    delete [] mEncoderBuffer;
  }
  mEncoderBuffer = new char[mEncoderBufferSize];
  encoderRewind();
  return false;
}

bool
TOFdecomp::decoderOpen(std::string name)
{
  if (mDecoderFile.is_open()) {
    std::cout << colorYellow
	      << "-W- a file was already open, closing"
	      << std::endl;
    mDecoderFile.close();
  }
  mDecoderFile.open(name.c_str(), std::fstream::in | std::fstream::binary);
  if (!mDecoderFile.is_open()) {
    std::cerr << colorRed
	      << "-E- Cannot open input file: " << name
	      << std::endl;
    return true;
  }
  return false;
}

bool
TOFdecomp::encoderOpen(std::string name)
{
  if (mEncoderFile.is_open()) {
    std::cout << colorYellow
	      << "-W- a file was already open, closing"
	      << std::endl;
    mEncoderFile.close();
  }
  mEncoderFile.open(name.c_str(), std::fstream::out | std::fstream::binary);
  if (!mEncoderFile.is_open()) {
    std::cerr << colorRed << "-E- Cannot open output file: " << name
	      << std::endl;
    return true;
  }
  return false;
}



bool
TOFdecomp::decoderClose()
{
  if (mDecoderFile.is_open()) {
    mDecoderFile.close();
    return false;
  }
  return true;
}

bool
TOFdecomp::encoderClose()
{
  if (mEncoderFile.is_open())
    mEncoderFile.close();
  return false;
}

bool
TOFdecomp::decoderRead()
{
  if (!mDecoderFile.is_open()) {
    std::cout << colorRed << "-E- no input file is open"
	      << std::endl;      
    return true;
  }
  mDecoderFile.read(mDecoderBuffer, mDecoderBufferSize);
  decoderRewind();
  if (!mDecoderFile) {
    std::cout << colorRed << "--- Nothing else to read"
	      << std::endl;
    return true; 
  }
#ifdef DECODER_VERBOSE
  if (mDecoderVerbose) {
    std::cout << colorBlue
	      << "--- DECODER READ PAGE: " << mDecoderBufferSize << " bytes"
	      << std::endl;
  }
#endif
  return false;
}

bool
TOFdecomp::encoderWrite()
{
#ifdef ENCODER_VERBOSE
  if (mEncoderVerbose) {
    std::cout << colorBlue
	      << "--- ENCODER WRITE BUFFER: " << mEncoderByteCounter << " bytes"
	      << std::endl;
  }
#endif
  mEncoderFile.write(mEncoderBuffer, mEncoderByteCounter);
  encoderRewind();
  return false;
}

void
TOFdecomp::decoderClear()
{
  mRawSummary.DRMCommonHeader  = 0x0;
  mRawSummary.DRMOrbitHeader   = 0x0;
  mRawSummary.DRMGlobalHeader  = 0x0;
  mRawSummary.DRMStatusHeader1 = 0x0;
  mRawSummary.DRMStatusHeader2 = 0x0;
  mRawSummary.DRMStatusHeader3 = 0x0;
  mRawSummary.DRMStatusHeader4 = 0x0;
  mRawSummary.DRMStatusHeader5 = 0x0;
  mRawSummary.DRMGlobalTrailer = 0x0;
  mRawSummary.faultFlags = 0x0;
  for (int itrm = 0; itrm < 10; itrm++) {
    mRawSummary.TRMGlobalHeader[itrm]  = 0x0;
    mRawSummary.TRMGlobalTrailer[itrm] = 0x0;
    mRawSummary.HasHits[itrm] = false;
    for (int ichain = 0; ichain < 2; ichain++) {
      mRawSummary.TRMChainHeader[itrm][ichain]  = 0x0;
      mRawSummary.TRMChainTrailer[itrm][ichain] = 0x0;
      mRawSummary.HasErrors[itrm][ichain] = false;
    }}
}

inline void
TOFdecomp::decoderNext32()
{
  mDecoderPointer += mDecoderNextWord;
  mDecoderNextWord = (mDecoderNextWord + 2) % 4;
  mDecoderByteCounter += 4;
}

void
TOFdecomp::encoderNext32()
{
  mEncoderPointer++;
  mEncoderByteCounter += 4;
}

inline void
TOFdecomp::decoderNext128()
{
  mDecoderPointer += 4;
  mRDH = reinterpret_cast<raw::RDH_t *>(mDecoderPointer);
}
  
bool
TOFdecomp::decodeRDH()
{
  mRDH = reinterpret_cast<raw::RDH_t *>(mDecoderPointer);
    
#ifdef DECODER_VERBOSE
  if (mDecoderVerbose) {
    std::cout << colorBlue
	      << "--- DECODE RDH"
	      << std::endl;    
  }
#endif

  mRawSummary.RDHWord0 = mRDH->Word0;
#ifdef DECODER_VERBOSE
  if (mDecoderVerbose) {
    uint32_t BlockLength = mRDH->Word0.BlockLength;
    uint32_t PacketCounter = mRDH->Word0.PacketCounter;
    uint32_t HeaderSize = mRDH->Word0.HeaderSize;
    uint32_t MemorySize = mRDH->Word0.MemorySize;
    printf(" %08x%08x%08x%08x RDH Word0 (MemorySize=%d, PacketCounter=%d) \n", mRDH->Data[3], mRDH->Data[2], mRDH->Data[1], mRDH->Data[0],
	   MemorySize, PacketCounter);
  }
#endif
  decoderNext128();

  mRawSummary.RDHWord1 = mRDH->Word1;
#ifdef DECODER_VERBOSE
  if (mDecoderVerbose) {
    uint32_t TrgOrbit = mRDH->Word1.TrgOrbit;
    uint32_t HbOrbit = mRDH->Word1.HbOrbit;
    printf(" %08x%08x%08x%08x RDH Word1 (TrgOrbit=%d, HbOrbit=%d) \n", mRDH->Data[3], mRDH->Data[2], mRDH->Data[1], mRDH->Data[0],
	   TrgOrbit, HbOrbit);
  }
#endif
  decoderNext128();

  mRawSummary.RDHWord2 = mRDH->Word2;
#ifdef DECODER_VERBOSE
  if (mDecoderVerbose) {
    uint32_t TrgBC = mRDH->Word2.TrgBC;
    uint32_t HbBC = mRDH->Word2.HbBC;
    uint32_t TrgType = mRDH->Word2.TrgType;
    printf(" %08x%08x%08x%08x RDH Word2 (TrgBC=%d, HbBC=%d, TrgType=%d) \n", mRDH->Data[3], mRDH->Data[2], mRDH->Data[1], mRDH->Data[0],
	   TrgBC, HbBC, TrgType);
  }
#endif
  decoderNext128();

  mRawSummary.RDHWord3 = mRDH->Word3;
#ifdef DECODER_VERBOSE
  if (mDecoderVerbose) {
    printf(" %08x%08x%08x%08x RDH Word3 \n", mRDH->Data[3], mRDH->Data[2], mRDH->Data[1], mRDH->Data[0]);
  }
#endif
  decoderNext128();

  return false;
}

bool
TOFdecomp::decode()
{
  
  /** check if we have memory to decode **/
  if ((char *)mDecoderPointer - mDecoderBuffer >= mRawSummary.RDHWord0.MemorySize) {
#ifdef DECODER_VERBOSE
    if (mDecoderVerbose) {
      std::cout << colorYellow
		<< "-W- decode request exceeds memory size: "
		<< (void *)mDecoderPointer << " | " << (void *)mDecoderBuffer << " | " << mRawSummary.RDHWord0.MemorySize 
 		<< std::endl;
    }
#endif
    return true;
  }

#ifdef DECODER_VERBOSE
  if (mDecoderVerbose) {
    std::cout << colorBlue << "--- DECODE EVENT"
	      << std::endl;    
  }
#endif

  /** init decoder **/
  auto start = std::chrono::high_resolution_clock::now();
  mDecoderNextWord = 1;
  decoderClear();
    
  /** check DRM Common Header **/
  if (!IS_DRM_COMMON_HEADER(*mDecoderPointer)) {
#ifdef DECODER_VERBOSE
    printf("%s %08x [ERROR] fatal error \n", colorRed, *mDecoderPointer);
#endif
    return true;
  }
  mRawSummary.DRMCommonHeader = *mDecoderPointer;
#ifdef DECODER_VERBOSE
  if (mDecoderVerbose) {
    auto DRMCommonHeader = reinterpret_cast<raw::DRMCommonHeader_t *>(mDecoderPointer);
    auto Payload = DRMCommonHeader->Payload;
    printf(" %08x DRM Common Header     (Payload=%d) \n", *mDecoderPointer, Payload);
  }
#endif
  decoderNext32();

  /** DRM Orbit Header **/
  mRawSummary.DRMOrbitHeader = *mDecoderPointer;
#ifdef DECODER_VERBOSE
  if (mDecoderVerbose) {
    auto DRMOrbitHeader = reinterpret_cast<raw::DRMOrbitHeader_t *>(mDecoderPointer);
    auto Orbit = DRMOrbitHeader->Orbit;
    printf(" %08x DRM Orbit Header      (Orbit=%d) \n", *mDecoderPointer, Orbit);
  }
#endif
  decoderNext32();    

  /** check DRM Global Header **/
  if (!IS_DRM_GLOBAL_HEADER(*mDecoderPointer)) {
#ifdef DECODER_VERBOSE
    printf("%s %08x [ERROR] fatal error \n", colorRed, *mDecoderPointer);
#endif
    return true;
  }
  mRawSummary.DRMGlobalHeader = *mDecoderPointer;
#ifdef DECODER_VERBOSE
  if (mDecoderVerbose) {
    auto DRMGlobalHeader = reinterpret_cast<raw::DRMGlobalHeader_t *>(mDecoderPointer);
    auto DRMID = DRMGlobalHeader->DRMID;
    printf(" %08x DRM Global Header     (DRMID=%d) \n", *mDecoderPointer, DRMID);
  }
#endif
#ifdef ALLOW_DRMID
  if (mDRM != -1 && GET_DRMGLOBALHEADER_DRMID(*mDecoderPointer) != mDRM)
    return true;
#endif
  decoderNext32();

  /** DRM Status Header 1 **/
  mRawSummary.DRMStatusHeader1 = *mDecoderPointer;
#ifdef DECODER_VERBOSE
  if (mDecoderVerbose) {
    auto DRMStatusHeader1 = reinterpret_cast<raw::DRMStatusHeader1_t *>(mDecoderPointer);
    auto ParticipatingSlotID = DRMStatusHeader1->ParticipatingSlotID;
    auto CBit = DRMStatusHeader1->CBit;
    auto DRMhSize = DRMStatusHeader1->DRMhSize;
    printf(" %08x DRM Status Header 1   (ParticipatingSlotID=0x%03x, CBit=%d, DRMhSize=%d) \n", *mDecoderPointer, ParticipatingSlotID, CBit, DRMhSize);
  }
#endif
  decoderNext32();

  /** DRM Status Header 2 **/
  mRawSummary.DRMStatusHeader2 = *mDecoderPointer;
#ifdef DECODER_VERBOSE
  if (mDecoderVerbose) {
    auto DRMStatusHeader2 = reinterpret_cast<raw::DRMStatusHeader2_t *>(mDecoderPointer);
    auto SlotEnableMask = DRMStatusHeader2->SlotEnableMask;
    auto FaultID = DRMStatusHeader2->FaultID;
    auto RTOBit = DRMStatusHeader2->RTOBit;
    printf(" %08x DRM Status Header 2   (SlotEnableMask=0x%03x, FaultID=%d, RTOBit=%d) \n", *mDecoderPointer, SlotEnableMask, FaultID, RTOBit);
  }
#endif
  decoderNext32();

  /** DRM Status Header 3 **/
  mRawSummary.DRMStatusHeader3 = *mDecoderPointer;
#ifdef DECODER_VERBOSE
  if (mDecoderVerbose) {
    auto DRMStatusHeader3 = reinterpret_cast<raw::DRMStatusHeader3_t *>(mDecoderPointer);
    auto L0BCID = DRMStatusHeader3->L0BCID;
    auto RunTimeInfo = DRMStatusHeader3->RunTimeInfo;
    printf(" %08x DRM Status Header 3   (L0BCID=%d, RunTimeInfo=0x%03x) \n", *mDecoderPointer, L0BCID, RunTimeInfo);
  }
#endif
  decoderNext32();

  /** DRM Status Header 4 **/
  mRawSummary.DRMStatusHeader4 = *mDecoderPointer;
#ifdef DECODER_VERBOSE
  if (mDecoderVerbose) {
    printf(" %08x DRM Status Header 4 \n", *mDecoderPointer);
  }
#endif
  decoderNext32();

  /** DRM Status Header 5 **/
  mRawSummary.DRMStatusHeader5 = *mDecoderPointer;
#ifdef DECODER_VERBOSE
  if (mDecoderVerbose) {
    printf(" %08x DRM Status Header 5 \n", *mDecoderPointer);
  }
#endif
  decoderNext32();

  /** encode Crate Header **/
  *mEncoderPointer  = 0x80000000;
  *mEncoderPointer |= GET_DRMSTATUSHEADER2_SLOTENABLEMASK(mRawSummary.DRMStatusHeader2) << 12;
  *mEncoderPointer |= GET_DRMGLOBALHEADER_DRMID(mRawSummary.DRMGlobalHeader) << 24;
  *mEncoderPointer |= GET_DRMSTATUSHEADER3_L0BCID(mRawSummary.DRMStatusHeader3);
#ifdef ENCODER_VERBOSE
    if (mEncoderVerbose) {
      auto CrateHeader = reinterpret_cast<compressed::CrateHeader_t *>(mEncoderPointer);
      auto BunchID = CrateHeader->BunchID;
      auto DRMID = CrateHeader->DRMID;
      auto SlotEnableMask = CrateHeader->SlotEnableMask;
      printf("%s %08x Crate header          (DRMID=%d, BunchID=%d, SlotEnableMask=0x%x) \n", colorGreen, *mEncoderPointer, DRMID, BunchID, SlotEnableMask);
    }
#endif
  encoderNext32();
    
  /** encode Crate Orbit **/
  *mEncoderPointer = mRawSummary.DRMOrbitHeader;
#ifdef ENCODER_VERBOSE
    if (mEncoderVerbose) {
      auto CrateOrbit = reinterpret_cast<compressed::CrateOrbit_t *>(mEncoderPointer);
      auto OrbitID = CrateOrbit->OrbitID;
      printf("%s %08x Crate orbit           (OrbitID=%d) \n", colorGreen, *mEncoderPointer, OrbitID);
    }
#endif
    encoderNext32();
    
  /** loop over DRM payload **/
  while (true) {

    /** LTM global header detected **/
    if (IS_LTM_GLOBAL_HEADER(*mDecoderPointer)) {
	
#ifdef DECODER_VERBOSE
      if (mDecoderVerbose) {
	printf(" %08x LTM Global Header \n", *mDecoderPointer);
      }
#endif
      decoderNext32();

      /** loop over LTM payload **/
      while (true) {

	/** LTM global trailer detected **/
	if (IS_LTM_GLOBAL_TRAILER(*mDecoderPointer)) {
#ifdef DECODER_VERBOSE
	  if (mDecoderVerbose) {
	    printf(" %08x LTM Global Trailer \n", *mDecoderPointer);
	  }
#endif
	  decoderNext32();
	  break;
	}

#ifdef DECODER_VERBOSE
	if (mDecoderVerbose) {
	  printf(" %08x LTM data \n", *mDecoderPointer);
	}
#endif
	decoderNext32();
      }
	
    }
      
    /** TRM global header detected **/
    if (IS_TRM_GLOBAL_HEADER(*mDecoderPointer) && GET_TRMGLOBALHEADER_SLOTID(*mDecoderPointer) > 2) {
      uint32_t SlotID = GET_TRMGLOBALHEADER_SLOTID(*mDecoderPointer);
      int itrm = SlotID - 3;
      mRawSummary.TRMGlobalHeader[itrm] = *mDecoderPointer;
#ifdef DECODER_VERBOSE
      if (mDecoderVerbose) {
	auto TRMGlobalHeader = reinterpret_cast<raw::TRMGlobalHeader_t *>(mDecoderPointer);
	auto EventWords = TRMGlobalHeader->EventWords;
	auto EventNumber = TRMGlobalHeader->EventNumber;
	auto EBit = TRMGlobalHeader->EBit;
	printf(" %08x TRM Global Header     (SlotID=%d, EventWords=%d, EventNumber=%d, EBit=%01x) \n", *mDecoderPointer, SlotID, EventWords, EventNumber, EBit);
      }
#endif
      decoderNext32();
	
      /** loop over TRM payload **/
      while (true) {

	/** TRM chain-A header detected **/
	if (IS_TRM_CHAINA_HEADER(*mDecoderPointer) && GET_TRMCHAINHEADER_SLOTID(*mDecoderPointer) == SlotID) {
	  int ichain = 0;
	  mRawSummary.TRMChainHeader[itrm][ichain] = *mDecoderPointer;
#ifdef DECODER_VERBOSE
	  if (mDecoderVerbose) {
	    auto TRMChainHeader = reinterpret_cast<raw::TRMChainHeader_t *>(mDecoderPointer);
	    auto BunchID = TRMChainHeader->BunchID;
	    printf(" %08x TRM Chain-A Header    (SlotID=%d, BunchID=%d) \n", *mDecoderPointer, SlotID, BunchID);
	  }
#endif
	  decoderNext32();

	  /** loop over TRM chain-A payload **/
	  while (true) {
	      
	    /** TDC hit detected **/
	    if (IS_TDC_HIT(*mDecoderPointer)) {
	      mRawSummary.HasHits[itrm] = true;
	      auto itdc = GET_TDCHIT_TDCID(*mDecoderPointer);
	      auto ihit = mRawSummary.nTDCUnpackedHits[ichain][itdc];
	      mRawSummary.TDCUnpackedHit[ichain][itdc][ihit] = *mDecoderPointer;
	      mRawSummary.nTDCUnpackedHits[ichain][itdc]++;
#ifdef DECODER_VERBOSE
	      if (mDecoderVerbose) {
		auto TDCUnpackedHit = reinterpret_cast<raw::TDCUnpackedHit_t *>(mDecoderPointer);
		auto HitTime = TDCUnpackedHit->HitTime;
		auto Chan = TDCUnpackedHit->Chan;
		auto TDCID = TDCUnpackedHit->TDCID;
		auto EBit = TDCUnpackedHit->EBit;
		auto PSBits = TDCUnpackedHit->PSBits;
		printf(" %08x TDC Hit               (HitTime=%d, Chan=%d, TDCID=%d, EBit=%d, PSBits=%d \n", *mDecoderPointer, HitTime, Chan, TDCID, EBit, PSBits);
	      }
#endif
	      decoderNext32();
	      continue;
	    }
	      
	    /** TDC error detected **/
	    if (IS_TDC_ERROR(*mDecoderPointer)) {
	      mRawSummary.HasErrors[itrm][ichain] = true;
#ifdef DECODER_VERBOSE
	      if (mDecoderVerbose) {
		printf("%s %08x TDC error \n", colorRed, *mDecoderPointer);
	      }
#endif
	      decoderNext32();
	      continue;
	    }
	      
	    /** TRM chain-A trailer detected **/
	    if (IS_TRM_CHAINA_TRAILER(*mDecoderPointer)) {
	      mRawSummary.TRMChainTrailer[itrm][ichain] = *mDecoderPointer;
#ifdef DECODER_VERBOSE
	      if (mDecoderVerbose) {
		auto TRMChainTrailer = reinterpret_cast<raw::TRMChainTrailer_t *>(mDecoderPointer);
		auto EventCounter = TRMChainTrailer->EventCounter;
		printf(" %08x TRM Chain-A Trailer   (SlotID=%d, EventCounter=%d) \n", *mDecoderPointer, SlotID, EventCounter);
	      }
#endif
	      decoderNext32();
	      break;
	    }
	      
#ifdef DECODER_VERBOSE
	    if (mDecoderVerbose) {
	      printf("%s %08x [ERROR] breaking TRM Chain-A decode stream \n", colorRed, *mDecoderPointer);
	    }
#endif
	    decoderNext32();
	    break;
	      
	  }} /** end of loop over TRM chain-A payload **/	    
	  
	/** TRM chain-B header detected **/
	if (IS_TRM_CHAINB_HEADER(*mDecoderPointer) && GET_TRMCHAINHEADER_SLOTID(*mDecoderPointer) == SlotID) {
	  int ichain = 1;
	  mRawSummary.TRMChainHeader[itrm][ichain] = *mDecoderPointer;
#ifdef DECODER_VERBOSE
	  if (mDecoderVerbose) {
	    auto TRMChainHeader = reinterpret_cast<raw::TRMChainHeader_t *>(mDecoderPointer);
	    auto BunchID = TRMChainHeader->BunchID;
	    printf(" %08x TRM Chain-B Header    (SlotID=%d, BunchID=%d) \n", *mDecoderPointer, SlotID, BunchID);
	  }
#endif
	  decoderNext32();
	    
	  /** loop over TRM chain-B payload **/
	  while (true) {
	      
	    /** TDC hit detected **/
	    if (IS_TDC_HIT(*mDecoderPointer)) {
	      mRawSummary.HasHits[itrm] = true;
	      auto itdc = GET_TDCHIT_TDCID(*mDecoderPointer);
	      auto ihit = mRawSummary.nTDCUnpackedHits[ichain][itdc];
	      mRawSummary.TDCUnpackedHit[ichain][itdc][ihit] = *mDecoderPointer;
	      mRawSummary.nTDCUnpackedHits[ichain][itdc]++;
#ifdef DECODER_VERBOSE
	      if (mDecoderVerbose) {
		auto TDCUnpackedHit = reinterpret_cast<raw::TDCUnpackedHit_t *>(mDecoderPointer);
		auto HitTime = TDCUnpackedHit->HitTime;
		auto Chan = TDCUnpackedHit->Chan;
		auto TDCID = TDCUnpackedHit->TDCID;
		auto EBit = TDCUnpackedHit->EBit;
		auto PSBits = TDCUnpackedHit->PSBits;
		printf(" %08x TDC Hit               (HitTime=%d, Chan=%d, TDCID=%d, EBit=%d, PSBits=%d \n", *mDecoderPointer, HitTime, Chan, TDCID, EBit, PSBits);
	      }
#endif
	      decoderNext32();
	      continue;
	    }
	      
	    /** TDC error detected **/
	    if (IS_TDC_ERROR(*mDecoderPointer)) {
	      mRawSummary.HasErrors[itrm][ichain] = true;
#ifdef DECODER_VERBOSE
	      if (mDecoderVerbose) {
		printf("%s %08x TDC error \n", colorRed, *mDecoderPointer);
	      }
#endif
	      decoderNext32();
	      continue;
	    }
	      
	    /** TRM chain-B trailer detected **/
	    if (IS_TRM_CHAINB_TRAILER(*mDecoderPointer)) {
	      mRawSummary.TRMChainTrailer[itrm][ichain] = *mDecoderPointer;
#ifdef DECODER_VERBOSE
	      if (mDecoderVerbose) {
		auto TRMChainTrailer = reinterpret_cast<raw::TRMChainTrailer_t *>(mDecoderPointer);
		auto EventCounter = TRMChainTrailer->EventCounter;
		printf(" %08x TRM Chain-B Trailer   (SlotID=%d, EventCounter=%d) \n", *mDecoderPointer, SlotID, EventCounter);
	      }
#endif
	      decoderNext32();
	      break;
	    }
	      
#ifdef DECODER_VERBOSE
	    if (mDecoderVerbose) {
	      printf("%s %08x [ERROR] breaking TRM Chain-B decode stream \n", colorRed, *mDecoderPointer);
	    }
#endif
	    decoderNext32();
	    break;
	      
	  }} /** end of loop over TRM chain-A payload **/	    
	  
	/** TRM global trailer detected **/
	if (IS_TRM_GLOBAL_TRAILER(*mDecoderPointer)) {
	  mRawSummary.TRMGlobalTrailer[itrm] = *mDecoderPointer;
#ifdef DECODER_VERBOSE
	  if (mDecoderVerbose) {
	    auto TRMGlobalTrailer = reinterpret_cast<raw::TRMGlobalTrailer_t *>(mDecoderPointer);
	    auto EventCRC = TRMGlobalTrailer->EventCRC;
	    auto LBit = TRMGlobalTrailer->LBit;
	    printf(" %08x TRM Global Trailer    (SlotID=%d, EventCRC=%d, LBit=%d) \n", *mDecoderPointer, SlotID, EventCRC, LBit);
	  }
#endif
	  decoderNext32();

	  /** encoder SPIDER **/
	  if (mRawSummary.HasHits[itrm]) {

	    spider();
	    
	    /** loop over frames **/
	    for (int iframe = mRawSummary.FirstFilledFrame; iframe < mRawSummary.LastFilledFrame + 1; iframe++) {
		
	      /** check if frame is empty **/
	      if (mRawSummary.nFramePackedHits[iframe] == 0)
		continue;
		
	      // encode Frame Header
	      *mEncoderPointer  = 0x00000000;
	      *mEncoderPointer |= SlotID << 24;
	      *mEncoderPointer |= iframe << 16;
	      *mEncoderPointer |= mRawSummary.nFramePackedHits[iframe];
#ifdef ENCODER_VERBOSE
	      if (mEncoderVerbose) {
		auto FrameHeader = reinterpret_cast<compressed::FrameHeader_t *>(mEncoderPointer);
		auto NumberOfHits = FrameHeader->NumberOfHits;
		auto FrameID = FrameHeader->FrameID;
		auto TRMID = FrameHeader->TRMID;
		printf("%s %08x Frame header          (TRMID=%d, FrameID=%d, NumberOfHits=%d) \n", colorGreen, *mEncoderPointer, TRMID, FrameID, NumberOfHits);
	      }
#endif
	      encoderNext32();
	      
	      // packed hits
	      for (int ihit = 0; ihit < mRawSummary.nFramePackedHits[iframe]; ++ihit) {
		*mEncoderPointer = mRawSummary.FramePackedHit[iframe][ihit];
#ifdef ENCODER_VERBOSE
		if (mEncoderVerbose) {
		  auto PackedHit = reinterpret_cast<compressed::PackedHit_t *>(mEncoderPointer);
		  auto Chain = PackedHit->Chain;
		  auto TDCID = PackedHit->TDCID;
		  auto Channel = PackedHit->Channel;
		  auto Time = PackedHit->Time;
		  auto TOT = PackedHit->TOT;
		  printf("%s %08x Packed hit            (Chain=%d, TDCID=%d, Channel=%d, Time=%d, TOT=%d) \n", colorGreen, *mEncoderPointer, Chain, TDCID, Channel, Time, TOT);
		}
#endif
		encoderNext32();
	      }
	      
	      mRawSummary.nFramePackedHits[iframe] = 0;
	    }
	  }
	    
	  /** filler detected **/
	  if (IS_FILLER(*mDecoderPointer)) {
#ifdef DECODER_VERBOSE
	    if (mDecoderVerbose) {
	      printf(" %08x Filler \n", *mDecoderPointer);
	    }
#endif
	    decoderNext32();
	  }
	    
	  break;
	}
	  
#ifdef DECODER_VERBOSE
	if (mDecoderVerbose) {
	  printf("%s %08x [ERROR] breaking TRM decode stream \n", colorRed, *mDecoderPointer);
	}
#endif
	decoderNext32();
	break;
	  
      } /** end of loop over TRM payload **/
	
      continue;
    }
      
    /** DRM global trailer detected **/
    if (IS_DRM_GLOBAL_TRAILER(*mDecoderPointer)) {
      mRawSummary.DRMGlobalTrailer = *mDecoderPointer;
#ifdef DECODER_VERBOSE
      if (mDecoderVerbose) {
	auto DRMGlobalTrailer = reinterpret_cast<raw::DRMGlobalTrailer_t *>(mDecoderPointer);
	auto LocalEventCounter = DRMGlobalTrailer->LocalEventCounter;
	printf(" %08x DRM Global Trailer    (LocalEventCounter=%d) \n", *mDecoderPointer, LocalEventCounter);
      }
#endif
      decoderNext32();

      /** filler detected **/
      if (IS_FILLER(*mDecoderPointer)) {
#ifdef DECODER_VERBOSE
	if (mDecoderVerbose) {
	  printf(" %08x Filler \n", *mDecoderPointer);
	}
#endif
	decoderNext32();
      }

      /** check event **/
      check();

      /** encode Crate Trailer **/
      *mEncoderPointer  = 0x80000000;
      *mEncoderPointer |= mRawSummary.nDiagnosticWords;
      *mEncoderPointer |= GET_DRMGLOBALTRAILER_LOCALEVENTCOUNTER(mRawSummary.DRMGlobalTrailer) << 4;
#ifdef ENCODER_VERBOSE
      if (mEncoderVerbose) {
	auto CrateTrailer = reinterpret_cast<compressed::CrateTrailer_t *>(mEncoderPointer);
	auto EventCounter = CrateTrailer->EventCounter;
	auto NumberOfDiagnostics = CrateTrailer->NumberOfDiagnostics;
	printf("%s %08x Crate trailer         (EventCounter=%d, NumberOfDiagnostics=%d) \n", colorGreen, *mEncoderPointer, EventCounter, NumberOfDiagnostics);
      }
#endif
      encoderNext32();

      /** encode Diagnostic Words **/
      for (int iword = 0; iword < mRawSummary.nDiagnosticWords; ++iword) {
	*mEncoderPointer = mRawSummary.DiagnosticWord[iword];
#ifdef ENCODER_VERBOSE
	if (mEncoderVerbose) {
	  auto Diagnostic = reinterpret_cast<compressed::Diagnostic_t *>(mEncoderPointer);
	  auto SlotID = Diagnostic->SlotID;
	  auto FaultBits = Diagnostic->FaultBits;
	  printf("%s %08x Diagnostic            (SlotID=%d, FaultBits=0x%x) \n", colorGreen, *mEncoderPointer, SlotID, FaultBits);
	}
#endif
	encoderNext32();
      }

      mRawSummary.nDiagnosticWords = 0;

      break;
    }
      
#ifdef DECODER_VERBOSE
    if (mDecoderVerbose) {
      printf("%s %08x [ERROR] trying to recover DRM decode stream \n", colorRed, *mDecoderPointer);
    }
#endif
    decoderNext32();
      
  } /** end of loop over DRM payload **/
    
  auto finish = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> elapsed = finish - start;
    
  mIntegratedBytes += mDecoderByteCounter;
  mIntegratedTime += elapsed.count();
    
#ifdef DECODER_VERBOSE
  if (mDecoderVerbose) {
    std::cout << colorBlue
	      << "--- END DECODE EVENT: " << mDecoderByteCounter << " bytes"
	      << std::endl;
  }
#endif

  return false;
}

void
TOFdecomp::spider()
{
  /** reset packed hits counter **/
  mRawSummary.FirstFilledFrame = 255;
  mRawSummary.LastFilledFrame = 0;
  
  /** loop over TRM chains **/
  for (int ichain = 0; ichain < 2; ++ichain) {
    
    /** loop over TDCs **/
    for (int itdc = 0; itdc < 15; ++itdc) {
      
      auto nhits = mRawSummary.nTDCUnpackedHits[ichain][itdc];
      if (nhits == 0)
	continue;
      
      /** loop over hits **/
      for (int ihit = 0; ihit < nhits; ++ihit) {
	
	auto lhit = mRawSummary.TDCUnpackedHit[ichain][itdc][ihit];
	if (GET_TDCHIT_PSBITS(lhit) != 0x1)
	  continue; // must be a leading hit
	
	auto Chan    = GET_TDCHIT_CHAN(lhit);
	auto HitTime = GET_TDCHIT_HITTIME(lhit);
	auto EBit    = GET_TDCHIT_EBIT(lhit);
	uint32_t TOTWidth = 0;
	
	// check next hits for packing
	for (int jhit = ihit + 1; jhit < nhits; ++jhit) {
	  auto thit = mRawSummary.TDCUnpackedHit[ichain][itdc][jhit];
	  if (GET_TDCHIT_PSBITS(thit) == 0x2 && GET_TDCHIT_CHAN(thit) == Chan) { // must be a trailing hit from same channel
	    TOTWidth = GET_TDCHIT_HITTIME(thit) - HitTime; // compute TOT
	    lhit = 0x0; // mark as used
	    break;
	  }
	}
	
	auto iframe = HitTime >> 13;
	auto phit = mRawSummary.nFramePackedHits[iframe];
	
	mRawSummary.FramePackedHit[iframe][phit]  = 0x00000000;
	mRawSummary.FramePackedHit[iframe][phit] |= (TOTWidth & 0x7FF) <<  0;
	mRawSummary.FramePackedHit[iframe][phit] |= (HitTime  & 0x1FFF) << 11;
	mRawSummary.FramePackedHit[iframe][phit] |= Chan << 24;
	mRawSummary.FramePackedHit[iframe][phit] |= itdc << 27;
	mRawSummary.FramePackedHit[iframe][phit] |= ichain << 31;
	mRawSummary.nFramePackedHits[iframe]++;
	
	if (iframe < mRawSummary.FirstFilledFrame)
	  mRawSummary.FirstFilledFrame = iframe;
	if (iframe > mRawSummary.LastFilledFrame)
	  mRawSummary.LastFilledFrame = iframe;
	
      }
      
      mRawSummary.nTDCUnpackedHits[ichain][itdc] = 0;
    }
  }
  
}

bool
TOFdecomp::check()
{
  bool status = false;
  mRawSummary.nDiagnosticWords = 0;
  mRawSummary.DiagnosticWord[0] = 0x00000001;
  //  mRawSummary.CheckStatus = false;
  mCounter++;
  
  auto start = std::chrono::high_resolution_clock::now();
   
#ifdef CHECKER_VERBOSE
    if (mCheckerVerbose) {
      std::cout << colorBlue
		<< "--- CHECK EVENT"
		<< std::endl;    
    }
#endif

    /** increment check counter **/
    //    mCheckerCounter++;
    
    /** check DRM Global Header **/
    if (mRawSummary.DRMGlobalHeader == 0x0) {
      status = true;
      mRawSummary.faultFlags |= 1;
      mRawSummary.DiagnosticWord[0] |= DIAGNOSTIC_DRM_HEADER;
      mRawSummary.nDiagnosticWords++;
#ifdef CHECKER_VERBOSE
      if (mCheckerVerbose) {
	printf(" Missing DRM Global Header \n");
      }
#endif
      return status;
    }
    
    /** check DRM Global Trailer **/
    if (mRawSummary.DRMGlobalTrailer == 0x0) {
      status = true;
      mRawSummary.faultFlags |= 1;
      mRawSummary.DiagnosticWord[0] |= DIAGNOSTIC_DRM_TRAILER;
      mRawSummary.nDiagnosticWords++;
#ifdef CHECKER_VERBOSE
      if (mCheckerVerbose) {
	printf(" Missing DRM Global Trailer \n");
      }
#endif
      return status;
    }

    /** increment DRM header counter **/
    mDRMCounters.Headers++;
      
    /** get DRM relevant data **/
    uint32_t ParticipatingSlotID = GET_DRMSTATUSHEADER1_PARTICIPATINGSLOTID(mRawSummary.DRMStatusHeader1);
    uint32_t SlotEnableMask      = GET_DRMSTATUSHEADER2_SLOTENABLEMASK(mRawSummary.DRMStatusHeader2);
    uint32_t L0BCID              = GET_DRMSTATUSHEADER3_L0BCID(mRawSummary.DRMStatusHeader3);
    uint32_t LocalEventCounter   = GET_DRMGLOBALTRAILER_LOCALEVENTCOUNTER(mRawSummary.DRMGlobalTrailer);

    if (ParticipatingSlotID != SlotEnableMask) {
#ifdef CHECKER_VERBOSE
      if (mCheckerVerbose) {
	printf(" Warning: enable/participating mask differ: %03x/%03x \n", SlotEnableMask, ParticipatingSlotID);
      }
#endif
      mRawSummary.DiagnosticWord[0] |= DIAGNOSTIC_DRM_ENABLEMASK;
    }
    
    /** check DRM CBit **/
    if (GET_DRMSTATUSHEADER1_CBIT(mRawSummary.DRMStatusHeader1)) {
      status = true;
      mRawSummary.faultFlags |= 1;
      mDRMCounters.CBit++;
      mRawSummary.DiagnosticWord[0] |= DIAGNOSTIC_DRM_CBIT;
#ifdef CHECKER_VERBOSE
      if (mCheckerVerbose) {
	printf(" DRM CBit is on \n");
      }
#endif	
    }
      
    /** check DRM FaultID **/
    if (GET_DRMSTATUSHEADER2_FAULTID(mRawSummary.DRMStatusHeader2)) {
      status = true;
      mRawSummary.faultFlags |= 1;
      mDRMCounters.Fault++;
      mRawSummary.DiagnosticWord[0] |= DIAGNOSTIC_DRM_FAULTID;
#ifdef CHECKER_VERBOSE
      if (mCheckerVerbose) {
	printf(" DRM FaultID: %x \n", GET_DRMSTATUSHEADER2_FAULTID(mRawSummary.DRMStatusHeader2));
      }
#endif	
    }
      
    /** check DRM RTOBit **/
    if (GET_DRMSTATUSHEADER2_RTOBIT(mRawSummary.DRMStatusHeader2)) {
      status = true;
      mRawSummary.faultFlags |= 1;
      mDRMCounters.RTOBit++;
      mRawSummary.DiagnosticWord[0] |= DIAGNOSTIC_DRM_RTOBIT;
#ifdef CHECKER_VERBOSE
      if (mCheckerVerbose) {
	printf(" DRM RTOBit is on \n");
      }
#endif	
    }

    /** loop over TRMs **/
    for (int itrm = 0; itrm < 10; ++itrm) {
      uint32_t SlotID = itrm + 3;
      uint32_t trmFaultBit = 1 << (1 + itrm * 3);

      /** check current diagnostic word **/
      auto iword = mRawSummary.nDiagnosticWords;
      if (mRawSummary.DiagnosticWord[iword] & 0xFFFFFFF0) {
	mRawSummary.nDiagnosticWords++;
	iword++;
      }
      
      /** set current slot id **/
      mRawSummary.DiagnosticWord[iword] = SlotID;
      
      /** check participating TRM **/
      if (!(ParticipatingSlotID & 1 << (itrm + 1))) {
	if (mRawSummary.TRMGlobalHeader[itrm] != 0x0) {
	  status = true;
	  mRawSummary.DiagnosticWord[iword] |= DIAGNOSTIC_TRM_UNEXPECTED;
#ifdef CHECKER_VERBOSE
	  if (mCheckerVerbose) {
	    printf(" Non-participating header found (SlotID=%d) \n", SlotID);	
	  }
#endif
	}
	mRawSummary.faultFlags |= trmFaultBit;
       	continue;
      }
      
      /** check TRM Global Header **/
      if (mRawSummary.TRMGlobalHeader[itrm] == 0x0) {
	status = true;
	mRawSummary.faultFlags |= trmFaultBit;
	mRawSummary.DiagnosticWord[iword] |= DIAGNOSTIC_TRM_HEADER;
#ifdef CHECKER_VERBOSE
	if (mCheckerVerbose) {
	  printf(" Missing TRM Header (SlotID=%d) \n", SlotID);
	}
#endif
	continue;
      }

      /** check TRM Global Trailer **/
      if (mRawSummary.TRMGlobalTrailer[itrm] == 0x0) {
	status = true;
	mRawSummary.faultFlags |= trmFaultBit;
	mRawSummary.DiagnosticWord[iword] |= DIAGNOSTIC_TRM_TRAILER;
#ifdef CHECKER_VERBOSE
	if (mCheckerVerbose) {
	  printf(" Missing TRM Trailer (SlotID=%d) \n", SlotID);
	}
#endif
	continue;
      }

      /** increment TRM header counter **/
      mTRMCounters[itrm].Headers++;

      /** check TRM empty flag **/
      if (!mRawSummary.HasHits[itrm])
	mTRMCounters[itrm].Empty++;
      
      /** check TRM EventCounter **/
      uint32_t EventCounter = GET_TRMGLOBALHEADER_EVENTNUMBER(mRawSummary.TRMGlobalHeader[itrm]);
      if (EventCounter != LocalEventCounter % 1024) {
	status = true;
	mRawSummary.faultFlags |= trmFaultBit;
	mTRMCounters[itrm].EventCounterMismatch++;
	mRawSummary.DiagnosticWord[iword] |= DIAGNOSTIC_TRM_EVENTCOUNTER;
#ifdef CHECKER_VERBOSE
	if (mCheckerVerbose) {
	  printf(" TRM EventCounter / DRM LocalEventCounter mismatch: %d / %d (SlotID=%d) \n", EventCounter, LocalEventCounter, SlotID);
	}
#endif
	continue;
      }

      /** check TRM EBit **/
      if (GET_TRMGLOBALHEADER_EBIT(mRawSummary.TRMGlobalHeader[itrm])) {
	status = true;
	mRawSummary.faultFlags |= trmFaultBit;
	mTRMCounters[itrm].EBit++;
	mRawSummary.DiagnosticWord[iword] |= DIAGNOSTIC_TRM_EBIT;
#ifdef CHECKER_VERBOSE
	if (mCheckerVerbose) {
	  printf(" TRM EBit is on (SlotID=%d) \n", SlotID);
	}
#endif	
      }
      
      /** loop over TRM chains **/
      for (int ichain = 0; ichain < 2; ichain++) {
	uint32_t chainFaultBit = trmFaultBit << (ichain + 1);
	
 	/** check TRM Chain Header **/
	if (mRawSummary.TRMChainHeader[itrm][ichain] == 0x0) {
	  status = true;
	  mRawSummary.faultFlags |= chainFaultBit;
	  mRawSummary.DiagnosticWord[iword] |= DIAGNOSTIC_TRMCHAIN_HEADER(ichain);
#ifdef CHECKER_VERBOSE
	  if (mCheckerVerbose) {
	    printf(" Missing TRM Chain Header (SlotID=%d, chain=%d) \n", SlotID, ichain);
	  }
#endif
	  continue;
	}

 	/** check TRM Chain Trailer **/
	if (mRawSummary.TRMChainTrailer[itrm][ichain] == 0x0) {
	  status = true;
	  mRawSummary.faultFlags |= chainFaultBit;
	  mRawSummary.DiagnosticWord[iword] |= DIAGNOSTIC_TRMCHAIN_TRAILER(ichain);
#ifdef CHECKER_VERBOSE
	  if (mCheckerVerbose) {
	    printf(" Missing TRM Chain Trailer (SlotID=%d, chain=%d) \n", SlotID, ichain);
	  }
#endif
	  continue;
	}

	/** increment TRM Chain header counter **/
	mTRMChainCounters[itrm][ichain].Headers++;

	/** check TDC errors **/
	if (mRawSummary.HasErrors[itrm][ichain]) {
	  status = true;
	  mRawSummary.faultFlags |= chainFaultBit;
	  mTRMChainCounters[itrm][ichain].TDCerror++;
	  mRawSummary.DiagnosticWord[iword] |= DIAGNOSTIC_TRMCHAIN_TDCERRORS(ichain);
#ifdef CHECKER_VERBOSE
	  if (mCheckerVerbose) {
	    printf(" TDC error detected (SlotID=%d, chain=%d) \n", SlotID, ichain);
	  }
#endif	  
	}
	
	/** check TRM Chain EventCounter **/
	auto EventCounter = GET_TRMCHAINTRAILER_EVENTCOUNTER(mRawSummary.TRMChainTrailer[itrm][ichain]);
	if (EventCounter != LocalEventCounter) {
	  status = true;
	  mRawSummary.faultFlags |= chainFaultBit;
	  mTRMChainCounters[itrm][ichain].EventCounterMismatch++;
	  mRawSummary.DiagnosticWord[iword] |= DIAGNOSTIC_TRMCHAIN_EVENTCOUNTER(ichain);
#ifdef CHECKER_VERBOSE
	  if (mCheckerVerbose) {
	    printf(" TRM Chain EventCounter / DRM LocalEventCounter mismatch: %d / %d (SlotID=%d, chain=%d) \n", EventCounter, EventCounter, SlotID, ichain);
	  }
#endif
	}
      
	/** check TRM Chain Status **/
        auto Status = GET_TRMCHAINTRAILER_STATUS(mRawSummary.TRMChainTrailer[itrm][ichain]);
	if (Status != 0) {
	  status = true;
	  mRawSummary.faultFlags |= chainFaultBit;
	  mTRMChainCounters[itrm][ichain].BadStatus++;
	  mRawSummary.DiagnosticWord[iword] |= DIAGNOSTIC_TRMCHAIN_STATUS(ichain);
#ifdef CHECKER_VERBOSE
	  if (mCheckerVerbose) {
	    printf(" TRM Chain bad Status: %d (SlotID=%d, chain=%d) \n", Status, SlotID, ichain);
	  }
#endif	  
	}

	/** check TRM Chain BunchID **/
	uint32_t BunchID = GET_TRMCHAINHEADER_BUNCHID(mRawSummary.TRMChainHeader[itrm][ichain]);
	if (BunchID != L0BCID) {
	  status = true;
	  mRawSummary.faultFlags |= chainFaultBit;
	  mTRMChainCounters[itrm][ichain].BunchIDMismatch++;
	  mRawSummary.DiagnosticWord[iword] |= DIAGNOSTIC_TRMCHAIN_BUNCHID(ichain);
#ifdef CHECKER_VERBOSE
	  if (mCheckerVerbose) {
	    printf(" TRM Chain BunchID / DRM L0BCID mismatch: %d / %d (SlotID=%d, chain=%d) \n", BunchID, L0BCID, SlotID, ichain);
	  }
#endif	  
	}

	
      } /** end of loop over TRM chains **/
    } /** end of loop over TRMs **/

    /** check current diagnostic word **/
    auto iword = mRawSummary.nDiagnosticWords;
    if (mRawSummary.DiagnosticWord[iword] & 0xFFFFFFF0)
      mRawSummary.nDiagnosticWords++;

#ifdef CHECKER_VERBOSE
    if (mCheckerVerbose) {
      std::cout << colorBlue
		<< "--- END CHECK EVENT: " << mRawSummary.nDiagnosticWords << " diagnostic words"
		<< std::endl;
    }
#endif


    auto finish = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = finish - start;

    mIntegratedTime += elapsed.count();
    
    return status;

}

void
TOFdecomp::checkSummary()
{
  char chname[2] = {'a', 'b'};
  
  std::cout << colorBlue
	    <<"--- SUMMARY COUNTERS: " << mCounter << " events"
	    << std::endl;
  if (mCounter == 0) return;
  printf("\n");
  printf("    DRM ");
  float drmheaders = 100. * (float)mDRMCounters.Headers / (float)mCounter;
  printf("  \033%sheaders: %5.1f %%\033[0m ", drmheaders < 100. ? "[1;31m" : "[0m", drmheaders);
  if (mDRMCounters.Headers == 0) return;
  float cbit   = 100. * (float)mDRMCounters.CBit / float(mDRMCounters.Headers);
  printf("     \033%sCbit: %5.1f %%\033[0m ", cbit > 0. ? "[1;31m" : "[0m", cbit);
  float fault  = 100. * (float)mDRMCounters.Fault / float(mDRMCounters.Headers);
  printf("    \033%sfault: %5.1f %%\033[0m ", fault > 0. ? "[1;31m" : "[0m", cbit);
  float rtobit = 100. * (float)mDRMCounters.RTOBit / float(mDRMCounters.Headers);
  printf("   \033%sRTObit: %5.1f %%\033[0m ", rtobit > 0. ? "[1;31m" : "[0m", cbit);
  printf("\n");
  //      std::cout << "-----------------------------------------------------------" << std::endl;
  //      printf("    LTM | headers: %5.1f %% \n", 0.);
  for (int itrm = 0; itrm < 10; ++itrm) {
    printf("\n");
    printf(" %2d TRM ", itrm+3);
    float trmheaders = 100. * (float)mTRMCounters[itrm].Headers / float(mDRMCounters.Headers);
    printf("  \033%sheaders: %5.1f %%\033[0m ", trmheaders < 100. ? "[1;31m" : "[0m", trmheaders);
    if (mTRMCounters[itrm].Headers == 0.) {
      printf("\n");
      continue;
    }
    float empty   = 100. * (float)mTRMCounters[itrm].Empty   / (float)mTRMCounters[itrm].Headers;
    printf("    \033%sempty: %5.1f %%\033[0m ", empty > 0. ? "[1;31m" : "[0m", empty);
    float evCount = 100. * (float)mTRMCounters[itrm].EventCounterMismatch / (float)mTRMCounters[itrm].Headers;
    printf("  \033%sevCount: %5.1f %%\033[0m ", evCount > 0. ? "[1;31m" : "[0m", evCount);
    float ebit = 100. * (float)mTRMCounters[itrm].EBit / (float)mTRMCounters[itrm].Headers;
    printf("     \033%sEbit: %5.1f %%\033[0m ", ebit > 0. ? "[1;31m" : "[0m", ebit);
    printf(" \n");
    for (int ichain = 0; ichain < 2; ++ichain) {
      printf("      %c ", chname[ichain]);
      float chainheaders = 100. * (float)mTRMChainCounters[itrm][ichain].Headers / (float)mTRMCounters[itrm].Headers;
      printf("  \033%sheaders: %5.1f %%\033[0m ", chainheaders < 100. ? "[1;31m" : "[0m", chainheaders);
      if (mTRMChainCounters[itrm][ichain].Headers == 0) {
	printf("\n");
	continue;
      }
      float status = 100. * mTRMChainCounters[itrm][ichain].BadStatus / (float)mTRMChainCounters[itrm][ichain].Headers;
      printf("   \033%sstatus: %5.1f %%\033[0m ", status > 0. ? "[1;31m" : "[0m", status);	
      float bcid = 100. * mTRMChainCounters[itrm][ichain].BunchIDMismatch / (float)mTRMChainCounters[itrm][ichain].Headers;
      printf("     \033%sbcID: %5.1f %%\033[0m ", bcid > 0. ? "[1;31m" : "[0m", bcid);	
      float tdcerr = 100. * mTRMChainCounters[itrm][ichain].TDCerror / (float)mTRMChainCounters[itrm][ichain].Headers;
      printf("   \033%sTDCerr: %5.1f %%\033[0m ", tdcerr > 0. ? "[1;31m" : "[0m", tdcerr);	
      printf("\n");
    }
  }
  printf("\n");
}

}}
