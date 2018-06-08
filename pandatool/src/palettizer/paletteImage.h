/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file paletteImage.h
 * @author drose
 * @date 2000-11-28
 */

#ifndef PALETTEIMAGE_H
#define PALETTEIMAGE_H

#include "pandatoolbase.h"

#include "imageFile.h"

#include "pnmImage.h"

class PalettePage;
class TexturePlacement;

/**
 * This is a single palette image, one of several within a PalettePage, which
 * is in turn one of several pages within a PaletteGroup.  Each palette image
 * is a collage of several different textures that were all assigned to the
 * same PaletteGroup, and all share the same properties of the PalettePage.
 */
class PaletteImage : public ImageFile {
private:
  PaletteImage();

public:
  PaletteImage(PalettePage *page, int index);
  PaletteImage(PalettePage *page, int index, unsigned swapIndex);

  PalettePage *get_page() const;

  bool is_empty() const;
  double count_utilization() const;
  double count_coverage() const;

  bool place(TexturePlacement *placement);
  void unplace(TexturePlacement *placement);
  void check_solitary();

  void optimal_resize();
  bool resize_image(int x_size, int y_size);
  void resize_swapped_image(int x_size, int y_size);

  void write_placements(std::ostream &out, int indent_level = 0) const;
  void reset_image();
  void setup_shadow_image();
  void update_image(bool redo_all);

  bool update_filename();

private:
  bool setup_filename();
  bool find_hole(int &x, int &y, int x_size, int y_size) const;
  TexturePlacement *find_overlap(int x, int y, int x_size, int y_size) const;
  void get_image();
  void release_image();
  void remove_image();
  void get_swapped_image(int index);
  void get_swapped_images();

  // The ClearedRegion object keeps track of TexturePlacements that were
  // recently removed and thus need to be set to black.
  class ClearedRegion {
  public:
    ClearedRegion();
    ClearedRegion(TexturePlacement *placement);
    ClearedRegion(const ClearedRegion &copy);
    void operator = (const ClearedRegion &copy);
    void clear(PNMImage &image);

    void write_datagram(Datagram &datagram) const;
    void fillin(DatagramIterator &scan);

  private:
    int _x, _y;
    int _x_size, _y_size;
  };

  typedef pvector<ClearedRegion> ClearedRegions;
  ClearedRegions _cleared_regions;

  typedef pvector<TexturePlacement *> Placements;
  Placements _placements;

  Placements *_masterPlacements;

  PalettePage *_page;
  int _index;
  std::string _basename;

  bool _new_image;
  bool _got_image;
  PNMImage _image;

  unsigned _swapped_image; // 0 for non swapped image

  ImageFile _shadow_image;

  typedef pvector<PaletteImage *> SwappedImages;
  SwappedImages _swappedImages;

  // The TypedWritable interface follows.
public:
  static void register_with_read_factory();
  virtual void write_datagram(BamWriter *writer, Datagram &datagram);
  virtual int complete_pointers(TypedWritable **p_list,
                                BamReader *manager);

protected:
  static TypedWritable *make_PaletteImage(const FactoryParams &params);
  void fillin(DatagramIterator &scan, BamReader *manager);

private:
  // This value is only filled in while reading from the bam file; don't use
  // it otherwise.
  int _num_placements;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    ImageFile::init_type();
    register_type(_type_handle, "PaletteImage",
                  ImageFile::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }

private:
  static TypeHandle _type_handle;
};

#endif
