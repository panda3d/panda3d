// Filename: stitchImageCommandOutput.cxx
// Created by:  drose (29Nov99)
// 
////////////////////////////////////////////////////////////////////

#include "stitchImageCommandOutput.h"
#include "stitchImage.h"
#include "stitchLens.h"
#include "stitcher.h"
#include "stitchCommand.h"

#include <pnmImage.h>
#include <compose_matrix.h>

StitchImageCommandOutput::
StitchImageCommandOutput() {
}


void StitchImageCommandOutput::
add_input_image(StitchImage *image) {
  _input_images.push_back(image);
}

void StitchImageCommandOutput::
add_output_image(StitchImage *image) {
  _output_images.push_back(image);
}

void StitchImageCommandOutput::
add_stitcher(Stitcher *stitcher) {
  _stitchers.push_back(stitcher);
}

void StitchImageCommandOutput::
execute() {
  StitchCommand root;

  Stitchers::const_iterator si;
  for (si = _stitchers.begin(); si != _stitchers.end(); ++si) {
    Stitcher *stitcher = (*si);
    Stitcher::LoosePoints::const_iterator pi;
    for (pi = stitcher->_loose_points.begin();
         pi != stitcher->_loose_points.end();
         ++pi) {
      StitchCommand *cmd = new StitchCommand(&root, StitchCommand::C_point3d);
      cmd->set_name((*pi)->_name);
      cmd->set_point3d((*pi)->_space);
    }
  }

  Images::const_iterator ii;
  for (ii = _input_images.begin(); ii != _input_images.end(); ++ii) {
    StitchImage *input = (*ii);
    StitchCommand *image_cmd = new StitchCommand(&root, StitchCommand::C_input_image);
    fill_image_cmd(image_cmd, input);
  }

  for (ii = _output_images.begin(); ii != _output_images.end(); ++ii) {
    StitchImage *output = (*ii);
    StitchCommand *image_cmd = new StitchCommand(&root, StitchCommand::C_output_image);
    fill_image_cmd(image_cmd, output);
  }

  cout << root << "\n";
}

void StitchImageCommandOutput::
fill_image_cmd(StitchCommand *image_cmd, StitchImage *image) {
  if (image->has_name()) {
    image_cmd->set_name(image->get_name());
  }

  StitchCommand *cmd;
  cmd = new StitchCommand(image_cmd, StitchCommand::C_filename);
  cmd->set_str(image->get_filename());

  cmd = new StitchCommand(image_cmd, StitchCommand::C_image_size);
  cmd->set_point2d(image->get_size_pixels());

  cmd = new StitchCommand(image_cmd, StitchCommand::C_film_size);
  cmd->set_length_pair(image->get_size_mm());

  image->_lens->make_lens_command(image_cmd);

  LVecBase3d scale, hpr;
  if (decompose_matrix(image->_rotate, scale, hpr)) {
    cmd = new StitchCommand(image_cmd, StitchCommand::C_hpr);
    cmd->set_point3d(hpr);
  }
}
