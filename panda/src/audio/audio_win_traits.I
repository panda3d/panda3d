// Filename: audio_win_traits.I
// Created by:  cary (27Sep00)
// 
////////////////////////////////////////////////////////////////////

INLINE WinSample::WinSample(void) : _channel(NULL), _data(NULL), _len(0) {
}

INLINE LPDIRECTSOUNDBUFFER WinSample::get_channel(void) {
  return _channel;
}

INLINE WinMusic::WinMusic(void) : _performance(NULL), _music(NULL),
				  _buffer(NULL), _synth(NULL), _data(NULL),
				  _len(0) {
  init();
}

INLINE IDirectMusicPerformance* WinMusic::get_performance(void) {
  return _performance;
}

INLINE IDirectMusicSegment* WinMusic::get_music(void) {
  return _music;
}

INLINE WinPlayer::WinPlayer(void) {
}
