// Filename: HelixClient.h
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
#ifndef HELIXCLIENT_H
#define HELIXCLIENT_H

////////////////////////////////////////////////////////////////////
// Panda Header Files
////////////////////////////////////////////////////////////////////
#include "pandabase.h"

#include "typedObject.h"
#include "texture.h"
#include "pmap.h"

////////////////////////////////////////////////////////////////////
// Helix Main Header Files
////////////////////////////////////////////////////////////////////
#include <mainHelix.h>

////////////////////////////////////////////////////////////////////
// Normal Header Files
////////////////////////////////////////////////////////////////////
#include <string>
#include <iostream>

////////////////////////////////////////////////////////////////////
// Class: HelixClient
// Purpose: This proxy class provides the top-level interface for
//          loading the Helix Client Engine, creating players on
//          the Helix engine, and initializing the media playback.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA HelixClient {
public: 
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    register_type(_type_handle, "HelixClient");
  }
private:
  static TypeHandle _type_handle;
PUBLISHED:
  HelixClient();
  ~HelixClient();
  
////////////////////////////////////////////////////////////////////
// IHXClientEngine Interface Methods
// Purpose: These are wrapper methods that invoke the actual Helix
//          Engine Interface method calls. For instance, the method
//          create_player invotes the Helix Engine method
//          engine->CreatePlayer().
////////////////////////////////////////////////////////////////////
  void create_player(const string &name, Texture* tex = 0, bool sink_on = false);
  void close_player(const string &name);
  void close_all_players();
  int get_player_count();
  
////////////////////////////////////////////////////////////////////
// IHXPlayer Interface Methods Prototypes
// Purpose: These are wrapper methods that invoke the actual Helix
//          Player Interface method calls. For instance, the method
//          begin invotes the Helix Engine method player->Begin().
////////////////////////////////////////////////////////////////////
  void begin(const string &name);
  void pause(const string &name);
  void seek_time(const string &name, long time);
  void stop(const string &name);

  void begin_all();
  void pause_all();
  void stop_all();

  void open_url(const string &name, string url);
  int get_source_count(const string &name);
  bool is_done(const string &name);
  bool is_live(const string &name);
  bool are_players_done();
  
////////////////////////////////////////////////////////////////////
// Utility Method Prototypes
// Purpose: These methods provide basic utility functions which are
//          necessary for the Helix engine to play and close.
////////////////////////////////////////////////////////////////////
  void do_event();
  void do_events();
  void shutdown();
private:
  // Private Member Functions
  bool init();

  // Private Member variables
  DLLAccess* _dll_access;

  string _dll_home;
  static const int _time_delta;
  static const int _sleep_time;
  static const int _guid_length;

  // Where the Engine will actually reside.
  IHXClientEngine* _engine;

  typedef pmap<string, IHXPlayer*> PLAYERS;
  typedef pmap<string, PT(Texture)> TEXTURES;

  PLAYERS _players;
  TEXTURES _textures;
}; 
#endif