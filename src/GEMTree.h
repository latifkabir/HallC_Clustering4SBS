//==================================================================//
//                                                                  //
// Xinzhan Bai                                                      //
// 03/20/2016                                                       //
// xb4zp@virginia.edu                                               //
//==================================================================//

#ifndef GEMTREE_H
#define GEMTREE_H
#include "GEMDataStruct.h"
#include <vector>
#include <unordered_map>
#include <cstring>
#include "datastruct.h"
#include <unordered_map>
#include <TString.h>
#include "THcGEMCluster.h"

#define NDETECTOR 1

class TTree;
class TFile;
class TH1F;
class GEMConfigure;

class GEMTree
{
    // ------------------------------- common setting --------------------------------------
public:
    GEMTree();
    ~GEMTree();
    void WriteToDisk();
    void SetGEMConfigure(GEMConfigure *con);
    void SetFilePath(TString fileName);
private:
    TFile *file;
    GEMConfigure *configure;

    // for all trees
    unsigned int evt_id;


    // ------------------------------- gem tree -------------------------------------
public:
    void PushData(std::vector<GEMClusterStruct> & gem1,
	    std::vector<GEMClusterStruct> &gem2,
	    std::unordered_multimap<std::string, double> &tdc_map);

    void PushDetector(int, std::vector<GEMClusterStruct>);
    void PushCoordinate(GEMCoordinate gem_coord);
    void PushTDCValue(std::unordered_multimap<std::string, double> &tdc_map);

    void FillGEMTree();
    void InitGEMTree(int ndet);
    void InitTDCGroup();

    int GetNHits(int idet) {return nhits[idet];}
    float* GetXP(int idet) {return x[idet];}
    float* GetYP(int idet) {return y[idet];}
    float* GetXChargeP(int idet) {return x_charge[idet];}
    float* GetYChargeP(int idet) {return y_charge[idet];}
    float* GetXSizeP(int idet) {return x_size[idet];}
    float* GetYSizeP(int idet) {return y_size[idet];}
    float* GetEnergy(int idet) {return energy[idet];}

private:
    TTree *gem_tree;

    bool empty_gem_event;
    // number of hits on each detector
    const int static _nhits = 2000; // if _nhits is not large enough, it will become a potential break threat
    std::string scin_group[2];
    int hycal_group_q;
    std::string hycal_group[9];

    // max 4 detectors
    int nhits[NDETECTOR] = {0};
    float x[NDETECTOR][_nhits];
    float y[NDETECTOR][_nhits];
    float x_charge[NDETECTOR][_nhits];
    float y_charge[NDETECTOR][_nhits];
    float energy[NDETECTOR][_nhits];
    float z[NDETECTOR][_nhits];
    float x_size[NDETECTOR][_nhits];
    float y_size[NDETECTOR][_nhits];

    // tdc information
    int n_S2;
    int n_S1;
    int n_hycal;
    float TDCS2[_nhits];
    float TDCS1[_nhits];
    float TDCHyCal[_nhits];

    // ------------------------------- epics tree -------------------------------------
public:
    void PushEpics(std::unordered_map<std::string, double> &);
    void FillEpicsTree();
    void InitEpicsTree();
    void SetEventID(unsigned int &id);

private:
    TTree *epics_tree;
    bool empty_epics_event;
    EpicsStruct _epics_tree;
    int event_id;

};

#endif
