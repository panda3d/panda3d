// Filename: textureEggRef.h
// Created by:  drose (08Nov00)
// 
////////////////////////////////////////////////////////////////////

#ifndef TEXTUREEGGREF_H
#define TEXTUREEGGREF_H

#include <pandatoolbase.h>

#include "paletteGroup.h"

#include <vector>

class SourceEgg;
class PTexture;
class TexturePacking;
class EggTexture;

////////////////////////////////////////////////////////////////////
// 	 Class : TextureEggRef
// Description : This associates a texture with the egg files it is
//               placed on, and also associated an egg file with the
//               various textures it contains.
////////////////////////////////////////////////////////////////////
class TextureEggRef {
public:
  TextureEggRef(SourceEgg *egg, PTexture *texture,
		TexturePacking *packing,
		bool repeats, bool alpha);

  void require_groups(PaletteGroup *preferred, const PaletteGroups &groups);

  SourceEgg *_egg;
  PTexture *_texture;
  TexturePacking *_packing;
  bool _repeats;
  bool _alpha;
  
  EggTexture *_eggtex;
};

#endif
