// Filename: audio_linux_traits.I
// Created by:  cary (02Oct00)
// 
////////////////////////////////////////////////////////////////////

INLINE LinuxSample::LinuxSample(byte* b, unsigned long s)
  : AudioTraits::SoundClass(), _data(b), _size(s) {
}

INLINE byte* LinuxSample::get_data(void) const {
  return _data;
}

INLINE unsigned long LinuxSample::get_size(void) const {
  return _size;
}

INLINE LinuxMusic::LinuxMusic(void) : AudioTraits::SoundClass() {
}

INLINE LinuxSamplePlaying::LinuxSamplePlaying(AudioTraits::SoundClass* s)
  : AudioTraits::PlayingClass(s), _buff((Buffer*)0L) {
  LinuxSample* smpl = (LinuxSample*)s;
  _buff = new Buffer(smpl->get_data(), smpl->get_size());
}

INLINE Buffer* LinuxSamplePlaying::get_data(void) {
  return _buff;
}

INLINE LinuxMusicPlaying::LinuxMusicPlaying(AudioTraits::SoundClass* s)
  : AudioTraits::PlayingClass(s) {
}

INLINE LinuxSamplePlayer::LinuxSamplePlayer(void)
  : AudioTraits::PlayerClass() {
}

INLINE LinuxMusicPlayer::LinuxMusicPlayer(void)
  : AudioTraits::PlayerClass() {
}
