// Filename: sourceTextureImage.h
// Created by:  drose (28Nov00)
// 
////////////////////////////////////////////////////////////////////

#ifndef SOURCETEXTUREIMAGE_H
#define SOURCETEXTUREIMAGE_H

#include <pandatoolbase.h>

#include "imageFile.h"

class TextureImage;

////////////////////////////////////////////////////////////////////
// 	 Class : SourceTextureImage
// Description : This is a texture image reference as it appears in an
//               egg file: the source image of the texture.
////////////////////////////////////////////////////////////////////
class SourceTextureImage : public ImageFile {
private:
  SourceTextureImage();

public:
  SourceTextureImage(TextureImage *texture, const Filename &filename,
		     const Filename &alpha_filename);

  TextureImage *get_texture() const;
  
  bool get_size();
  bool read_header();

  void output(ostream &out) const;

private:
  bool _read_header;
  bool _successfully_read_header;
  TextureImage *_texture;

  // The TypedWriteable interface follows.
public:
  static void register_with_read_factory();
  virtual void write_datagram(BamWriter *writer, Datagram &datagram); 
  virtual int complete_pointers(vector_typedWriteable &plist, 
				BamReader *manager);

protected:
  static TypedWriteable *make_SourceTextureImage(const FactoryParams &params);
  void fillin(DatagramIterator &scan, BamReader *manager);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    ImageFile::init_type();
    register_type(_type_handle, "SourceTextureImage",
		  ImageFile::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }

private:
  static TypeHandle _type_handle;
};

INLINE ostream &
operator << (ostream &out, const SourceTextureImage &source) {
  source.output(out);
  return out;
}

#endif

