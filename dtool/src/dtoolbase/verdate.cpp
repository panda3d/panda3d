// used by version.rc
#define DEF #define
#define BD(x,y) DEF VER_BUILD_DATE_STR x "  " y

BD(__DATE__,__TIME__)
