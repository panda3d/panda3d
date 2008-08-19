#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <crtdbg.h>
#include "errno.h"
#include "Max.h"
#include "eggGroup.h"
#include "eggTable.h"
#include "eggXfmSAnim.h"
#include "eggData.h"
#include "pandatoolbase.h"
#include "referenceCount.h"
#include "pointerTo.h"
#include "namable.h"
#include "modstack.h"

#include <iostream>
#include <fstream>
#include <vector>

#include "windef.h"
#include "windows.h"

#include "Max.h"
#include "iparamb2.h"
#include "iparamm2.h"
#include "istdplug.h"
#include "iskin.h"
#include "maxResource.h"
#include "stdmat.h"
#include "phyexp.h"
#include "surf_api.h"
#include "bipexp.h"

#include "eggCoordinateSystem.h"
#include "eggGroup.h"
#include "eggPolygon.h"
#include "eggTextureCollection.h"
#include "eggTexture.h"
#include "eggVertex.h"
#include "eggVertexPool.h"
#include "eggNurbsCurve.h"
#include "pandatoolbase.h"
#include "somethingToEgg.h"
#include "somethingToEggConverter.h"
#include "eggXfmSAnim.h"

#include "maxNodeDesc.h"
#include "maxNodeTree.h"
#include "maxLogger.h"
#include "maxToEgg.h"
#include "maxEggExpOptions.h"
#include "maxToEggConverter.h"
#include "maxEgg.h"

#include "maxDllEntry.cxx"
#include "maxLogger.cxx"
#include "maxEgg.cxx"
#include "maxEggExpOptions.cxx"
#include "maxNodeDesc.cxx"
#include "maxNodeTree.cxx"
#include "maxToEgg.cxx"
#include "maxToEggConverter.cxx"
