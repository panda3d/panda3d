// Filename: HelixClient.cxx
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

////////////////////////////////////////////////////////////////////
// Panda Header Files
////////////////////////////////////////////////////////////////////
#include "helixclient.h"

// Initialize Static Data Members.
TypeHandle HelixClient::_type_handle;

// Static Members used for the DoEvents Method. These might
// be removed in the future since Panda calls the Win32 API
// Message calls internally.
const int HelixClient::_time_delta = 2000;
const int HelixClient::_sleep_time = 10;
const int HelixClient::_guid_length = 64;

////////////////////////////////////////////////////////////////////
// Method: HelixClient::HelixClient()
// Access: PUBLISHED
// Purpose: This is the default constructor for creating the
//          the wrapper object that encapsulates the Helix Engine.
//          The constructor calls the init() which loads the main
//          Helix DLL, clntcore.dll. 
////////////////////////////////////////////////////////////////////
// Params: None
// Return: None
////////////////////////////////////////////////////////////////////
HelixClient::HelixClient() 
  : _engine(0),
    _dll_access(0) {
  const char * envName = "PLAYER";
  char* envPath = getenv( envName );
  _dll_home = string(string(envPath) + "\\ExtLib\\HelixLib\\.");
  init();
}

////////////////////////////////////////////////////////////////////
// Method: HelixClient::~HelixClient()
// Access: PUBLISHED
// Purpose: This is the destructor for the HelixClient proxy class.
//          The destructor simply invokes the shutdown(), which 
//          closes any open players on the engine, successfully
//          frees the DLLAccess and engine objects.
////////////////////////////////////////////////////////////////////
// Params: None
// Return: None
////////////////////////////////////////////////////////////////////
HelixClient::~HelixClient() {
  shutdown();
}

////////////////////////////////////////////////////////////////////
// Method: HelixClient::do_event()
// Access: PUBLISHED
// Purpose: This method is was used primarily in the Win32 
//          test application to give the Helix Engine the necessary
//          time slices to keep it running properly. In addition,
//          it has been modified to manually set each texture in
//          the _textures map to dirty. This means that the image
//          will be loaded once again into memory, essentially
//          updating the texture each frame with the new video
//          image applied to it.
////////////////////////////////////////////////////////////////////
// Params: None
// Return: None
////////////////////////////////////////////////////////////////////
void HelixClient::do_event() {
  // Legacy Win32 API code from the test application. This can 
  // probably be removed since Panda already calls these methods
  // for retreiving and dispatching Win32 messages.
  MSG msg;
  GetMessage(&msg, NULL, 0, 0);
  DispatchMessage(&msg);

  // Simply iterate through the _textures map and set the texture
  // dirty so that it will be updated in memory.
  TEXTURES::iterator iter;
  for(iter = _textures.begin(); iter != _textures.end(); iter++) {
    iter->second->mark_dirty(Texture::DF_image);
  }
}

////////////////////////////////////////////////////////////////////
// Method: HelixClient::do_events()
// Access: PUBLISHED
// Purpose: This method was used primarily in the Win32 
//          test application to give the Helix Engine the necessary
//          time slices to keep it running properly. It is still
//          used in the HelixClient::begin() to get the engine
//          started for playback, but it most likely can be safely
//          removed in the future.
////////////////////////////////////////////////////////////////////
// Params: None
// Return: None
////////////////////////////////////////////////////////////////////
void HelixClient::do_events() {
  MSG msg;
  DWORD start_time, end_time, i;
  BOOL sleep = TRUE;
  static const int check_interval = 10;
  start_time = GetTickCount();
  end_time = start_time + _time_delta;
  i = 0;
  while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
    DispatchMessage(&msg);
    if((i % check_interval) == 0) {
      if(GetTickCount() > end_time) {
        break;
      }
      ++i;
    }
    sleep = FALSE;
  }
  if(sleep) {
    Sleep(_sleep_time);
  }
}


////////////////////////////////////////////////////////////////////
// Method: HelixClient::create_player(...)
// Access: PUBLISHED
// Purpose: This wrapper method encapsulates the creation of a
//          player object for the Helix Engine. 
////////////////////////////////////////////////////////////////////
// Params: const string &name - Desired name of the Player
//        Texture* tex - Panda Texture that is to be udpated with
//                       the video that this player will run.
//        bool sink_on - Determines with the HxAdviseSink Object 
//                       should print out the presentation stats
//                       in the command prompt.
// Return: None
////////////////////////////////////////////////////////////////////
void HelixClient::create_player(const string &name, Texture* tex, bool sink_on) {
  // Check to make certain that the engine interface has been
  // instantiated, otherwise, players cannot be created!
  if (_engine != 0) {

    // For now, check if there is an actual Panda texture present. If not, 
    // then this player should not be created.
    
    // NOTE: This portion of the interface must be redesigned as the current
    // design forces the user to specify a valid texture. A more suitable
    // approach would be to automatically generate a panda texture and allow
    // the user to retreive that texture and set it for any piece of geometry
    // within the world.
    if( tex == 0 ) {
      STDOUT("ERROR: HelixClient::CreatePlayer, INVALID Texture Object!\n");
      STDOUT("ERROR: NO PLAYER CREATED!!\n");
      return;
    }
    
    // Initialize necessary Helix pointers.
    IHXPlayer* tmp_player = 0;
    HxClientContext* context = 0;
    IHXErrorSinkControl* controller = 0;
    IHXErrorSink* error_sink = 0;

    // Tell the texture object to keep the actual image buffer in memory. If
    // the image buffer is deallocated, which is the default in panda, Helix
    // and Panda because the engine will try to Blit to an invalid buffer.
    tex->set_keep_ram_image(true);

    // Try and create a valid helix player on the engine
    if (HXR_OK != _engine->CreatePlayer(tmp_player)) {
      STDOUT("ERROR: HelixClient::CreatePlayer, Failed Player Creation!\n");
    }
    else {
      // Since a player has been successfully instantiated, a context
      // for that player must be created.
      context = new HxClientContext(_players.size());
      if(context != 0) {
        context->AddRef();
      
        // Specify a default GUID. Not really necessary, but here for
        // convenience if needed later on.
        char guid[_guid_length + 1];
        IHXPreferences * pref = 0;
        guid[0] = '\0';

        // Query the Preferences Interface for the player.
        tmp_player->QueryInterface(IID_IHXPreferences, (void**)&pref);
      
        // Send the Texture buffer down into the Context. It will then
        // ship the buffer to the site supplier, where the actual site
        // and surface are generated.
        context->init(tmp_player, pref, guid, sink_on, tex);
        tmp_player->SetClientContext(context);
        HX_RELEASE(pref);

        // Query the Error Sink Controller
        tmp_player->QueryInterface(IID_IHXErrorSinkControl, (void**)&controller);
        if(controller != 0) {
          context->QueryInterface(IID_IHXErrorSink, (void**)&error_sink);
          if(error_sink != 0) {
            controller->AddErrorSink(error_sink, HXLOG_EMERG, HXLOG_INFO);
          }
          HX_RELEASE(error_sink);
          error_sink = 0;
        }
        HX_RELEASE(controller);
        controller = 0;
        context = 0;
      }

      // Create a new map object for the player and its respective texture.
      pair<string, IHXPlayer*> player(name, tmp_player);
      pair<string, PT(Texture)> texture(name, tex);
    
      // Now that the pair has been created, set the tmp_player
      // address to 0 to protect against dangling references.
      tmp_player = 0;

      // Now, actually insert the pairs into their respective maps.
      _players.insert(player);
      _textures.insert(texture);
    }
  }
  else {
    STDOUT("ERROR: In HelixClient::CreatePlayer, pEngine = NULL");
  }
}

////////////////////////////////////////////////////////////////////
// Method: HelixClient::close_player(...)
// Access: PUBLISHED
// Purpose: This wrapper method encapsulates the closing of a
//          player object for the Helix Engine. This method
//          determines if a player with the designated name exists
//          by searching the _players map. If it does, it properly
//          closes the player, deletes it from memory, and removes
//          the pair instance from the map as well as its texture
//          counter part.
////////////////////////////////////////////////////////////////////
// Params: const string &name - Desired name of the Player to close.
// Return: None
////////////////////////////////////////////////////////////////////
void HelixClient::close_player(const string &name) {
  if (_engine != 0) {
    IUnknown* context = 0;
    // Determine if the player is in the map. If so, go about
    // removing that player from the engine.
    PLAYERS::iterator iter = _players.find(name);
    if (iter != _players.end()) {
      iter->second->Stop();
      iter->second->GetClientContext(context);
      context->Release();
      context = 0;
      _engine->ClosePlayer(iter->second);
      iter->second->Release();
      iter->second = 0;
      // Remove the player its associated texture from their 
      // respective maps.
      _players.erase(name);
      _textures.erase(name);
    }
  }
}

////////////////////////////////////////////////////////////////////
// Method: HelixClient::close_all_players()
// Access: PUBLISHED
// Purpose: This wrapper method encapsulates the closing of every
//          player object for the Helix Engine. This method simply
//          iterates through the _players map, and invokes the
//          the close_player routine to actually close the player.
////////////////////////////////////////////////////////////////////
// Params: none
// Return: None
////////////////////////////////////////////////////////////////////
void HelixClient::close_all_players() {
  if (_engine != 0) {
    if (!_players.empty()) {
      IUnknown* context = 0;
      PLAYERS::iterator iter;

      // Iterate through the _players map to close each player
      // currently associated with the engine.
      unsigned int total_players = _players.size();
      for (unsigned int cntr = 0;  cntr <= total_players; cntr++) {
        close_player(_players.begin()->first); 
      }
    }
  }
}

////////////////////////////////////////////////////////////////////
// Method: HelixClient::get_player_count()
// Access: PUBLISHED
// Purpose: This wrapper method simply returns the current number
//          of players that are associated with the Helix Engine.
////////////////////////////////////////////////////////////////////
// Params: None
// Return: Int - the number of players currently associated with
//               the Helix Engine.
////////////////////////////////////////////////////////////////////
int HelixClient::get_player_count() {
  if (_engine != 0)  {
    return _engine->GetPlayerCount();
  }
  else {
    return 0;
  }
}

////////////////////////////////////////////////////////////////////
// Method: HelixClient::init()
// Access: Private
// Purpose: This member function provides all of the dirty work
//          for instantiating a Helix Engine object. First it 
//          searches for the main Helix DLL, clntcore.dll, and then
//          it instantiates the actual engine itself.
////////////////////////////////////////////////////////////////////
// Params: None
// Return: bool - True if the engine is successfully initialized.
////////////////////////////////////////////////////////////////////
bool HelixClient::init() {
  // Local Variable Defination and Initialization
  FPRMCREATEENGINE create_engine = 0;
  FPRMSETDLLACCESSPATH set_dll_access_path = 0;
  // Instantiate a DLLAccess object which will be used to properly
  // load the clntcore.dll at runtime.
  _dll_access = new DLLAccess();

  // Attempt to load the clntcore.dll. If this fails, an error message will
  // be generated and printed to the console.
  STDOUT("HelixClient is looking for the client core at %s\n", _dll_home.c_str());
  if(DLLAccess::DLL_OK != _dll_access->open(string(_dll_home+"\\clntcore.dll").c_str())) {
    const char * error_string = 0;
    error_string = _dll_access->getErrorString();
    STDERR("HelixClient: %s\n\n", error_string);

    // Print out an explanation of what is wrong.
    STDERR("You must tell the player where to find the client core and\n");
    STDERR("all of its supporting DLLs and codecs.\n");

    // Deallocate the memory from the heap to avoid a memory leak.
    delete _dll_access;
    _dll_access = 0;
    return false;
  }

  // Now that the client core has been loaded, retrieve the function pointer
  // to the engine and the DLL access path.
  create_engine = (FPRMCREATEENGINE) _dll_access->getSymbol("CreateEngine");
  set_dll_access_path = (FPRMSETDLLACCESSPATH) _dll_access->getSymbol("SetDLLAccessPath");

  // If either function pointer was not retreived, print an error message to
  // the console, deallocate the DLLAccess object from memory, and abort the
  // initialization of the engine.
  if((create_engine == 0) || (set_dll_access_path == 0)) {
    STDOUT("---{ERROR: HelixClient::Init, set_dll_access_path}---");
    
    delete _dll_access;
    _dll_access = 0;
    return false;
  }
  
  // Now that the clntcore.dll has been succesfully loaded. Specify where it
  // can find all of the accompanying DLLs that are necessary for Helix to 
  // function.
  char paths[256]; 
  char* path_next_position = paths;
  memset(paths, 0, 256);
  UINT32 bytes_left = 256;

  char next_path[256]; 
  memset(next_path, 0, 256);

  // Apply the Common DLL Path to the paths string. For simplicity,
  // this is the same path as the clntcore.dll path.
  SafeSprintf(next_path, 256, "DT_Common=%s", _dll_home.c_str());
  STDERR("Common DLL path %s\n", next_path );
  UINT32 bytes_to_copy = strlen(next_path) + 1;
  if (bytes_to_copy <= bytes_left)
    {
      memcpy(path_next_position, next_path, bytes_to_copy);
      path_next_position += bytes_to_copy;
      bytes_left -= bytes_to_copy;
    }

  // Apply the Plug-in DLL Path to the paths string. For simplicity,
  // this is the same path as the clntcore.dll path.
  SafeSprintf(next_path, 256, "DT_Plugins=%s", _dll_home.c_str());
  STDERR("Plugin path %s\n", next_path ); 
  bytes_to_copy = strlen(next_path) + 1;
  
  if (bytes_to_copy <= bytes_left)
    {
      memcpy(path_next_position, next_path, bytes_to_copy); 
      path_next_position += bytes_to_copy;
      bytes_left -= bytes_to_copy;
    }

  // Apply the Codecs DLL Path to the paths string. For simplicity,
  // this is the same path as the clntcore.dll path.
  SafeSprintf(next_path, 256, "DT_Codecs=%s", _dll_home.c_str());
  bytes_to_copy = strlen(next_path) + 1;
  if (bytes_to_copy <= bytes_left)
    {
      memcpy(path_next_position, next_path, bytes_to_copy); 
      path_next_position += bytes_to_copy;
      bytes_left -= bytes_to_copy;
      *path_next_position='\0';
    }
  STDOUT((char*)paths);
  set_dll_access_path((char*)paths);
  
  // The dll_access_path has been set, so the engine can now be initialized. If
  // this fails, then deallocate the DLLAccess object from memory and abort the
  // initialization of the engine.
  if (HXR_OK != create_engine((IHXClientEngine**) &_engine)) {
    STDOUT("---{ERROR: HelixClient::Init, Creating Engine Problem}---");

    delete _dll_access;
    _dll_access = 0;
    return false;
  }
  return false;
}

////////////////////////////////////////////////////////////////////
// Method: HelixClient::shutdown()
// Access: PUBLISHED
// Purpose: This member function closes all open players before it
//          proceeds to close the engine itself.
////////////////////////////////////////////////////////////////////
// Params: None
// Return: None
////////////////////////////////////////////////////////////////////
void HelixClient::shutdown() {
  if (_engine != 0) {
    // Close All Open Players that are associated with the
    // given Helix Engine.
    close_all_players();
    
  
    // Retrieve the Function Pointer for closing the Helix engine.
    FPRMCLOSEENGINE close_engine = (FPRMCLOSEENGINE) _dll_access->getSymbol("CloseEngine");
  
    // If the function pointer was successfully retrieved, close 
    // the helix engine.
    if (close_engine != 0) {
      STDOUT("Closing Helix Engine...\n");
      close_engine(_engine);
      _engine = 0;
    }

    // Close the link with the clntCore.dll and freeing the DLLAccess
    // object from memory to prevent a memory leak on the heap.
    STDOUT("Closing the link to clntCore.dll...\n");
    _dll_access->close();
    delete _dll_access;
    _dll_access = 0;
  }
}

////////////////////////////////////////////////////////////////////
// Method: HelixClient::begin(...)
// Access: PUBLISHED
// Purpose: This wrapper function encapsulates the Helix Player
//          Begin method. It first searches for the name of the 
//          specified player within the _players map. If that 
//          is found, it calls the IHXPlayer::Begin method.
////////////////////////////////////////////////////////////////////
// Params: const string &name - Name of the Player to begin playback.
// Return: None
////////////////////////////////////////////////////////////////////
void HelixClient::begin(const string &name) {
  if (_engine != 0) {

    // Search for the specified player in the map. If the player
    // is found, begin playback of the media file.
    PLAYERS::iterator iter = _players.find(name);
    if (iter != _players.end()) {
      iter->second->Begin();
    }
    
    // This is legacy code from the test application, however, it
    // was necessary for helix to function since do_events was called
    // to give helix the necessary time-slice of the CPU. I think this
    // most likely can be removed since Panda calls the necessary 
    // Win32 API Message calls.
    //UINT curr_time = 0;
    //UINT32 start_time = GetTickCount();
    //UINT32 end_time = start_time + _time_delta;
    //while(1) {
    // do_events();
    //curr_time = GetTickCount();
    //if(curr_time >= end_time)
    // break;
    //}
  }
}

////////////////////////////////////////////////////////////////////
// Method: HelixClient::begin_all()
// Access: PUBLISHED
// Purpose: This wrapper function simply calls the IHXPlayer::Begin
//          routine for each player currently associated with the
//          the engine.
////////////////////////////////////////////////////////////////////
// Params: None
// Return: None
////////////////////////////////////////////////////////////////////
void HelixClient::begin_all() {
  if (_engine != 0) {
    
    // Iterate through the _players map and call the Begin 
    // routine to start playback of the media.
    PLAYERS::iterator iter;
    for(iter = _players.begin(); iter != _players.end(); iter++) {
      iter->second->Begin();
    } 
    
    // This is legacy code from the test application, however, it
    // was necessary for helix to function since do_events was called
    // to give helix the necessary time-slice of the CPU. I think this
    // most likely can be removed since Panda calls the necessary 
    // Win32 API Message calls.
    //    UINT curr_time = 0;
    //    UINT32 start_time = GetTickCount();
    //      UINT32 end_time = start_time + _time_delta;
    //      while(1) {
    //        do_events();
    //        curr_time = GetTickCount();
    //        if (curr_time >= end_time)
    //         break;
    //      }
  }
}

////////////////////////////////////////////////////////////////////
// Method: HelixClient::pause(...)
// Access: PUBLISHED
// Purpose: This wrapper function encapsulates the Helix Player
//          Pause method. It first searches for the name of the 
//          specified player within the _players map. If that 
//          is found, it calls the IHXPlayer::Pause method.
////////////////////////////////////////////////////////////////////
// Params: const string &name - Name of the Player to pause playback.
// Return: None
////////////////////////////////////////////////////////////////////
void HelixClient::pause(const string &name) {
  // Search for the specified player in the map. If the player
  // is found, pause the playback of the media file.
  if (_engine != 0) {
    PLAYERS::iterator iter = _players.find(name);
    if (iter != _players.end()) {
      iter->second->Pause();
    }
  }
}

////////////////////////////////////////////////////////////////////
// Method: HelixClient::pause_all()
// Access: PUBLISHED
// Purpose: This wrapper function simply calls the IHXPlayer::Pause
//          routine for each player currently associated with the
//          the engine.
////////////////////////////////////////////////////////////////////
// Params: None
// Return: None
////////////////////////////////////////////////////////////////////
void HelixClient::pause_all() {
  if (_engine != 0) {
    // Iterate through the _players map and call the Pause 
    // routine to start playback of the media.
    PLAYERS::iterator iter;
    for(iter = _players.begin(); iter != _players.end(); iter++) {
      iter->second->Pause();
    }
  }
}

////////////////////////////////////////////////////////////////////
// Method: HelixClient::seek(...)
// Access: PUBLISHED
// Purpose: This wrapper function encapsulates the Helix Player
//          Pause method. It first searches for the name of the 
//          specified player within the _players map. If that 
//          is found, it calls the IHXPlayer::Pause method.
////////////////////////////////////////////////////////////////////
// Params: const string &name - Name of the Player to seek to a 
//                             current time in the playback.
//        long &time  - the time to seek to in the playback.
// Return: None
////////////////////////////////////////////////////////////////////
void HelixClient::seek_time(const string &name, long time) {
  if (_engine != 0) {
    // Search for the specified player in the map. If the player
    // is found, seek to the current time in the playback of the
    // media file.
    PLAYERS::iterator iter = _players.find(name);
    if (iter != _players.end()) {
      iter->second->Seek(time);
    }
  }
}

////////////////////////////////////////////////////////////////////
// Method: HelixClient::stop(...)
// Access: PUBLISHED
// Purpose: This wrapper function encapsulates the Helix Player
//          Stop method. It first searches for the name of the 
//          specified player within the _players map. If that 
//          is found, it calls the IHXPlayer::Stop method which
//          stops the playback of the media.
////////////////////////////////////////////////////////////////////
// Params: const string &name - Name of the Player to stop playback.
// Return: None
////////////////////////////////////////////////////////////////////
void HelixClient::stop(const string &name) {
  if (_engine != 0) {
    // Search for the specified player in the map. If the player
    // is found, stop the playback of the media file.
    PLAYERS::iterator iter = _players.find(name);
    if (iter != _players.end()) {
      iter->second->Stop();
    }
  } 
}

////////////////////////////////////////////////////////////////////
// Method: HelixClient::stop_all()
// Access: PUBLISHED
// Purpose: This wrapper function simply calls the IHXPlayer::Stop
//          routine for each player currently associated with the
//          the engine.
////////////////////////////////////////////////////////////////////
// Params: None
// Return: None
////////////////////////////////////////////////////////////////////
void HelixClient::stop_all() {
  if (_engine != 0) {
    // Iterate through the _players map and call the Pause 
    // routine to stop playback of the media.
    PLAYERS::iterator iter;
    for(iter = _players.begin(); iter != _players.end(); iter++) {
      iter->second->Stop();
    }
  }
}

////////////////////////////////////////////////////////////////////
// Method: HelixClient::open_url(...)
// Access: PUBLISHED
// Purpose: This wrapper function encapsulates the Helix Player
//          OpenURL method. It first searches for the name of the 
//          specified player within the _players map. If that 
//          is found, it calls the IHXPlayer::OpenURL method which
//          actually opens the specified media format. URLs can 
//          come in two forms:
//
//          Remote File: rtsp://www.realone.com/somevideo.ram
//          Local File: file://f:\\smildemohurl.ram 
////////////////////////////////////////////////////////////////////
// Params: const string &name - Name of the Player to open the URL.
//        string url - Name of the URL to be opened for playback.
// Return: None
////////////////////////////////////////////////////////////////////
void HelixClient::open_url(const string &name, string url) {
  if (_engine != 0) {
    // Determine if a URL has been specified within the string.
    // If it hasn't, assume that the request has been made for
    // a local file, so insert, file://, at the beginning of the
    // string.
    if (url.find("://")==string::npos) {
      url.insert(0,"file://");
    }

    // Check wheter the player specified exists
    // within the map. If it does, call the IHXPlayer::OpenURL(...)
    PLAYERS::iterator iter = _players.find(name);
    if (iter != _players.end()) {
      iter->second->OpenURL(url.c_str());
    }
  }
}

////////////////////////////////////////////////////////////////////
// Method: HelixClient::is_live(...)
// Access: PUBLISHED
// Purpose: This wrapper function encapsulates the Helix Player
//          IsLive method. It first searches for the name of the 
//          specified player within the _players map. If that 
//          is found, it calls the IHXPlayer::IsLive method. This
//          indicates whether the player contains a live source
//          or not.
////////////////////////////////////////////////////////////////////
// Params: const string &name - Name of the Player to check.
// Return: None
////////////////////////////////////////////////////////////////////
bool HelixClient::is_live(const string &name) {
  if (_engine != 0) {
    // Search for the specified player in the map. If the player
    // is found, check to see if it contains a live media source.
    PLAYERS::iterator iter = _players.find(name);
    if (iter != _players.end()) {
      return (iter->second->IsLive()) ? true : false;
    }
  }
  return false;
}

////////////////////////////////////////////////////////////////////
// Method: HelixClient::is_done(...)
// Access: PUBLISHED
// Purpose: This wrapper function encapsulates the Helix Player
//          IsDone method. It first searches for the name of the 
//          specified player within the _players map. If that 
//          is found, it calls the IHXPlayer::IsDone method. This
//          indicates whether the player is finished playing the
//          the current presentation.
////////////////////////////////////////////////////////////////////
// Params: const string &name - Name of the Player to check.
// Return: bool - true if the player has finished playback, 
//                otherwise it is false.
////////////////////////////////////////////////////////////////////
bool HelixClient::is_done(const string &name) {
  if (_engine != 0) {
    // Search for the specified player in the map. If the player
    // is found, check to see if it has finished playing the
    // the current presentation.
    PLAYERS::iterator iter = _players.find(name);
    if (iter != _players.end()) {
      return iter->second->IsDone();
    }
  }
  return true;
} 

////////////////////////////////////////////////////////////////////
// Method: HelixClient::are_players_done()
// Access: PUBLISHED
// Purpose: This wrapper function simply calls the IHXPlayer::IsDone
//          routine for each player currently associated with the
//          the engine. If all of the players have finished with
//          their respective presentations, it returns true. 
//          Otherwise it would return false.
////////////////////////////////////////////////////////////////////
// Params: None
// Return: bool - true if all players finished playback, otherwise
//                it is false.
////////////////////////////////////////////////////////////////////
bool HelixClient::are_players_done() {
  if (_engine != 0) {
    bool done = true;
    PLAYERS::iterator iter;
    for(iter = _players.begin(); iter != _players.end(); iter++) {
      if(!iter->second->IsDone()) {
        done = false;
      }
    }
    return done;
  }
  return true;
} 

////////////////////////////////////////////////////////////////////
// Method: HelixClient::get_source_count()
// Access: PUBLISHED
// Purpose: This wrapper function simply calls the
//          IHXPlayer::GetSourceCount routine. This returns the
//          number of stream source instances currently used by
//          the player. In case of audio, it would be 1. In case
//          of a movie file, it would be 2. 
////////////////////////////////////////////////////////////////////
// Params: const string &name - Name of the Player to check.
// Return: Int - value of source count or -1 if there is no player
//               or engine.
////////////////////////////////////////////////////////////////////
int HelixClient::get_source_count(const string &name) {
  if (_engine != 0) {
    // Search for the specified player in the map. If the player
    // is found, check to see if it has finished playing the
    // the current presentation.
    PLAYERS::iterator iter = _players.find(name);
    if (iter != _players.end()) {
      return iter->second->GetSourceCount();
    }
  }
  return -1;
}
