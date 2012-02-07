// Filename: globalMilesManager.cxx
// Created by:  drose (26Jul07)
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

#include "globalMilesManager.h"

#ifdef HAVE_RAD_MSS //[

#include "lightMutexHolder.h"
#include "milesAudioManager.h"
#include "milesAudioSample.h"
#include "milesAudioSequence.h"

#ifdef WIN32
// For midiOutReset()
#include <windows.h>  
#include <mmsystem.h>
#endif

GlobalMilesManager *GlobalMilesManager::_global_ptr;

////////////////////////////////////////////////////////////////////
//     Function: GlobalMilesManager::Constructor
//       Access: Private
//  Description: 
////////////////////////////////////////////////////////////////////
GlobalMilesManager::
GlobalMilesManager() : 
  _managers_lock("GlobalMilesManager::_managers_lock"),
  _samples_lock("GlobalMilesManager::_samples_lock"),
  _sequences_lock("GlobalMilesManager::_sequences_lock")
{
  _digital_driver = 0;
  _midi_driver = 0;
  _dls_device = 0;
  _dls_file = 0;
  _is_open = false;
}

////////////////////////////////////////////////////////////////////
//     Function: GlobalMilesManager::add_manager
//       Access: Public
//  Description: Records a new MilesAudioManager in the world.  This
//               will open the Miles API when the first audio manager
//               is added.
////////////////////////////////////////////////////////////////////
void GlobalMilesManager::
add_manager(MilesAudioManager *manager) {
  LightMutexHolder holder(_managers_lock);
  _managers.insert(manager);
  if (!_is_open) {
    open_api();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GlobalMilesManager::remove_manager
//       Access: Public
//  Description: Records that a MilesAudioManager is destructing.
//               This will clsoe the Miles API when the last audio
//               manager is removed.
////////////////////////////////////////////////////////////////////
void GlobalMilesManager::
remove_manager(MilesAudioManager *manager) {
  LightMutexHolder holder(_managers_lock);
  _managers.erase(manager);
  if (_managers.empty() && _is_open) {
    close_api();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GlobalMilesManager::cleanup
//       Access: Public
//  Description: Calls cleanup() on all MilesAudioManagers, to cause a
//               clean shutdown.
////////////////////////////////////////////////////////////////////
void GlobalMilesManager::
cleanup() {
  LightMutexHolder holder(_managers_lock);
  Managers::iterator mi;
  for (mi = _managers.begin(); mi != _managers.end(); ++mi) {
    (*mi)->cleanup();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GlobalMilesManager::get_sample
//       Access: Public
//  Description: Gets a sample handle from the global pool for the
//               digital output device, to be used with the indicated
//               AudioSound.  
//
//               If successful, sets the sample handle and the index
//               (which should later be used to release the sample)
//               and returns true.  If unsuccessful (because there are
//               no more available handles), returns false.
//
//               This is a very limited resource; you should only get
//               a sample just before playing a sound.
////////////////////////////////////////////////////////////////////
bool GlobalMilesManager::
get_sample(HSAMPLE &sample, size_t &index, MilesAudioSample *sound) {
  LightMutexHolder holder(_samples_lock);

  for (size_t i = 0; i < _samples.size(); ++i) {
    SampleData &smp = _samples[i];
    if (AIL_sample_status(smp._sample) == SMP_DONE) {
      if (smp._sound != NULL) {
        // Tell the last sound that was using this sample that it's
        // done now.
        smp._sound->internal_stop();
      }
      smp._sound = sound;
      sample = smp._sample;
      index = i;
      return true;
    }
  }

  // No more already-allocated samples; get a new one from the system.
  sample = AIL_allocate_sample_handle(_digital_driver);
  if (sample == 0) {
    return false;
  }
  
  AIL_init_sample(sample, DIG_F_STEREO_16, 0);
  index = _samples.size();

  SampleData smp;
  smp._sound = sound;
  smp._sample = sample;
  _samples.push_back(smp);
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: GlobalMilesManager::release_sample
//       Access: Public
//  Description: Indicates that the indicated AudioSound no longer
//               needs this sample.
////////////////////////////////////////////////////////////////////
void GlobalMilesManager::
release_sample(size_t index, MilesAudioSample *sound) {
  LightMutexHolder holder(_samples_lock);
  nassertv(index < _samples.size());

  SampleData &smp = _samples[index];
  if (smp._sound == sound) {
    smp._sound = NULL;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GlobalMilesManager::get_sequence
//       Access: Public
//  Description: Gets a sequence handle from the global pool for the
//               digital output device, to be used with the indicated
//               AudioSound.  
//
//               If successful, sets the sequence handle and the index
//               (which should later be used to release the sequence)
//               and returns true.  If unsuccessful (because there are
//               no more available handles), returns false.
//
//               This is a very limited resource; you should only get
//               a sequence just before playing a sound.
////////////////////////////////////////////////////////////////////
bool GlobalMilesManager::
get_sequence(HSEQUENCE &sequence, size_t &index, MilesAudioSequence *sound) {
  LightMutexHolder holder(_sequences_lock);

  for (size_t i = 0; i < _sequences.size(); ++i) {
    SequenceData &seq = _sequences[i];
    if (AIL_sequence_status(seq._sequence) == SEQ_DONE) {
      if (seq._sound != NULL) {
        // Tell the last sound that was using this sequence that it's
        // done now.
        seq._sound->internal_stop();
      }
      seq._sound = sound;
      sequence = seq._sequence;
      index = i;
      return true;
    }
  }

  // No more already-allocated sequences; get a new one from the system.
  sequence = AIL_allocate_sequence_handle(_midi_driver);
  if (sequence == 0) {
    return false;
  }
  
  index = _sequences.size();

  SequenceData seq;
  seq._sound = sound;
  seq._sequence = sequence;
  _sequences.push_back(seq);
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: GlobalMilesManager::release_sequence
//       Access: Public
//  Description: Indicates that the indicated AudioSound no longer
//               needs this sequence.
////////////////////////////////////////////////////////////////////
void GlobalMilesManager::
release_sequence(size_t index, MilesAudioSequence *sound) {
  LightMutexHolder holder(_sequences_lock);
  nassertv(index < _sequences.size());

  SequenceData &seq = _sequences[index];
  if (seq._sound == sound) {
    seq._sound = NULL;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GlobalMilesManager::force_midi_reset
//       Access: Public
//  Description: Sometimes Miles seems to leave midi notes hanging,
//               even after stop is called, so call this method to
//               perform an explicit reset using winMM.dll calls, just
//               to ensure silence.
////////////////////////////////////////////////////////////////////
void GlobalMilesManager::
force_midi_reset() {
  if (!miles_audio_force_midi_reset) {
    audio_debug("MilesAudioManager::skipping force_midi_reset");  
    return;
  }
  audio_debug("MilesAudioManager::force_midi_reset");

#ifdef WIN32
  if ((_midi_driver!=NULL) && (_midi_driver->deviceid != MIDI_NULL_DRIVER) && (_midi_driver->hMidiOut != NULL)) {
    audio_debug("MilesAudioManager::calling midiOutReset");
    midiOutReset(_midi_driver->hMidiOut);
  }
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: GlobalMilesManager::get_global_ptr
//       Access: Public, Static
//  Description: Returns the pointer to the one GlobalMilesManager
//               object.
////////////////////////////////////////////////////////////////////
GlobalMilesManager *GlobalMilesManager::
get_global_ptr() {
  if (_global_ptr == NULL) {
    _global_ptr = new GlobalMilesManager;
  }
  return _global_ptr;
}

////////////////////////////////////////////////////////////////////
//     Function: GlobalMilesManager::open_api
//       Access: Private
//  Description: Called internally to initialize the Miles API.
////////////////////////////////////////////////////////////////////
void GlobalMilesManager::
open_api() {
  audio_debug("GlobalMilesManager::open_api()")
  nassertv(!_is_open);

  bool use_digital = (audio_play_wave || audio_play_mp3);
  if (audio_play_midi && audio_software_midi) {
    use_digital = true;
  }

#ifdef IS_OSX
  audio_software_midi = true;
#endif
  
  audio_debug("  use_digital="<<use_digital);
  audio_debug("  audio_play_midi="<<audio_play_midi);
  audio_debug("  audio_software_midi="<<audio_software_midi);
  audio_debug("  audio_output_rate="<<audio_output_rate);
  audio_debug("  audio_output_bits="<<audio_output_bits);
  audio_debug("  audio_output_channels="<<audio_output_channels);
  audio_debug("  audio_software_midi="<<audio_software_midi);

#if !defined(NDEBUG) && defined(AIL_MSS_version) //[
  char version[8];
  AIL_MSS_version(version, 8);
  audio_debug("  Mss32.dll Version: "<<version);
#endif //]

  if (!AIL_startup()) {
    milesAudio_cat.warning()
      << "Miles Sound System already initialized!\n";
  }

  AIL_set_file_callbacks(open_callback, close_callback,
                         seek_callback, read_callback);

  if (use_digital) {
    _digital_driver = 
      AIL_open_digital_driver(audio_output_rate, audio_output_bits, 
                              audio_output_channels, 0);
  }

  if (audio_play_midi) {
    if (audio_software_midi) {
      _midi_driver = AIL_open_XMIDI_driver(AIL_OPEN_XMIDI_NULL_DRIVER);

      // Load the downloadable sounds file:
      _dls_device = AIL_DLS_open(_midi_driver, _digital_driver, NULL, 0,
                                 audio_output_rate, audio_output_bits,
                                 audio_output_channels);
      
      Filename dls_pathname = AudioManager::get_dls_pathname();

      VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
      vfs->resolve_filename(dls_pathname, get_model_path());

      _dls_data.clear();
      PT(VirtualFile) file = vfs->get_file(dls_pathname);
      if (file == (VirtualFile *)NULL) {
        milesAudio_cat.warning()
          << "DLS file does not exist: " << dls_pathname << "\n";

      } else if (!file->read_file(_dls_data, true)) {
        milesAudio_cat.warning()
          << "Could not read DLS file: " << dls_pathname << "\n";
        
      } else if (_dls_data.empty()) {
        milesAudio_cat.warning()
          << "DLS file is empty: " << dls_pathname << "\n";

      } else {
        _dls_file = AIL_DLS_load_memory(_dls_device, &_dls_data[0], 0);
      }
      
      if (_dls_file == 0) {
        audio_error("  Could not get DLS file, switching to hardware MIDI.");
        AIL_DLS_close(_dls_device, 0);
        _dls_device = 0;
        AIL_close_XMIDI_driver(_midi_driver);
        _midi_driver = AIL_open_XMIDI_driver(0);
        
      } else {
        audio_info("  using Miles software midi");
      }
    } else {
      _midi_driver = AIL_open_XMIDI_driver(0);
      audio_info("  using Miles hardware midi");
    }
  }
    
  _is_open = true;
}

////////////////////////////////////////////////////////////////////
//     Function: GlobalMilesManager::close_api
//       Access: Private
//  Description: Called internally to shut down the Miles API.
////////////////////////////////////////////////////////////////////
void GlobalMilesManager::
close_api() {
  audio_debug("GlobalMilesManager::close_api()")
  nassertv(_is_open);

  Samples::iterator si;
  for (si = _samples.begin(); si != _samples.end(); ++si) {
    SampleData &smp = (*si);
    AIL_release_sample_handle(smp._sample);
  }
  _samples.clear();

  Sequences::iterator qi;
  for (qi = _sequences.begin(); qi != _sequences.end(); ++qi) {
    SequenceData &smp = (*qi);
    AIL_release_sequence_handle(smp._sequence);
  }
  _sequences.clear();

  if (_dls_file != 0) {
    AIL_DLS_unload(_dls_device, _dls_file);
    _dls_file = 0;
  }

  if (_dls_device != 0) {
    AIL_DLS_close(_dls_device, 0);
    _dls_device = 0;
  }

  if (_midi_driver != 0) {
    AIL_close_XMIDI_driver(_midi_driver);
    _midi_driver = 0;
  }

  if (_digital_driver != 0) {
    AIL_close_digital_driver(_digital_driver);
    _digital_driver = 0;
  }

  AIL_shutdown();

  _is_open = false;
}

////////////////////////////////////////////////////////////////////
//     Function: GlobalMilesManager::open_callback
//       Access: Private, Static
//  Description: This callback function is given to Miles to handle
//               file I/O via the Panda VFS.  It's only used to
//               implemented streaming audio files, since in all other
//               cases we open files directly.
////////////////////////////////////////////////////////////////////
U32 AILCALLBACK GlobalMilesManager::
open_callback(char const *filename, UINTa *file_handle) {
  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
  istream *strm = vfs->open_read_file(Filename::binary_filename(string(filename)), true);
  if (strm == NULL) {
    // Failure.
    return 0;
  }
  // Success.
  (*file_handle) = (UINTa)strm;
  return 1;
}

////////////////////////////////////////////////////////////////////
//     Function: GlobalMilesManager::close_callback
//       Access: Private, Static
//  Description: This callback function is given to Miles to handle
//               file I/O via the Panda VFS.
////////////////////////////////////////////////////////////////////
void AILCALLBACK GlobalMilesManager::
close_callback(UINTa file_handle) {
  istream *strm = (istream *)file_handle;
  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
  vfs->close_read_file(strm);
}

////////////////////////////////////////////////////////////////////
//     Function: GlobalMilesManager::seek_callback
//       Access: Private, Static
//  Description: This callback function is given to Miles to handle
//               file I/O via the Panda VFS.
////////////////////////////////////////////////////////////////////
S32 AILCALLBACK GlobalMilesManager::
seek_callback(UINTa file_handle, S32 offset, U32 type) {
  istream *strm = (istream *)file_handle;
  strm->clear();
  switch (type) {
  case AIL_FILE_SEEK_BEGIN:
    strm->seekg(offset, ios::beg);
    break;

  case AIL_FILE_SEEK_CURRENT:
    strm->seekg(offset, ios::cur);
    break;

  case AIL_FILE_SEEK_END:
    strm->seekg(offset, ios::end);
    break;
  }

  return strm->tellg();
}

////////////////////////////////////////////////////////////////////
//     Function: GlobalMilesManager::read_callback
//       Access: Private, Static
//  Description: This callback function is given to Miles to handle
//               file I/O via the Panda VFS.
////////////////////////////////////////////////////////////////////
U32 AILCALLBACK GlobalMilesManager::
read_callback(UINTa file_handle, void *buffer, U32 bytes) {
  istream *strm = (istream *)file_handle;
  strm->read((char *)buffer, bytes);
  return strm->gcount();
}

#endif //]

