#ifndef odeHelperStructs_h
#define odeHelperStructs_h

#ifndef CPPPARSER
struct sSurfaceParams
    {
        dSurfaceParameters colparams;
        dReal dampen;
    };

struct sBodyParams
    {
//        int surfaceType;
        dReal dampen;
    };
#endif

#endif
