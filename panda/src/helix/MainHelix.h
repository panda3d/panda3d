#if !defined (_MAINHELIX_H_)
#define _MAINHELIX_H_

// This file basically just contains all of the header
// files that I will need to invoke the client core
// and start an engine.
#include "pandabase.h"
#include "helixDefs.h"

#include <hxtypes.h>

#include <stdlib.h>

#include <hlxclib/time.h>

#include <hxwintyp.h>
#include <hxcom.h>
#include <ihxpckts.h>
#include <hxcomm.h>
#include <hxmon.h>
#include <hxfiles.h>
#include <hxengin.h>
#include <hxcore.h>
#include <hxclsnk.h>
#include <hxerror.h>
#include <hxauth.h>
#include <hxwin.h>
#include <hxprefs.h>
#include <hxtbuf.h>
#include <hxbuffer.h>
#include <hxmangle.h>
//#include "fivemmap.h"
#include <dllacces.h>
#include <dllpath.h>
#include <hxstrutl.h>

#include <HxAdviseSink.h>
#include <HxErrorSink.h>
#include <HxSiteSupplier.h>
#include <HxAuthenticationManager.h>
#include <HxClientContext.h>
#include "print.h"

typedef HX_RESULT (HXEXPORT_PTR FPRMSETDLLACCESSPATH) (const char*);

#endif
/*

#include "hxtypes.h>

#include <stdlib.h>

#include <common/runtime/pub/hlxclib/time.h>

#include <common/include/hxwintyp.h>
#include <common/include/hxcom.h>
#include <common/include/ihxpckts.h>
#include <common/include/hxcomm.h>
#include <common/include/hxmon.h>
#include <common/include/hxfiles.h>
#include <common/include/hxengin.h>
#include <common/include/hxcore.h>
#include <client/include/hxclsnk.h>
#include <common/include/hxerror.h>
#include <common/include/hxauth.h>
#include <common/include/hxwin.h>
#include <common/include/hxprefs.h>
#include <common/include/hxtbuf.h>
#include <common/container/pub/hxbuffer.h>
#include <common/util/pub/hxmangle.h>
//#include "fivemmap.h"
#include <common/system/pub/dllacces.h>
#include <common/system/pub/dllpath.h>
#include <common/util/pub/hxstrutl.h> */