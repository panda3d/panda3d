// Filename: textureEggRef.cxx
// Created by:  drose (08Nov00)
// 
////////////////////////////////////////////////////////////////////

#include "textureEggRef.h"
#include "texturePacking.h"
#include "pTexture.h"

#include <notify.h>

#include <algorithm>

////////////////////////////////////////////////////////////////////
//     Function: TextureEggRef::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
TextureEggRef::
TextureEggRef(SourceEgg *egg, PTexture *texture, TexturePacking *packing, 
	      bool repeats, bool alpha) :
  _egg(egg),
  _texture(texture),
  _packing(packing),
  _repeats(repeats),
  _alpha(alpha) 
{
  _eggtex = NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: TextureEggRef::require_groups
//       Access: Public
//  Description: Indicates the set of PaletteGroups that this texture
//               (as appearing on this egg file) wants to be listed
//               on.  If the texture is already listed on one of these
//               groups, does nothing; otherwise, moves the texture.
////////////////////////////////////////////////////////////////////
void TextureEggRef::
require_groups(PaletteGroup *preferred, const PaletteGroups &groups) {
  nassertv(!groups.empty());

  if (_packing != (TexturePacking *)NULL) {
    PaletteGroup *now_on = _packing->get_group();
    if (groups.count(now_on) != 0) {
      // The group we're on is on the list; no problem.
      return;
    }
  }

  // Has the texture already been packed into any of the groups?
  PaletteGroups::const_iterator gi;
  for (gi = groups.begin(); gi != groups.end(); ++gi) {
    TexturePacking *packing = _texture->check_group(*gi);
    if (packing != (TexturePacking *)NULL) {
      // It has, use that group.
      _packing = packing;
      return;
    }
  }

  // We need to pack the texture into one of the requested groups.

  // For now, we arbitrarily pick the preferred group.  Later, maybe
  // we'll try to be smart about this and do some kind of graph
  // minimization to choose the group the leads to the least redundant
  // packing.
  _packing = _texture->add_to_group(preferred);
}
