#define NOEVIO

#include "GEMEvioParser.h"
#include "GEMDataHandler.h"
#include "GEMDataStruct.h"
#include "datastruct.h"
#include "GEMConfigure.h"
#ifndef NOEVIO
#include <evioUtil.hxx>
#include <evioFileChannel.hxx>
#endif
#include "EventUpdater.h"

#define HEADER_SIZE 2

using namespace std;
#ifndef NOEVIO
using namespace evio;
#endif

GEMEvioParser::GEMEvioParser() {
    eventLimit = 3000;
    limit = 0;
}

GEMEvioParser::~GEMEvioParser()
{
}


void GEMEvioParser::SetGEMConfigure(GEMConfigure* con)
{
    configure = con;

    eventLimit = configure->GetNumEvtForPed();
}


void GEMEvioParser::SetDataHandler(GEMDataHandler *fHandler)
{
    handler = fHandler;
}

void GEMEvioParser::SetEventUpdater(EventUpdater *fupdate)
{
    update_event = fupdate;
}


#ifdef NOEVIO
void GEMEvioParser::ParseFile(string & file)
{
}
#else
void GEMEvioParser::ParseFile(string & file)
{
    cout<<"evio Parser: pasing file: "
	<<file<<endl;
    try {
	if(limit == eventLimit)
	    return;
	evioFileChannel chan(file.c_str(),"r", BUF_SIZE);
	chan.open();
	int event_index = 0;

	while(chan.read())
	{
	    if(event_index == eventLimit)
		break;
	    event_index ++;
	    limit ++;
	    //cout<<"event: "<<event_index<<endl;
	    ParseEvent(const_cast<unsigned int *> (chan.getBuffer() ) );
	    update_event->Update();
	}
	chan.close();
    } catch (evioException e) {
	cerr << endl <<e.toString()<<endl<<endl;
    }

}
#endif

void GEMEvioParser::ParseEvent(unsigned int * buf)
{
    EventHeader * header = (EventHeader*) &buf[0];

    switch(header->tag)
    {
        case CODA_Event1: // For the time being check all possible event tags
        case CODA_Event2:
        case CODA_Event3:
        case CODA_Event4:
        case CODA_Event5:
        case CODA_Event6:
        case CODA_Event7:
        case CODA_Event8:
        case CODA_Event9:
        case CODA_Event10:
	{
	    handler -> Decode(&buf[HEADER_SIZE], buf[0]-HEADER_SIZE);
	    break;
	}
	case CODA_Sync:
	    break;
	case EPICS_Info:
	{
	    handler -> DecodeEpics(&buf[HEADER_SIZE], buf[0]-HEADER_SIZE);
	    break;
	}
	case CODA_Prestart:
	    break;
	case CODA_Go:
	    break;
	case CODA_End:
	    break;
	default:
	    return;
    }
}

