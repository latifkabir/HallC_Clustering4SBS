//==================================================================//
//                                                                  //
// Xinzhan Bai                                                      //
// 03/20/2016                                                       //
// xb4zp@virginia.edu                                               //
//==================================================================//

#ifndef GEMANALYZER_H
#define GEMANALYZER_H

#include <string>

class GEMConfigure;
class GEMEvioParser;
class GEMDataHandler;
class GEMEventAnalyzer;
class GEMPedestal;
class GEMPhysics;
class EpicsEventAnalyzer;
class EpicsPhysics;
class GEMTree;
class TDCEventAnalyzer;
class GEMEfficiency;
class EventUpdater;

class GEMAnalyzer
{
public:
    GEMAnalyzer();
    GEMAnalyzer(GEMConfigure* config);
    ~GEMAnalyzer();

    void Init(GEMConfigure* config, bool saveRootFile = false);
    void ProcessFiles();
    void Process();
    void SetDataHandler();
    void RegisterClass();
    void ProcessResults();
    void GetRunType();
    GEMEvioParser* GetParser() {return parser;}
    GEMDataHandler* GetHandler() {return handler;}
    GEMTree* GetTree() {return res_tree;}

private:
    std::string file;
    GEMConfigure *configure;
    GEMEvioParser *parser;
    GEMDataHandler *handler;
    GEMEventAnalyzer *gem_event_analyzer;
    GEMPedestal * pedestal;
    GEMPhysics * physics;
    EpicsEventAnalyzer * epics_event_analyzer;
    EpicsPhysics * epics_physics;
    GEMTree *res_tree;
    TDCEventAnalyzer *tdc_event_analyzer;
    GEMEfficiency * gem_efficiency;
    EventUpdater * event_updater;

    // run type
    int gem_physics;
    int gem_pedestal;
    int gem_raw;
};

#endif
