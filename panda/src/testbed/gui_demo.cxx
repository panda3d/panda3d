#include "framework.h"

#include <eventHandler.h>
#include <chancfg.h>
#include <string>
#include <renderModeTransition.h>
#include <colorTransition.h>
#include <colorBlendTransition.h>
#include <cullFaceTransition.h>
#include <depthTestTransition.h>
#include <depthWriteTransition.h>
#include <textureTransition.h>
#include <textureApplyTransition.h>
#include <lightTransition.h>
#include <materialTransition.h>
#include <transformTransition.h>
#include <transparencyTransition.h>
#include <drawBoundsTransition.h>
#include <pruneTransition.h>
#include <get_rel_pos.h>
#include <boundingSphere.h>
#include <geomSphere.h>
#include <geomNode.h>
#include <notify.h>
#include <directionalLight.h>
#include <renderRelation.h>
#include <camera.h>
#include <frustum.h>
#include <orthoProjection.h>
#include <perspectiveProjection.h>
#include <textNode.h>
#include <shaderTransition.h>
#include <colorMatrixTransition.h>
#include <alphaTransformTransition.h>
#include <lensFlareNode.h>
#include <texture.h>
#include <texturePool.h>
#include <spotlight.h>
#include <nodePath.h>
#include <pta_Colorf.h>
#include <pta_float.h>
#include <pt_Node.h>

#include <guiManager.h>

//From framework
extern PT(GeomNode) geomnode;
extern RenderRelation* first_arc;

static void setup_gui(void) {
  GuiManager* mgr = GuiManager::get_ptr(main_win, mak);
}

static void event_2(CPT_Event) {
  static bool is_setup = false;
  if (!is_setup) {
    setup_gui();
    is_setup = true;
  }
}

void demo_keys(EventHandler&) {
  new RenderRelation( lights, dlight );
  have_dlight = true;

  event_handler.add_hook("2", event_2);
}

int main(int argc, char *argv[]) {
  define_keys = &demo_keys;
  return framework_main(argc, argv);
}
