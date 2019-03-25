// Filename: THcGEMDataProcessor.h
// Description: Process decoded GEM data to prepare for next level of analysis :
// i) Remove headers
// ii) Apply ADC channel mapping + Strip mapping
// iii) Subtract dynamic pedestal
// Author: Latif Kabir < latif@jlab.org >
// Created: Mon Oct 23 21:27:52 2017 (-0400)
// URL: jlab.org/~latif

#ifndef THCGEMDATAPROCESSOR_H
#define THCGEMDATAPROCESSOR_H
#include <vector>
#include "TObject.h"
#include "GEMEventAnalyzer.h"

using namespace std;

class THcGEMDataProcessor : public TObject
{
private:    
    Int_t ***fMappedRawData;
    Double_t **fPedestal;
    Int_t GetFirstIndex(vector <Int_t> V);
    Int_t GetStripMapping(Int_t chNo);    
public:
    void Init();
    THcGEMDataProcessor();
    ~THcGEMDataProcessor();

    Bool_t ProcessDecodedData(unordered_map<int, vector<int> > raw_event);
    Bool_t ProcessEvent(unordered_map<int, vector<int> > raw_event);    
    void SubtractPedestal();
    Int_t*** GetProcessedData();
    void PrintPedestal();

    ClassDef(THcGEMDataProcessor,0)
};

#endif

