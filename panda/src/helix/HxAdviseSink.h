// Filename: HxAdviseSink.h
// Created by:  jjtaylor (10Feb04)
//
////////////////////////////////////////////////////////////////////
//
//
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////
#ifndef HXADVISESINK_H
#define HXADVISESINK_H

////////////////////////////////////////////////////////////////////
// Panda Header Files
////////////////////////////////////////////////////////////////////
#include "pandabase.h"

////////////////////////////////////////////////////////////////////
// Helix Main Header Files
////////////////////////////////////////////////////////////////////
#include "mainHelix.h"

////////////////////////////////////////////////////////////////////
// Prototype/Struct/Class Forward Delcarations
////////////////////////////////////////////////////////////////////
struct IHXClientAdviseSink;
struct IUnknown;
struct IHXRegistry;
struct IHXScheduler;

////////////////////////////////////////////////////////////////////
// Class: HxAdviseSink
// Purpose: This is a derived class from the IHXClientAdviseSink
//          base interface. This class is meant to receive 
//          notifications from the client engine about changes in
//          or about a presentation's playback status.
////////////////////////////////////////////////////////////////////
class HxAdviseSink : public IHXClientAdviseSink {
  public:
    HxAdviseSink(IUnknown* unknown, LONG32 client_index, bool sink_on);

////////////////////////////////////////////////////////////////////
// IHUnkown Interface Methods Prototypes
// Purpose: Implements the basic COM interface methods for reference
//          coutning and querying of related interfaces.
////////////////////////////////////////////////////////////////////
    STDMETHOD(QueryInterface) (THIS_ REFIID id, void** interface_obj);
    STDMETHOD_(ULONG32,AddRef) (THIS);
    STDMETHOD_(ULONG32,Release) (THIS);

////////////////////////////////////////////////////////////////////
// IHXClientAdviseSink Interface Methods Prototypes
// Purpose: Implements the necessary interface methods that may
//          be called during a presentations status.
////////////////////////////////////////////////////////////////////
    STDMETHOD(OnPosLength) (THIS_ UINT32 position, UINT32 length);
    STDMETHOD(OnPresentationOpened) (THIS);
    STDMETHOD(OnPresentationClosed) (THIS);
    STDMETHOD(OnStatisticsChanged) (THIS);
    STDMETHOD(OnPreSeek) (THIS_ ULONG32 old_time, ULONG32  new_time);
    STDMETHOD(OnPostSeek) (THIS_ ULONG32 old_time, ULONG32 new_time);
    STDMETHOD(OnStop) (THIS);
    STDMETHOD(OnPause) (THIS_ ULONG32 Time);
    STDMETHOD(OnBegin) (THIS_ ULONG32 Time);
    STDMETHOD(OnBuffering) (THIS_ ULONG32 flags, UINT16 percent_complete);
    STDMETHOD(OnContacting) (THIS_ const char* host_name);
  private:
	// Private Member Functions
    ~HxAdviseSink();
	HX_RESULT dump_reg_tree(const char* tree_name );
	void get_statistics (char*  registry_key);
    void get_all_statistics (void);

	// Private Member variables
	LONG32 _ref_count;
    LONG32 _client_index;
	UINT32 _start_time;
    UINT32 _stop_time;
	UINT32 _current_bandwidth;
    UINT32 _average_bandwidth;
    BOOL   _on_stop;

	bool _sink_on;
    
    IUnknown* _unknown;
    IHXRegistry* _registry;
    IHXScheduler* _scheduler;
};
#endif 