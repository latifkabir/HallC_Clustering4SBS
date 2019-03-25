// Filename: THcGEMCluster.cc
// Description: 
// Author: Latif Kabir < latif@jlab.org >
// Created: Wed Oct 25 00:11:08 2017 (-0400)
// URL: jlab.org/~latif

#include"THcGEMCluster.h"
#include "TGraph.h"
#include "TF1.h"
#include "TString.h"
#include "HcGEMConstants.h"

ClassImp(THcGEMCluster)
//Constructor
THcGEMCluster::THcGEMCluster()
{
    fEventNumber = -1;
    
    fStatusFlag = new Int_t*[NTIME_BINS];
    for(Int_t j = 0; j < NTIME_BINS; ++j)	
	fStatusFlag[j] = new Int_t[N_STRIPS];
    ResetFlag();
}
//Destructor
THcGEMCluster::~THcGEMCluster()
{
    for(Int_t j = 0; j < NTIME_BINS; ++j)	
	delete[] fStatusFlag[j];
    delete[] fStatusFlag;        
}
// Set pointer to processed data
void THcGEMCluster::RegisterBuffer(Int_t ***processedData)
{
    fProcessedData = processedData;
}
Bool_t THcGEMCluster::IsClusterCandidate(Int_t adc, Int_t tbin, Int_t strip)
{
    Int_t x_th = THRESHOLD_LOW_X;
    Int_t y_th = THRESHOLD_LOW_Y;    
    Int_t cutOffLower[NADC] = {x_th, x_th, x_th, y_th, y_th, y_th};
    
    if(strip >= (N_STRIPS - 1) || strip < 1)
	return kFALSE;
    
    if(tbin >= NTIME_BINS)
    	return kFALSE;
    if(fProcessedData[adc][tbin][strip] < cutOffLower[adc])
	return kFALSE;

    if(fStatusFlag[tbin][strip] == 0 ||
       fStatusFlag[tbin][strip-1] == 0 ||
       fStatusFlag[tbin][strip+1] == 0)
	return kFALSE;
    
    if(fProcessedData[adc][tbin][strip] > fProcessedData[adc][tbin][strip-1]
       && fProcessedData[adc][tbin][strip] > fProcessedData[adc][tbin][strip+1])
	return kTRUE;    
}

// Find clusters with new clustering algorithm
void THcGEMCluster::FindClusters()
{
    this->fMapOfFitFunctions.clear();
    this->fMapOfFitGraphs.clear();
    this->fMapOfGEMClusters.clear();
    
    Int_t x_th = THRESHOLD_HI_X;
    Int_t y_th = THRESHOLD_HI_Y;    
    Int_t cutOffHigher[NADC] = {x_th, x_th, x_th, y_th, y_th, y_th};
    const Int_t nTbins = FITTED_TBINS;
    Int_t clusterHeights[nTbins];
    Int_t maxHeight = -1;
    Int_t maxTBin = -1;
    GEMCluster gemCluster;
    vector <GEMCluster> clusterVector;
    Int_t counter = 0;
    
    for(Int_t adc = 0; adc < NADC; ++adc)
    {
	clusterVector.clear();
	ResetFlag();
	for(Int_t tbin = 0; tbin < NTIME_BINS ; ++tbin)
	{
	    for(Int_t strip = 0; strip < N_STRIPS; ++strip)
	    {
		counter = 0;
		if (strip == 0 || strip == (N_STRIPS -1))
		    continue;
		
		if(tbin+nTbins >= NTIME_BINS)
		    continue;
		
		for (int k = 0; k < nTbins; ++k)
		{
		    if(IsClusterCandidate(adc, tbin + k, strip))
			++counter;
		}

		if(counter < MATCHING_LIMIT)
		    continue;
		
		// Get maximum ADC height of the 10 Tbins. If (max ADC height < cutOffHigher). Then it is NOT a True cluster
		for(Int_t i = 0; i < nTbins; ++i)
		    clusterHeights[i] = fProcessedData[adc][tbin + i][strip];			
		maxHeight = *max_element(clusterHeights, clusterHeights + nTbins);
		maxTBin = tbin + (max_element(clusterHeights, clusterHeights + nTbins) - clusterHeights);
		if(maxHeight < cutOffHigher[adc])
		    continue;
		    
		if(!FitClusterCandidates(adc, tbin, strip))
		    continue;
		
		// Save hit wire. Save hit time bin. Save hit max height. Flag those three wires in all tbins
		gemCluster.time_bin = maxTBin;
		gemCluster.wire = strip;
		gemCluster.height = maxHeight;
		gemCluster.energy = fClusterFitFnc->Integral(fClusterFitFnc->GetXmin(),fClusterFitFnc->GetXmax());

		if(gemCluster.time_bin > TIME_LOW && gemCluster.time_bin < TIME_HIGH && APPLY_TIMING_CUT)
		    clusterVector.push_back(gemCluster);
		else if(! APPLY_TIMING_CUT)
		    clusterVector.push_back(gemCluster);

		//This piece of code dictates resilution of cluester (minimum distance between two clusters that are considered two distinct clusters).
		for(Int_t i = 0; i < (nTbins + EXTENDED_BINS); ++i)
		{
		    if( (tbin + i) >= NTIME_BINS)
			continue;

		    fStatusFlag[tbin + i][strip-1] = 0;
		    fStatusFlag[tbin + i][strip] = 0;
		    fStatusFlag[tbin + i][strip+1] = 0;
		}				
	    }
	}
	sort(clusterVector.begin(),clusterVector.end(), CompareTimeAndEnergy);
	fMapOfGEMClusters.insert({adc, clusterVector});
    }
}
//Sort clusters based on total area/energy or time bis of max height
Bool_t THcGEMCluster::CompareTimeAndEnergy(GEMCluster i, GEMCluster j)
{
    if((i.time_bin == j.time_bin) || (abs(i.time_bin - j.time_bin) == 1))
	return (i.energy > j.energy);
    else
    	return (i.time_bin < j.time_bin);
}

// Accumulate raw coordinate, OLD single clustering algorithm (for single cluster per event only)
void THcGEMCluster::FillRawCoordinate()
{
    Int_t cutOffPrimary = 400;
    Int_t cutOffSecondary = 250;
    Double_t sigma;
    Double_t ratio;
    Bool_t status;
        
    Int_t previousVal;
    Int_t presentVal;
    Bool_t clusterFound = false;

    Int_t tbin_;
    Int_t strip_;
    
    fRawCoord.clear();
    
    for(Int_t adc = 0; adc < NADC; ++adc)
    {
	clusterFound = false;
	for(Int_t tbin = 0; tbin < (NTIME_BINS -3); ++tbin)
	{
	    for(Int_t strip = 0; strip < N_STRIPS; ++strip)
	    {
		tbin_ = tbin;
		strip_ = strip;

		if(fProcessedData[adc][tbin_][strip_]<cutOffSecondary)
		    continue;

		previousVal = fProcessedData[adc][tbin_][strip_];
		presentVal = fProcessedData[adc][tbin_][strip_];
		
		while(presentVal>=previousVal)
		{
		    if(strip_ >= (N_STRIPS - 1))
			break;
		    ++strip_;
		    previousVal = presentVal;
		    presentVal = fProcessedData[adc][tbin_][strip_];
		}
		if(strip_>0)
		    --strip_;
		presentVal = fProcessedData[adc][tbin_][strip_];
		previousVal = presentVal;
		
		while(presentVal>=previousVal)
		{
		    if(tbin_ >= (NTIME_BINS - 4))
			break;
		    ++tbin_;
		    previousVal = presentVal;
		    presentVal = fProcessedData[adc][tbin_][strip_];
		}
		if(tbin_>0)
		    --tbin_;
		presentVal = fProcessedData[adc][tbin_][strip_];

		if(presentVal > cutOffSecondary && presentVal < cutOffPrimary)
		{
		    status = FitClusterCandidates(adc,tbin_,strip_);
		    if(!status)
			continue;
		}

		if(APPLY_TIMING_CUT)
		{
		    if(!(tbin_ > TIME_LOW && tbin_ < TIME_HIGH))  
			continue;
		}
		fCoord.adcNo = adc;
		fCoord.stripNo = strip_;
		fCoord.ADCValue = presentVal;
		clusterFound = true;
		fRawCoord.push_back(fCoord);
		// cout << "Found GEM Cluster !!! Event: "<< fEventNumber <<"\t ADC: "<<coord.adcNo<<"\t Strip no."<<coord.stripNo<<"\t ADC value: "<< coord.ADCValue<<endl;
		break;  //<------------- Assume we will have single track events only
	    }
	    if(clusterFound)
		break;
	}
    }
}

//------------ Fit the signal to check if it's a GEM signal ------
Bool_t THcGEMCluster:: FitClusterCandidates(Int_t adc, Int_t tbin, Int_t strip)
{
    Int_t maxTbins = FITTED_TBINS;
    TGraph gr;
    TString title = "GEM Event: ";
    title += fEventNumber;
    title += " ADC: ";
    title += adc;
    gr.SetTitle(title);
    Int_t point = 0;
    for (Int_t i = tbin; i < (tbin + maxTbins); i++) 
    {
	if(i >= NTIME_BINS || i < 0)
	{
	    maxTbins = i - tbin;
	    continue;
	}
	gr.SetPoint(point, i*N_STRIPS + strip, fProcessedData[adc][i][strip]);
	++point;
    }
    TF1 *f1 = new TF1("fitFnc","landau",tbin*N_STRIPS+strip,(tbin + maxTbins) * N_STRIPS + strip);
    gr.Fit("fitFnc","QR");
    
    Double_t ratio0 = f1->GetParError(0) / f1->GetParameter(0);
    Double_t ratio1 = f1->GetParError(1) / f1->GetParameter(1);
    Double_t ratio2 = f1->GetParError(2) / f1->GetParameter(2);
    Double_t ratio3 = f1->GetChisquare() / f1->GetNDF();
    
    if(ratio0 > 0.1*2.0 || ratio1 > 0.1*2.0 || ratio2 > 0.1*2.0 || ratio3 > 3000.0*2.0 || f1->GetParameter(1) < 0)
    	return false;
    else
    {
	fMapOfFitFunctions[adc].push_back(*f1);
	fMapOfFitGraphs[adc].push_back(gr);
	fClusterGraph = &fMapOfFitGraphs[adc].back();
	fClusterFitFnc = &fMapOfFitFunctions[adc].back();
	return true;   
    }
}

// New Algorithm, work for multiple clusters , slow approach
void THcGEMCluster:: ComputeCoordinate(Bool_t /*newApproach*/)
{
    fGEM_Coord.X = -1.0;
    fGEM_Coord.Y = -1.0;
    fGEM_Coord.charge_x = -1;
    fGEM_Coord.charge_y = -1;
    
    const Double_t length = GEM_LENGTH;
    const Double_t stripNoToCm = length/(3.0*N_STRIPS);

    Int_t X1 = 0;
    Int_t X2 = 1;
    Int_t X3 = 2;
    Int_t Y1 = 0;
    Int_t Y2 = 1;
    Int_t Y3 = 2;
    
    for(Int_t adc_No = 0; adc_No < NADC; ++adc_No)
    {     
	//----------- Last level (APV orientation) of mapping correction happes here ------- 
        // The GEM coordinate is also transformed to align the axes with Hall C transport coordinate system. 

	if(fMapOfGEMClusters[adc_No].size() == 0)
	    continue;

	// First let's verify considering first cluster only. Also (x,y) pair is not proper here. //<----------- FIX IT to get full advantage of multi-cluster algorithm
	 switch (adc_No)
	 {
	   case X1_ADC_NO:
	     fGEM_Coord.X = X1*(Double_t)N_STRIPS*stripNoToCm + (N_STRIPS - fMapOfGEMClusters[adc_No][0].wire - 1)*stripNoToCm;
	     fGEM_Coord.charge_x = fMapOfGEMClusters[adc_No][0].height;
	     break;
	   case X2_ADC_NO:
	     fGEM_Coord.X = X2*(Double_t)N_STRIPS*stripNoToCm + (N_STRIPS - fMapOfGEMClusters[adc_No][0].wire - 1)*stripNoToCm;
	     fGEM_Coord.charge_x = fMapOfGEMClusters[adc_No][0].height;
	     break;
	   case X3_ADC_NO:
	     fGEM_Coord.X = X3*(Double_t)N_STRIPS*stripNoToCm + (N_STRIPS - fMapOfGEMClusters[adc_No][0].wire - 1)*stripNoToCm;
	     fGEM_Coord.charge_x = fMapOfGEMClusters[adc_No][0].height;
	     break;
	   case Y1_ADC_NO:
	     fGEM_Coord.Y = Y1*(Double_t)N_STRIPS*stripNoToCm + (fMapOfGEMClusters[adc_No][0].wire + 1)*stripNoToCm;
	     fGEM_Coord.Y = length - fGEM_Coord.Y;
	     fGEM_Coord.charge_y = fMapOfGEMClusters[adc_No][0].height;
	     break;
	   case Y2_ADC_NO:
	     fGEM_Coord.Y = Y2*(Double_t)N_STRIPS*stripNoToCm + (fMapOfGEMClusters[adc_No][0].wire + 1)*stripNoToCm;
	     fGEM_Coord.Y = length - fGEM_Coord.Y;
	     fGEM_Coord.charge_y = fMapOfGEMClusters[adc_No][0].height;
	     break;
	   case Y3_ADC_NO:
	     fGEM_Coord.Y = Y3*(Double_t)N_STRIPS*stripNoToCm + (fMapOfGEMClusters[adc_No][0].wire + 1)*stripNoToCm;
	     fGEM_Coord.Y = length - fGEM_Coord.Y;
	     fGEM_Coord.charge_y = fMapOfGEMClusters[adc_No][0].height;
	     break;
           default:
	     break;
	}
    }         
}
// Old Algorithm, work for Single cluster only, fast approach
void THcGEMCluster:: ComputeCoordinate()
{
    ResetCoordinate();
    const Double_t length = GEM_LENGTH;
    const Double_t stripNoToCm = length/(3.0*N_STRIPS);

    Int_t X1 = 0;
    Int_t X2 = 1;
    Int_t X3 = 2;
    Int_t Y1 = 0;
    Int_t Y2 = 1;
    Int_t Y3 = 2;
    
    for(RawCoordinate rc : fRawCoord)
    {     
	//----------- Last level (APV orientation) of mapping correction happes here ------- 
        // The GEM coordinate is also transformed to align the axes with Hall C transport coordinate system
	 switch (rc.adcNo)
	 {
	   case X1_ADC_NO:
	     fGEM_Coord.X = X1*(Double_t)N_STRIPS*stripNoToCm + (N_STRIPS - rc.stripNo - 1)*stripNoToCm;
	     fGEM_Coord.charge_x = rc.ADCValue;
	     break;
	   case X2_ADC_NO:
	     fGEM_Coord.X = X2*(Double_t)N_STRIPS*stripNoToCm + (N_STRIPS - rc.stripNo - 1)*stripNoToCm;
	     fGEM_Coord.charge_x = rc.ADCValue;
	     break;
	   case X3_ADC_NO:
	     fGEM_Coord.X = X3*(Double_t)N_STRIPS*stripNoToCm + (N_STRIPS - rc.stripNo - 1)*stripNoToCm;
	     fGEM_Coord.charge_x = rc.ADCValue;
	     break;
	   case Y1_ADC_NO:
	     fGEM_Coord.Y = Y1*(Double_t)N_STRIPS*stripNoToCm + (rc.stripNo + 1)*stripNoToCm;
	     fGEM_Coord.Y = length - fGEM_Coord.Y;
	     fGEM_Coord.charge_y = rc.ADCValue;
	     break;
	   case Y2_ADC_NO:
	     fGEM_Coord.Y = Y2*(Double_t)N_STRIPS*stripNoToCm + (rc.stripNo + 1)*stripNoToCm;
	     fGEM_Coord.Y = length - fGEM_Coord.Y;
	     fGEM_Coord.charge_y = rc.ADCValue;
	     break;
	   case Y3_ADC_NO:
	     fGEM_Coord.Y = Y3*(Double_t)N_STRIPS*stripNoToCm + (rc.stripNo + 1)*stripNoToCm;
	     fGEM_Coord.Y = length - fGEM_Coord.Y;
	     fGEM_Coord.charge_y = rc.ADCValue;
	     break;
	}
    }    
}

void THcGEMCluster::ResetCoordinate()
{
    fGEM_Coord.X = -1.0;
    fGEM_Coord.Y = -1.0;
    fGEM_Coord.charge_x = -1;
    fGEM_Coord.charge_y = -1;    
}
//------ Set the event number ------------
void THcGEMCluster::SetEventNumber(Int_t eventNumber)
{
    fEventNumber = eventNumber;
}
// ----------- Reset the flag ----------
void THcGEMCluster::ResetFlag()
{
    for(Int_t i = 0; i < NTIME_BINS; ++i)
    {
	for (Int_t j = 0; j < N_STRIPS; j++) 
	    fStatusFlag[i][j] = 1;
    }
}
