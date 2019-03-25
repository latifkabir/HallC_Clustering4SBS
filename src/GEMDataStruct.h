//==================================================================//
//                                                                  //
// Xinzhan Bai                                                      //
// 03/20/2016                                                       //
// xb4zp@virginia.edu                                               //
//==================================================================//

#ifndef __GEMDATASTRUCT__H__
#define __GEMDATASTRUCT__H__

#include <vector>

struct EventHeader
{
    unsigned int length;
    unsigned char num;
    unsigned char type;
    unsigned short tag;
};

struct GEMClusterStruct
{
    float x;
    float y;
    float x_charge;
    float y_charge;
    float energy;
    float z;
    int x_size;
    int y_size;
    int chamber_id;

    GEMClusterStruct(){};
    ~GEMClusterStruct(){};

    GEMClusterStruct(float xi, float yi, 
	    float cix = 0., float ciy = 0., 
	    float ei = 0) 
	: x(xi), y(yi), x_charge(cix), y_charge(ciy), energy(ei) 
    {
	z = 0;
	x_size = 0;
	y_size = 0;
	chamber_id = -99;
    }

    GEMClusterStruct( const GEMClusterStruct & gem_cluster)
	: x(gem_cluster.x), y(gem_cluster.y),  
	x_charge(gem_cluster.x_charge), 
	y_charge(gem_cluster.y_charge),
	energy(gem_cluster.energy), 
	z(gem_cluster.z), 
	x_size(gem_cluster.x_size),
	y_size(gem_cluster.y_size),
	chamber_id(gem_cluster.chamber_id){}

    void SetEnergy(float e) {energy = e;}
    void SetX(float xp) {x = xp;}
    void SetY(float xp) {y = xp;}
    void SetXCharge(float xp) {x_charge = xp;}
    void SetYCharge(float xp) {y_charge = xp;}
    void SetZ(float xp) { z = xp;}
    void SetXClusterSize( int s) {x_size = s;}
    void SetYClusterSize( int s) {y_size = s;}
};

struct HyCalClusterStruct
{
#define TIME_MEASURE_SIZE 3
    int flag;    // overall status of cluster
    int det_id;  // detecotr id
    int type;    // cluster type: 0, 1, 2, 3, 4; -1
    int status;  // spliting status
    int nblocks; // nb of blocks in a cluster
    int cid;     // cluster's central cell id
    float energy;// cluster energy
    float x;     // x-position (mm)
    float y;     // y-position (mm)
    float z;     // z-position (mm)
    float chi2;  // chi2 comparing to shower profile
    float sigma_E;
    int time[TIME_MEASURE_SIZE]; // time information from central TDC group

    HyCalClusterStruct()
	: flag(0), det_id(0), type(0), status(0), nblocks(0), cid(0),
	energy(0), x(0), y(0), z(0), chi2(0), sigma_E(0)
    {
        clear_time();
    };

    HyCalClusterStruct(float xi, float yi, float energyi)
	: flag(0), det_id(0), type(0), status(0), nblocks(0), cid(0),
	energy(energyi), x(xi), y(yi), z(0), chi2(0), sigma_E(0)
    {
	clear_time();
    }

    HyCalClusterStruct( const HyCalClusterStruct & h)
	: flag(h.flag), det_id(h.det_id), type(h.type), status(h.status), nblocks(h.nblocks), cid(h.cid),
	energy(h.energy), x(h.x), y(h.y), z(h.z), chi2(h.chi2), sigma_E(h.sigma_E)
    {
        for(int i=0;i<TIME_MEASURE_SIZE;++i)
	{
	    time[i] = h.time[i];
	}
    }

    void clear_time()
    {
	for(int i=0;i<TIME_MEASURE_SIZE;++i)
	    time[i] = 0;
    }

    void set_time(const std::vector<unsigned short> &t)
    {
	for(int i=0;i<TIME_MEASURE_SIZE;++i)
	{
	    if( i<(int)t.size())
		time[i] = (int)t[i];
	    else
		time[i] = 0;
	}
    }
};

struct EpicsStruct
{
    double MFC_Flow;
    double Cell_Gas_T;
    double Tank_P_P;
    double Chamber_P;
    double Cell_P;
    double Cell_Body_T;
    double hallb_ptrans_y2_encoder;
    double hallb_ptrans_y1_encoder;
    double hallb_ptrans_x_encoder;
    double ptrans_y;
    double ptrans_x;
    double hallb_IPM2H01_CUR;
    double hallb_IPM2H01_YPOS;
    double hallb_IPM2H01_XPOS;
    double hallb_IPM2C24A_CUR;
    double hallb_IPM2C24A_XPOS;
    double hallb_IPM2C21A_CUR;
    double hallb_IPM2C21A_YPOS;
    double hallb_IPM2C21A_XPOS;
    double hallb_IPM2C24A_YPOS;
    double scaler_calc1;
    double VCG2H02A;
    double VCG2H01A;
    double VCG2H00A;
    double VCG2C24A;
    double VCG2C21A;
    double VCG2C21;
    double MBSY2C_energy;

    EpicsStruct()
	: MFC_Flow(-1000), Cell_Gas_T(-1000), Tank_P_P(-1000),
	Chamber_P(-1000), Cell_P(-1000), Cell_Body_T(-1000),
	hallb_ptrans_y2_encoder(-1000), hallb_ptrans_y1_encoder(-1000),
	hallb_ptrans_x_encoder(-1000), ptrans_y(-1000), ptrans_x(-1000),
	hallb_IPM2H01_CUR(-1000), hallb_IPM2H01_YPOS(-1000),
	hallb_IPM2H01_XPOS(-1000), hallb_IPM2C24A_CUR(-1000),
	hallb_IPM2C24A_XPOS(-1000), hallb_IPM2C21A_CUR(-1000),
	hallb_IPM2C21A_YPOS(-1000), hallb_IPM2C21A_XPOS(-1000),
	hallb_IPM2C24A_YPOS(-1000), scaler_calc1(-1000),
	VCG2H02A(-1000), VCG2H01A(-1000), VCG2H00A(-1000),
	VCG2C24A(-1000), VCG2C21A(-1000), VCG2C21(-1000),
	MBSY2C_energy(-1000)
    {};

    void Clear()
    {
	MFC_Flow = -1000.; Cell_Gas_T = -1000.; Tank_P_P = -1000.;
	Chamber_P = -1000.; Cell_P = -1000.;  Cell_Body_T = -1000.;
	hallb_ptrans_y2_encoder = -1000.; hallb_ptrans_y1_encoder = -1000.;
	hallb_ptrans_x_encoder = -1000.; ptrans_y = -1000.; ptrans_x = -1000.;
	hallb_IPM2H01_CUR = -1000.; hallb_IPM2H01_YPOS = -1000.;
	hallb_IPM2H01_XPOS = -1000.; hallb_IPM2C24A_CUR = -1000.;
	hallb_IPM2C24A_XPOS = -1000.; hallb_IPM2C21A_CUR = -1000.;
	hallb_IPM2C21A_YPOS = -1000.; hallb_IPM2C21A_XPOS = -1000.;
	hallb_IPM2C24A_YPOS=-1000.; scaler_calc1=-1000.;
	VCG2H02A = -1000.; VCG2H01A = -1000.; VCG2H00A = -1000.;
	VCG2C24A = -1000.; VCG2C21A =-1000.; VCG2C21 =-1000.;
	MBSY2C_energy = -1000.;
    }
};

struct moller_data_t
{
    unsigned int event_id;
    int chamber_id1;
    float x1;
    float y1;
    float x1_hycal;
    float y1_hycal;
    float e1;
    float angle1;
    int chamber_id2;
    float x2;
    float y2;
    float x2_hycal;
    float y2_hycal;
    float e2;
    float angle2;

    float coplanarity;

    moller_data_t()
	: event_id(0), chamber_id1(0),  x1(0),y1(0), 
	x1_hycal(0), y1_hycal(0),
	e1(0), angle1(0), chamber_id2(0), x2(0), y2(0), 
	x2_hycal(0), y2_hycal(0),
	e2(0), angle2(0), coplanarity(0)
    {}
};

struct ep_data_t
{
    unsigned int event_id;
    int chamber_id;
    float x;
    float y;
    float x_hycal;
    float y_hycal;
    float e;
    float angle;

    ep_data_t()
	: event_id(0), chamber_id(0), x(0),
	y(0), x_hycal(0), y_hycal(0), e(0), angle(0)
    {}
};

struct Energy_Angle
{
    double energy;
    double angle;
    Energy_Angle(double e, double a)
	: energy(e), angle(a)
    {}
    Energy_Angle()
	: energy(-10000), angle(-10000)
    {}
};


#endif
