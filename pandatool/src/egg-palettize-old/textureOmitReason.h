// Filename: textureOmitReason.h
// Created by:  drose (06Nov00)
// 
////////////////////////////////////////////////////////////////////

#ifndef TEXTUREOMITREASON_H
#define TEXTUREOMITREASON_H

#include <pandatoolbase.h>

////////////////////////////////////////////////////////////////////
// 	  Enum : TextureOmitReason
// Description : This enumerates the various reasons a texture might
//               have been omitted from a palette.
////////////////////////////////////////////////////////////////////
enum TextureOmitReason {
  OR_none,      // No reason: not omitted
  OR_size,      // Too big to put on a palette
  OR_repeats,   // The texture repeats and can't be palettized
  OR_omitted,   // Explicitly omitted by user in .txa file
  OR_unused,    // Not used by any egg files
  OR_unknown,   // The texture file is unknown, so can't determine its size
  OR_solitary   // Would be palettized, but no other textures are on the palette.
};

#endif
