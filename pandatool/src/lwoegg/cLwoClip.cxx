// Filename: cLwoClip.cxx
// Created by:  drose (27Apr01)
// 
////////////////////////////////////////////////////////////////////

#include "cLwoClip.h"
#include "lwoToEggConverter.h"

#include <lwoClip.h>
#include <lwoStillImage.h>


////////////////////////////////////////////////////////////////////
//     Function: CLwoClip::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
CLwoClip::
CLwoClip(LwoToEggConverter *converter, const LwoClip *clip) :
  _converter(converter),
  _clip(clip)
{
  _still_image = false;

  // Walk through the chunk list, looking for some basic properties.
  int num_chunks = _clip->get_num_chunks();
  for (int i = 0; i < num_chunks; i++) {
    const IffChunk *chunk = _clip->get_chunk(i);

    if (chunk->is_of_type(LwoStillImage::get_class_type())) {
      const LwoStillImage *image = DCAST(LwoStillImage, chunk);
      _filename = image->_filename;
      _still_image = true;
    }      
  }
}
