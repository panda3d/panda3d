#include <eventHandler.h>
#include <chancfg.h>
#include <textNode.h>
#include <eggLoader.h>
#include <mouse.h>
#include <graphicsWindow.h>
#include <chatInput.h>
#include <camera.h>
#include <transformTransition.h>
#include <lightTransition.h>
#include <chatHelpers.h> 
#include <notify.h>
#include <pt_NamedNode.h>
#include <framework.h>
#include <dataRelation.h>

PT(ChatInput) chat_input;
PT(TextNode) input_text_node;
PT(TextNode) output_text_node;

bool in_chat_mode = false;

extern int framework_main(int argc, char *argv[]);
extern void (*define_keys)(EventHandler&);

void event_enter(CPT_Event) {
  if (!in_chat_mode) {
    nout << "Enter chat mode" << endl;
    chat_input->reset();
    new DataRelation(mak, chat_input);  
    in_chat_mode = true;
  }
}

void event_chat_exit(CPT_Event) {
  if (in_chat_mode) {
    nout << "Exit chat mode" << endl;
    remove_child(mak, chat_input, DataRelation::get_class_type());
    output_text_node->set_text(chat_input->get_string());
    in_chat_mode = false;
  }
}

void event_chat_overflow(CPT_Event) {
  nout << "Too many characters." << endl;
}

void chat_keys(EventHandler& eh) {
  eh.add_hook("enter", event_enter);
  eh.add_hook("chat_exit", event_chat_exit);
  eh.add_hook("chat_overflow", event_chat_overflow);

  PT_NamedNode font = loader.load_sync("ttf-comic"); 

  // Create the input text node
  input_text_node = new TextNode("input_text_node");
  input_text_node->set_billboard(false);
  input_text_node->set_font(font.p());
  input_text_node->set_text("Press Enter to begin chat mode.");
  RenderRelation *text_arc = new RenderRelation(cameras, input_text_node);
  LMatrix4f mat = LMatrix4f::scale_mat(0.25);
  mat.set_row(3, LVector3f(-3, 8, -2.4));
  text_arc->set_transition(new TransformTransition(mat));
  LightTransition *no_light = new LightTransition(LightTransition::all_off());
  text_arc->set_transition(no_light);

  chat_input = new ChatInput(input_text_node, "chat input");
  chat_input->set_max_chars(20);

  // Create the output text node
  output_text_node = new TextNode("output_text_node");
  output_text_node->set_billboard(true);
  output_text_node->set_font(font.p());
  output_text_node->set_text_color(0, 0, 0, 1);
  output_text_node->set_card_as_margin(0.2, 0.2, 0.2, 0.2);
  output_text_node->set_card_color(1, 1, 1, 1);
  output_text_node->set_align(TM_ALIGN_CENTER);
  output_text_node->set_wordwrap(10.0);
  output_text_node->set_text("Output text");

  RenderRelation *out_arc = new RenderRelation(root, output_text_node);
  out_arc->set_transition(no_light);
}

int main(int argc, char *argv[]) {
  define_keys = &chat_keys;
  return framework_main(argc, argv);
}
