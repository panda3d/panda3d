// This file defines the current version number for Panda.  It is read
// by Package.pp, which puts it in the global namespace for all
// ppremake scripts for Panda.

// Use spaces to separate the major, minor, and sequence numbers here.
#define PANDA_VERSION 1 10 0

// This variable will be defined to false in the CVS repository, but
// scripts that generate source tarballs and/or binary releases for
// distribution, by checking out Panda from an official CVS tag,
// should explictly set this to true.  When false, it indicates that
// the current version of Panda was checked out from CVS, so it may
// not be a complete representation of the indicated version.
#define PANDA_OFFICIAL_VERSION

// This string is reported verbatim by PandaSystem::get_distributor().
// It should be set by whoever provides a particular distribution of
// Panda.  If you build your own Panda, leave this unchanged.
#define PANDA_DISTRIBUTOR homebuilt

// This string is used to describe the Panda3D "package" associated
// with this current build of Panda.  It should increment with major
// and minor version changes, but not sequence (or "bugfix") changes.
// It should be unique for each unique distributor.  The default is
// the empty string, which means this build does not precisely match
// any distributable Panda3D packages.  If you are making a Panda3D
// build which you will be using to produce a distributable Panda3D
// package, you should set this string appropriately.
#define PANDA_PACKAGE_VERSION 

// We also define a version for the Panda3D plugin/runtime,
// i.e. nppanda3d.dll, p3dactivex.ocx, and panda3d.exe.  This is an
// independent version number from PANDA_VERSION or
// PANDA_PACKAGE_VERSION, because it is anticipated that this plugin
// code, once settled, will need to be updated much less frequently
// than Panda itself.
#define P3D_PLUGIN_VERSION 1 0 4

// Finally, there's a separate version number for the Core API.  At
// first, we didn't believe we needed a Core API version number, but
// in this belief we were naive.  This version number is a little less
// strict in its format requirements than P3D_PLUGIN_VERSION, above,
// and it doesn't necessarily consist of a specific number of
// integers, but by convention it will consist of four integers, with
// the first three matching the plugin version, and the fourth integer
// being incremented with each new Core API revision.
#define P3D_COREAPI_VERSION $[P3D_PLUGIN_VERSION] 2
