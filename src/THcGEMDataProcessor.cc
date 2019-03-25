// Filename: THcGEMDataProcessor.cc
// Description: Process decoded GEM data to prepare for next level of analysis :
// i)Remove headers
// ii) Apply ADC channel mapping + Strip mapping
// iii) Subtract dynamic pedestal
// Author: Latif Kabir < latif@jlab.org >
// Created: Mon Oct 23 21:19:32 2017 (-0400)
// URL: jlab.org/~latif

#include <iostream>
#include <fstream>
#include "TApplication.h"
#include "THcGEMDataProcessor.h"
#include "HcGEMConstants.h"

ClassImp(THcGEMDataProcessor)

//----------------------- The constructor --------------------------------------
THcGEMDataProcessor::THcGEMDataProcessor()
{
    Init();
}
//----------------------- De-allocate heap memory ---------------------------
THcGEMDataProcessor::~THcGEMDataProcessor()
{
    for(Int_t i = 0; i < NADC; ++i)
    {
	for(Int_t j = 0; j < NTIME_BINS;)	
	    delete[] fMappedRawData[i][j];
	delete[]  fMappedRawData[i];
    }
    delete[] fMappedRawData;

    for(Int_t i = 0; i < NADC; ++i)
	delete[] fPedestal[i];
    delete[] fPedestal;	    
}
//-------------------Create buffer to keep processed data --------------------
void THcGEMDataProcessor::Init()
{
    fMappedRawData = new Int_t**[NADC];
    for(Int_t i = 0; i < NADC; ++i)
    {
	fMappedRawData[i] = new Int_t*[NTIME_BINS];
	for(Int_t j = 0; j < NTIME_BINS; ++j)	
	    fMappedRawData[i][j] = new Int_t[N_STRIPS];
    }

        fPedestal = new Double_t*[NADC];
    for(Int_t i = 0; i < NADC; ++i)
	fPedestal[i] = new Double_t[N_STRIPS];

    std::ifstream PedDataFile("pedestal/GEMPedData.dat", std::ifstream::in | std::ifstream::binary);

    if(!PedDataFile)
    {
	cout<<" Pedestal data file NOT found"<<endl;
	gApplication->Terminate();
    }
    for(Int_t i = 0; i < NADC; ++i)
	PedDataFile.read((char*)fPedestal[i], N_STRIPS*sizeof(Double_t));
}
//--------------Remove headers and apply ADC Channel mapping + Strip mapping --------------------
Bool_t THcGEMDataProcessor::ProcessDecodedData(unordered_map<int, vector<int> > raw_event)
{
    Int_t event_size = raw_event.size();

    if(event_size < 1)
    {
	//cout<<"Empty Event. Skipped"<<endl;
	return false;
    }
	
    Int_t index = 0;
    Int_t fec_id = 0;
    Int_t adc_ch = 0;
    vector <Int_t> V = raw_event.begin()->second;
    Int_t firstIndex = GetFirstIndex(V);
    if(firstIndex == -1)
    {
	cout << "Unable to locate first time bin" <<endl;
	return false;
    }
    Int_t nTimeBins = NTIME_BINS*BLOCK_LENGTH; //21*140;
    Int_t stripNo = 0;    
    for(auto &i: raw_event)
    {
	index = i.first;
	adc_ch = index & 0xf;
	fec_id = (index>>4)&0xf;
	Int_t N = i.second.size();
	for(Int_t j=firstIndex; j<(nTimeBins+firstIndex); ++j)
	{
	    if((j-firstIndex)%140<12)
		continue;
	    stripNo = (Int_t)((j-firstIndex)/140)*128 + GetStripMapping((j-firstIndex)%140 - 12);
	    fMappedRawData[adc_ch][stripNo/N_STRIPS][stripNo%N_STRIPS] = i.second[j];
	}
    }
    return true;	
}
// ---------------- Scan for header index ----------------------------------
Int_t THcGEMDataProcessor::GetFirstIndex( vector <Int_t> V)
{
    Int_t headerCutOff = HEADER_CUTOFF;
    Int_t index = -1;

    for(Int_t i = 0; i< V.size(); ++i)
    {
	if(V[i] < headerCutOff && V[i+1] < headerCutOff && V[i+2] < headerCutOff)
	{
	    index = i;
	    break;
	}
    }
    return index;
}
// ----------------- APV25 internal channel mapping + GEM strip mapping ----------------
Int_t THcGEMDataProcessor::GetStripMapping(Int_t chNo)
{    
    //--- APV25 Internal Channel Mapping ----
    chNo = (32 * (chNo%4)) + (8 * (Int_t)(chNo/4)) - (31 * (Int_t)(chNo/16));

    //---- GEM Strip Mapping ----
    if(chNo % 4 == 0) chNo = chNo + 2;
    else if(chNo % 4 == 1) chNo = chNo - 1;
    else if(chNo % 4 == 2) chNo = chNo + 1;
    else chNo = chNo - 2;
    
    return chNo;        
}
//--------- Use the last time bin of the event as the dynamic pedestal, subtract all other time bins from the pedestal ----------
void THcGEMDataProcessor::SubtractPedestal()
{
    for(Int_t adc = 0; adc < NADC; ++adc)
    {
	for(Int_t tbin = 0; tbin < NTIME_BINS; ++tbin)
	{
	    for(Int_t strip = 0; strip < N_STRIPS; ++strip)
	    {
		if(USE_PEDESTAL_DATA)
		    fMappedRawData[adc][tbin][strip] = fPedestal[adc][strip] - fMappedRawData[adc][tbin][strip];
		else
		    fMappedRawData[adc][tbin][strip] = fMappedRawData[adc][NTIME_BINS -1][strip] - fMappedRawData[adc][tbin][strip];
	    }
	}
    }
}
//-------- Process curent event: Remove header, apply mapping, subtract pedestal -------------------
Bool_t THcGEMDataProcessor::ProcessEvent(unordered_map<int, vector<int> > raw_event)
{
    bool hasGEMData = false;
    hasGEMData = ProcessDecodedData(raw_event);
    if(hasGEMData)
	SubtractPedestal();
    return hasGEMData;
}
//---------------------- Access point for the processed data --------------------------------
Int_t*** THcGEMDataProcessor::GetProcessedData()
{
    return fMappedRawData;    
}
//------------------- Print Loaded Pedestal for Confirmation -------------------------
void THcGEMDataProcessor::PrintPedestal()
{
    cout<<"======================== Printing Pedestal Data ========================"<<endl;
    for(Int_t i = 0; i < NADC; ++i)
    {
	for(Int_t j = 0; j < N_STRIPS; ++j)
	    cout <<fPedestal[i][j]<<"\t";
	cout<<endl;
    }
}


