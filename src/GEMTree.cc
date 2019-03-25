//========================================================================//
//                                                                        //
// Xinzhan Bai                                                            //
// 08/05/2016                                                             //
//========================================================================//
#include "GEMTree.h"
#include "TTree.h"
#include "TFile.h"
#include <cassert>
#include <iostream>
#include "GEMDataStruct.h"
#include <vector>
#include "TH1F.h"
#include "GEMConfigure.h"

using namespace std;

// ------------------------------- common setting --------------------------------------
GEMTree::GEMTree()
{
    TH1::AddDirectory(kFALSE);
    event_id = 0;
}

void GEMTree::SetFilePath(TString fileName)
{
    file = new TFile(fileName, "recreate");
}

void GEMTree::WriteToDisk()
{
    file->Write();
    //file->Save();
}

GEMTree::~GEMTree()
{
}

void GEMTree::SetGEMConfigure(GEMConfigure * c)
{
    configure = c;

    // init res tree
    this -> InitGEMTree(1); // 1 gem detectors
    this -> InitEpicsTree();
}


// ------------------------------- gem tree -------------------------------------
void GEMTree::SetEventID(unsigned int &id)
{
    //event_id = (int) id;
    evt_id = (int) id;  // latif
}

void GEMTree::InitGEMTree(int ndet)
{
    gem_tree = new TTree("T", "res");
    gem_tree->SetDirectory(file);

    assert( ndet <= NDETECTOR);

    // event id
    gem_tree->Branch("event_id", &evt_id, "event_id/I");

    // gem information
    for(int i=0;i<ndet;i++)
    {
	gem_tree->Branch(Form("nhits%d",i),&nhits[i], Form("nhits%d/I",i));
	gem_tree->Branch(Form("x%d",i), &x[i], Form("x%d[nhits%d]/F",i,i));
	gem_tree->Branch(Form("y%d",i), &y[i], Form("y%d[nhits%d]/F",i,i));
	gem_tree->Branch(Form("x%d_charge",i), &x_charge[i], Form("x%d_charge[nhits%d]",i,i));
	gem_tree->Branch(Form("y%d_charge",i), &y_charge[i], Form("y%d_charge[nhits%d]",i,i));
	gem_tree->Branch(Form("energy%d",i), &energy[i], Form("energy%d[nhits%d]",i,i));
	gem_tree->Branch(Form("z%d",i), &z[i], Form("z%d[nhits%d]",i,i));
	gem_tree->Branch(Form("x%d_size",i), &x_size[i], Form("x%d_size[nhits%d]",i,i));
	gem_tree->Branch(Form("y%d_size",i), &y_size[i], Form("y%d_size[nhits%d]",i,i));
    }

    // tdc information
    gem_tree->Branch("n_S2", &n_S2, "n_S2/I");
    gem_tree->Branch("n_S1", &n_S1, "n_S1/I");
    gem_tree->Branch("TDCS2", &TDCS2, "TDCS2[n_S2]/F");
    gem_tree->Branch("TDCS1", &TDCS1, "TDCS1[n_S1]/F");
    gem_tree->Branch("n_hycal", &n_hycal, "n_hycal/I");
    gem_tree->Branch("TDCHyCal", &TDCHyCal, "TDCHyCal[n_hycal]/F");

    InitTDCGroup();
}

void GEMTree::InitTDCGroup()
{
    hycal_group_q = configure->TDC_Quan;

    for(int i=0;i<hycal_group_q;i++)
    {
	hycal_group[i] = configure->TDC[i];
    }

    scin_group[0] = "S1";
    scin_group[1] = "S2";
}

void GEMTree::PushData(vector<GEMClusterStruct> &gem1,
	vector<GEMClusterStruct> &gem2,
	unordered_multimap<string, double> &tdc_map)
{
    PushDetector(0, gem1);
    //PushDetector(1, gem2);
    PushTDCValue(tdc_map);
}


void GEMTree::PushDetector(int nthDet, std::vector<GEMClusterStruct> Gem)
{
    int index = 0;
    int n = nthDet;
    int nh = Gem.size();
    if(nh != 0)
	empty_gem_event = false;

    nhits[n] = nh;
    for(auto &i: Gem)
    {
	x[n][index]=i.x;
	y[n][index] = i.y;
	x_charge[n][index] = i.x_charge;
	y_charge[n][index] = i.y_charge;
	energy[n][index] = i.energy;
	z[n][index] = i.z;
	x_size[n][index] = i.x_size;
	y_size[n][index] = i.y_size;
	index++;
    }
}

void GEMTree::PushCoordinate(GEMCoordinate gem_coord)
{
    int index = 0;
    int n = 0;

    empty_gem_event = false; // We want to fill coord for every event, even if no GEM hit
    
    nhits[n] = 1;
    x[n][index]=gem_coord.X;
    y[n][index] = gem_coord.Y;
    x_charge[n][index] = gem_coord.charge_x;
    y_charge[n][index] = gem_coord.charge_y;
    energy[n][index] = 0.0;
    z[n][index] = 0.0;
    x_size[n][index] = 1.0;
    y_size[n][index] = 1.0;
}



void GEMTree::FillGEMTree()
{
    if( ! empty_gem_event){
	gem_tree->Fill();
        //cout<<nhits[0]<<", "<<nhits[1]<<", "<<nhits[2]<<", "<<nhits[3]<<endl;
        //getchar();
    }
    empty_gem_event = true;
}


void GEMTree::PushTDCValue(unordered_multimap<string, double> &tdc_map)
{
    vector<float> scin_val[2];

    for(int i=0;i<2;i++)
    {
	auto range = tdc_map.equal_range( (string)scin_group[i] );
	for(auto it=range.first;it!=range.second;++it)
	    scin_val[i].push_back( (*it).second );
    }

    vector<float> hycal_val[4];

    for(int i=0;i<hycal_group_q;i++)
    {
	auto range = tdc_map.equal_range( (string)hycal_group[i] );
	for(auto it=range.first;it!=range.second;++it)
	    hycal_val[i].push_back( (*it).second );
    }

    int index = 0;
    // fill scintillator tdc
    n_S1 = scin_val[0].size();
    for(auto &ii: scin_val[0])
    {
	TDCS1[index] = ii;
	index++;
    }
    index = 0;
    n_S2 = scin_val[1].size();
    for(auto &ii: scin_val[1])
    {
	TDCS2[index] = ii;
	index++;
    }
    // fill hycal tdc
    index = 0; n_hycal = 0;
    for(int i=0;i<hycal_group_q;i++)
    {
	n_hycal += hycal_val[i].size();
	for(auto &ii: hycal_val[i])
	{
	    TDCHyCal[index] = ii;
	    index++;
	}
    }
}

// ------------------------------- epics tree -------------------------------------
void GEMTree::InitEpicsTree()
{
    //file = new TFile("root_file/epics_res.root", "recreate");
    epics_tree = new TTree("epics_tree", "epics tree");
    epics_tree -> SetDirectory(file);

    string expression("");

    expression+="MFC_Flow/D:Cell_Gas_T:Tank_P_P:Chamber_P:";
    expression+="Cell_P:Cell_Body_T:hallb_ptrans_y2_encoder:";
    expression+="hallb_ptrans_y1_encoder: hallb_ptrans_x_encoder:";
    expression+="ptrans_y:ptrans_x:hallb_IPM2H01_CUR:";
    expression+="hallb_IPM2H01_YPOS:hallb_IPM2H01_XPOS:";
    expression+="hallb_IPM2C24A_CUR:hallb_IPM2C24A_XPOS:";
    expression+="hallb_IPM2C21A_CUR:hallb_IPM2C21A_YPOS:";
    expression+="hallb_IPM2C21A_XPOS:hallb_IPM2C24A_YPOS:";
    expression+="scaler_calc1:VCG2H02A:VCG2H01A:VCG2H00A:";
    expression+="VCG2C24A:VCG2C21A:VCG2C21:MBSY2C_energy";

    epics_tree->Branch("epics", &_epics_tree, expression.c_str());
    epics_tree->Branch("evtID", &event_id, "evtID/I");
}

void GEMTree::PushEpics(unordered_map<string, double> & epics_map)
{
    //memset(&_epics_tree, 0, sizeof _epics_tree);
    _epics_tree.Clear();
    if(epics_map.size() == 0)
	return;

    if(epics_map.find("TGT:PRad:MFC_Flow") != epics_map.end())
	_epics_tree.MFC_Flow = epics_map["TGT:PRad:MFC_Flow"];
    if(epics_map.find("TGT:PRad:Cell_Gas_T") != epics_map.end() )
	_epics_tree.Cell_Gas_T = epics_map["TGT:PRad:Cell_Gas_T"];
    if(epics_map.find("TGT:PRad:Tank_P_P") != epics_map.end() )
	_epics_tree.Tank_P_P = epics_map["TGT:PRad:Tank_P_P"];
    if(epics_map.find("TGT:PRad:Chamber_P") != epics_map.end() )
	_epics_tree.Chamber_P = epics_map["TGT:PRad:Chamber_P"];
    if(epics_map.find("TGT:PRad:Cell_P") != epics_map.end() )
	_epics_tree.Cell_P = epics_map["TGT:PRad:Cell_P"];
    if(epics_map.find("TGT:PRad:Cell_Body_T") != epics_map.end() )
	_epics_tree.Cell_Body_T = epics_map["TGT:PRad:Cell_Body_T"];
    if(epics_map.find("hallb_ptrans_y2_encoder") != epics_map.end() )
	_epics_tree.hallb_ptrans_y2_encoder = epics_map["hallb_ptrans_y2_encoder"];
    if(epics_map.find("hallb_ptrans_y1_encoder") != epics_map.end() )
	_epics_tree.hallb_ptrans_y1_encoder = epics_map["hallb_ptrans_y1_encoder"];
    if(epics_map.find("hallb_ptrans_x_encoder") != epics_map.end() )
	_epics_tree.hallb_ptrans_x_encoder = epics_map["hallb_ptrans_x_encoder"];
    if(epics_map.find("ptrans_y") != epics_map.end() )
	_epics_tree.ptrans_y = epics_map["ptrans_y"];
    if(epics_map.find("ptrans_x") != epics_map.end() )
	_epics_tree.ptrans_x = epics_map["ptrans_x"];
    if(epics_map.find("hallb_IPM2H01_CUR") != epics_map.end() )
	_epics_tree.hallb_IPM2H01_CUR = epics_map["hallb_IPM2H01_CUR"];
    if(epics_map.find("hallb_IPM2H01_YPOS") != epics_map.end() )
	_epics_tree.hallb_IPM2H01_YPOS = epics_map["hallb_IPM2H01_YPOS"];
    if(epics_map.find("hallb_IPM2H01_XPOS") != epics_map.end() )
	_epics_tree.hallb_IPM2H01_XPOS = epics_map["hallb_IPM2H01_XPOS"];
    if(epics_map.find("hallb_IPM2C24A_CUR") != epics_map.end() )
	_epics_tree.hallb_IPM2C24A_CUR = epics_map["hallb_IPM2C24A_CUR"];
    if(epics_map.find("hallb_IPM2C24A_XPOS") != epics_map.end() )
	_epics_tree.hallb_IPM2C24A_XPOS = epics_map["hallb_IPM2C24A_XPOS"];
    if(epics_map.find("hallb_IPM2C21A_CUR") != epics_map.end() )
	_epics_tree.hallb_IPM2C21A_CUR = epics_map["hallb_IPM2C21A_CUR"];
    if(epics_map.find("hallb_IPM2C21A_YPOS") != epics_map.end() )
	_epics_tree.hallb_IPM2C21A_YPOS = epics_map["hallb_IPM2C21A_YPOS"];
    if(epics_map.find("hallb_IPM2C21A_XPOS") != epics_map.end() )
	_epics_tree.hallb_IPM2C21A_XPOS = epics_map["hallb_IPM2C21A_XPOS"];
    if(epics_map.find("hallb_IPM2C24A_YPOS") != epics_map.end() )
	_epics_tree.hallb_IPM2C24A_YPOS = epics_map["hallb_IPM2C24A_YPOS"];
    if(epics_map.find("scaler_calc1") != epics_map.end() )
	_epics_tree.scaler_calc1 = epics_map["scaler_calc1"];
    if(epics_map.find("VCG2H02A") != epics_map.end() )
	_epics_tree.VCG2H02A = epics_map["VCG2H02A"];
    if(epics_map.find("VCG2H01A") != epics_map.end() )
	_epics_tree.VCG2H01A = epics_map["VCG2H01A"];
    if(epics_map.find("VCG2H00A") != epics_map.end() )
	_epics_tree.VCG2H00A = epics_map["VCG2H00A"];
    if(epics_map.find("VCG2C24A") != epics_map.end() )
	_epics_tree.VCG2C24A = epics_map["VCG2C24A"];
    if(epics_map.find("VCG2C21A") != epics_map.end() )
	_epics_tree.VCG2C21A = epics_map["VCG2C21A"];
    if(epics_map.find("VCG2C21") != epics_map.end() )
	_epics_tree.VCG2C21 = epics_map["VCG2C21"];
    if(epics_map.find("MBSY2C_energy") != epics_map.end() )
	_epics_tree.MBSY2C_energy = epics_map["MBSY2C_energy"];
}

void GEMTree::FillEpicsTree()
{
    if( _epics_tree.MBSY2C_energy == -1000 )
	return;

    epics_tree->Fill();
}

