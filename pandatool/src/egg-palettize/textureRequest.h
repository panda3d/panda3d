// Filename: textureRequest.h
// Created by:  drose (29Nov00)
// 
////////////////////////////////////////////////////////////////////

#ifndef TEXTUREREQUEST_H
#define TEXTUREREQUEST_H

#include <pandatoolbase.h>

#include "textureProperties.h"

#include <eggTexture.h>

////////////////////////////////////////////////////////////////////
// 	 Class : TextureRequest
// Description : These are the things that a user might explicitly
//               request to adjust on a texture via a line in the .txa
//               file.
////////////////////////////////////////////////////////////////////
class TextureRequest {
public:
  TextureRequest();
  void pre_txa_file();

  TextureProperties _properties;

  bool _got_size;
  bool _got_num_channels;
  int _x_size;
  int _y_size;
  int _num_channels;
  EggTexture::Format _format;
  EggTexture::FilterType _minfilter;
  EggTexture::FilterType _magfilter;
  bool _omit;
  int _margin;
  double _coverage_threshold;
};

#endif

