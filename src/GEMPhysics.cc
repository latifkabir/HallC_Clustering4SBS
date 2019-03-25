#include "GEMPhysics.h"
#include <iostream>
#include "GEMPedestal.h"
#include "GEMMapping.h"
#include "GEMOnlineHitDecoder.h"
#include "GEMSignalFitting.h"
#include <TCanvas.h>
#include "GEMTree.h"
#include "GEMCoord.h"

//#define PROD_GEM_OFFSET

using namespace std;

GEMPhysics::GEMPhysics()
{
    cout<<"GEMPhysics Constructor..."
        <<endl;
    mapping = GEMMapping::GetInstance();
    fGEMDataProcessor = new THcGEMDataProcessor();
    fHcGEMCluster = new THcGEMCluster();
}

GEMPhysics::~GEMPhysics()
{
}

void GEMPhysics::SetGEMPedestal(GEMPedestal *ped)
{
    pedestal = ped;
    pedestal -> LoadPedestal();

    hit_decoder = new GEMOnlineHitDecoder();
    hit_decoder -> SetPedestal(pedestal);

    sig_fitting = new GEMSignalFitting();
    sig_fitting->SetGEMOnlineHitDecoder(hit_decoder);

    gem_coord = new GEMCoord();
    gem_coord -> SetHitDecoder(hit_decoder);
}

void GEMPhysics::SetGEMConfigure(GEMConfigure *c)
{
    config = c;

    hit_decoder -> SetGEMConfigure(config);
}

void GEMPhysics::SetGEMTree(GEMTree *tree)
{
    rst_tree = tree;
}

//------------- Hall B implementation ------------------
// void GEMPhysics::AccumulateEvent(int evtID, unordered_map<int, vector<int> > & event)
// {
//     //cout<<"event number from gem: "<<evtID<<endl;
//     SetEvtID( evtID );
//     // Modify from here : Latif
//     hit_decoder -> ProcessEvent(event);
//     CharactorizeGEM();
//     CharactorizePhysics();

//     sig_fitting -> Fit();
// }

// ---------------- Hall B Implementation -----------------
// void GEMPhysics::CharactorizeGEM()
// {
//     int n = mapping->GetNbOfDetectors();

//     vector<GEMClusterStruct> gem;
//     for(int i=0;i<n;i++)
//     {
// 	gem.clear();
// 	gem_coord->GetClusterGEM(i, gem);
// 	rst_tree -> PushDetector(i, gem);
//     }
//     rst_tree -> FillGEMTree();
// }

// -------------- Hall C GEM implementation:Old Algorithm, works for single cluster only -----------------
// void GEMPhysics::AccumulateEvent(int evtID, std::unordered_map<int, std::vector<int> > event)
// {
//     //cout<<"event number from gem: "<<evtID<<endl;
//     SetEvtID( evtID );

//     bool hasGEMData = false;
//     hasGEMData = fGEMDataProcessor->ProcessEvent(event);
//     if(!hasGEMData)
//     {
// 	fHcGEMCluster->ResetCoordinate();
// 	CharactorizeGEM();
// 	return;
//     }
	
//     fHcGEMCluster->RegisterBuffer(fGEMDataProcessor->GetProcessedData());

//     fHcGEMCluster->FillRawCoordinate();	
//     // if(fHcGEMCluster->fRawCoord.size() != 2) // For now lets consider only pair of clusters
//     // 	fHcGEMCluster->ResetCoordinate();
//     // else
//     {
// 	fHcGEMCluster->ComputeCoordinate();
// 	if(fHcGEMCluster->fGEM_Coord.X == -1 || fHcGEMCluster->fGEM_Coord.Y == -1) // Discard double hits on same axis
// 	    fHcGEMCluster->ResetCoordinate();
//     }

//     CharactorizeGEM();
// }

// -------------- Hall C GEM implementation:New Algorithm -----------------
void GEMPhysics::AccumulateEvent(int evtID, std::unordered_map<int, std::vector<int> > event)
{
    //cout<<"event number from gem: "<<evtID<<endl;
    SetEvtID( evtID );

    bool hasGEMData = false;
    hasGEMData = fGEMDataProcessor->ProcessEvent(event);
    if(!hasGEMData)
    {
	fHcGEMCluster->ResetCoordinate();
	CharactorizeGEM();
	return;
    }
	
    fHcGEMCluster->RegisterBuffer(fGEMDataProcessor->GetProcessedData());
    fHcGEMCluster->FindClusters();    
    fHcGEMCluster->ComputeCoordinate(kTRUE);
    
    if(fHcGEMCluster->fGEM_Coord.X == -1 || fHcGEMCluster->fGEM_Coord.Y == -1)
    	fHcGEMCluster->ResetCoordinate();
    
    CharactorizeGEM();
}

// ------------------ Hall C Implementation ------------------
void GEMPhysics::CharactorizeGEM()
{
    rst_tree->SetEventID(evt_id);
    rst_tree->PushCoordinate(fHcGEMCluster->fGEM_Coord);
    rst_tree->FillGEMTree();
}

void GEMPhysics::CharactorizePhysics()
{
    //#ifndef PROD_GEM_OFFSET
    CharactorizePlanePhysics();
    //#else
    CharactorizeOverlapPhysics();
    //#endif
}

void GEMPhysics::CharactorizePlanePhysics()
{
    gem_coord -> SetGEMOffsetX(0.);
    gem_coord -> SetGEMOffsetY(0.);

    // place holer
}

void GEMPhysics::CharactorizeOverlapPhysics()
{
    gem_coord -> SetGEMOffsetX(0.);
    gem_coord -> SetGEMOffsetY(0.);

    // place holer
}

void GEMPhysics::SavePhysResults()
{
    //rst_tree->WriteToDisk();
    sig_fitting->Write();
}

void GEMPhysics::SetEvtID(unsigned int id)
{
    evt_id = id;
}

unsigned int GEMPhysics::GetEvtID()
{
    return evt_id;
}

