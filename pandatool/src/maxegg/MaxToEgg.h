/*
  MaxToEgg.h 
  Created by Ken Strickland, 02/24/03
  Modified + Maintained by Corey Revilla, (05/22/03-Present)
  CMU's Entertainment Technology Center
  
  This file defines the MaxToEgg class, a class derived from SomethingToEgg,
  which means it was designed to be a standalone application that would get
  called by Ppremake during compilation to convert models of the appropriate
  type. As there doesn't seem to be a way get at the 3dsMax API without going
  the plug-in route, however, this class is actually run by another wrapper
  class, MaxEggPlugin. This class, in turn, is a wrapper about the 
  MaxToEggConverter class, which actually twiddles the bits, as they say.
*/
#ifndef __MaxToEgg__H
#define __MaxToEgg__H

#pragma conform(forScope, off)

#include "pandatoolbase.h"

#include "MaxToEggConverter.h"

/* Error-Reporting Includes
 */
#include "Logger.h"
#define MTE Logger::ST_MAP_ME_TO_APP_SPECIFIC_SYSTEM5

/**
 * This class defines a "converter" between Max files and eggs.  All it
 * does is define the output file, call MaxToEggConverter to actually
 * convert the geometry, and then write out the output file.
 */
class MaxToEgg : public SomethingToEgg 
{
 protected:
  // If true, various windows pop-up alerts will announce when certain tasks
  // begin.
  bool confirmExport;
  // The local pointer to the 3ds max interface we keep around to get the
  // scene graph. If we ever get this to run standalone, we'll need an
  // alternate way to set this other than through MaxEggPlugin. 
  Interface *pMaxInterface;
  // False initially, but premanently switches to true when a file is 
  // sucessfully converted.
  bool successfulOutput;
  bool doubleSided;
  virtual bool handle_args(Args &args);

 public:
  MaxToEgg();
  ~MaxToEgg();
  bool IsSuccessful();
  //Returns a pointer to the class name.
  char *MyClassName();
  void Run(ULONG *selection_list, int len);
  void SetMaxInterface(Interface *pInterface);
};

#endif
