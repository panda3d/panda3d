// Filename: audio_win_traits.I
// Created by:  cary (27Sep00)
// 
////////////////////////////////////////////////////////////////////

INLINE WinSample::WinSample(void) : AudioTraits::SoundClass(), _data(NULL),
				    _len(0) {
}

INLINE WinMusic::WinMusic(void) : AudioTraits::SoundClass(),
				  _performance(NULL), _music(NULL),
			          _buffer(NULL), _synth(NULL) {
  init();
}

INLINE IDirectMusicPerformance* WinMusic::get_performance(void) const {
  return _performance;
}

INLINE IDirectMusicSegment* WinMusic::get_music(void) {
  return _music;
}

INLINE LPDIRECTSOUNDBUFFER WinSamplePlaying::get_channel(void) {
  return _channel;
}

INLINE IDirectMusicPerformance* WinMusicPlaying::get_performance(void) const {
  WinMusic* wmusic = (WinMusic*)_sound;
  return wmusic->get_performance();
}

INLINE WinSamplePlayer::WinSamplePlayer(void) : AudioTraits::PlayerClass() {
}

INLINE WinMusicPlayer::WinMusicPlayer(void) : AudioTraits::PlayerClass() {
}
