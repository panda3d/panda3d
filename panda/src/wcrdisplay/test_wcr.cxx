// Filename: test_wcr.cxx
// Created by:  skyler, based on wgl* file.
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

#include "graphicsPipe.h"
#include "interactiveGraphicsPipe.h"
#include "notify.h"
#include "pt_NamedNode.h"

#include "wcrGraphicsWindow.h"

PT(GraphicsPipe) main_pipe;
PT(GraphicsWindow) main_win;
PT_NamedNode render;

void render_frame(GraphicsPipe *pipe) {
  GraphicsPipe::wins_iterator wi;
  for (wi = pipe->get_win_begin();
       wi != pipe->get_win_end();
       ++wi) {
    (*wi)->get_gsg()->render_frame();
  }
}

void display_func() {
  render_frame(main_pipe);
}

int main() {

  GraphicsPipe::resolve_modules();

  cout << "Known pipe types:" << endl;
  GraphicsPipe::PipeFactory::FactoryTypesIter pti;
  for (pti = GraphicsPipe::_factory.get_types_begin();
       pti != GraphicsPipe::_factory.get_types_end(); ++pti) {
    cout << " " << (*pti) << endl;
  }
  cout << "after Known pipe types" << endl;

  GraphicsPipe::PipeFactory& factory = GraphicsPipe::_factory;
  GraphicsPipe::PipePrioritiesIter pribegin =
    GraphicsPipe::get_priorities_begin();
  GraphicsPipe::PipePrioritiesIter priend = GraphicsPipe::get_priorities_end();

  GraphicsPipe::PipeParams ps1;
  if (pribegin == priend)
    main_pipe = factory.instance(InteractiveGraphicsPipe::get_class_type(),
                                 ps1.begin(), ps1.end());
  else
    main_pipe = factory.instance(InteractiveGraphicsPipe::get_class_type(),
                                 ps1.begin(), ps1.end(), pribegin, priend);
  nassertr(main_pipe != (GraphicsPipe*)0L, 0);
  cout << "Opened a '" << main_pipe->get_type().get_name()
       << "' interactive graphics pipe." << endl;

  main_win = new wcrGraphicsWindow(main_pipe);
#if 0
  main_win->register_draw_function(display_func);
  main_win->main_loop();
#endif

  return 0;
}
