#include "GEMSignalFitting.h"
#include "GEMMinuitFitting.h"
#include "GEMOnlineHitDecoder.h"
#include "GEMHit.h"
#include "GEMCluster.h"
#include "TGraph.h"
#include "TVirtualFitter.h"
#include "TGraph2D.h"
#include "TCanvas.h"
#include "TFile.h"
#include "TMinuit.h"
#include "TFile.h"
#include <map>
#include <list>
#include <iostream>
#include <vector>

using namespace std;

GEMSignalFitting::GEMSignalFitting()
{
    h_start = new TH1F("h_start", "h_start", 500, -250, 250);
    h_shape = new TH1F("h_shape", "h_shape", 500, 0, 500);
    h_chisquare = new TH1F("h_chisquare", "h_chisquare", 1000, 0, 500);
    h_shape_max = new TH2F("h_shape_max", "h_shape_max", 20000, 0, 2000, 5000, 0, 500);
    h_max_adc = new TH1F("h_max_adc", "h_max_adc", 5000, 0, 5000);
}

GEMSignalFitting::~GEMSignalFitting()
{
}

void GEMSignalFitting::Write()
{
    TFile *f = new TFile("signal.root", "recreate");
    h_start->Write();
    h_shape->Write();
    h_chisquare->Write();
    h_shape_max->Write();
    h_max_adc -> Write();
    f->Close();
}

void GEMSignalFitting::Fit()
{
    SetTSADC();
}


void GEMSignalFitting::SetGEMOnlineHitDecoder(GEMOnlineHitDecoder* d)
{
    hit_decoder = d;
}


void GEMSignalFitting::SetTSADC()
{
    hit_ts_adc.clear();
    cluster_ts_adc.clear();

    map<TString, list<GEMHit*>>* mhit 
	= hit_decoder->GetListOfHitsCleanFromPlanes();

    map<TString, list<GEMCluster*>>* mcluster 
	= hit_decoder->GetListOfClustersCleanFromPlane();

    // hit adc vs time bin
    for(auto &i: *mhit)
    {
        //cout<<"Plane: "<<i.first<<endl;
	for(auto &j: i.second)
	{
	    vector<double> time_bin_adc;
            double _max_adc = 0.;

	    for(auto &k: j->GetTimeBinADCs()){
	        //cout<<"time bin: "<<k.first<<", adc: "<<k.second<<endl;
		time_bin_adc.push_back(k.second);
                if(k.second > _max_adc) _max_adc = k.second;
	    }

            // this histogram for data quality check, do not remove it
            h_max_adc -> Fill(_max_adc);

	    //cout<<"pedestal noise: "<<j->GetPedestalNoise()<<endl;
	    //cout<<"enter to continue ..."<<endl;
	    //getchar();

	    FitGraphChi2(time_bin_adc);

            //double noise = j->GetPedestalNoise();
            //FitGraphNLL(time_bin_adc, noise);

	    //cout<<"next strip: "<<endl;
	}
    }

    // cluster adc vs time bin
    for(auto &i: *mcluster)
    {
        //cout<<"Plane: "<<i.first<<endl;
	for(auto &j: i.second){

	    int z = 0;
	    for(auto &k: j->GetClusterTimeBinADCs()){
	        //cout<<"time bin: "<<z<<", adc: "<<k<<endl;
		z++;
	    }
	}
    }

    //getchar();
}

void GEMSignalFitting::FitGraphChi2(const vector<double> & v)
{
    int N = v.size();

    double x[N];
    double y[N];

    //cout<<"time bins: "<<N<<endl;
    double max_adc = 0;
    int max_bin = 0;

    for(int i=0;i<N;i++)
    {
        x[i] = 25*i;
        y[i] = v[i];
        if(y[i]> max_adc){
            max_adc = y[i];
            max_bin = i;
        }
    }

    //if( !(max_bin == 1 || max_bin == 2) ) return;
    if( max_bin != 2) return;
    if(max_adc < 150) return;
    if(max_adc > 1500) return;

    // fitting shape
    TF1 fV("fV", "[0]*(x-[2])/([1])*exp(-(x-[2])/[1])"); //fitting funtion
    fV.SetRange(-300,500);
    fV.SetParameters(700.0, 65.0, 20.0);
    TGraph g(N, x, y);
    g.Fit(&fV, "Q");

    double a = fV.GetParameter(0);
    double shapet = fV.GetParameter(1);
    double startt = fV.GetParameter(2);
    double chi2 = fV.GetChisquare();
    int ndf = fV.GetNDF();

    //if(a < 1500)
    {
        h_start -> Fill(startt);
        h_shape -> Fill(shapet);
        h_shape_max -> Fill(max_adc, shapet);
        h_chisquare->Fill(chi2/ndf);
    }

/*
    TFile *f = new TFile("f.root", "recreate");
    TCanvas *c = new TCanvas("c", "c", 10, 10, 800, 600);
    g.Draw("APL*");
    c->Modified();
    c->Update();
    c->Write();
    cout<<"enter to contiue..."<<endl;
    f->Close();
    getchar();
    delete c;
*/
}
void GEMSignalFitting::FitGraphNLL(const vector<double> & v, const double &noise)
{
    int N = v.size();

    double x[N];
    double y[N];

    // for calculating parameter start values
    double MaxTsAdc=0; int MaxTimeBin=0;


    //cout<<"time bins: "<<N<<endl;

    for(int i=0;i<N;i++)
    {
        x[i] = 25*i+12.5;
        y[i] = v[i];
        Sigma[i] = noise;

        if(y[i]>MaxTsAdc){
            MaxTsAdc = y[i];
            MaxTimeBin = i;
        }
    }


    TGraph agr;
    for(int i=0;i<N;i++){
        int j = agr.GetN();
        agr.SetPoint(j, x[i], y[i]);
    }

    // For Fitting
    int npar = 3;
    TMinuit minuit(npar);
    minuit.SetPrintLevel(-1);
    minuit.SetFCN(fcn);

    // define the model to be used
    Double_t xmin = -500;
    Double_t xmax = (kMAXADC + .5)*25.0;
    TF1 fV("fV", Pdf, xmin, xmax, npar);
    // parameters for the model
    Double_t par[npar];
    Double_t stepSize[npar];
    Double_t minVal[npar];
    Double_t maxVal[npar];
    TString parName[npar];
    // initial values
    par[0] = MaxTsAdc * 2.5;//700;       // guesses for starting the fit
    par[1] = 55;       // this MUST be done by some means to get things started
    par[2] = (MaxTimeBin+0.5)*25.0 - 60.0;//120;       //start time

    // step size
    //stepSize[0] = TMath::Abs(par[0]*0.0001);   // usually 10% is OK for an initial step size, YMMV
    //stepSize[1] = TMath::Abs(par[1]*0.001);   // step size MUST be positive!
    //stepSize[2] = TMath::Abs(par[2]*0.001);
    stepSize[0] = TMath::Abs(par[0]*0.1);   // usually 10% is OK for an initial step size, YMMV
    stepSize[1] = TMath::Abs(par[1]*0.1);   // step size MUST be positive!
    stepSize[2] = TMath::Abs(par[2]*0.1);


    // parameter range
    minVal[0] = 0.0;      // if min and max values = 0, parameter is unbounded.
    maxVal[0] = 0;//5000000;
    minVal[1] = 30;//30.0; 
    maxVal[1] = 90;//300;
    minVal[2] = -50;//10;
    maxVal[2] = 50;//300;
    // parameter name
    parName[0] = "A";
    parName[1] = "shape";
    parName[2] = "start";

    for(int i=0;i<npar;i++)
    {
        minuit.DefineParameter(i, parName[i].Data(), par[i], stepSize[i], minVal[i], maxVal[i]);
    }
    fparam = &fV; // Here passes our pdf to fcn
    minuit.Migrad(); //Minuit's best minimization do the fitting

    // get fitting results
    Double_t outpar[npar];
    Double_t err[npar];
    for(int i=0;i<npar;i++)
    {
        minuit.GetParameter(i,outpar[i],err[i]);
    }

    fV.SetParameters(outpar);
    double MaxFromFitting = fV.GetMaximum(xmin, xmax);
    double MaxTimeFromFitting = fV.GetMaximumX();
    //ReducedChisquareFromFitting = fV.GetChisquare()/fV.GetNDF();

    double StartTime=fV.GetParameter(2);
    double ShappingTime=fV.GetParameter(1);
    cout<<"start time: "<<StartTime<<", shapping time: "<<ShappingTime<<endl;

    TCanvas *c = new TCanvas("c", "", 10, 10, 800, 600);
    agr.Draw("APL*");
    fV.Draw("same");
    c->Update();
    cout<<"e to c ..."<<endl;
    getchar();

}

