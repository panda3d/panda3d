// Filename: imageBase.h
// Created by:  drose (19Jun00)
// 
////////////////////////////////////////////////////////////////////

#ifndef IMAGEBASE_H
#define IMAGEBASE_H

#include <pandatoolbase.h>

#include <programBase.h>
#include <coordinateSystem.h>
#include <pnmImage.h>

////////////////////////////////////////////////////////////////////
//       Class : ImageBase
// Description : This specialization of ProgramBase is intended for
//               programs that read and/or write a single image file.
//               (See ImageMultiBase for programs that operate on
//               multiple image files at once.)
//
//               This is just a base class; see ImageReader, ImageWriter,
//               or ImageFilter according to your particular I/O needs.
////////////////////////////////////////////////////////////////////
class ImageBase : public ProgramBase {
public:
  ImageBase();

protected:
  virtual bool post_command_line();

protected:
  PNMImage _image;
};

#endif


