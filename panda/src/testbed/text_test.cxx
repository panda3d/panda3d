// Filename: text_test.cxx
// Created by:  
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#include "eventHandler.h"
#include "chancfg.h"
#include "textNode.h"
#include "eggLoader.h"
#include "pnotify.h"
#include "pt_NamedNode.h"

extern PT_NamedNode render;
extern PT_NamedNode egg_root;
extern EventHandler event_handler;

extern int framework_main(int argc, char *argv[]);
extern void (*define_keys)(EventHandler&);

PT(TextNode) text_node;
char *textStr;

void event_p(CPT_Event) {
  text_node->set_text("I'm a woo woo woo!");

  nout << "text is " << text_node->get_width() << " by "
       << text_node->get_height() << "\n";
}

void event_s(CPT_Event) {
  text_node->set_wordwrap(5.0);

  nout << "text is " << text_node->get_width() << " by "
       << text_node->get_height() << "\n";
}

void text_keys(EventHandler& eh) {
  eh.add_hook("p", event_p);
  eh.add_hook("s", event_s);

  text_node = new TextNode("text_node");
  PT_NamedNode font = loader.load_sync("cmr12");
  text_node->set_font(font.p());
  text_node->set_wordwrap(20.0);
  text_node->set_card_as_margin(0.25, 0.25, 0.25, 0.25);
  PT(Texture) tex = new Texture;
  tex->set_name("genericButton.rgb");
  tex->set_minfilter(SamplerState::FT_linear);
  tex->set_magfilter(SamplerState::FT_linear);
  tex->read("/beta/toons/textures/smGreyButtonUp.rgb");
  text_node->set_card_texture( tex );
  text_node->set_card_border(0.1, 0.1);
  text_node->set_text( textStr );
  text_node->set_text_color( 0.0, 0.0, 0.0, 1.0 );
  if (text_node->has_card_texture())
    nout << "I've got a texture!" << "\n";
  else
    nout << "I don't have a texture..." << "\n";
  nout << "text is " << text_node->get_width() << " by "
       << text_node->get_height() << "\n";

  new RenderRelation(egg_root, text_node);
}

int main(int argc, char *argv[]) {
  define_keys = &text_keys;
        if (argc > 1)
                textStr = argv[1];
        else
                textStr = argv[0];
  return framework_main(argc, argv);
}
