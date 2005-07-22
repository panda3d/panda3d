
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
  
  bool            haveReadMethod () const;
  bool            haveWriteMethod () const;
  MString         defaultExtension () const;
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
  fprintf(stderr, "MayaEggImporter::reader called in error\n");
  return MS::kFailure;
}

MStatus MayaEggImporter::writer ( const MFileObject& file,
                                const MString& options,
                                FileAccessMode mode )

{
  fprintf(stderr, "MayaEggImporter::writer called in error\n");
  return MS::kFailure;
}

bool MayaEggImporter::haveReadMethod () const
{
  return false;
}

bool MayaEggImporter::haveWriteMethod () const
{
  return false;
}

MString MayaEggImporter::defaultExtension () const
{
  return "egg";
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
                                        "merge=1;model=1;anim=1;");
}

EXPCL_MISC MStatus uninitializePlugin( MObject obj )
{
  MFnPlugin plugin( obj );
  return plugin.deregisterFileTranslator( "Panda3D Egg Import" );
}
