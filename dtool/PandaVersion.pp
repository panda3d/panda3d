// This file defines the current version number for Panda.  It is read
// by Package.pp, which puts it in the global namespace for all
// ppremake scripts for Panda.

// Actually, we don't have ppremake any more, but this file is still
// being parsed today by makepanda.  We should probably find a better
// place to put this.

// Use spaces to separate the major, minor, and sequence numbers here.
#define PANDA_VERSION 1 11 0

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
