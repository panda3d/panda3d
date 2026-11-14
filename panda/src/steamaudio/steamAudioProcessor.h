/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file steamAudioProcessor.h
 * @author Jackson Sutherland
 */

#ifndef STEAMAUDIOPROCESSOR_H
#define STEAMAUDIOPROCESSOR_H

#include "pandabase.h"
#include "typedObject.h"

#include "plist.h"
#include "pmap.h"
#include "pset.h"
#include "pvector.h"

#include "pta_LVecBase3.h"

#include <phonon.h>

#include <string>
#include <memory>

class NodePath;
class MovieAudioCursor;
class UserDataAudio;

class SteamAudioEffect;

/**
*This class is what processes audio data and applies steam audio effects.
**/
class EXPCL_STEAM_AUDIO SteamAudioProcessor : public TypedReferenceCount {

  friend class SteamMovieAudioCursor;
PUBLISHED:
  SteamAudioProcessor();
  ~SteamAudioProcessor();

  //source methods
  bool make_source(std::string source_name);

  //Put source transformation methods here

  bool set_source_transform(std::string source_name, const LVecBase3& pos);//Needs to be implemented

  bool has_source(std::string name);
  int get_num_sources();
  void clear_all_sources();

  //audio_loading
  void buffer_audio(MovieAudioCursor& cursor, int samples, std::string source = "");

  void render_effects();

  void read_into_UserDataAudio(UserDataAudio& uda);

  vector_uchar get_output();

  //configuration
  int get_frame_size();

  int get_sample_rate();

  //effects
  int add_steam_audio_effect(const SteamAudioEffect& effect);
  int find_steam_audio_effect(SteamAudioEffect effect);
  SteamAudioEffect get_steam_audio_effect(int index);
  bool remove_steam_audio_effect(int index);
  bool remove_steam_audio_effect(SteamAudioEffect effect);
  //TODO::add a method to commit effect mask changes

public:
  void force_simulator_commit();

  struct SteamGlobalHolder {
  public:
    IPLAudioSettings* _audio_settings;
    IPLContext* _steam_context;
    //With the exception of listenerPosition, values below this point get set per buffer.
    int _channels;
    int _samples;
    source_properties* source;
    IPLVECTOR3 listenerPosition;
  };

private:
  void sa_coordinate_transform(float x1, float y1, float z1, IPLVector3& vals);

  IPLVector3* _listenerPos;

  IPLAUDIOSETTINGS* _audio_settings;

  //buffers
  struct buffer_properties {
  private:
    friend class SteamAudioProcessor;

    bool isGlobal = true;//true by default as a safeguard to a null sourceName getting accessed
    IPLAudioBuffer* buffer;
    std::string sourceName = "";
  };
  typedef pset<std::unique_ptr<buffer_properties>> SteamBuffers;
  SteamBuffers _buffers;//set of buffers that contain sound data that is processed all at once, by merging into one single buffer.

  //sources/simulation
  class source_properties {
  private:
    friend class SteamAudioProcessor;
    source_properties();
    ~source_properties();

    IPLVector3 sourcePos;//supprisingly, this and sourceCoord are not part of the source struct.
    IPLCoordinateSpace3 sourceCoord;//They are a part of IPLSimulationInputs however, so they only exist here to be quickly sent to non-simulation effects.

    IPLSimulationInputs inputs;

    IPLSource source;//This will be done hackilly; see constructor function for details.

    bool source_invalidated;//if true, then the inputs for this source were changed and iplSourceSetInputs needs to be called on this source before the next simulation.
  };

  typedef pmap<std::string, std::unique_ptr<source_properties>> Sources;
  Sources _sources;

  std::unique_ptr<source_properties> _listener;

  IPLSimulator* _simulator;
  IPLSimulationSettings _simulation_settings;

  //effects
  typedef pvector<PT(SteamAudioEffect)> SAEffects;
  SAEffects _steam_effects;

  //configuration
  unsigned int _frame_size;
  unsigned int _sample_rate;

  IPLContext* _steamContext;//This risks being inefficient; but we don't have any near-gaurentee that we'll be used at app startup like the audio system

  //output
  pvector<int16_t> output_samples;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedObject::init_type();
    register_type(_type_handle, "SteamAudioProcessor", TypedReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {
    init_type();
    return get_class_type();
  }
private:
  static TypeHandle _type_handle;
};

#endif
