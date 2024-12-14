/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file steamMovieAudioCursor.cxx
 * @author Jackson Sutherland
 */
#include "steamMovieAudioCursor.h"


SteamMovieAudioCursor::
SteamMovieAudioCursor(SteamMovieAudio* src) :
  MovieAudioCursor(src)
{
  //make audiosettings
  _steamAudioSettings = &IPLAudioSettings{};
  _steamAudioSettings->samplingRate = audio_source.open().audio_rate();
  _steamAudioSettings->frameSize = 8192;
  //IPLAudioSetting's values tell Steam Audio's classes the speed and amount of data being processed.
  //frameSize is just the length of the buffers we're handing them.

  _source_cursor = &_source->open();
}

SteamMovieAudioCursor::
~SteamMovieAudioCursor() {

}

int SteamMovieAudioCursor::
read_samples(int n, int16_t* data) {
  if (n <= 0) {
    return 0;
  }

  int length = n * _audio_channels;
  _steamAudioSettings->frameSize = n;

  int16_t srcData[length];

  _source_cursor->read_samples(n, srcData);

  IPLfloat32 fData[length];//we need to change 16ints to IPLfloats

  for (size_t i = 0; i < length; i++) {
    fData[i] = (IPLfloat32)srcData[i];//get int16_t and cast to IPLfloat32
  }

  IPLAudioBuffer inBuffer;
  iplAudioBufferAllocate(*_source->_steamContext, _audio_channels, n, &inBuffer);
  iplAudioBufferDeinterleave(*_source->_steamContext, fData, &inBuffer);
  SteamAudioSound::SteamGlobalHolder globals(_steamAudioSettings, _source->_steamContext, _audio_channels, n, &_source);

  for (size_t i = 0; i < _source->_steam_effects.size(); i++) {
    SteamAudioEffect effect = *_source->_steam_effects[i];
    if (effect._isActive) {//TODO:: Add conditions for skipping simulated effects if no simulator, effects that encode to ambisonics, etc
      IPLAudioBuffer outBuffer = effect.apply_effect(&globals, inBuffer);
      std::swap(inBuffer, outBuffer);
      iplAudioBufferFree(*_source->_steamContext, &outBuffer);
    }
  }

  iplAudioBufferInterleave(*(_source->_steamContext), &inBuffer, fData);
  iplAudioBufferFree(*_source->_steamContext, &inBuffer);

  for (size_t i = 0; i < length; i++) {
    data[i] = (int16_t)fData[i];
  }
  return n;
}

SteamMovieAudioCursor::SteamGlobalHolder
::SteamGlobalHolder(IPLAudioSettings* audio_settings, IPLContext* steam_context, int channels, int samples, SteamMovieAudio* _source) :
  _audio_settings(audio_settings),
  _steam_context(steam_context),
  _channels(channels),
  _samples(samples),
  source(_source)
{
  
}
