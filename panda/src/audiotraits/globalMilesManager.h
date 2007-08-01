// Filename: globalMilesManager.h
// Created by:  drose (26Jul07)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#ifndef GLOBALMILESMANAGER_H
#define GLOBALMILESMANAGER_H

#include "pandabase.h"
#ifdef HAVE_RAD_MSS //[

#include "mss.h"
#include "pset.h"
#include "pmutex.h"

class MilesAudioSample;
class MilesAudioSequence;

////////////////////////////////////////////////////////////////////
//       Class : GlobalMilesManager
// Description : This is a wrapper around the parts of the Miles API
//               that should only be created once.  This represents
//               the global data common to all MilesAudioManagers.
////////////////////////////////////////////////////////////////////
class EXPCL_MILES_AUDIO GlobalMilesManager {
private:
  GlobalMilesManager();

public:
  void add_manager(MilesAudioManager *manager);
  void remove_manager(MilesAudioManager *manager);
  void cleanup();
  INLINE bool is_open() const;

  bool get_sample(HSAMPLE &sample, size_t &index, MilesAudioSample *sound);
  void release_sample(size_t index, MilesAudioSample *sound);

  bool get_sequence(HSEQUENCE &sequence, size_t &index, MilesAudioSequence *sound);
  void release_sequence(size_t index, MilesAudioSequence *sound);

  void force_midi_reset();

  static GlobalMilesManager *get_global_ptr();

public:
  HDIGDRIVER _digital_driver;
  HMDIDRIVER _midi_driver;

  // For software MIDI:
  HDLSDEVICE _dls_device;
  HDLSFILEID _dls_file;
  pvector<unsigned char> _dls_data;

private:
  void open_api();
  void close_api();

  static U32 AILCALLBACK open_callback(char const *filename, UINTa *file_handle);
  static void AILCALLBACK close_callback(UINTa file_handle);
  static S32 AILCALLBACK seek_callback(UINTa file_handle, S32 offset, U32 type);
  static U32 AILCALLBACK read_callback(UINTa file_handle, void *buffer, U32 bytes);


private:
  bool _is_open;

  typedef pset<MilesAudioManager *> Managers;
  Managers _managers;
  Mutex _managers_lock;

  class SampleData {
  public:
    HSAMPLE _sample;
    MilesAudioSample *_sound;
  };

  typedef pvector<SampleData> Samples;
  Samples _samples;
  Mutex _samples_lock;

  class SequenceData {
  public:
    HSEQUENCE _sequence;
    MilesAudioSequence *_sound;
  };

  typedef pvector<SequenceData> Sequences;
  Sequences _sequences;
  Mutex _sequences_lock;
  
  static GlobalMilesManager *_global_ptr;
};

#include "globalMilesManager.I"

#endif //]

#endif


  
