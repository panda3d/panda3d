// Filename: test_glut.cxx
// Created by:  mike (02Feb00)
// 
////////////////////////////////////////////////////////////////////

//#include "glutGraphicsPipe.h"
//#include "glutGraphicsWindow.h"
#include <graphicsPipe.h>
#include <interactiveGraphicsPipe.h>
#include <allAttributesWrapper.h>
#include <pt_NamedNode.h>

#include <notify.h>

PT(GraphicsPipe) main_pipe;
PT(GraphicsWindow) main_win;
PT_NamedNode render;
NodeAttributes initial_state;

void render_frame(GraphicsPipe *pipe) {
  GraphicsPipe::wins_iterator wi;
  for (wi = pipe->get_win_begin();
       wi != pipe->get_win_end();
       ++wi) {
    (*wi)->get_gsg()->render_frame(render, initial_state);
  }
}

void display_func(void) {
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

#if 0
  main_win = new glutGraphicsWindow(main_pipe);
  main_win->register_draw_function(display_func);
  main_win->main_loop();
#endif

  return 0;
}
