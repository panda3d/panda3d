// Filename: audio_sample.I
// Created by:  cary (23Sep00)
// 
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//     Function: AudioSample::constructor
//       Access: Protected
//  Description: initialize a new sample
////////////////////////////////////////////////////////////////////
INLINE AudioSample::AudioSample(AudioTraits::SampleClass* sample,
				AudioTraits::PlayerClass* player,
				AudioTraits::DeleteSampleFunc* destroy,
				const string& filename) : Namable(filename),
							  _sample(sample),
							  _player(player),
							  _destroy(destroy) {}

////////////////////////////////////////////////////////////////////
//     Function: AudioSample::copy constructor
//       Access: Protected
//  Description: copy a sample, but we don't really want to allow this
////////////////////////////////////////////////////////////////////
INLINE AudioSample::AudioSample(const AudioSample& c) : Namable(c.get_name()),
							_sample(c._sample),
							_player(c._player),
							_destroy(c._destroy) {}

////////////////////////////////////////////////////////////////////
//     Function: AudioSample::assignment operator
//       Access: Protected
//  Description: copy a sample, but we don't really want to allow this
////////////////////////////////////////////////////////////////////
INLINE AudioSample& AudioSample::operator=(const AudioSample& c) {
  this->set_name(c.get_name());
  this->_sample = c._sample;
  this->_player = c._player;
  this->_destroy = c._destroy;
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: AudioSample::get_player
//       Access: Protected
//  Description: return the player for this sample
////////////////////////////////////////////////////////////////////
INLINE AudioTraits::PlayerClass* AudioSample::get_player(void) {
  return _player;
}

////////////////////////////////////////////////////////////////////
//     Function: AudioSample::get_sample
//       Access: Protected
//  Description: return the trait sample class for this sample
////////////////////////////////////////////////////////////////////
INLINE AudioTraits::SampleClass* AudioSample::get_sample(void) {
  return _sample;
}

////////////////////////////////////////////////////////////////////
//     Function: AudioSample::equality operator
//       Access: Public
//  Description: test to see if two samples are the same
////////////////////////////////////////////////////////////////////
INLINE bool AudioSample::operator==(const AudioSample& c) const {
  return (_sample == c._sample);
}
