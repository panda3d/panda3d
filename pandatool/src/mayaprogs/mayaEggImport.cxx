/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file mayaEggImport.cxx
 * @author jyelon
 * @date 2005-07-20
 *
 * This is the wrapper code for the maya importer plugin.
 * It includes:
 *
 *   - user interface dialogs and popups
 *   - plugin initialization/registration
 *
 * It does not include the actual code to traverse the EggData.
 */

#include <string.h>
#include <sys/types.h>

#include "dtoolbase.h"

// We must define this to prevent Maya from doubly-declaring its MApiVersion
// string in this file as well as in libmayaegg.
#define _MApiVersion

#include "pre_maya_include.h"
#include <maya/MStatus.h>
#include <maya/MPxCommand.h>
#include <maya/MString.h>
#include <maya/MStringArray.h>
#include <maya/MArgList.h>
#include <maya/MGlobal.h>
#include <maya/MFnPlugin.h>
#include <maya/MObject.h>
#include <maya/MPlug.h>
#include <maya/MPxFileTranslator.h>
#include "post_maya_include.h"

#include "mayaEggLoader.h"
#include "notifyCategoryProxy.h"


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
  MString fileName = file.fullName();
  bool model=false;
  bool anim=false;

  if (options.length() > 0) {
    const MString flagModel("model");
    const MString flagAnim("anim");

    // Start parsing.
    MStringArray optionList;
    MStringArray theOption;
    options.split(';', optionList);

    unsigned nOptions = optionList.length();
    for (unsigned i = 0; i < nOptions; i++) {

      theOption.clear();
      optionList[i].split('=', theOption);
      if (theOption.length() < 1) {
        continue;
      }

      if (theOption[0] == flagModel && theOption.length() > 1) {
        model = atoi(theOption[1].asChar()) ? true:false;
      } else if (theOption[0] == flagAnim && theOption.length() > 1) {
        anim = atoi(theOption[1].asChar()) ? true:false;
      }
    }
  }

  if ((mode != kImportAccessMode)&&(mode != kOpenAccessMode))
    return MS::kFailure;

  bool merge = (mode == kImportAccessMode);
  std::ostringstream log;
  Notify::ptr()->set_ostream_ptr(&log, false);
  bool ok = MayaLoadEggFile(fileName.asChar(), merge, model, anim, false);
  std::string txt = log.str();
  if (txt != "") {
    MGlobal::displayError(txt.c_str());
  } else {
    if (!ok) MGlobal::displayError("Cannot import Egg file, unknown reason");
  }
  return ok ? MS::kSuccess : MS::kFailure;
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

  if ((nameLength > 4) && !strcmp(name+nameLength-4, ".egg"))
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
