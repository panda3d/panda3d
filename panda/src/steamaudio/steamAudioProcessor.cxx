/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file steamAudioProcessor.cxx
 * @author Jackson Sutherland
 */

#include "steamAudioProcessor.h"

/**
*
* frame_rate sets how long a segment of audio is processed when render_effects is called.
* Essentially, audio sources and moving objects do not move in steam audio's simulations
* during a single "frame".
* It's a good idea to tie the frame size with how often you pass updated positions to this class.
* (Or vice-versa.)
*
* sample rate is both the rate that steam audio processes sound, and the rate at which it will be played back, regardless the rate of any audio you buffer in.
**/
SteamAudioProcessor::
SteamAudioProcessor() {
  _frame_size = 512;
  _sample_rate = 44100;

  //create context
  IPLContextSettings contextSettings{};
  contextSettings.version = STEAMAUDIO_VERSION;
  iplContextCreate(&contextSettings, _steamContext);

  //create simulator
  _simulation_settings = IPLSimulationSettings{};
  _simulation_settings.flags = IPL_SIMULATIONFLAGS_DIRECT | IPL_SIMULATIONFLAGS_REFLECTIONS | IPL_SIMULATIONFLAGS_PATHING;//I'm not sure whether these can be changed later. I'll leave like this for now...
  _simulation_settings.sceneType = IPL_SCENETYPE_DEFAULT;//This actually refers to the raytracer we use.

  iplSimulatorCreate(*_steamContext, &_simulation_settings, _simulator);
}

SteamAudioProcessor::
~SteamAudioProcessor() {
  _sources.clear();
  _buffers.clear();
  _steam_effects.clear();

  iplContextRelease(_steamContext);
}

/**
*Steam Audio has different different axes, so this quickly translates them.
*/
inline void SteamAudioProcessor::
sa_coordinate_transform(float x1, float y1, float z1, IPLVector3& vals) {
  vals.x = x1;
  vals.y = z1;
  vals.z = -y1;
}


//sources
/**
*
**/
bool SteamAudioProcessor::
make_source(std::string source_name) {
  if ((_sources.count(source_name) < 1) && _sources.max_size() > _sources.size())
  {
    _sources[source_name] = source_properties();
      return true;
  }
  else
  {
    return false;
  }
}

/**
*
**/
bool SteamAudioProcessor::
has_source(std::string source_name) {
  return (_sources.count(source_name) == 1);
}

/**
*
**/
int SteamAudioProcessor::
get_num_sources() {
  return _sources.size();
}

/**
*
**/
void SteamAudioProcessor::
clear_all_sources() {
  _sources.clear();
//TODO:: Make sure the absence of sources doesn't break anything
}


//audio loading
/**
* Load the provided number of samples from cursor, and send to steam audio.
* If source is not provided, then the audio is assumed to be played from the listner's perspective.
**/
void SteamAudioProcessor::
buffer_audio(const MovieAudioCursor& cursor, int samples, std::string source) {
  int numChannels = cursor.audio_channels();
  int cursorRate = cursor.audio_rate();

  int numInptSamples;//How large we need to make our imput array
  if (_sample_rate <= cursorRate) {
    numInptSamples = _frame_size;//We don't actually need as many if the cursorrate is larger than our rate, but this makes the code simpler and more readable.
  }
  else {
    numInptSamples = ceil((_frame_size * cursorRate) / _sample_rate) + 1;
  }
  int16_t rawSamples[numInptSamples * numChannels];
  cursor.read_samples(numInptSamples, rawSamples);

  float resampleRatio = static_cast<float>(_sample_rate) / cursorRate;

  float compatibleSamples[_frame_size];
  if (resampleRatio == 1) {//No need to resample
    for (int i = 0; i < _frame_size; ++i) {
      compatibleSamples[i] = static_cast<float>(rawSamples[i]) / 32768.0f;
    }
  }
  else {//source rate is different than our rate
    for (int i = 0; i < _frame_size; ++i) {
      float j = static_cast<float>(i) / resampleRatio;//The actual index representing the timepoint we're attempting to get a sample for
      int j0 = floor(j);
      int j1 = j0 + 1;
      float offset = j - j0;
      assert( (j0 < numInptSamples) );

      if (j1 > numInptSamples) { j1 = j0; }

      float interpSample = ((1 - offset) * rawSamples[j0]) + (offset * rawSamples[j1]);

      compatibleSamples[i] = interpSample / 32768.0f;//finish off by changing sample format to 32 bit float
    }
  }
  

  buffer_properties newBuffer;
  newBuffer.buffer = new IPLAudioBuffer;
  iplAudioBufferDeinterleave(_steamContext, compatibleSamples, newBuffer.buffer);

  if (!source.empty() && has_source(source)) {
    newBuffer.isGlobal = false;
    newBuffer.sourceName = source;
  }

  _buffers.insert(newBuffer);
}


//configuration

/**
*
**/
int SteamAudioProcessor::
get_frame_size() {
  return _frame_size;
}

/**
*
**/
int SteamAudioProcessor::
get_sample_rate() {
  return _sample_rate;
}

/**
*
**/
void SteamAudioProcessor::
force_simulator_commit() {
  iplSimulatorCommit(_simulator);
}

//effects
/**
*Adds a SteamAudioEffect, and then returns it's index.
*/
int SteamAudioProcessor::
add_steam_audio_effect(const SteamAudioEffect& effect) {
  _steam_effects.push_back(&effect);
  pvector<PT(SteamAudioEffect)>::iterator i = std::find(_steam_effects.begin(), _steam_effects.end(), &effect);
  return std::distance(_steam_effects.begin(), i);
}

/**
*Returns the index of a SteamAudioEffect, or -1 if not found.
*/
int SteamAudioProcessor::
find_steam_audio_effect(SteamAudioEffect effect) {
  auto i = std::find(_steam_effects.begin(), _steam_effects.end(), &effect);
  if (i != _steam_effects.end()) {
    return std::distance(_steam_effects.begin(), i);
  }
  else {
    return -1;
  }

}

/**
*Returns the SteamAudioEffect at *index*, else returns a null effect.
*/
SteamAudioEffect SteamAudioProcessor::
get_steam_audio_effect(int index) {
  if (index < _steam_effects.size()) {
    return *_steam_effects[index];
  }
  else {
    return SteamAudioEffect();
  }
}

/**
*Removes an effect from this object, then returns true if successful.
*/
bool SteamAudioProcessor::
remove_steam_audio_effect(int index) {
  if (!_steam_effects.empty()) {
    _steam_effects.erase(_steam_effects.begin() + index);
    return true;
  }
  else {
    return false;
  }
}

bool SteamAudioProcessor::
remove_steam_audio_effect(SteamAudioEffect effect) {
  auto it = std::find(_steam_effects.begin(), _steam_effects.end(), effect);
  if (!_steam_effects.empty()) {
    _steam_effects.erase(it);
    return true;
  }
  else {
    return false;
  }
}

//Rendering/Output
/**
* Renders one frame of all buffered audio,
* and outputs it to this instance's steamMovieAudio.
* Note that the duration of one frame is not tied to the application framerate;
* See set_frame_size for details!
**/
void SteamAudioProcessor::
render_effects() {
  //TODO:: delete buffered audio. Use the delete keyword!!
}



//source_properties
SteamAudioProcessor::source_properties::
source_properties() {
  sourcePos = IPLVector3{0.0f,0.0f,0.0f};

  sourceCoord.up = IPLVector3{ 0.0f, 1.0f, 0.0f };
  sourceCoord.right = IPLVector3{ 1.0f, 0.0f, 0.0f };
  sourceCoord.ahead = IPLVector3{ 0.0f, 0.0f, -1.0f };//In steam audio, -z is forward
  sourceCoord.origin = IPLVector3{ 0.0f, 0.0f, 0.0f };

  //So, we're going to be a bit hacky and set all of IPLSourceSettings's flags to true.
  //This isn't the cleanest and doesn't allow for the end-developer to have full control over the optimisation,
  //But IPLSourceSettings cannot be dynamically changed.
  //Instead, we can use IPLSimulationInputs for dynamically setting if simulations are run with a source.

  IPLSourceSettings sourceSettings{};
  sourceSettings.flags = IPL_SIMULATIONFLAGS_DIRECT | IPL_SIMULATIONFLAGS_REFLECTIONS | IPL_SIMULATIONFLAGS_PATHING;
  iplSourceCreate(_simulator, &sourceSettings, &source);

  iplSourceAdd(source, _simulator);
  iplSimulatorCommit(_simulator);
}

SteamAudioProcessor::source_properties::
~source_properties() {
  iplSourceRemove(source, _simulator);
  iplSourceRelease(&source);
  _buffer_list.clear();
}
