typedef unsigned char   jboolean;       /* unsigned 8 bits */
typedef signed char     jbyte;          /* signed 8 bits */
typedef unsigned short  jchar;          /* unsigned 16 bits */
typedef short           jshort;         /* signed 16 bits */
typedef int             jint;           /* signed 32 bits */
typedef long long       jlong;          /* signed 64 bits */
typedef float           jfloat;         /* 32-bit IEEE 754 */
typedef double          jdouble;        /* 64-bit IEEE 754 */

typedef jint jsize;

typedef void*           jobject;
typedef jobject         jclass;
typedef jobject         jstring;
typedef jobject         jarray;
typedef jarray          jobjectArray;
typedef jarray          jbooleanArray;
typedef jarray          jbyteArray;
typedef jarray          jcharArray;
typedef jarray          jshortArray;
typedef jarray          jintArray;
typedef jarray          jlongArray;
typedef jarray          jfloatArray;
typedef jarray          jdoubleArray;
typedef jobject         jthrowable;
typedef jobject         jweak;

struct _jfieldID;                       /* opaque structure */
typedef struct _jfieldID* jfieldID;     /* field IDs */

struct _jmethodID;                      /* opaque structure */
typedef struct _jmethodID* jmethodID;   /* method IDs */

struct JNIInvokeInterface;

struct _JNIEnv;
struct _JavaVM;
typedef _JNIEnv JNIEnv;
typedef _JavaVM JavaVM;
