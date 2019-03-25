//==================================================================//
//                                                                  //
// Xinzhan Bai                                                      //
// 02/10/2017                                                       //
// xb4zp@virginia.edu                                               //
//==================================================================//

#ifndef SIGNAL_FITTING_h
#define SIGNAL_FITTING_h

#include <vector>
#include <TH1F.h>
#include <TH2F.h>

class GEMOnlineHitDecoder;
class GEMHit;
class GEMCluster;

class GEMSignalFitting
{
public:
    GEMSignalFitting();
    ~GEMSignalFitting();

    void SetGEMOnlineHitDecoder(GEMOnlineHitDecoder* decoder);

    void Fit();
    void SetTSADC();
    void FitGraphChi2(const std::vector<double> &);
    void FitGraphNLL(const std::vector<double> & dat, const double & noise);

    void Write();


private:
    GEMOnlineHitDecoder *hit_decoder;

    std::vector<double> hit_ts_adc;
    std::vector<double> cluster_ts_adc;

    TH1F* h_start;
    TH1F* h_shape;
    TH1F* h_chisquare;
    TH2F* h_shape_max;

    TH1F* h_max_adc;
};

#endif
