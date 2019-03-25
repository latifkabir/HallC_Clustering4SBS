#ifndef THcGEM_
#define THcGEM_

/////////////////////////////////////////////////////////////////////
//
//   THcGEM
//
/////////////////////////////////////////////////////////////////////

#include "THaNonTrackingDetector.h"
#include "GEMAnalyzer.h"
#include "GEMEvioParser.h"
#include "GEMDataHandler.h"
#include "EventUpdater.h"
//#include "VarDef.h"

class THcGEM : public THaNonTrackingDetector {

public:

   THcGEM(const char*, const char*, THaApparatus* a = NULL);
   virtual ~THcGEM();

   Int_t Decode(THaEvData *evdata);
   virtual EStatus Init( const TDatime& run_time);

   virtual Int_t Decode( const THaEvData& );
   virtual Int_t CoarseProcess( TClonesArray& tracks );
   virtual Int_t FineProcess( TClonesArray& tracks );


   virtual Int_t End( THaRunBase* r=0 );
   virtual Int_t ReadDatabase( const TDatime& date );
   virtual Int_t DefineVariables( EMode mode = kDefine );

   void SetConfigFile( const char* name) { fConfigFileName = name; }


protected:

   std::string fConfigFileName;

   GEMAnalyzer* fGEMAnalyzer;
   GEMEvioParser* fParser;
   GEMDataHandler* fHandler;
   EventUpdater* fUpdateEvent;
   GEMTree* fGEMTree;

   int fNHits;
   float* fX; //[fNHits]
   float* fY; //[fNHits]
   float* fXCharge; //[fNHits]
   float* fYCharge; //[fNHits]
   float* fXSize; //[fNHits]
   float* fYSize; //[fNHits]
   float* fEnergy; //[fNHits]

   ClassDef(THcGEM,0)




};
#endif
