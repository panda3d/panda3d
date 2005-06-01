// Filename: indexImage.h
// Created by:  drose (03Apr02)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#ifndef INDEXIMAGE_H
#define INDEXIMAGE_H

#include "pandatoolbase.h"

#include "filename.h"
#include "pvector.h"

class RollDirectory;
class Photo;
class PNMTextMaker;
class PNMImage;
class PNMImageHeader;

////////////////////////////////////////////////////////////////////
//       Class : IndexImage
// Description : An aggregate image containing several thumbnails, one
//               for each of several photos.  A roll directory will
//               collect its photos into one or more IndexImages.
////////////////////////////////////////////////////////////////////
class IndexImage {
public:
  IndexImage(RollDirectory *dir, int index);
  ~IndexImage();

  bool add_photo(int index);
  bool generate_images(const Filename &archive_dir, PNMTextMaker *text_maker);
  bool make_reduced_image(Photo *photo, PNMImage &reduced_image, 
                          bool generate_index_image, bool force_reduced);

  bool generate_html(ostream &root_html, const Filename &archive_dir,
                     const Filename &roll_dir_root);

  bool copy_reduced(const Filename &archive_dir);

  void output(ostream &out) const;
  void write(ostream &out, int indent_level) const;

private:
  bool generate_reduced_html(ostream &html, Photo *photo, int photo_index,
                             int pi, const Filename &roll_dir_root);
  void generate_nav_buttons(ostream &html, const Filename &prev_photo_filename,
                            const Filename &next_photo_filename, 
                            const string &up_href);

  static void compute_reduction(const PNMImageHeader &source_image,
                                PNMImage &dest_image,
                                int x_size, int y_size);
  static void draw_box(PNMImage &image);

  static void draw_frame(PNMImage &image,
			 int frame_left, int frame_top,
			 int frame_width, int frame_height,
			 int hole_left, int hole_top,
			 int hole_width, int hole_height);

private:
  RollDirectory *_dir;
  int _index;
  string _name;

  class PhotoInfo {
  public:
    int _photo_index;
    int _x_place;
    int _y_place;
  };

  typedef pvector<PhotoInfo> Photos;
  Photos _photos;

  int _index_x_size;
  int _index_y_size;
  string _index_basename;
};

INLINE ostream &operator << (ostream &out, const IndexImage &d) {
  d.output(out);
  return out;
}

#endif
