#ifndef _TOF_DATAFORMAT_H_
#define _TOF_DATAFORMAT_H_

#include <stdint.h>

#define IS_DRM_COMMON_HEADER(x)        ( (x & 0xF0000000) == 0x40000000 )
#define IS_DRM_GLOBAL_HEADER(x)        ( (x & 0xF000000F) == 0x40000001 )
#define IS_DRM_GLOBAL_TRAILER(x)       ( (x & 0xF000000F) == 0x50000001 )
#define IS_LTM_GLOBAL_HEADER(x)        ( (x & 0xF000000F) == 0x40000002 )
#define IS_LTM_GLOBAL_TRAILER(x)       ( (x & 0xF000000F) == 0x50000002 )
#define IS_TRM_GLOBAL_HEADER(x)        ( (x & 0xF0000000) == 0x40000000 )
#define IS_TRM_GLOBAL_TRAILER(x)       ( (x & 0xF0000003) == 0x50000003 )
#define IS_TRM_CHAINA_HEADER(x)        ( (x & 0xF0000000) == 0x00000000 )
#define IS_TRM_CHAINA_TRAILER(x)       ( (x & 0xF0000000) == 0x10000000 )
#define IS_TRM_CHAINB_HEADER(x)        ( (x & 0xF0000000) == 0x20000000 )
#define IS_TRM_CHAINB_TRAILER(x)       ( (x & 0xF0000000) == 0x30000000 )
#define IS_TDC_ERROR(x)                ( (x & 0xF0000000) == 0x60000000 )
#define IS_FILLER(x)                   ( (x & 0xFFFFFFFF) == 0x70000000 )
#define IS_TDC_HIT(x)                  ( (x & 0x80000000) == 0x80000000 )

// DRM getters
#define GET_DRMGLOBALHEADER_DRMID(x)                  ( (x & 0x0FE00000) >> 21 )
#define GET_DRMSTATUSHEADER1_PARTICIPATINGSLOTID(x)   ( (x & 0x00007FF0) >>  4 )
#define GET_DRMSTATUSHEADER1_CBIT(x)     ( (x & 0x00008000) >>  15 )
#define GET_DRMSTATUSHEADER2_SLOTENABLEMASK(x)        ( (x & 0x00007FF0) >>  4 )
#define GET_DRMSTATUSHEADER2_FAULTID(x)  ( (x & 0x07FF0000) >>  16 )
#define GET_DRMSTATUSHEADER2_RTOBIT(x)   ( (x & 0x08000000) >>  27 )
#define GET_DRMSTATUSHEADER3_L0BCID(x)                ( (x & 0x0000FFF0) >>  4 )
#define GET_DRMGLOBALTRAILER_LOCALEVENTCOUNTER(x)     ( (x & 0x0000FFF0) >>  4 )

// TRM getter
#define GET_TRMGLOBALHEADER_SLOTID(x)              ( (x & 0x0000000F) )
#define GET_TRMGLOBALHEADER_EVENTNUMBER(x)         ( (x & 0x07FE0000) >> 17 )
#define GET_TRM_EVENTWORDS(x)          ( (x & 0x0001FFF0) >>  4 )
#define GET_TRMGLOBALHEADER_EBIT(x)    ( (x & 0x08000000) >>  27 )

// TRM Chain getters
#define GET_TRMCHAINHEADER_SLOTID(x)              ( (x & 0x0000000F) )
#define GET_TRMCHAINHEADER_BUNCHID(x)        ( (x & 0x0000FFF0) >>  4 )
#define GET_TRMCHAINTRAILER_EVENTCOUNTER(x)   ( (x & 0x0FFF0000) >> 16 )
#define GET_TRMCHAINTRAILER_STATUS(x)         ( (x & 0x0000000F) )

// TDC getters
#define GET_TDCHIT_HITTIME(x)          ( (x & 0x001FFFFF) )
#define GET_TDCHIT_CHAN(x)             ( (x & 0x00E00000) >>  3 )
#define GET_TDCHIT_TDCID(x)            ( (x & 0x0F000000) >> 24 )
#define GET_TDCHIT_EBIT(x)             ( (x & 0x10000000) >> 29 )
#define GET_TDCHIT_PSBITS(x)           ( (x & 0x60000000) >> 29 )

#define DIAGNOSTIC_DRM_HEADER                   0x80000000
#define DIAGNOSTIC_DRM_TRAILER                  0x40000000
#define DIAGNOSTIC_DRM_CRC                      0x20000000
#define DIAGNOSTIC_DRM_ENABLEMASK               0x08000000
#define DIAGNOSTIC_DRM_CBIT                     0x04000000
#define DIAGNOSTIC_DRM_FAULTID                  0x02000000
#define DIAGNOSTIC_DRM_RTOBIT                   0x01000000

#define DIAGNOSTIC_TRM_HEADER                   0x80000000
#define DIAGNOSTIC_TRM_TRAILER                  0x40000000
#define DIAGNOSTIC_TRM_CRC                      0x20000000
#define DIAGNOSTIC_TRM_UNEXPECTED               0x20000000
#define DIAGNOSTIC_TRM_EVENTCOUNTER             0x08000000
#define DIAGNOSTIC_TRM_EBIT                     0x06000000
#define DIAGNOSTIC_TRM_LBIT                     0x02000000

#define DIAGNOSTIC_TRMCHAIN_HEADER(x)         ( 0x00080000 << (8 * x) )
#define DIAGNOSTIC_TRMCHAIN_TRAILER(x)        ( 0x00040000 << (8 * x) )
#define DIAGNOSTIC_TRMCHAIN_STATUS(x)         ( 0x00020000 << (8 * x) )
#define DIAGNOSTIC_TRMCHAIN_EVENTCOUNTER(x)   ( 0x00008000 << (8 * x) )
#define DIAGNOSTIC_TRMCHAIN_TDCERRORS(x)      ( 0x00004000 << (8 * x) )
#define DIAGNOSTIC_TRMCHAIN_BUNCHID(x)        ( 0x00002000 << (8 * x) )
  
namespace tof {
namespace data {

/** 
 ** RAW DATA FORMAT 
 **/
  
namespace raw {
  
  /** RDH **/

  struct RDHWord_t {
    uint32_t Data[4];
  };
  
  struct RDHWord0_t {
    uint32_t HeaderVersion   :  8;
    uint32_t HeaderSize      :  8;
    uint32_t BlockLength     : 16;
    uint32_t FeeID           : 16;
    uint32_t PriorityBit     :  8;
    uint32_t RESERVED        :  8;
    uint32_t OffsetNewPacket : 16;
    uint32_t MemorySize      : 16;
    uint32_t UnkID           :  8;
    uint32_t PacketCounter   :  8;
    uint32_t CruID           : 12;
    uint32_t DataPathWrapper :  4;
  };

  struct RDHWord1_t {
    uint32_t TrgOrbit        : 32;
    uint32_t HbOrbit         : 32;
    uint32_t RESERVED1       : 32;
    uint32_t RESERVED2       : 32;
  };
  
  struct RDHWord2_t {
    uint32_t TrgBC        : 12;
    uint32_t RESERVED1    :  4;
    uint32_t HbBC         : 12;
    uint32_t RESERVED2    :  4;
    uint32_t TrgType      : 32;
    uint32_t RESERVED3    : 32;
    uint32_t RESERVED4    : 32;
  };

  struct RDHWord3_t {
    uint32_t DetectorField : 16;
    uint32_t Par           : 16;
    uint32_t StopBit       :  8;
    uint32_t PagesCounter  : 16;
    uint32_t RESERVED1     : 32;
    uint32_t RESERVED2     : 32;
    uint32_t RESERVED3     :  8;
  };

  union RDH_t
  {
    uint32_t    Data[4];
    RDHWord0_t  Word0;
    RDHWord1_t  Word1;
    RDHWord2_t  Word2;
    RDHWord3_t  Word3;
  };

  /** DRM data **/
  
  struct DRMCommonHeader_t {
    uint32_t Payload   : 28;
    uint32_t WordType  :  4;
  };
  
  struct DRMOrbitHeader_t {
    uint32_t Orbit     : 32;
  };

  struct DRMGlobalHeader_t
  {
    uint32_t SlotID     :  4;
    uint32_t EventWords : 17;
    uint32_t DRMID      :  7;
    uint32_t WordType   :  4;
  };
  
  struct DRMStatusHeader1_t
  {
    uint32_t SlotID              :  4;
    uint32_t ParticipatingSlotID : 11;
    uint32_t CBit                :  1;
    uint32_t VersID              :  5;
    uint32_t DRMhSize            :  4;
    uint32_t UNDEFINED           :  3;
    uint32_t WordType            :  4;
  };
  
  struct DRMStatusHeader2_t
  {
    uint32_t SlotID         :  4;
    uint32_t SlotEnableMask : 11;
    uint32_t MBZ            :  1;
    uint32_t FaultID        : 11;
    uint32_t RTOBit         :  1;
    uint32_t WordType       :  4;
  };
  
  struct DRMStatusHeader3_t
  {
    uint32_t SlotID      : 4;
    uint32_t L0BCID      : 12;
    uint32_t RunTimeInfo : 12; // check
    uint32_t WordType    : 4;
  };
  
  struct DRMStatusHeader4_t
  {
    uint32_t SlotID      :  4;
    uint32_t Temperature : 10;
    uint32_t MBZ1        :  1;
    uint32_t ACKBit      :  1;
    uint32_t SensAD      :  3;
    uint32_t MBZ2        :  1;
    uint32_t UNDEFINED   :  8;
    uint32_t WordType    :  4;
  };
  
  struct DRMStatusHeader5_t
  {
    uint32_t UNKNOWN    : 32;
  };
  
  struct DRMGlobalTrailer_t
  {
    uint32_t SlotID            :  4;
    uint32_t LocalEventCounter : 12;
    uint32_t UNDEFINED         : 12;
    uint32_t WordType          :  4;
  };
  
  /** TRM data **/
  
  struct TRMGlobalHeader_t
  {
    uint32_t SlotID      :  4;
    uint32_t EventWords  : 13;
    uint32_t EventNumber : 10;
    uint32_t EBit        :  1;
    uint32_t WordType    :  4;
  };
  
  struct TRMGlobalTrailer_t
  {
    uint32_t MustBeThree  :  2;
    uint32_t EventCRC     : 12;
    uint32_t Temp         :  8;
    uint32_t SendAd       :  3;
    uint32_t Chain        :  1;
    uint32_t TSBit        :  1;
    uint32_t LBit         :  1;
    uint32_t WordType     :  4;
  };
  
  /** TRM-chain data **/

  struct TRMChainHeader_t
  {
    uint32_t SlotID   :  4;
    uint32_t BunchID  : 12;
    uint32_t PB24Temp :  8;
    uint32_t PB24ID   :  3;
    uint32_t TSBit    :  1;
    uint32_t WordType :  4;
  };
  
  struct TRMChainTrailer_t
  {
    uint32_t Status       :  4;
    uint32_t MBZ          : 12;
    uint32_t EventCounter : 12;
    uint32_t WordType     :  4;
  };
  
  /** TDC hit **/
  
  struct TDCPackedHit_t
  {
    uint32_t HitTime  : 13;
    uint32_t TOTWidth :  8;
    uint32_t Chan     :  3;
    uint32_t TDCID    :  4;
    uint32_t EBit     :  1;
    uint32_t PSBits   :  2;
    uint32_t MBO      :  1;
  };
  
  struct TDCUnpackedHit_t
  {
    uint32_t HitTime : 21; // leading or trailing edge measurement
    uint32_t Chan    :  3; // TDC channel number
    uint32_t TDCID   :  4; // TDC ID
    uint32_t EBit    :  1; // E bit
    uint32_t PSBits  :  2; // PS bits
    uint32_t MBO     :  1; // must-be-one bit
  };

  /** summary data **/

} /** namespace raw **/

/** 
 ** COMPRESSED DATA FORMAT 
 **/
 
namespace compressed {

  /** generic word **/
  
  struct Word_t
  {
    uint32_t UNDEFINED : 31;
    uint32_t WordType  : 1;
  };

  /** data format **/
  
  struct CrateHeader_t
  {
    uint32_t BunchID        : 12;
    uint32_t SlotEnableMask : 11;
    uint32_t UNDEFINED      :  1;
    uint32_t DRMID          :  7;
    uint32_t MustBeOne      :  1;
  };

  struct CrateOrbit_t
  {
    uint32_t OrbitID      : 32;
  };
  
  struct FrameHeader_t
  {
    uint32_t NumberOfHits : 16;
    uint32_t FrameID      :  8;
    uint32_t TRMID        :  4;
    uint32_t DeltaBC      :  3;
    uint32_t MustBeZero   :  1;
  };
  
  struct PackedHit_t
  {
    uint32_t TOT          : 11;
    uint32_t Time         : 13;
    uint32_t Channel      :  3;
    uint32_t TDCID        :  4;
    uint32_t Chain        :  1;
  };

  struct CrateTrailer_t
  {
    uint32_t NumberOfDiagnostics :   4;
    uint32_t EventCounter        :  12;
    uint32_t UNDEFINED           :   3;
    uint32_t MustBeOne           :   1;
  };

  struct Diagnostic_t
  {
    uint32_t SlotID    : 4;
    uint32_t FaultBits : 28;
  };
  
  /** union **/

  union Union_t
  {
    uint32_t       Data;
    Word_t         Word;
    CrateHeader_t  CrateHeader;
    CrateOrbit_t   CrateOrbit;
    FrameHeader_t  FrameHeader;
    PackedHit_t    PackedHit;
    CrateTrailer_t CrateTrailer;
  };

  /** summary **/
  struct Summary_t
  {
    CrateHeader_t  CrateHeader;
    CrateOrbit_t   CrateOrbit;
    uint32_t nHits;
    FrameHeader_t FrameHeader[1024];
    PackedHit_t   PackedHit[1024];
    CrateTrailer_t CrateTrailer;
  };
  
}
   
/**
 ** SUMMARY DATA
 **/

namespace summary {

 struct RawSummary_t
 {
   raw::RDHWord0_t         RDHWord0;
   raw::RDHWord1_t         RDHWord1;
   raw::RDHWord2_t         RDHWord2;
   raw::RDHWord3_t         RDHWord3;
   uint32_t DRMCommonHeader;
   uint32_t DRMOrbitHeader;
   uint32_t DRMGlobalHeader;
   uint32_t DRMStatusHeader1;
   uint32_t DRMStatusHeader2;
   uint32_t DRMStatusHeader3;
   uint32_t DRMStatusHeader4;
   uint32_t DRMStatusHeader5;
   uint32_t DRMGlobalTrailer;
   uint32_t TRMGlobalHeader[10];
   uint32_t TRMGlobalTrailer[10];
   uint32_t TRMChainHeader[10][2];
   uint32_t TRMChainTrailer[10][2];
   uint32_t TDCUnpackedHit[2][15][256];
   uint8_t  nTDCUnpackedHits[2][15];

   uint32_t FramePackedHit[256][256];
   uint8_t  nFramePackedHits[256];
   uint8_t  FirstFilledFrame;
   uint8_t  LastFilledFrame;
    
   // derived data
   bool HasHits[10];
   bool HasErrors[10][2];
   // status
   bool decodeError;

   // checker data

   uint32_t nDiagnosticWords;
   uint32_t DiagnosticWord[12];
  
   uint32_t faultFlags;
 };

} /** namespace summary **/

namespace counters {

 struct DRMCounters_t
 {
   uint32_t Headers;
   uint32_t EventWordsMismatch;
   uint32_t CBit;
   uint32_t Fault;
   uint32_t RTOBit;
 };

 struct TRMCounters_t
 {
   uint32_t Headers;
   uint32_t Empty;
   uint32_t EventCounterMismatch;
   uint32_t EventWordsMismatch;
   uint32_t EBit;
 };
  
 struct TRMChainCounters_t
 { 
   uint32_t Headers;
   uint32_t EventCounterMismatch;
   uint32_t BadStatus;
   uint32_t BunchIDMismatch;
   uint32_t TDCerror;
 };
  
} /** namespace counters **/
  
} /** namespace data **/
} /** namespace tof **/


#endif /** _TOF_DATAFORMAT_H_ **/
