// Filename: HxAdviseSink.cpp
// Created by:  jjtaylor (27Jan04)
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
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
// Panda Header Files
////////////////////////////////////////////////////////////////////
#include "HxAdviseSink.h"

////////////////////////////////////////////////////////////////////
// Normal Header Files
////////////////////////////////////////////////////////////////////
#include <stdio.h>

////////////////////////////////////////////////////////////////////
//  Method: HxAdviseSink::Constructor
//  Access: Public
//  Purpose: Initializes member variables and sets up object
//               related interfaces.
////////////////////////////////////////////////////////////////////
//  Input: pUnknown - Pointer to an IUnknown Interface
//         client_index - Client Instance Identifier
//  Output: None
////////////////////////////////////////////////////////////////////
HxAdviseSink::HxAdviseSink(IUnknown* unknown, LONG32 client_index, bool sink_on)
  : _ref_count (0), 
    _client_index (client_index),
	_unknown (0),
    _registry (0),
    _scheduler (0),
    _current_bandwidth(0),
    _average_bandwidth(0),
    _on_stop(0),
	_sink_on(sink_on)
{
    if (unknown != 0) {
	  _unknown = unknown;
	  _unknown->AddRef();

	  if (HXR_OK != _unknown->QueryInterface(IID_IHXRegistry, (void**)&_registry)) {
	    _registry = 0;
	  }

	  if (HXR_OK != _unknown->QueryInterface(IID_IHXScheduler, (void**)&_scheduler)) {
	    _scheduler = 0;
	  }

	  IHXPlayer* player;
	  if(HXR_OK == _unknown->QueryInterface(IID_IHXPlayer, (void**)&player)) {
	    player->AddAdviseSink(this);
	    player->Release();
	  }
	}
}

////////////////////////////////////////////////////////////////////
//  Method: HxClientAdviseSink::~HxClientAdviseSink
//  Access: Private
//  Purpose: Class destructor which releases corresponding
//               interfaces which are related to this class.
////////////////////////////////////////////////////////////////////
//  Params: None
//  Return: None
////////////////////////////////////////////////////////////////////
HxAdviseSink::~HxAdviseSink() {
  // Release components and set the member variables to the
  // NULL-state.
  if (_scheduler != 0) {
	_scheduler->Release();
	_scheduler = 0;
  }

  if (_registry != 0) {
    _registry->Release();
	_registry = 0;
  }
 
  if (_unknown != 0) {
    _unknown->Release();
	_unknown = 0;
  }
}

////////////////////////////////////////////////////////////////////
//  Method: HxClientAdviseSink::QueryInterface
//  Access: Public
//  Purpose: Queries this class to determine whether it supports
//               supports a specific interface. If the call succeeds
//               then the methods of that interface are made 
//               available for use.
////////////////////////////////////////////////////////////////////
//  Params: id - Indicates the reference identifier of the 
//                 the interface being queried.
//          interface_obj - Points to an interface pointer that is
//                           filled in if the query succeeds.
//  Return: HX_RESULT - Varies based on wheter the interface is
//                      supported or not.
////////////////////////////////////////////////////////////////////
STDMETHODIMP HxAdviseSink::QueryInterface(REFIID id, void** interface_obj) {
  // Determine if the IUnknown and ClientAdviseSink interfaces
  // are supported.
	if (IsEqualIID(id, IID_IUnknown)) {
	  // Increase the reference count, set the Interface Object,
	  // and return that the interface is supported within this
	  // object.
	  AddRef();
	  *interface_obj = (IUnknown*)(IHXClientAdviseSink*)this;
	}
	else if (IsEqualIID(id, IID_IHXClientAdviseSink)) {
	  // Same as above.
	  AddRef();
	  *interface_obj = (IHXClientAdviseSink*)this;
	}
	else {
	  // This Interface is not supported by this object. Set the
	  // Interface Object to the NULL-state and return.
	  *interface_obj = 0;
	  return HXR_NOINTERFACE;
	}
  return HXR_OK;
}


////////////////////////////////////////////////////////////////////
//  Method: HxClientAdviseSink::AddRef
//  Access: Public
//  Purpose: Increases the classes reference count by one. 
//               Whenever an object is created, it's reference count 
//               begins at 1. If an application calls IUnknown::AddRef, 
//               queries an interface belonging to a specific object, 
//               or uses a creation function like HXCreateInstance, 
//               the reference count is incremented by 1. 
////////////////////////////////////////////////////////////////////
//  Params: None
//  Return: Returns the new reference count.
////////////////////////////////////////////////////////////////////
STDMETHODIMP_(ULONG32) HxAdviseSink::AddRef() {
  return InterlockedIncrement(&_ref_count);
}

////////////////////////////////////////////////////////////////////
//  Method: HxClientAdviseSink::Release
//  Access: Public
//  Purpose: Decreases the object's reference count by one. 
//               Every call to IUnknown::AddRef, 
//               IUnknown::QueryInterface, or a creation function 
//               such as HXCreateInstance must have a corresponding 
//               call to IUnknown::Release. When the reference count 
//               reaches 0 (zero), the object is destroyed. 
////////////////////////////////////////////////////////////////////
//  Params: None
//  Return: Returns the new reference count.
////////////////////////////////////////////////////////////////////
STDMETHODIMP_(ULONG32) HxAdviseSink::Release() {
  // As long as the reference count is greater than 0, then this
  // object is still in "scope." 
  if (InterlockedDecrement(&_ref_count) > 0 ) {
    return _ref_count;
  }

  // Otherwise, this object is no longer necessary and should be 
  // removed from memory.
  delete this;
  return 0;
}

////////////////////////////////////////////////////////////////////
//  Method: HxClientAdviseSink::OnPosLength
//  Access: Public
//  Purpose: Advises the client that the position or length of 
//               the current playback context has changed
////////////////////////////////////////////////////////////////////
//  Params: ulPosition - The new position of the playback.
//          ulLength - The new length of the playback.
//  Return: HXR_OK - Helix Specific result saying things are "Okay".
////////////////////////////////////////////////////////////////////
STDMETHODIMP HxAdviseSink::OnPosLength(ULONG32 ulPosition, ULONG32 ulLength) {
  // Initialize Variables
  if(_sink_on) {
	STDOUT("OnPosLength(%ld, %ld)\n", ulPosition, ulLength);
  }
  return HXR_OK;
}








////////////////////////////////////////////////////////////////////
//  Method: HxClientAdviseSink::OnPresentationOpened
//  Access: Public
//  Purpose: Advises the client that a presentation has been 
//               opened.
////////////////////////////////////////////////////////////////////
//  Params: None
//  Return: HXR_OK - Helix Specific result saying things are "Okay".
////////////////////////////////////////////////////////////////////
STDMETHODIMP HxAdviseSink::OnPresentationOpened() {
  // For now, check if advise sinking is enabled. If so, print that
  // a presenatation has been opened.
  if(_sink_on) {
    STDOUT("OnPresentationOpened()\n");
  }
  return HXR_OK;
}

////////////////////////////////////////////////////////////////////
//  Method: HxClientAdviseSink::OnPresentationClosed()
//  Access: Public
//  Purpose: Advises the cleint that a presentation has been
//               closed.
////////////////////////////////////////////////////////////////////
//  Params: None
//  Return: HXR_OK - Helix Specific result saying things are "Okay".
////////////////////////////////////////////////////////////////////
STDMETHODIMP HxAdviseSink::OnPresentationClosed() {
  // For now, check if advise sinking is enabled. If so, print that
  // a presenatation has been closed.
  if(_sink_on) {
    STDOUT("OnPresentationClosed()\n");
  }
  return HXR_OK;
}

////////////////////////////////////////////////////////////////////
//  Method: HxAdviseSink::get_statistics
//  Access: Private
//  Purpose: Retrieves the statistic for the particular key that
//               is in the Helix Registry Interface.
////////////////////////////////////////////////////////////////////
//  Params: char* registry_key - the key whose value we wish to 
//                               retrieve from the registry.
//  Return: None
////////////////////////////////////////////////////////////////////
void HxAdviseSink::get_statistics(char* registry_key) {
  if(_sink_on) {
	char    RegistryValue[MAX_DISPLAY_NAME] = {0}; 
    INT32   lValue = 0;
    INT32   i = 0;
    INT32   lStatistics = 8;
    UINT32 *plValue;

    // Collect all of the necessary statistics from the registry 
	// and print them to the screen.
    for (i = 0; i < lStatistics; i++) {
	  plValue = NULL;
	  switch (i) {
	    case 0:
	      SafeSprintf(RegistryValue, MAX_DISPLAY_NAME, "%s.Normal", registry_key);
	      break;
	    case 1:
	      SafeSprintf(RegistryValue, MAX_DISPLAY_NAME, "%s.Recovered", registry_key);
	      break;
	    case 2:
	      SafeSprintf(RegistryValue, MAX_DISPLAY_NAME, "%s.Received", registry_key);
	      break;
	    case 3:
	      SafeSprintf(RegistryValue, MAX_DISPLAY_NAME, "%s.Lost", registry_key);
	      break;
	    case 4:
	      SafeSprintf(RegistryValue, MAX_DISPLAY_NAME, "%s.Late", registry_key);
	      break;
	    case 5:
	      SafeSprintf(RegistryValue, MAX_DISPLAY_NAME, "%s.ClipBandwidth", registry_key);
	      break;
	    case 6:
	      SafeSprintf(RegistryValue, MAX_DISPLAY_NAME, "%s._average_bandwidth", registry_key);
	      plValue = &_average_bandwidth;
	      break;
	    case 7:
	      SafeSprintf(RegistryValue, MAX_DISPLAY_NAME, "%s._current_bandwidth", registry_key);
	      plValue = &_current_bandwidth;
	      break;
	    default:
	      break;
	  }

	  _registry->GetIntByName(RegistryValue, lValue);
	  if (plValue) {
	    if (_on_stop || lValue == 0) {
		  lValue = *plValue;
	    }
	    else {
		  *plValue = lValue;
	    }
	  }
	  STDOUT("%s = %ld\n", RegistryValue, lValue);
	}
  }
}

////////////////////////////////////////////////////////////////////
//  Method: HxClientAdviseSink::get_all_statistics
//  Access: Private
//  Purpose: Displays the content of the entire registry. Not
//               truly necessary for Panda, however, it is useful
//               for debugging purposes of streaming files.
////////////////////////////////////////////////////////////////////
//  Params: None
//  Return: None
////////////////////////////////////////////////////////////////////
void HxAdviseSink::get_all_statistics() {
  if(_sink_on) {
    UINT32  PlayerIndex = 0;
    UINT32  SourceIndex = 0;
    UINT32  StreamIndex = 0;

    char*   RegistryPrefix = "Statistics";
    char    RegistryName[MAX_DISPLAY_NAME] = {0};

    // Display the content of whole statistic registry
    if (_registry) {
      // Start from the first player.
	  SafeSprintf(RegistryName, MAX_DISPLAY_NAME, "%s.Player%ld", RegistryPrefix, _client_index);
	  if (PT_COMPOSITE == _registry->GetTypeByName(RegistryName)) {
	    // Display player statistic
	    get_statistics(RegistryName);

	    SafeSprintf(RegistryName, MAX_DISPLAY_NAME, "%s.Source%ld", RegistryName, SourceIndex);
	    while (PT_COMPOSITE == _registry->GetTypeByName(RegistryName)) {
		  // Display source statistic
		  get_statistics(RegistryName);

		  
		  SafeSprintf(RegistryName, MAX_DISPLAY_NAME, "%s.Stream%ld", RegistryName, StreamIndex);
		  while (PT_COMPOSITE == _registry->GetTypeByName(RegistryName)) {
		    // Display stream statistic
		    get_statistics(RegistryName);

		    StreamIndex++;

		    SafeSprintf(RegistryName, MAX_DISPLAY_NAME, "%s.Player%ld.Source%ld.Stream%ld", 
		    RegistryPrefix, PlayerIndex, SourceIndex, StreamIndex);
		  }
          SourceIndex++;
		  SafeSprintf(RegistryName, MAX_DISPLAY_NAME, "%s.Player%ld.Source%ld",
		              RegistryPrefix, PlayerIndex, SourceIndex);
	    }
        PlayerIndex++;
	    SafeSprintf(RegistryName, MAX_DISPLAY_NAME, "%s.Player%ld", 
		            RegistryPrefix, PlayerIndex);
	  }
    }
  }
}
////////////////////////////////////////////////////////////////////
//  Method: HxClientAdviseSink::OnStatisticsChanged
//  Access: Public
//  Purpose: Advises the client that the presentation statistics
//               have changed and prints those statistics out.
////////////////////////////////////////////////////////////////////
//  Params: None
//  Return: HXR_OK - Helix Specific result saying things are "Okay".
////////////////////////////////////////////////////////////////////
STDMETHODIMP HxAdviseSink::OnStatisticsChanged() {
  if(_sink_on) {  
	char        Buff[1024]; 
    HX_RESULT   res = HXR_OK;
    UINT16      Player = 0;

    STDOUT("OnStatisticsChanged():\n");
        
    SafeSprintf(Buff, 1024, "Statistics.Player%u", Player);
    while( HXR_OK == res ) {
      res = dump_reg_tree( Buff );
      Player++;
      SafeSprintf(Buff, 1024, "Statistics.Player%u", Player );
    }
  }
  return HXR_OK;
}

////////////////////////////////////////////////////////////////////
//  Method: HxClientAdviseSink::DumpRegTree
//  Access: Private
//  Purpose: Prints the contents of the Registry tree for this
//               particular object.
////////////////////////////////////////////////////////////////////
//  Params: pszTreeName - The name of the Registry tree that should
//                        be printed.
//  Return: HX_RESULT - result varies.
////////////////////////////////////////////////////////////////////
HX_RESULT HxAdviseSink::dump_reg_tree(const char* tree_name) {
  //Initialize Local Variables related to the Registry Tree	
  const char* pszName = NULL;
  ULONG32     ulRegID   = 0;
  HX_RESULT   res     = HXR_OK;
  INT32       nVal    = 0;
  IHXBuffer* pBuff   = NULL;
  IHXValues* pValues = NULL;

  
  // Check if the name exists in the registry. If its not
  // return a failure.
  res = _registry->GetPropListByName(tree_name, pValues);
  if (HXR_OK!=res || !pValues) {
    return HXR_FAIL;
  }
    
  // Make sure this is a PT_COMPOSITE type registry entry. If
  // its not, return a failure.
  if (PT_COMPOSITE != _registry->GetTypeByName(tree_name)) {
    return HXR_FAIL;
  }

  // Print out the value of each member of this tree.
  res = pValues->GetFirstPropertyULONG32( pszName, ulRegID );
  while( HXR_OK == res ) {
    // There is at least one entry in the Registry. Now
    // check the type.
    HXPropType pt = _registry->GetTypeById(ulRegID);
    switch(pt) {
      case PT_COMPOSITE:
        dump_reg_tree(pszName);
          break;
      case PT_INTEGER:
        nVal = 0;
        _registry->GetIntById( ulRegID, nVal );
        STDOUT("%s : %d\n", pszName, nVal ); 
        break;
      case PT_INTREF :
        nVal = 0;
        _registry->GetIntById( ulRegID, nVal );
        STDOUT("%s : %d\n", pszName, nVal ); 
        break;
      case PT_STRING :
        pBuff = NULL;
        _registry->GetStrById( ulRegID, pBuff );
        STDOUT("%s : \"", pszName ); 
        
		if( pBuff ) {
          STDOUT("%s", (const char *)(pBuff->GetBuffer()) );
		}
        STDOUT("\"\n" ); 
        HX_RELEASE(pBuff);
        break;
      case PT_BUFFER :
        STDOUT("%s : BUFFER TYPE NOT SHOWN\n", pszName, nVal ); 
        break;
      case PT_UNKNOWN:
        STDOUT("%s Unkown registry type entry\n", pszName );
        break;
      default:
        STDOUT("%s Unkown registry type entry\n", pszName );
        break;
	}    
    res = pValues->GetNextPropertyULONG32(pszName, ulRegID);
  }

  HX_RELEASE( pValues );
  return HXR_OK;
}








////////////////////////////////////////////////////////////////////
//  Method: HxClientAdviseSink::OnPreSeek
//  Access: Public
//  Purpose: Informs the client that a seek is about to occur. 
//               The client is informed of the last time in the 
//               stream's time line before the seek, as well as the 
//               first new time in the stream's time line after the 
//               seek is completed. 
////////////////////////////////////////////////////////////////////
//  Params: ulOldTime - The end of the stream's time line before the
//                      current seek.
//          ulNewTime - The beginning of the stream's time line after
//                      the current seek.
//  Return: HXR_OK - Helix Specific result saying things are "Okay".
////////////////////////////////////////////////////////////////////
STDMETHODIMP HxAdviseSink::OnPreSeek(ULONG32 ulOldTime, ULONG32 ulNewTime) {
  // For now, check if advise sinking is enabled. If so, print that
  // a seek is about to happen.
  if(_sink_on) {
    STDOUT("OnPreSeek(%ld, %ld)\n", ulOldTime, ulNewTime);
  }
  return HXR_OK;
}

////////////////////////////////////////////////////////////////////
//  Method: HxClientAdviseSink::OnPostSeek
//  Access: Public
//  Purpose: Informs the client that a seek has just occured. 
//               The client is informed of the last time in the 
//               stream's time line before the seek, as well as the 
//               first new time in the stream's time line after the 
//               seek is completed.         
////////////////////////////////////////////////////////////////////
//  Params: ulOldTime - The end of the stream's time line before the
//                      current seek.
//          ulNewTime - The beginning of the stream's time line after
//                      the current seek.
//  Return: HXR_OK - Helix Specific result saying things are "Okay".
////////////////////////////////////////////////////////////////////
STDMETHODIMP HxAdviseSink::OnPostSeek(ULONG32 ulOldTime, ULONG32 ulNewTime) {
  // For now, check if advise sinking is enabled. If so, print that
  // a seek just happened.
  if(_sink_on) {
    STDOUT("OnPostSeek(%ld, %ld)\n", ulOldTime, ulNewTime);
  }
  return HXR_OK;
}

////////////////////////////////////////////////////////////////////
//  Method: HxClientAdviseSink::OnStop
//  Access: Public
//  Purpose: Informs the client that a stop has just occured
//               in a presentation, and calculates the total time
//               that a file has been playing.
////////////////////////////////////////////////////////////////////
//  Params: None
//  Return: HXR_OK - Helix Specific result saying things are "Okay".
////////////////////////////////////////////////////////////////////
STDMETHODIMP HxAdviseSink::OnStop() {
  if(_sink_on) {
    HXTimeval curr_time;

    STDOUT("OnStop()\n");

    STDOUT("Player %ld stopped.\n", _client_index);
    _on_stop = TRUE;
    get_all_statistics();

    // Retrieve the current time and subtract the beginning time to figure out
    // how long the file has played.
    curr_time = _scheduler->GetCurrentSchedulerTime();
    _stop_time = curr_time.tv_sec;
  }
  return HXR_OK;
}

////////////////////////////////////////////////////////////////////
//  Method: HxClientAdviseSink::OnPause
//  Access: Public
//  Purpose: Informs the client that a pause has just occured.
////////////////////////////////////////////////////////////////////
//  Params: time - time in the stream's time line that the pause
//                   occured.
//  Return: HXR_OK - Helix Specific result saying things are "Okay".
////////////////////////////////////////////////////////////////////
STDMETHODIMP HxAdviseSink::OnPause(ULONG32 time) {
  // For now, check if advise sinking is enabled. If so, print that
  // the a pause in the presentation has occured for debugging
  // purposes.
  if(_sink_on) {
    STDOUT("OnPause(%ld)\n", time);
  }
  return HXR_OK;
}

////////////////////////////////////////////////////////////////////
//  Method: HxClientAdviseSink::OnBegin
//  Access: Public
//  Purpose: Informs the client that a presenentation has begun.
////////////////////////////////////////////////////////////////////
//  Params: time - The time that the presentation has begun.
//  Return: HXR_OK - Helix Specific result saying things are "Okay".
////////////////////////////////////////////////////////////////////
STDMETHODIMP HxAdviseSink::OnBegin(ULONG32 time) {
  if(_sink_on) {
    // Local Variable Declarations
    HXTimeval curr_time;
  
    STDOUT("OnBegin(%ld)\n", time);
    STDOUT("Player %ld beginning playback...\n", _client_index);

    // Record the current time that this begins so that the number
    // of seconds the media file has been playing can be calculated.
    curr_time = _scheduler->GetCurrentSchedulerTime();
    _start_time = curr_time.tv_sec;
  }
  return HXR_OK;
}


















////////////////////////////////////////////////////////////////////
//  Method: HxClientAdviseSink::OnBuffering
//  Access: Public
//  Purpose: Informs the client that buffering of data is occuring. 
//               The render is informed of the reason for the buffering 
//               (start-up of stream, seek has occured, network congestion, 
//               etc.), as well as percentage complete of the buffering process.
////////////////////////////////////////////////////////////////////
//  Params: flags - The reason for buffering. Can be any of the following:
//                    BUFFERING_START_UP, BUFFERING_SEEK,
//                    BUFFERING_CONGESTION, BUFFERING_LIVE_PAUSE
//          percent_complete - The percentage fo the buffering that
//                              has been completed.
//  Return: HXR_OK - Helix Specific result saying things are "Okay".
////////////////////////////////////////////////////////////////////
STDMETHODIMP HxAdviseSink::OnBuffering(ULONG32 flags, UINT16 percent_complete) {
  // For now, check if advise sinking is enabled. If so, print that
  // the a presentation is now buffering.
  if(_sink_on) {
    STDOUT("OnBuffering(%ld, %d)\n", flags, percent_complete);
  }
  return HXR_OK;
}

////////////////////////////////////////////////////////////////////
//  Method: HxClientAdviseSink::OnContacting
//  Access: Public
//  Purpose: Informs the client that the engine is contacting
//               a host.
////////////////////////////////////////////////////////////////////
//  Params: host_name - Host name that the engine is contacting.
//  Return: HXR_OK - Helix Specific result saying things are "Okay".
////////////////////////////////////////////////////////////////////
STDMETHODIMP HxAdviseSink::OnContacting(const char* host_name) {
  // For now, check if advise sinking is enabled. If so, print that
  // the client is contacting a host for streaming.
  if(_sink_on) {
    STDOUT("OnContacting(\"%s\")\n", host_name);
  }
  return HXR_OK;
}