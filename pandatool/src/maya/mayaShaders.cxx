// Filename: mayaShaders.cxx
// Created by:  drose (11Feb00)
// 
////////////////////////////////////////////////////////////////////

#include "mayaShaders.h"
#include "mayaShader.h"
#include "global_parameters.h"

#include <maya/MStatus.h>
#include <maya/MObject.h>
#include <maya/MFnDependencyNode.h>
#include <maya/MPlug.h>
#include <maya/MPlugArray.h>
#include <maya/MFn.h>

MayaShader *MayaShaders::
find_shader_for_node(MObject node) {
  MStatus status;
  MFnDependencyNode node_fn(node);

  // Look on the instObjGroups attribute for shading engines.
  MObject iog_attr = node_fn.attribute("instObjGroups", &status);
  if (!status) {
    // The node is not renderable.  What are you thinking?
    nout << node_fn.name() << " : not a renderable object.\n";
    return (MayaShader *)NULL;
  }

  // instObjGroups is a multi attribute, whatever that means.  For
  // now, we'll just get the first connection, since that's what the
  // example code did.  Is there any reason to search deeper?

  MPlug iog_plug(node, iog_attr);
  MPlugArray iog_pa;
  iog_plug.elementByLogicalIndex(0).connectedTo(iog_pa, false, true, &status);
  if (!status) {
    // No shading group defined for this object.
    nout << node_fn.name() << " : no shading group defined.\n";
    return (MayaShader *)NULL;
  }

  // Now we have a number of ShadingEngines defined, one for each of
  // these connections we just turned up.  Usually there will only be
  // one.  In fact, we'll just take the first one we find.

  size_t i;
  for (i = 0; i < iog_pa.length(); i++) {
    MObject engine = iog_pa[i].node();
    if (engine.hasFn(MFn::kShadingEngine)) {
      return find_shader_for_shading_engine(engine);
    }
  }

  // Well, we didn't find a ShadingEngine after all.  Huh.
  if (verbose >= 2) {
    nout << node_fn.name() << " : no shading engine found.\n";
  }
  return (MayaShader *)NULL;
}

MayaShader *MayaShaders::
find_shader_for_shading_engine(MObject engine) {
  MFnDependencyNode engine_fn(engine);

  // See if we have already decoded this engine.
  string engine_name = engine_fn.name().asChar();
  Shaders::const_iterator si = _shaders.find(engine_name);
  if (si != _shaders.end()) {
    return (*si).second;
  }

  // All right, this is a newly encountered shading engine.  Create a
  // new MayaShader object to represent it.
  MayaShader *shader = new MayaShader(engine);

  // Record this for the future.
  _shaders.insert(Shaders::value_type(engine_name, shader));
  return shader;
}


