// Filename: audio_null_traits.I
// Created by:  cary (25Sep00)
// 
////////////////////////////////////////////////////////////////////

INLINE NullSound::NullSound(void) : AudioTraits::SoundClass() {
}

INLINE NullPlaying::NullPlaying(AudioTraits::SoundClass* sound)
  : AudioTraits::PlayingClass(sound) {
}

INLINE NullPlayer::NullPlayer(void) : AudioTraits::PlayerClass() {
}
