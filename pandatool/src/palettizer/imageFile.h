/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file imageFile.h
 * @author drose
 * @date 2000-11-28
 */

#ifndef IMAGEFILE_H
#define IMAGEFILE_H

#include "pandatoolbase.h"

#include "textureProperties.h"

#include "filename.h"
#include "typedWritable.h"

class PNMImage;
class EggTexture;
class PaletteGroup;

/**
 * This is the base class of both TextureImage and PaletteImage.  It
 * encapsulates all the information specific to an image file that can be
 * assigned as a texture image to egg geometry.
 */
class ImageFile : public TypedWritable {
public:
  ImageFile();

  bool make_shadow_image(const std::string &basename);

  bool is_size_known() const;
  int get_x_size() const;
  int get_y_size() const;
  bool has_num_channels() const;
  int get_num_channels() const;

  const TextureProperties &get_properties() const;
  void clear_basic_properties();
  void update_properties(const TextureProperties &properties);

  bool set_filename(PaletteGroup *group, const std::string &basename);
  bool set_filename(const std::string &dirname, const std::string &basename);
  const Filename &get_filename() const;
  const Filename &get_alpha_filename() const;
  int get_alpha_file_channel() const;
  bool exists() const;

  bool read(PNMImage &image) const;
  bool write(const PNMImage &image) const;
  void unlink();

  void update_egg_tex(EggTexture *egg_tex) const;

  void output_filename(std::ostream &out) const;

protected:
  TextureProperties _properties;
  Filename _filename;
  Filename _alpha_filename;
  int _alpha_file_channel;

  bool _size_known;
  int _x_size, _y_size;

  // The TypedWritable interface follows.
public:
  virtual void write_datagram(BamWriter *writer, Datagram &datagram);
  virtual int complete_pointers(TypedWritable **p_list,
                                BamReader *manager);

protected:
  void fillin(DatagramIterator &scan, BamReader *manager);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedWritable::init_type();
    register_type(_type_handle, "ImageFile",
                  TypedWritable::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }

private:
  static TypeHandle _type_handle;

};

#endif
