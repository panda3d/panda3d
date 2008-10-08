// Filename: globalMilesManager.h
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

#ifndef GLOBALMILESMANAGER_H
#define GLOBALMILESMANAGER_H

#include "pandabase.h"
#ifdef HAVE_RAD_MSS //[

#include "mss.h"
#include "pset.h"
#include "lightMutex.h"
#include "lightMutexHolder.h"

#ifndef UINTa
#define UINTa U32
#endif

#ifndef SINTa
#define SINTa S32
#endif

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
  INLINE int get_num_samples() const;

  bool get_sequence(HSEQUENCE &sequence, size_t &index, MilesAudioSequence *sound);
  void release_sequence(size_t index, MilesAudioSequence *sound);
  INLINE int get_num_sequences() const;

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
  LightMutex _managers_lock;

  class SampleData {
  public:
    HSAMPLE _sample;
    MilesAudioSample *_sound;
  };

  typedef pvector<SampleData> Samples;
  Samples _samples;
  LightMutex _samples_lock;

  class SequenceData {
  public:
    HSEQUENCE _sequence;
    MilesAudioSequence *_sound;
  };

  typedef pvector<SequenceData> Sequences;
  Sequences _sequences;
  LightMutex _sequences_lock;
  
  static GlobalMilesManager *_global_ptr;
};

#include "globalMilesManager.I"

#endif //]

#endif


  
