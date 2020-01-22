Directory Info
--------------
**Directory:** /cmake/scripts  
**License:** Unlicense  
**Description:** This directory is used for cmake files which are not meant to
be included using CMake's normal include() directive.  Typically, files in this
directory will be invoked as a custom command/target in the form of:
```
 cmake -P <CustomScriptName.cmake> [... other options ...]
```
