// Filename: audio_null_traits.I
// Created by:  cary (25Sep00)
// 
////////////////////////////////////////////////////////////////////

INLINE NullSample::NullSample(void) : AudioTraits::SampleClass() {
}

INLINE NullMusic::NullMusic(void) : AudioTraits::MusicClass() {
}

INLINE NullPlaying::NullPlaying(void) : AudioTraits::PlayingClass() {
}

INLINE NullPlayer::NullPlayer(void) : AudioTraits::PlayerClass() {
}
