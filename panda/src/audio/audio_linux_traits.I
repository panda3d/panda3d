// Filename: audio_linux_traits.I
// Created by:  cary (02Oct00)
// 
////////////////////////////////////////////////////////////////////

INLINE LinuxSample::LinuxSample(Buffer* b) : AudioTraits::SampleClass(),
					     _data(b) {
}

INLINE Buffer* LinuxSample::get_data(void) {
  return _data;
}

INLINE LinuxMusic::LinuxMusic(void) : AudioTraits::MusicClass() {
}

INLINE LinuxPlaying::LinuxPlaying(void) : AudioTraits::PlayingClass() {
}

INLINE LinuxPlayer::LinuxPlayer(void) : AudioTraits::PlayerClass() {
}
