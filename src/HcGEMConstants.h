// Filename: HcGEMConstants.h
// Description: 
// Author: Latif Kabir < latif@jlab.org >
// Created: Tue Oct 24 01:35:05 2017 (-0400)
// URL: latifkabir.github.io


//Modify these path before compiling on new system
#define PATH_FOR_CONFIG "/home/latif/GIT/HCANA/hcana_gem/config/gem.cfg"
#define PATH_FOR_MAPPING "/home/latif/GIT/HCANA/hcana_gem/config/mapping.cfg"


#define USE_PEDESTAL_DATA 1   // 1: Used pedestal data file under pedestal, 0: use last time bin for pedestal

#define NADC 6                 // Number of ADC
#define NTIME_BINS 21          // NUmber of time bins
#define N_STRIPS 128           // Number of strip of the GEM readout board
#define HEADER_CUTOFF 1500     // Header cutoff level
#define BLOCK_LENGTH 140       // 12 headers + 128 channels 
#define GEM_LENGTH 15.36       // 15.36 cm x 15.36 cm GEM


//For Multi-Cluster Algorithm, lower threshold is fine
//For Cosmic
// #define THRESHOLD_HI_X 150
// #define THRESHOLD_HI_Y 100

// #define THRESHOLD_LOW_X 100
// #define THRESHOLD_LOW_Y 50

//With Beam
#define THRESHOLD_HI_X 200
#define THRESHOLD_HI_Y 150

#define THRESHOLD_LOW_X 150
#define THRESHOLD_LOW_Y 100


//Number of time-bins to be fitted
//Best combination for cosmic 8, 5, 4
#define FITTED_TBINS 8
#define MATCHING_LIMIT 4  //5
#define EXTENDED_BINS 4

//For Single-Cluster Algorithm, needs higher threshold
#define THRESHOLD_HI 400
#define THRESHOLD_LOW 250

//Timing Cut
//(3-7) Best, (3-6) better, (2-7) default
#define APPLY_TIMING_CUT 1
#define TIME_LOW 3
#define TIME_HIGH 7 //6 best


//---------- Hall C GEM Mapping -----------------------

#define X1_ADC_NO 2
#define X2_ADC_NO 0
#define X3_ADC_NO 1

#define Y1_ADC_NO 3
#define Y2_ADC_NO 5
#define Y3_ADC_NO 4



/*

GEM Coordinate System:

     Y1         Y2            Y3
     --------------------------> Y     HV Connector Here
X1   |
     |     
X2   |       Readout Board
     |
X3   |
    \|/
     X


Hall C Coordinate System:

 Beam left  Y<------------------------x Z (beam direction)
	                              |
				      |
				      |
				      |
				      |
				      |
				      |
				     \|/
				      X (Towards the floor) 


*/
