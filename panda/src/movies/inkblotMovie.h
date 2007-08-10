// Filename: inkblotMovie.h
// Created by: jyelon (02Jul07)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#ifndef INKBLOTMOVIE_H
#define INKBLOTMOVIE_H

#include "pandabase.h"
#include "texture.h"
#include "pointerTo.h"
#include "movie.h"

////////////////////////////////////////////////////////////////////
//       Class : InkblotMovie
// Description : This cellular automaton generates an attractive
//               pattern of swirling colors.  It is called "Digital
//               Inkblots," it was invented by Jason Rampe, who in
//               turn based it on Rudy Rucker's automaton "Rug."
//               Both automata were included in the program "Mirek's
//               Cellebration," which is a fantastic exploration
//               of different kinds of cellular automata.  I
//               encourage anyone to download it, it's a blast.
//               I find that 128x128 at about 15 fps is just right
//               for this automaton.
//
//               I have included a cellular automaton here mainly
//               as a simple example of how to derive from class
//               Movie, and to demonstrate that a "Movie" can be
//               anything that generates a series of video frames,
//               not just an AVI file.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_MOVIES InkblotMovie : public Movie {

PUBLISHED:
  InkblotMovie(const string &name, double len,
               int sizex=256, int sizey=256, int fps=10);

  INLINE int get_fps() const;
  virtual PT(MovieVideo) get_video(double offset=0.0) const;
  virtual PT(MovieAudio) get_audio(double offset=0.0) const;

protected:
  int _fps;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    Movie::init_type();
    register_type(_type_handle, "InkblotMovie",
                  Movie::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "inkblotMovie.I"

#endif
