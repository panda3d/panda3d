// Filename: textureMemoryCounter.h
// Created by:  drose (19Dec00)
// 
////////////////////////////////////////////////////////////////////

#ifndef TEXTUREMEMORYCOUNTER_H
#define TEXTUREMEMORYCOUNTER_H

#include <pandatoolbase.h>

class ImageFile;
class PaletteImage;
class TextureImage;
class DestTextureImage;
class TexturePlacement;

#include <map>
#include <set>

////////////////////////////////////////////////////////////////////
//       Class : TextureMemoryCounter
// Description : This class is used to gather statistics on texture
//               memory usage, etc.  It adds up the total texture
//               memory required by a number of image files, and
//               reports it at the end.
////////////////////////////////////////////////////////////////////
class TextureMemoryCounter {
public:
  TextureMemoryCounter();

  void reset();
  void add_placement(TexturePlacement *placement);

  void report(ostream &out, int indent_level);

private:
  static ostream &format_memory_fraction(ostream &out, int fraction_bytes,
                                         int palette_bytes);
  void add_palette(PaletteImage *image);
  void add_texture(TextureImage *texture, int bytes);
  int count_bytes(ImageFile *image);
  int count_bytes(ImageFile *image, int x_size, int y_size);

  int _num_textures;
  int _num_placed;
  int _num_unplaced;
  int _num_palettes;

  int _bytes;
  int _unused_bytes;
  int _duplicate_bytes;
  int _coverage_bytes;

  typedef map<TextureImage *, int> Textures;
  Textures _textures;

  typedef set<PaletteImage *> Palettes;
  Palettes _palettes;
};

#endif
