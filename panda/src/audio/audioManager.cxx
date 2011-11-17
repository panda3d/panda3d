// Filename: audioManager.cxx
// Created by:  skyler (June 6, 2001)
// Prior system by: cary
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#include "config_audio.h"
#include "audioManager.h"
#include "atomicAdjust.h"
#include "nullAudioManager.h"
#include "windowsRegistry.h"
#include "virtualFileSystem.h"
#include "config_util.h"
#include "load_dso.h"

#ifdef WIN32
#include <windows.h>  // For GetSystemDirectory()
#endif


TypeHandle AudioManager::_type_handle;


namespace {
  AudioManager *create_NullAudioManager() {
    audio_debug("create_NullAudioManager()");
    return new NullAudioManager();
  }
}

Create_AudioManager_proc* AudioManager::_create_AudioManager
    =create_NullAudioManager;

void AudioManager::register_AudioManager_creator(Create_AudioManager_proc* proc) {
  nassertv(_create_AudioManager==create_NullAudioManager);
  _create_AudioManager=proc;
}



// Factory method for getting a platform specific AudioManager:
PT(AudioManager) AudioManager::create_AudioManager() {
  audio_debug("create_AudioManager()\n  audio_library_name=\""<<audio_library_name<<"\"");
  static bool lib_load = false;
  if (!lib_load) {
    lib_load = true;
    if (!audio_library_name.empty() && !(audio_library_name == "null")) {
      Filename dl_name = Filename::dso_filename(
          "lib"+string(audio_library_name)+".so");
      dl_name.to_os_specific();
      audio_debug("  dl_name=\""<<dl_name<<"\"");
      void *handle = load_dso(get_plugin_path().get_value(), dl_name);
      if (handle == (void *)NULL) {
        audio_error("  load_dso(" << dl_name << ") failed, will use NullAudioManager");
        audio_error("    "<<load_dso_error());
        nassertr(_create_AudioManager == create_NullAudioManager, NULL);
      } else {
        // Get the special function from the dso, which should return
        // the AudioManager factory function.
        string lib_name = audio_library_name;
        if (lib_name.substr(0, 2) == "p3") {
          lib_name = lib_name.substr(2);
        }
        string symbol_name = "get_audio_manager_func_" + lib_name;
        void *dso_symbol = get_dso_symbol(handle, symbol_name);
        if (audio_cat.is_debug()) {
          audio_cat.debug()
            << "symbol of " << symbol_name << " = " << dso_symbol << "\n";
        }
        
        if (dso_symbol == (void *)NULL) {
          // Couldn't find the module function.
          unload_dso(handle);
          handle = NULL;
          audio_error("  Audio library did not provide get_audio_manager_func, will use NullAudioManager");
        } else {
          typedef Create_AudioManager_proc *FuncType();
          Create_AudioManager_proc *factory_func = (*(FuncType *)dso_symbol)();
          AudioManager::register_AudioManager_creator(factory_func);          
        }
      }
    }
  }
  PT(AudioManager) am = (*_create_AudioManager)();
  if (!am->is_exact_type(NullAudioManager::get_class_type()) && !am->is_valid()) {
    audio_error("  " << am->get_type() << " is not valid, will use NullAudioManager");
    am = create_NullAudioManager();
  }
  return am;
}

////////////////////////////////////////////////////////////////////
//     Function: AudioManager::Destructor
//       Access: Published, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
AudioManager::
~AudioManager() {
  if (_null_sound != (AudioSound *)NULL) {
    unref_delete((AudioSound *)_null_sound);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: AudioManager::Constructor
//       Access: Protected
//  Description: 
////////////////////////////////////////////////////////////////////
AudioManager::
AudioManager() {
  _null_sound = NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: AudioManager::shutdown
//       Access: Published, Virtual
//  Description: Call this at exit time to shut down the audio system.
//               This will invalidate all currently-active
//               AudioManagers and AudioSounds in the system.  If you
//               change your mind and want to play sounds again, you
//               will have to recreate all of these objects.
////////////////////////////////////////////////////////////////////
void AudioManager::
shutdown() {
}

////////////////////////////////////////////////////////////////////
//     Function: AudioManager::get_null_sound
//       Access: Public
//  Description: Returns a special NullAudioSound object that has all
//               the interface of a normal sound object, but plays no
//               sound.  This same object may also be returned by
//               get_sound() if it fails.
////////////////////////////////////////////////////////////////////
PT(AudioSound) AudioManager::
get_null_sound() {
  if (_null_sound == (AudioSound *)NULL) {
    AudioSound *new_sound = new NullAudioSound;
    new_sound->ref();
    void *result = AtomicAdjust::compare_and_exchange_ptr(_null_sound, (void *)NULL, (void *)new_sound);
    if (result != NULL) {
      // Someone else must have assigned the AudioSound first.  OK.
      nassertr(_null_sound != new_sound, NULL);
      unref_delete(new_sound);
    }
    nassertr(_null_sound != NULL, NULL);
  }
  
  return (AudioSound *)_null_sound;
}

////////////////////////////////////////////////////////////////////
//     Function: AudioManager::getSpeakerSetup()
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
int AudioManager::
getSpeakerSetup() {
  // intentionally blank
  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: AudioManager::setSpeakerSetup()
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void AudioManager::
setSpeakerSetup(SpeakerModeCategory cat) {
  // intentionally blank
}

////////////////////////////////////////////////////////////////////
//     Function: AudioManager::configure_filters
//       Access: Published
//  Description: Configures the global DSP filter chain.
//
//               There is no guarantee that any given configuration
//               will be supported by the implementation.  The only 
//               way to find out what's supported is to call 
//               configure_filters.  If it returns true, the
//               configuration is supported.
////////////////////////////////////////////////////////////////////
bool AudioManager::
configure_filters(FilterProperties *config) {
  const FilterProperties::ConfigVector &conf = config->get_config();
  if (conf.empty()) {
    return true;
  } else {
    return false;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: AudioManager::update()
//       Access: Published, Virtual
//  Description: Must be called every frame.  Failure to call this
//               every frame could cause problems for some audio
//               managers.
////////////////////////////////////////////////////////////////////
void AudioManager::
update() {
  // Intentionally blank.
}

////////////////////////////////////////////////////////////////////
//     Function: AudioManager::audio_3d_set_listener_attributes
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void AudioManager::audio_3d_set_listener_attributes(PN_stdfloat px, PN_stdfloat py, PN_stdfloat pz, PN_stdfloat vx, PN_stdfloat vy, PN_stdfloat vz, PN_stdfloat fx, PN_stdfloat fy, PN_stdfloat fz, PN_stdfloat ux, PN_stdfloat uy, PN_stdfloat uz) {
    // intentionally blank.
}

////////////////////////////////////////////////////////////////////
//     Function: AudioManager::audio_3d_get_listener_attributes
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void AudioManager::audio_3d_get_listener_attributes(PN_stdfloat *px, PN_stdfloat *py, PN_stdfloat *pz, PN_stdfloat *vx, PN_stdfloat *vy, PN_stdfloat *vz, PN_stdfloat *fx, PN_stdfloat *fy, PN_stdfloat *fz, PN_stdfloat *ux, PN_stdfloat *uy, PN_stdfloat *uz) {
    // intentionally blank.
}

////////////////////////////////////////////////////////////////////
//     Function: AudioManager::audio_3d_set_distance_factor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void AudioManager::audio_3d_set_distance_factor(PN_stdfloat factor) {
    // intentionally blank.
}

////////////////////////////////////////////////////////////////////
//     Function: AudioManager::audio_3d_get_distance_factor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
PN_stdfloat AudioManager::audio_3d_get_distance_factor() const {
    // intentionally blank.
    return 0.0f;
}

////////////////////////////////////////////////////////////////////
//     Function: AudioManager::audio_3d_set_doppler_factor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void AudioManager::audio_3d_set_doppler_factor(PN_stdfloat factor) {
    // intentionally blank.
}

////////////////////////////////////////////////////////////////////
//     Function: AudioManager::audio_3d_get_doppler_factor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
PN_stdfloat AudioManager::audio_3d_get_doppler_factor() const {
    // intentionally blank.
    return 0.0f;
}

////////////////////////////////////////////////////////////////////
//     Function: AudioManager::audio_3d_set_drop_off_factor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void AudioManager::audio_3d_set_drop_off_factor(PN_stdfloat factor) {
    // intentionally blank.
}

////////////////////////////////////////////////////////////////////
//     Function: AudioManager::audio_3d_get_drop_off_factor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
PN_stdfloat AudioManager::audio_3d_get_drop_off_factor() const {
    // intentionally blank.
    return 0.0f;
}

////////////////////////////////////////////////////////////////////
//     Function: AudioManager::get_dls_pathname
//       Access: Published, Static
//  Description: Returns the full pathname to the DLS file, as
//               specified by the Config.prc file, or the default for
//               the current OS if appropriate.  Returns empty string
//               if the DLS file is unavailable.
////////////////////////////////////////////////////////////////////
Filename AudioManager::
get_dls_pathname() {
  Filename dls_filename = audio_dls_file;
  if (!dls_filename.empty()) {
    VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
    vfs->resolve_filename(dls_filename, get_model_path());
    
    return dls_filename;
  }

#ifdef WIN32
  Filename pathname;

  // Get the registry key from DirectMusic
  string os_filename = WindowsRegistry::get_string_value("SOFTWARE\\Microsoft\\DirectMusic", "GMFilePath", "");

  if (!os_filename.empty()) {
    pathname = Filename::from_os_specific(os_filename);
  } else {
    char sysdir[MAX_PATH+1];
    GetSystemDirectory(sysdir,MAX_PATH+1);
    pathname = Filename(Filename::from_os_specific(sysdir), Filename("drivers/gm.dls"));
  }
  pathname.make_true_case();
  return pathname;

#elif defined(IS_OSX)
  // This appears to be the standard place for this file on OSX 10.4.
  return Filename("/System/Library/Components/CoreAudio.component/Contents/Resources/gs_instruments.dls");

#else
  return Filename();
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: AudioManager::output
//       Access: Published, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void AudioManager::
output(ostream &out) const {
  out << get_type();
}

////////////////////////////////////////////////////////////////////
//     Function: AudioManager::write
//       Access: Published, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void AudioManager::
write(ostream &out) const {
  out << (*this) << "\n";
}

////////////////////////////////////////////////////////////////////
//     Function: AudioManager::set_speaker_configuration
//       Access: Published
//  Description: For use only with Miles.
////////////////////////////////////////////////////////////////////
void AudioManager::
set_speaker_configuration(LVecBase3 *speaker1, LVecBase3 *speaker2, LVecBase3 *speaker3, LVecBase3 *speaker4, LVecBase3 *speaker5, LVecBase3 *speaker6, LVecBase3 *speaker7, LVecBase3 *speaker8, LVecBase3 *speaker9) {
  // intentionally blank
}
