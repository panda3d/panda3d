// Filename: imageTrans.h
// Created by:  drose (19Jun00)
// 
////////////////////////////////////////////////////////////////////

#ifndef IMAGETRANS_H
#define IMAGETRANS_H

#include <pandatoolbase.h>

#include <imageFilter.h>

////////////////////////////////////////////////////////////////////
//       Class : ImageTrans
// Description : A program to read an image file and write an
//               equivalent image file, possibly performing some minor
//               operations along the way.
////////////////////////////////////////////////////////////////////
class ImageTrans : public ImageFilter {
public:
  ImageTrans();

  void run();
};

#endif

