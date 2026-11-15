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
#include "steamAudioEffect.h"

#include "userDataAudio.h"

#include <vector>
#include <algorithm>
#include <cmath>


TypeHandle SteamAudioProcessor::_type_handle;


/**
*
* frame_rate sets how long a segment of audio is processed when render_effects is called.
* Essentially, audio sources and moving objects do not move in steam audio's simulations
* during a single "frame".
* It's a good idea to tie the frame size with how often you pass updated positions to this class.
* (Or vice-versa.)
*
* sample rate is both the rate that steam audio processes sound, and the rate at which it will be played back, regardless the rate of any audio you buffer in.
*
* Currently there is no way to adjust these values, they are set at 512 and 44100 respectively.
*
* _frame_size cannot be larger than 4096.
**/
SteamAudioProcessor::
SteamAudioProcessor() {
  _frame_size = 512;
  _sample_rate = 44100;

  _audio_settings = new IPLAudioSettings{};
  _audio_settings->samplingRate = _sample_rate;
  _audio_settings->frameSize = _frame_size;

  //create context
  IPLContextSettings contextSettings{};
  contextSettings.version = STEAMAUDIO_VERSION;
  iplContextCreate(&contextSettings, _steamContext);

  //create simulator
  _simulation_settings = IPLSimulationSettings{};
  _simulation_settings.flags = IPLSimulationFlags(IPL_SIMULATIONFLAGS_DIRECT | IPL_SIMULATIONFLAGS_REFLECTIONS | IPL_SIMULATIONFLAGS_PATHING);
  _simulation_settings.sceneType = IPL_SCENETYPE_DEFAULT;//This actually refers to the raytracer we use. TODO:: If support for hardware ray-tracing is added,
  //detect the best option, and set it here.

  iplSimulatorCreate(*_steamContext, &_simulation_settings, _simulator);

  _listener = source_properties(_simulator);
}

SteamAudioProcessor::
~SteamAudioProcessor() {
  //TODO::delete anything created with new! (Buffer data, etc)
  //Remember to deallocate anything allocated for steam audio!

  _sources.clear();
  _buffers.clear();
  _steam_effects.clear();

  delete _audio_settings;

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
    _sources.emplace(source_name, std::make_unique<source_properties>(_simulator));
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
void SteamAudioProcessor::
set_source_transform(std::string source_name, const LVecBase3& pos) {
  sa_coordinate_transform(pos.get_x(), pos.get_y(), pos.get_z(), _sources[source_name]->sourcePos);
  _sources[source_name]->sourceCoord.origin = _sources[source_name]->sourcePos;
}

void SteamAudioProcessor::
set_listener_transform(const LVecBase3& pos) {
  sa_coordinate_transform(pos.get_x(), pos.get_y(), pos.get_z(), _listener.sourcePos);
  _listener.sourceCoord.origin = _listener.sourcePos;
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
*
* This function only supports mono or stereo audio. If you have more than two channels, it does nothing and returns.
*
* TODO::All buffered audio will immediately begin at the start of any audio frame. Frames are small enough that this is negligible,
* but I really should add a "offset" parameter that adds silence before actually copying in samples.
**/
void SteamAudioProcessor::
buffer_audio(MovieAudioCursor& cursor, int samples, std::string source) {
  int numChannels = cursor.audio_channels();
  int cursorRate = cursor.audio_rate();

  if (numChannels > 2) return;

  int numInptSamples;//How large we need to make our input array
  if (_sample_rate == cursorRate) {
    numInptSamples = _frame_size;
  }
  else {
    numInptSamples = ceil((_frame_size * cursorRate) / _sample_rate) + 1;
  }
  int16_t rawSamples[4096];//use a (seemingly) arbitrary size taken from MovieAudioCursor::read_samples(int n)
  cursor.read_samples(numInptSamples, rawSamples);

  float resampleRatio = static_cast<float>(_sample_rate) / cursorRate;

  float compatibleSamples[4096];//always set to two as we always want a two-channel buffer internally

  float sixteenToThirtyTwoScale = 1.0f / 32768.0f;
  if (resampleRatio == 1) {//No need to resample
    for (unsigned int i = 0; i < _frame_size * numChannels; ++i) {
      compatibleSamples[i] = static_cast<float>(rawSamples[i]) * sixteenToThirtyTwoScale;
    }
  }
  else {//source rate is different than our rate
    for (unsigned int i = 0; i < _frame_size * numChannels; ++i) {
      float j = static_cast<float>(i) / resampleRatio;//The actual index representing the timepoint we're attempting to get a sample for
      int j0 = floor(j);
      int j1 = j0 + 1;
      float offset = j - j0;
      assert( (j0 < numInptSamples) );

      if (j1 > numInptSamples) { j1 = numInptSamples; }

      float interpSample = ((1 - offset) * rawSamples[j0]) + (offset * rawSamples[j1]);

      compatibleSamples[i] = interpSample * sixteenToThirtyTwoScale;//finish off by changing sample format to 32 bit float
    }
  }

  auto [it, inserted] = _buffers.emplace(std::make_unique<buffer_properties>());

  if (!inserted) {
    return;
  }
  auto newBuffer = (*it).get();

  
  iplAudioBufferAllocate(*_steamContext, 2, _frame_size, newBuffer->buffer);

  if (numChannels == 1) {//we need to pad the data and put it in without deinterleaving.
    for (unsigned int i = 0; i < _frame_size; i++) {
      compatibleSamples[i + _frame_size] = compatibleSamples[i];
    }
    for (unsigned int i = 0; i < _frame_size * 2; i++) {
      if (i < _frame_size) {
        newBuffer->buffer->data[0][i] = compatibleSamples[i];
      }
      else {
        newBuffer->buffer->data[1][i - _frame_size] = compatibleSamples[i];//fill the second channel
      }
    }
  }
  else {
    iplAudioBufferDeinterleave(*_steamContext, compatibleSamples, newBuffer->buffer);
  }
  

  if (!source.empty() && has_source(source)) {
    newBuffer->isGlobal = false;
    newBuffer->sourceName = source;
  }
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
  iplSimulatorCommit(*_simulator);
}

//effects
/**
*Adds a SteamAudioEffect, and then returns it's index.
*/
int SteamAudioProcessor::
add_steam_audio_effect(SteamAudioEffect effect) {
  effect.ref();
  _steam_effects.push_back(&effect);
  SAEffects::iterator i = std::find(_steam_effects.begin(), _steam_effects.end(), &effect);
  return int(i - _steam_effects.begin());
}

/**
*Returns the index of a SteamAudioEffect, or -1 if not found.
*/
int SteamAudioProcessor::
find_steam_audio_effect(SteamAudioEffect effect) {
  auto i = std::find(_steam_effects.begin(), _steam_effects.end(), &effect);
  if (i != _steam_effects.end()) {
    return i - _steam_effects.begin();
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
  if (!_steam_effects.empty() && !(index > _steam_effects.size())) {
    _steam_effects[index]->unref();
    _steam_effects.erase(_steam_effects.begin() + index);
    return true;
  }
  else {
    return false;
  }
}

bool SteamAudioProcessor::
remove_steam_audio_effect(SteamAudioEffect effect) {
  int i = find_steam_audio_effect(effect);
  if (i == -1) {
    return false;
  }
  remove_steam_audio_effect(i);
  return true;
}

//Rendering/Output
/**
* Renders one frame of all buffered audio,
* and saves the output.
* 
* Note that the duration of one frame is not tied to the application framerate;
* it is instead an abstract number of samples. This number of samples / the sample rate determines how often you should
* render audio.
* At the time of writing, frame size and sample rate are hard-coded to 512 and 44100. We may add the ability to choose a frame size and sample rate in the future.
**/
void SteamAudioProcessor::
render_effects() {
  IPLAudioBuffer* finalBuffer;
  iplAudioBufferAllocate(*_steamContext, 2, _frame_size, finalBuffer);

  //TODO:: delete buffered audio. Use the delete keyword!!
  //TODO:: remember to select if a reflection mixer is used based on the model!
  SteamGlobalHolder globals = SteamGlobalHolder{};
  globals._audio_settings = _audio_settings;
  globals._steam_context = _steamContext;
  globals._channels = 2;
  globals._samples = _frame_size;
  globals.listenerPosition = _listener.sourcePos;

  //reset source inputs
  for (auto s1 = _sources.begin(); s1 != _sources.end(); s1++)
  {
    s1->second->inputs = {};
  }

  //Iterate over sources and prepare effects
  for (auto s2 = _sources.begin(); s2 != _sources.end(); s2++)
  {
    for (auto e1 = 0; e1 < _steam_effects.size(); e1++)
    {
      _steam_effects[e1]->get_render_flags(s2->second.get());
    }
  }

  //run simulation
  iplSimulatorRunDirect(*_simulator);
  //Todo:: add other simulator calls

  //Apply the effects
  for (auto b1 = _buffers.begin(); b1 != _buffers.end(); b1++)
  {

    //get source
    if ((*b1)->isGlobal)//The sound is global, aka played from the listener's position.
    {
      globals.source = &_listener;
    }
    else
    {
      globals.source = _sources[(*b1)->sourceName].get();
    }

    //run buffer through effects
    for (unsigned int e2 = 0; e2 < _steam_effects.size(); e2++)
    {
      //TODO::continue if effect mask does not match any bits of source mask
      IPLAudioBuffer* result = _steam_effects[e2]->apply_effect(&globals, *((*b1)->buffer));

      int sampleNum = int((*b1)->buffer->numSamples);
      for (auto i = 0; i < sampleNum; i++) {
        //delete *b1->buffer.data[i];
        (*b1)->buffer->data[i] = (*result).data[i];
      }
      iplAudioBufferFree(*_steamContext, result);

    }

    //mix the result in
    iplAudioBufferMix(*_steamContext, (*b1)->buffer, finalBuffer);

    //Remove used data
    /**
    size_t sampleNum = std::size(*b1->buffer.data);
    for (auto i = 0; i < sampleNum; i++) {
      delete* b1->buffer.data[i];
      *b1->buffer.data[i] = nullptr;
    }
    **/
    iplAudioBufferFree(*_steamContext, (*b1)->buffer);

  }

  //TODO:: push out final audio to cursors
  output_samples.clear();

  float output32[4096];//Make an array with the arbitrary size we used in buffer_audio
  iplAudioBufferInterleave(*_steamContext, finalBuffer, output32);

  for (unsigned int i = 0; i < _frame_size * 2; ++i) {
    float sample = std::max(-1.0f, std::min(1.0f, output32[i]));

    int16_t convertedSample = static_cast<int16_t>(std::round(sample * 32767.0f));//I guess 32767 is better when going from float to int??
    output_samples.push_back(convertedSample);
  }

  iplAudioBufferFree(*_steamContext, finalBuffer);
  //delete globals;
}

/**
* Reads the output for the previous frame into a UserDataAudio. Frome here, the audio can be handled as a standard MovieAudio.
* This is probably the easiest way to handle the output data.
*
* When using a UserDataAudio, be concious of if you have it configured for remove_after_read. Since this is likely to be implemented as a stream,
* the size of your UserDataAudio's internal buffer could get quite large.
* As the output data does not change after calling this function, consider feeding it into additional UserDataAudio instances if you need to send audio to multiple things.
* Alternatively, see get_output() function.
* (e.g. in a game where players communicate through walkie talkies, you can read this class's output into an additioal UserDataAudio, and in turn read that data into a datagram.
* Thus, whenever a player talks, you can hear monsters chasing them through the walkie talkie as well!)
**/
void SteamAudioProcessor::
read_into_UserDataAudio(UserDataAudio& uda) {
  uda.append(output_samples.data(), _frame_size);
}


/**
* Returns the output of the last rendered frame as a string. Depending on your needs, this may be a more efficient way to handle the output,
* particularly when output is being directed to multiple sources and/or isn't being directly sent to an AudioManager.
**/
vector_uchar SteamAudioProcessor::
get_output() {
  int16_t* data = output_samples.data();

  vector_uchar result;

  for (unsigned int i = 0; i < _frame_size; i++) {
    int16_t word = data[i];
    result.push_back((uint8_t)(word & 255u));
    result.push_back((uint8_t)((word >> 8) & 255u));
  }

  return result;
}

//source_properties
/**
*Constructs an invalid source_
* Nessicary for listener to exist.
**/
SteamAudioProcessor::source_properties::
source_properties() {
  simulatorPointer = nullptr;

  sourcePos = IPLVector3{ 0.0f,0.0f,0.0f };

  sourceCoord.up = IPLVector3{ 0.0f, 1.0f, 0.0f };
  sourceCoord.right = IPLVector3{ 1.0f, 0.0f, 0.0f };
  sourceCoord.ahead = IPLVector3{ 0.0f, 0.0f, -1.0f };//In steam audio, -z is forward
  sourceCoord.origin = IPLVector3{ 0.0f, 0.0f, 0.0f };

  IPLSourceSettings sourceSettings{};

  inputs = IPLSimulationInputs{};
}

SteamAudioProcessor::source_properties::
source_properties(IPLSimulator* _simulator) {
  simulatorPointer = _simulator;

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
  sourceSettings.flags = IPLSimulationFlags(IPL_SIMULATIONFLAGS_DIRECT | IPL_SIMULATIONFLAGS_REFLECTIONS | IPL_SIMULATIONFLAGS_PATHING);
  iplSourceCreate(*_simulator, &sourceSettings, &source);

  inputs = IPLSimulationInputs{};
  //flags are set when a source is added or modified.
  inputs.source = sourceCoord;

  iplSourceAdd(source, *_simulator);
  iplSimulatorCommit(*_simulator);
}

SteamAudioProcessor::source_properties::
~source_properties() {
  iplSourceRemove(source, *simulatorPointer);
  iplSourceRelease(&source);
}
