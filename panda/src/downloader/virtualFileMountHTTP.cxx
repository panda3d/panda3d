/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file virtualFileMountHTTP.cxx
 * @author drose
 * @date 2008-10-30
 */

#include "virtualFileMountHTTP.h"
#include "virtualFileHTTP.h"
#include "virtualFileSystem.h"

#ifdef HAVE_OPENSSL

using std::string;

TypeHandle VirtualFileMountHTTP::_type_handle;


/**
 *
 */
VirtualFileMountHTTP::
VirtualFileMountHTTP(const URLSpec &root, HTTPClient *http) :
  _http(http),
  _root(root)
{
  // Make sure the root ends on a slash.  The implicit trailing slash is a
  // semi-standard internet convention.
  string path = _root.get_path();
  if (!path.empty() && path[path.length() - 1] != '/') {
    path += '/';
    _root.set_path(path);
  }
}

/**
 *
 */
VirtualFileMountHTTP::
~VirtualFileMountHTTP() {
}


/**
 * Reads all of the vfs-mount-url lines in the Config.prc file and replaces
 * the mount settings to match them.
 *
 * This will mount any url's mentioned in the config file, and unmount and
 * unmount any url's no longer mentioned in the config file.  Normally, it is
 * called automatically at startup, and need not be called again, unless you
 * have fiddled with some config settings.
 */
void VirtualFileMountHTTP::
reload_vfs_mount_url() {
  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();

  // First, unload the existing mounts.
  int n = 0;
  while (n < vfs->get_num_mounts()) {
    PT(VirtualFileMount) mount = vfs->get_mount(n);
    if (mount->is_of_type(VirtualFileMountHTTP::get_class_type())) {
      vfs->unmount(mount);
      // Don't increment n.
    } else {
      ++n;
    }
  }

  // Now, reload the newly specified mounts.
  ConfigVariableList mounts
    ("vfs-mount-url",
     PRC_DESC("vfs-mount-url http://site/path[:port] mount-point [options]"));

  int num_unique_values = mounts.get_num_unique_values();
  for (int i = 0; i < num_unique_values; i++) {
    string mount_desc = mounts.get_unique_value(i);

    size_t space = mount_desc.rfind(' ');
    if (space == string::npos) {
      downloader_cat.warning()
        << "No space in vfs-mount-url descriptor: " << mount_desc << "\n";

    } else {
      string mount_point = mount_desc.substr(space + 1);
      while (space > 0 && isspace(mount_desc[space - 1])) {
        space--;
      }
      mount_desc = mount_desc.substr(0, space);
      string options;

      space = mount_desc.rfind(' ');
      if (space != string::npos) {
        // If there's another space, we have the optional options field.
        options = mount_point;
        mount_point = mount_desc.substr(space + 1);
        while (space > 0 && isspace(mount_desc[space - 1])) {
          --space;
        }
        mount_desc = mount_desc.substr(0, space);
      }

      mount_desc = ExecutionEnvironment::expand_string(mount_desc);
      URLSpec root(mount_desc);

      int flags = 0;
      string password;

      // Split the options up by commas.
      size_t p = 0;
      size_t q = options.find(',', p);
      while (q != string::npos) {
        vfs->parse_option(options.substr(p, q - p),
                          flags, password);
        p = q + 1;
        q = options.find(',', p);
      }
      vfs->parse_option(options.substr(p), flags, password);

      PT(VirtualFileMount) mount = new VirtualFileMountHTTP(root);
      vfs->mount(mount, mount_point, flags);
    }
  }
}

/**
 * Returns true if the indicated file exists within the mount system.
 */
bool VirtualFileMountHTTP::
has_file(const Filename &) const {
  return false;
}

/**
 * Returns true if the indicated file exists within the mount system and is a
 * directory.
 */
bool VirtualFileMountHTTP::
is_directory(const Filename &) const {
  return false;
}

/**
 * Returns true if the indicated file exists within the mount system and is a
 * regular file.
 */
bool VirtualFileMountHTTP::
is_regular_file(const Filename &) const {
  return false;
}

/**
 * Constructs and returns a new VirtualFile instance that corresponds to the
 * indicated filename within this mount point.  The returned VirtualFile
 * object does not imply that the given file actually exists; but if the file
 * does exist, then the handle can be used to read it.
 */
PT(VirtualFile) VirtualFileMountHTTP::
make_virtual_file(const Filename &local_filename,
                  const Filename &original_filename, bool implicit_pz_file,
                  int open_flags) {
  PT(VirtualFileHTTP) vfile =
    new VirtualFileHTTP(this, local_filename, implicit_pz_file, open_flags);
  vfile->set_original_filename(original_filename);

  return vfile;
}

/**
 * Opens the file for reading, if it exists.  Returns a newly allocated
 * istream on success (which you should eventually delete when you are done
 * reading). Returns NULL on failure.
 */
std::istream *VirtualFileMountHTTP::
open_read_file(const Filename &) const {
  return nullptr;
}

/**
 * Returns the current size on disk (or wherever it is) of the already-open
 * file.  Pass in the stream that was returned by open_read_file(); some
 * implementations may require this stream to determine the size.
 */
std::streamsize VirtualFileMountHTTP::
get_file_size(const Filename &, std::istream *) const {
  return 0;
}

/**
 * Returns the current size on disk (or wherever it is) of the file before it
 * has been opened.
 */
std::streamsize VirtualFileMountHTTP::
get_file_size(const Filename &) const {
  return 0;
}

/**
 * Returns a time_t value that represents the time the file was last modified,
 * to within whatever precision the operating system records this information
 * (on a Windows95 system, for instance, this may only be accurate to within 2
 * seconds).
 *
 * If the timestamp cannot be determined, either because it is not supported
 * by the operating system or because there is some error (such as file not
 * found), returns 0.
 */
time_t VirtualFileMountHTTP::
get_timestamp(const Filename &) const {
  return 0;
}

/**
 * Fills the given vector up with the list of filenames that are local to this
 * directory, if the filename is a directory.  Returns true if successful, or
 * false if the file is not a directory or cannot be read.
 */
bool VirtualFileMountHTTP::
scan_directory(vector_string &, const Filename &) const {
  return false;
}

/**
 *
 */
void VirtualFileMountHTTP::
output(std::ostream &out) const {
  out << _root;
}

/**
 * Returns an HTTPChannel object suitable for use for extracting a document
 * from the current URL root.
 */
PT(HTTPChannel) VirtualFileMountHTTP::
get_channel() {
  PT(HTTPChannel) channel;
  _channels_lock.lock();

  if (!_channels.empty()) {
    // If we have some channels sitting around, grab one.  Grab the one on the
    // end; it was most recently pushed, and therefore most likely to be still
    // alive.
    channel = _channels.back();
    _channels.pop_back();
  } else {
    // If we don't have any channels standing by, make a new one.
    channel = _http->make_channel(true);
  }

  _channels_lock.unlock();
  return channel;
}

/**
 * Accepts an HTTPChannel that is no longer being used, and restores it to
 * standby duty, so that it will be returned by a future call to
 * get_channel().
 */
void VirtualFileMountHTTP::
recycle_channel(HTTPChannel *channel) {
  _channels_lock.lock();
  _channels.push_back(channel);
  _channels_lock.unlock();
}

#endif  // HAVE_OPENSSL
