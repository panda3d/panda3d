// Filename: imageFile.h
// Created by:  drose (28Nov00)
// 
////////////////////////////////////////////////////////////////////

#ifndef IMAGEFILE_H
#define IMAGEFILE_H

#include <pandatoolbase.h>

#include "textureProperties.h"

#include <filename.h>
#include <typedWriteable.h>

class PNMImage;
class EggTexture;
class PaletteGroup;

////////////////////////////////////////////////////////////////////
// 	 Class : ImageFile
// Description : This is the base class of both TextureImage and
//               PaletteImage.  It encapsulates all the information
//               specific to an image file that can be assigned as a
//               texture image to egg geometry.
////////////////////////////////////////////////////////////////////
class ImageFile : public TypedWriteable {
public:
  ImageFile();

  void make_shadow_image(const string &basename);

  bool is_size_known() const;
  int get_x_size() const;
  int get_y_size() const;
  bool has_num_channels() const;
  int get_num_channels() const;

  const TextureProperties &get_properties() const;
  void update_properties(const TextureProperties &properties);

  void set_filename(PaletteGroup *group, const string &basename);
  void set_filename(const string &dirname, const string &basename);
  const Filename &get_filename() const;
  const Filename &get_alpha_filename() const;
  bool exists() const;

  bool read(PNMImage &image) const;
  bool write(const PNMImage &image) const;
  void unlink();

  void update_egg_tex(EggTexture *egg_tex) const;

  void output_filename(ostream &out) const;

protected:
  TextureProperties _properties;
  Filename _filename;
  Filename _alpha_filename;

  bool _size_known;
  int _x_size, _y_size;

  // The TypedWriteable interface follows.
public:
  virtual void write_datagram(BamWriter *writer, Datagram &datagram); 

protected:
  void fillin(DatagramIterator &scan, BamReader *manager);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedWriteable::init_type();
    register_type(_type_handle, "ImageFile",
		  TypedWriteable::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }

private:
  static TypeHandle _type_handle;

};

#endif

