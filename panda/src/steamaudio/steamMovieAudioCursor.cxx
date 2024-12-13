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
SteamMovieAudioCursor(SteamMovieAudio* src, NodePath& source, NodePath& listener) :
  MovieAudioCursor(src),
  _sourceNP(source),
  _listenerNP(listener)
{
  //make audiosettings
  _steamAudioSettings = &IPLAudioSettings{};
  _steamAudioSettings->samplingRate = audio_source.open().audio_rate();
  _steamAudioSettings->frameSize = 8192;
}

SteamMovieAudioCursor::
~SteamMovieAudioCursor() {

}

int SteamMovieAudioCursor::
read_samples(int n, int16_t* data) {//This isn't finished!!
  if (n <= 0) {
    return 0;
  }

  int length = n * _audio_channels;
  _steamAudioSettings->frameSize = n;

  unsigned char data[length];

  IPLfloat32 fData[8192];//we need to change 16ints to IPLfloats

  for (size_t i = 0; i < 65535; i++) {//loop over every data point. good thing data length is constant
    fData[i] = (IPLfloat32)(int16_t)data[i];//get int16_t and cast to IPLfloat32//TODO:: fix the conversion
  }

  IPLAudioBuffer inBuffer;
  iplAudioBufferAllocate(*_manager->_steamContext, channels, samples, &inBuffer);
  iplAudioBufferDeinterleave(*_manager->_steamContext, fData, &inBuffer);
  SteamAudioSound::SteamGlobalHolder globals(_manager->_steamAudioSettings, _manager->_steamContext, channels, samples, _sourceNP, &_manager->_listenerNP);//input variables

  for (size_t i = 0; i < _manager->_steam_effects.size(); i++) {
    SteamAudioEffect effect = *_manager->_steam_effects[i];
    if (effect._isActive) {
      IPLAudioBuffer outBuffer = effect.apply_effect(&globals, inBuffer);
      std::swap(inBuffer, outBuffer);
      iplAudioBufferFree(*_manager->_steamContext, &outBuffer);
    }
  }
  for (size_t i = 0; i < _steam_effects.size(); i++) {
    SteamAudioEffect effect = *_steam_effects[i];
    if (effect._isActive) {
      IPLAudioBuffer outBuffer = effect.apply_effect(&globals, inBuffer);
      std::swap(inBuffer, outBuffer);
      iplAudioBufferFree(*(_manager->_steamContext), &outBuffer);
    }
  }

  iplAudioBufferInterleave(*(_manager->_steamContext), &inBuffer, fData);
  iplAudioBufferFree(*_manager->_steamContext, &inBuffer);

  for (size_t i = 0; i < (sizeof(data) / sizeof(data[0])); i++) {
    data[i] = (int16_t)fData[i];
  }
}

SteamMovieAudioCursor::SteamGlobalHolder
::SteamGlobalHolder(IPLAudioSettings* audio_settings, IPLContext* steam_context, int channels, int samples, NodePath _source, NodePath* _listener) :
  _audio_settings(audio_settings),
  _steam_context(steam_context),
  _channels(channels),
  _samples(samples),
{
  source = _source;
  if (_listener != nullptr) {
    listener = *_listener;
  }
  else {
    listener = source;
  }
  //IPLAudioSetting's values tell Steam Audio's classes the speed and amount of data being processed.
  //frameSize is just the length of any buffers we're handing them.
}
