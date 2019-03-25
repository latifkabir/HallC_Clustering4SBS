// Filename: THcGEMCluster.h
// Description: 
// Author: Latif Kabir < latif@jlab.org >
// Created: Tue Oct 24 23:46:08 2017 (-0400)
// URL: jlab.org/~latif

#ifndef THCGEMCLUSTER_H
#define THCGEMCLUSTER_H

#include <iostream>
#include <vector>
#include <unordered_map>
#include "TObject.h"
#include "TF1.h"
#include "TGraph.h"

using namespace std;
struct GEMCoordinate
{
    Double_t X;
    Double_t Y;
    Int_t charge_x;
    Int_t charge_y;	
};
class THcGEMCluster : public TObject
{
    struct GEMCluster
    {
	Int_t time_bin;
	Int_t wire;
	Int_t height;
	Double_t energy;
    };
    struct RawCoordinate
    {
	Int_t adcNo;
	Int_t stripNo;
	Int_t ADCValue;
    };

    Int_t ***fProcessedData;
    Int_t fEventNumber;
    TGraph *fClusterGraph;
    TF1 *fClusterFitFnc;
    Int_t **fStatusFlag;
    void ResetFlag();
public:

    void RegisterBuffer(Int_t ***processedData);
    void FillRawCoordinate();
    void ComputeCoordinate();
    void ComputeCoordinate(Bool_t /*newApproach*/);
    Bool_t FitClusterCandidates(Int_t adc, Int_t tbin, Int_t strip);
    void PlotGEMCoord();
    void SetEventNumber(Int_t eventNumber);
    GEMCoordinate GetGEMCoordinate();
    void ResetCoordinate();
    
    RawCoordinate fCoord;
    std::vector <RawCoordinate> fRawCoord;
    GEMCoordinate fGEM_Coord;
    unordered_map <Int_t, vector<GEMCluster> > fMapOfGEMClusters;
    unordered_map <Int_t, vector <TF1> > fMapOfFitFunctions;
    unordered_map <Int_t, vector <TGraph> > fMapOfFitGraphs;
    
    Bool_t IsClusterCandidate(Int_t adc, Int_t tbin, Int_t strip);
    void FindClusters();
    static Bool_t CompareTimeAndEnergy(GEMCluster i, GEMCluster j);
    
    void Init();
    THcGEMCluster();
    ~THcGEMCluster();
    ClassDef(THcGEMCluster,0)
};

#endif
