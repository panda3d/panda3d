/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggToDAE.cxx
 * @author rdb
 * @date 2008-10-04
 */

#include "eggToDAE.h"
#include "dcast.h"
#include "pandaVersion.h"

#include <FCDocument/FCDocument.h>
#include <FCDocument/FCDAsset.h>
#include <FCDocument/FCDTransform.h>

// Useful conversion stuff
#define TO_VEC3(v) (LVecBase3d(v[0], v[1], v[2]))
#define TO_VEC4(v) (LVecBase4d(v[0], v[1], v[2], v[3]))
#define TO_COLOR(v) (LColor(v[0], v[1], v[2], v[3]))
#define FROM_VEC3(v) (FMVector3(v[0], v[1], v[2]))
#define FROM_VEC4(v) (FMVector4(v[0], v[1], v[2], v[3]))
#define FROM_MAT4(v) (FMMatrix44(v.get_data()))
#define FROM_FSTRING(fs) (fs.c_str())

using std::cerr;

/**
 *
 */
EggToDAE::
EggToDAE() :
  EggToSomething("COLLADA", ".dae", true, false)
{
  set_binary_output(false);
  set_program_brief("convert .egg files into COLLADA asset files");
  set_program_description
    ("This program converts files from the egg format to the COLLADA "
     ".dae (Digital Asset Exchange) format.");

  _document = nullptr;
}

/**
 *
 */
void EggToDAE::
run() {
  nassertv(has_output_filename());
  nassertv(_data != nullptr);

  FCollada::Initialize();
  _document = FCollada::NewTopDocument();

  // Add the contributor part to the asset
  FCDAssetContributor* contributor = _document->GetAsset()->AddContributor();
  const char* user_name = getenv("USER");
  if (user_name == nullptr) user_name = getenv("USERNAME");
  if (user_name != nullptr) contributor->SetAuthor(TO_FSTRING(user_name));
  // contributor->SetSourceData();
  char authoring_tool[1024];
  snprintf(authoring_tool, 1024, "Panda3D %s eggToDAE converter | FCollada v%d.%02d", PANDA_VERSION_STR, FCOLLADA_VERSION >> 16, FCOLLADA_VERSION & 0xFFFF);
  authoring_tool[1023] = 0;
  contributor->SetAuthoringTool(TO_FSTRING(authoring_tool));

  // Set coordinate system
  switch (_data->get_coordinate_system()) {
    case CS_zup_right:
      _document->GetAsset()->SetUpAxis(FMVector3::ZAxis);
      break;
    case CS_yup_right:
      _document->GetAsset()->SetUpAxis(FMVector3::YAxis);
      break;
  }

  // Now actually start processing the data.
  FCDSceneNode* visual_scene = _document->AddVisualScene();
  for (EggGroupNode::iterator it = _data->begin(); it != _data->end(); ++it) {
    if ((*it)->is_of_type(EggGroup::get_class_type())) {
      process_node(visual_scene, DCAST(EggGroup, *it));
    }
  }

  // We're done here.
  FCollada::SaveDocument(_document, get_output_filename().to_os_specific().c_str());
  SAFE_DELETE(_document);
  FCollada::Release();

  // if (!out) { nout << "An error occurred while writing.\n"; exit(1); }
}

void EggToDAE::process_node(FCDSceneNode* parent, const PT(EggGroup) node) {
  assert(node != nullptr);
  FCDSceneNode* scene_node = parent->AddChildNode();
  // Set the parameters
  scene_node->SetDaeId(node->get_name().c_str());
  scene_node->SetJointFlag(node->is_joint());
  // Apply the transforms
  apply_transform(scene_node, node);
  // Recursively process sub-nodes
  for (EggGroupNode::iterator it = node->begin(); it != node->end(); ++it) {
    if ((*it)->is_of_type(EggGroup::get_class_type())) {
      process_node(scene_node, DCAST(EggGroup, *it));
    }
  }
}

void EggToDAE::apply_transform(FCDSceneNode* to, const PT(EggGroup) from) {
  assert(to != nullptr);
  assert(from != nullptr);
  for (int co = 0; co < from->get_num_components(); ++co) {
    switch (from->get_component_type(co)) {
      case EggTransform::CT_translate2d:
        cerr << "Warning: ignoring non-supported 2d translation\n";
        break;
      case EggTransform::CT_rotate2d:
        cerr << "Warning: ignoring non-supported 2d rotation\n";
        break;
      case EggTransform::CT_scale2d:
        cerr << "Warning: ignoring non-supported 2d scaling\n";
        break;
      case EggTransform::CT_matrix3:
        cerr << "Warning: ignoring non-supported 2d matrix\n";
        break;
      case EggTransform::CT_translate3d: {
        FCDTTranslation* new_transform = (FCDTTranslation*) to->AddTransform(FCDTransform::TRANSLATION);
        new_transform->SetTranslation(FROM_VEC3(from->get_component_vec3(co)));
        break; }
      case EggTransform::CT_rotate3d: {
        FCDTRotation* new_transform = (FCDTRotation*) to->AddTransform(FCDTransform::ROTATION);
        new_transform->SetRotation(FROM_VEC3(from->get_component_vec3(co)), from->get_component_number(co));
        break; }
      case EggTransform::CT_scale3d: {
        FCDTScale* new_transform = (FCDTScale*) to->AddTransform(FCDTransform::SCALE);
        new_transform->SetScale(FROM_VEC3(from->get_component_vec3(co)));
        break; }
      case EggTransform::CT_matrix4: {
        FCDTMatrix* new_transform = (FCDTMatrix*) to->AddTransform(FCDTransform::MATRIX);
        new_transform->SetTransform(FROM_MAT4(from->get_component_mat4(co)));
        break; }
      case EggTransform::CT_rotx: {
        FCDTRotation* new_transform = (FCDTRotation*) to->AddTransform(FCDTransform::ROTATION);
        new_transform->SetRotation(FMVector3::XAxis, from->get_component_number(co));
        break; }
      case EggTransform::CT_roty: {
        FCDTRotation* new_transform = (FCDTRotation*) to->AddTransform(FCDTransform::ROTATION);
        new_transform->SetRotation(FMVector3::YAxis, from->get_component_number(co));
        break; }
      case EggTransform::CT_rotz: {
        FCDTRotation* new_transform = (FCDTRotation*) to->AddTransform(FCDTransform::ROTATION);
        new_transform->SetRotation(FMVector3::ZAxis, from->get_component_number(co));
        break; }
      case EggTransform::CT_uniform_scale: {
        FCDTScale* new_transform = (FCDTScale*) to->AddTransform(FCDTransform::SCALE);
        new_transform->SetScale(from->get_component_number(co), from->get_component_number(co), from->get_component_number(co));
        break; }
      default:
        cerr << "Warning: ignoring invalid transform\n";
    }
  }
}

int main(int argc, char *argv[]) {
  EggToDAE prog;
  prog.parse_command_line(argc, argv);
  prog.run();
  return 0;
}
