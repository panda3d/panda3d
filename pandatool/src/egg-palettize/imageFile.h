// Filename: imageFile.h
// Created by:  drose (07Sep99)
// 
////////////////////////////////////////////////////////////////////

#ifndef IMAGEFILE_H
#define IMAGEFILE_H

#include <pandatoolbase.h>

#include <filename.h>

////////////////////////////////////////////////////////////////////
// 	 Class : ImageFile
// Description : This is the base class for both Palette and Texture.
////////////////////////////////////////////////////////////////////
class ImageFile {
public:
  ImageFile();
  virtual ~ImageFile();

  virtual Filename get_filename() const=0;
  virtual Filename get_basename() const=0;

};

#endif

