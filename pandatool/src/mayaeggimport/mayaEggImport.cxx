// Filename: mayaEggImport.cxx
// Created by:  jyelon (20Jul05)
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
//
// This is the wrapper code for the maya importer plugin.
// It includes:
//
//   - user interface dialogs and popups
//   - plugin initialization/registration
//
// It does not include the actual code to traverse the EggData.
//
////////////////////////////////////////////////////////////////////

#include <string.h> 
#include <sys/types.h>

#include "dtoolbase.h"

#include "pre_maya_include.h"
#include <maya/MStatus.h>
#include <maya/MPxCommand.h>
#include <maya/MString.h>
#include <maya/MStringArray.h>
#include <maya/MArgList.h>
#include <maya/MGlobal.h>
#include <maya/MSelectionList.h>
#include <maya/MItSelectionList.h>
#include <maya/MPoint.h>
#include <maya/MPointArray.h>
#include <maya/MDagPath.h>
#include <maya/MDagPathArray.h>
#include <maya/MFnPlugin.h>
#include <maya/MFnMesh.h>
#include <maya/MFnSet.h>
#include <maya/MItMeshPolygon.h>
#include <maya/MItMeshVertex.h>
#include <maya/MItMeshEdge.h>
#include <maya/MFloatVector.h>
#include <maya/MFloatVectorArray.h>
#include <maya/MFloatArray.h>
#include <maya/MObjectArray.h>
#include <maya/MObject.h>
#include <maya/MPlug.h>
#include <maya/MPxFileTranslator.h>
#include <maya/MFnDagNode.h>
#include <maya/MItDag.h>
#include <maya/MDistance.h>
#include <maya/MIntArray.h>
#include "post_maya_include.h"

//////////////////////////////////////////////////////////////

class MayaEggImporter : public MPxFileTranslator
{
public:
  MayaEggImporter () {};
  virtual         ~MayaEggImporter () {};
  static void*    creator();
  
  MStatus         reader ( const MFileObject& file,
                           const MString& optionsString,
                           FileAccessMode mode);
  
  MStatus         writer ( const MFileObject& file,
                           const MString& optionsString,
                           FileAccessMode mode );
  
  bool            haveReadMethod () const { return true; }
  bool            haveWriteMethod () const { return false; }
  MString         defaultExtension () const { return "egg"; }
  MFileKind       identifyFile ( const MFileObject& fileName,
                                 const char* buffer,
                                 short size) const;
};


void* MayaEggImporter::creator()
{
  return new MayaEggImporter();
}

MStatus MayaEggImporter::reader ( const MFileObject& file,
                                const MString& options,
                                FileAccessMode mode)
{
  fprintf(stderr, "MayaEggImporter::reader not really implemented\n");
  return MS::kFailure;
}

MStatus MayaEggImporter::writer ( const MFileObject& file,
                                const MString& options,
                                FileAccessMode mode )

{
  fprintf(stderr, "MayaEggImporter::writer called in error\n");
  return MS::kFailure;
}

MPxFileTranslator::MFileKind MayaEggImporter::identifyFile (
                                                          const MFileObject& fileName,
                                                          const char* buffer,
                                                          short size) const
{
  const char * name = fileName.name().asChar();
  int   nameLength = strlen(name);
  
  if ((nameLength > 4) && !stricmp(name+nameLength-4, ".egg"))
    return kCouldBeMyFileType;
  else
    return kNotMyFileType;
}

EXPCL_MISC MStatus initializePlugin( MObject obj )
{
  MFnPlugin plugin( obj, "Alias", "3.0", "Any");
  
  // Register the translator with the system
  return plugin.registerFileTranslator( "Panda3D Egg Import", "none",
                                        MayaEggImporter::creator,
                                        
                                        "eggImportOptions",
                                        "merge=1;model=1;anim=0;");
}

EXPCL_MISC MStatus uninitializePlugin( MObject obj )
{
  MFnPlugin plugin( obj );
  return plugin.deregisterFileTranslator( "Panda3D Egg Import" );
}
