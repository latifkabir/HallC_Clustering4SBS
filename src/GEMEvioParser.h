//==================================================================//
//                                                                  //
// Xinzhan Bai                                                      //
// 03/20/2016                                                       //
// xb4zp@virginia.edu                                               //
//==================================================================//

#ifndef GEMEVIOPARSER_H
#define GEMEVIOPARSER_H
#include <string>

#define BUF_SIZE 1000000

class GEMDataHandler;
class GEMConfigure;
class EventUpdater;

class GEMEvioParser
{
public:
    GEMEvioParser();
    ~GEMEvioParser();

    void ParseFile(std::string &);
    void ParseEvent(unsigned int *);
    void SetDataHandler( GEMDataHandler * fHandler);
    void SetEventUpdater(EventUpdater *);
    void SetGEMConfigure(GEMConfigure* con);
    EventUpdater *GetEventUpdater() {return update_event;}

private:
    GEMDataHandler *handler;
    GEMConfigure * configure;
    EventUpdater *update_event;
    int eventLimit;
    int limit;
};

#endif
