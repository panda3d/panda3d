// Filename: audio_music.I
// Created by:  cary (26Sep00)
// 
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//     Function: AudioMusic::constructor
//       Access: Protected
//  Description: initialize a new music
////////////////////////////////////////////////////////////////////
INLINE AudioMusic::AudioMusic(AudioTraits::MusicClass* music,
			      AudioTraits::PlayingClass* state,
			      AudioTraits::PlayerClass* player,
			      AudioTraits::DeleteMusicFunc* destroy,
			      const string& filename) : Namable(filename),
							_music(music),
							_state(state),
							_player(player),
							_destroy(destroy) {}

////////////////////////////////////////////////////////////////////
//     Function: AudioMusic::copy constructor
//       Access: Protected
//  Description: copy a music, but we don't really want to allow this
////////////////////////////////////////////////////////////////////
INLINE AudioMusic::AudioMusic(const AudioMusic& c) : Namable(c.get_name()),
						     _music(c._music),
						     _state(c._state),
						     _player(c._player),
						     _destroy(c._destroy) {}

////////////////////////////////////////////////////////////////////
//     Function: AudioMusic::assignment operator
//       Access: Protected
//  Description: copy a music, but we don't really want to allow this
////////////////////////////////////////////////////////////////////
INLINE AudioMusic& AudioMusic::operator=(const AudioMusic& c) {
  this->set_name(c.get_name());
  this->_music = c._music;
  this->_state = c._state;
  this->_player = c._player;
  this->_destroy = c._destroy;
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: AudioMusic::get_player
//       Access: Protected
//  Description: return the player for this music
////////////////////////////////////////////////////////////////////
INLINE AudioTraits::PlayerClass* AudioMusic::get_player(void) {
  return _player;
}

////////////////////////////////////////////////////////////////////
//     Function: AudioMusic::get_music
//       Access: Protected
//  Description: return the trait music class for this music
////////////////////////////////////////////////////////////////////
INLINE AudioTraits::MusicClass* AudioMusic::get_music(void) {
  return _music;
}

////////////////////////////////////////////////////////////////////
//     Function: AudioMusic::equality operator
//       Access: Public
//  Description: test to see if two musics are the same
////////////////////////////////////////////////////////////////////
INLINE bool AudioMusic::operator==(const AudioMusic& c) const {
  return (_music == c._music);
}
