// Filename: eggTextureCollection.I
// Created by:  drose (16Feb00)
// 
////////////////////////////////////////////////////////////////////

INLINE EggTextureCollection::iterator EggTextureCollection::
begin() const {
  nassertr(_ordered_textures.size() == _textures.size(),
	   _ordered_textures.begin());
  return _ordered_textures.begin();
}
 
INLINE EggTextureCollection::iterator EggTextureCollection::
end() const {
  return _ordered_textures.end();
}
 
INLINE bool EggTextureCollection::
empty() const {
  return _ordered_textures.empty();
}
 
INLINE EggTextureCollection::size_type EggTextureCollection::
size() const {
  nassertr(_ordered_textures.size() == _textures.size(), 0);
  return _ordered_textures.size();
}
