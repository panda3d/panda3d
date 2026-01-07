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

#include "plist.h"//Don't know if I'll need these, but good idea to keep in hand
#include "pmap.h"
#include "pset.h"

#include "pta_LVecBase3.h"

#include <phonon.h>

class NodePath;
class MovieAudioCursor;

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

  bool set_source_transform(std::string source_name, const LVecBase3 &pos);//Needs to be implemented

  bool has_source(std::string name);
  int get_num_sources();
  void clear_all_sources();

  //audio_loading
  void buffer_audio(const MovieAudioCursor& cursor, int samples, std::string source = "");

  void render_effects();

  //configuration
  int get_frame_size();

  int get_sample_rate();

  //effects
  int add_steam_audio_effect(const SteamAudioEffect& effect);
  int find_steam_audio_effect(SteamAudioEffect effect);
  SteamAudioEffect get_steam_audio_effect(int index);
  bool remove_steam_audio_effect(int index);
  bool remove_steam_audio_effect(SteamAudioEffect effect);

public:
  void force_simulator_commit();

private:
  void sa_coordinate_transform(float x1, float y1, float z1, IPLVector3& vals);

  //buffers
  struct buffer_properties {
  private:
    friend class SteamAudioProcessor;

    bool isGlobal = true;//true by default as a safeguard to a null sourceName getting accessed
    IPLAudioBuffer buffer;
    std::string sourceName = "";
  };
  typedef pset<buffer_properties> SteamBuffers;
  SteamBuffers _buffers;//set of buffers that contain sound data that is processed all at once, by merging into one single buffer.

  //sources/simulation
  class source_properties {
  private:
    friend class SteamAudioProcessor;
    source_properties();
    ~source_properties();

    IPLVector3 sourcePos;//supprisingly, this and sourceCoord are not part of the source struct.
    IPLCoordinateSpace3 sourceCoord;//They are a part of IPLSimulationInputs however, so they only exist here to be quickly sent to non-simulation effects.

    IPLSimulationInputs inputs{};

    IPLSource source;//This will be done hackilly; see constructor function for details.

    SteamBuffers _buffer_list;//set of buffers that have by-source effects applied to them

    bool source_invalidated;//if true, then the inputs for this source were changed and iplSourceSetInputs needs to be called on this source before the next simulation.
  };

  typedef pmap<std::string, source_properties> Sources;
  Sources _sources;

  IPLSimulator* _simulator;
  IPLSimulationSettings _simulation_settings;

  //effects
  typedef pvector<PT(SteamAudioEffect)> SAEffects;
  SAEffects _steam_effects;

  //configuration
  unsigned int _frame_size;
  unsigned int _sample_rate;

  IPLContext* _steamContext;//This risks being inefficient; but we don't have any near-gaurentee that we'll be used at app startup like the audio system

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
