/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file android_main.cxx
 * @author rdb
 * @date 2021-12-06
 */

#include "config_android.h"
#include "config_putil.h"
#include "virtualFileMountAndroidAsset.h"
#include "virtualFileSystem.h"
#include "filename.h"
#include "thread.h"
#include "urlSpec.h"

#include "android_native_app_glue.h"

#include "Python.h"
#include "structmember.h"

#include <sys/mman.h>
#include <android/log.h>

// Leave room for future expansion.
#define MAX_NUM_POINTERS 24

/* Stored in the flags field of the blobinfo structure below. */
enum Flags {
  F_log_append = 1,
  F_log_filename_strftime = 2,
  F_keep_docstrings = 4,
  F_python_verbose = 8,
};

// Define an exposed symbol where we store the offset to the module data.
extern "C" {
  __attribute__((__visibility__("default"), used))
  volatile struct {
    uint64_t blob_offset;
    uint64_t blob_size;
    uint16_t version;
    uint16_t num_pointers;
    uint16_t codepage;
    uint16_t flags;
    uint64_t reserved;
    void *pointers[MAX_NUM_POINTERS];

    // The reason we initialize it to -1 is because otherwise, smart linkers may
    // end up putting it in the .bss section for zero-initialized data.
  } blobinfo = {(uint64_t)-1};
}

// Defined in android_support.cxx
extern "C" PyObject *PyInit_android_support();

/**
 * Maps the binary blob at the given memory address to memory, and returns the
 * pointer to the beginning of it.
 */
static void *map_blob(const char *path, off_t offset, size_t size) {
  FILE *runtime = fopen(path, "rb");
  assert(runtime != NULL);

  // Align the offset down to the page size boundary.
  size_t page_size = getpagesize();
  off_t aligned = offset & ~((off_t)page_size - 1);
  size_t delta = (size_t)(offset - aligned);

  void *blob = (void *)mmap(0, size + delta, PROT_READ, MAP_PRIVATE, fileno(runtime), aligned);
  assert(blob != MAP_FAILED);

  fclose(runtime);
  return (void *)((uintptr_t)blob + delta);
}

/**
 * The inverse of map_blob.
 */
static void unmap_blob(void *blob) {
  if (blob) {
    munmap(blob, blobinfo.blob_size);
  }
}

/**
 * This function is called by native_app_glue to initialize the program.
 *
 * Note that this does not run in the main thread, but in a thread created
 * specifically for this activity by android_native_app_glue.
 *
 * Unlike the regular deploy-stub, we need to interface directly with the
 * Panda3D libraries here, since we can't pass the pointers from Java to Panda
 * through the Python interpreter easily.
 */
void android_main(struct android_app *app) {
  panda_android_app = app;

  // Attach the app thread to the Java VM.
  JNIEnv *env;
  ANativeActivity *activity = app->activity;
  int attach_status = activity->vm->AttachCurrentThread(&env, nullptr);
  if (attach_status < 0 || env == nullptr) {
    android_cat.error() << "Failed to attach thread to JVM!\n";
    return;
  }

  jclass activity_class = env->GetObjectClass(activity->clazz);

  // Get the current Java thread name.  This just helps with debugging.
  jmethodID methodID = env->GetStaticMethodID(activity_class, "getCurrentThreadName", "()Ljava/lang/String;");
  jstring jthread_name = (jstring) env->CallStaticObjectMethod(activity_class, methodID);

  std::string thread_name;
  if (jthread_name != nullptr) {
    const char *c_str = env->GetStringUTFChars(jthread_name, nullptr);
    thread_name.assign(c_str);
    env->ReleaseStringUTFChars(jthread_name, c_str);
  }

  // Before we make any Panda calls, we must make the thread known to Panda.
  // This will also cause the JNIEnv pointer to be stored on the thread.
  // Note that we must keep a reference to this thread around.
  PT(Thread) current_thread = Thread::bind_thread(thread_name, "android_app");

  android_cat.info()
    << "New native activity started on " << *current_thread
    << " (" << current_thread << ")\n";

  // Fetch the data directory.
  jmethodID get_appinfo = env->GetMethodID(activity_class, "getApplicationInfo", "()Landroid/content/pm/ApplicationInfo;");

  jobject appinfo = env->CallObjectMethod(activity->clazz, get_appinfo);
  jclass appinfo_class = env->GetObjectClass(appinfo);

  // Fetch the path to the data directory.
  jfieldID datadir_field = env->GetFieldID(appinfo_class, "dataDir", "Ljava/lang/String;");
  jstring datadir = (jstring) env->GetObjectField(appinfo, datadir_field);
  const char *data_path = env->GetStringUTFChars(datadir, nullptr);

  if (data_path != nullptr) {
    Filename::_internal_data_dir = data_path;
    android_cat.info() << "Path to data: " << data_path << "\n";

    env->ReleaseStringUTFChars(datadir, data_path);
  }

  // Get the cache directory.  Set the model-path to this location.
  methodID = env->GetMethodID(activity_class, "getCacheDirString", "()Ljava/lang/String;");
  jstring jcache_dir = (jstring) env->CallObjectMethod(activity->clazz, methodID);

  if (jcache_dir != nullptr) {
    const char *cache_dir;
    cache_dir = env->GetStringUTFChars(jcache_dir, nullptr);
    android_cat.info() << "Path to cache: " << cache_dir << "\n";

    ConfigVariableFilename model_cache_dir("model-cache-dir", Filename());
    model_cache_dir.set_value(cache_dir);
    env->ReleaseStringUTFChars(jcache_dir, cache_dir);
  }

  // Fetch the path to the library directory.
  jfieldID libdir_field = env->GetFieldID(appinfo_class, "nativeLibraryDir", "Ljava/lang/String;");
  jstring libdir_jstr = (jstring) env->GetObjectField(appinfo, libdir_field);
  const char *libdir = env->GetStringUTFChars(libdir_jstr, nullptr);

  if (libdir != nullptr) {
    std::string dtool_name = std::string(libdir) + "/libp3dtool.so";
    ExecutionEnvironment::set_dtool_name(dtool_name);
    android_cat.info() << "Path to dtool: " << dtool_name << "\n";
  }

  // Get the path to the APK.
  methodID = env->GetMethodID(activity_class, "getPackageCodePath", "()Ljava/lang/String;");
  jstring code_path = (jstring) env->CallObjectMethod(activity->clazz, methodID);

  const char *apk_path;
  apk_path = env->GetStringUTFChars(code_path, nullptr);
  android_cat.info() << "Path to APK: " << apk_path << "\n";

  // Get the path to the native library.
  methodID = env->GetMethodID(activity_class, "getNativeLibraryPath", "()Ljava/lang/String;");
  jstring lib_path_jstr = (jstring) env->CallObjectMethod(activity->clazz, methodID);

  const char *lib_path;
  lib_path = env->GetStringUTFChars(lib_path_jstr, nullptr);
  android_cat.info() << "Path to native library: " << lib_path << "\n";
  ExecutionEnvironment::set_binary_name(lib_path);

  // Nowadays we store the blob in a raw resource.
  methodID = env->GetMethodID(activity_class, "mapBlobFromResource", "(J)J");
  assert(methodID != nullptr);
  void *blob = (void *)env->CallLongMethod(activity->clazz, methodID, blobinfo.blob_offset);

  if (blob == nullptr) {
    // Try the old method otherwise.
    blob = map_blob(lib_path, (off_t)blobinfo.blob_offset, (size_t)blobinfo.blob_size);
  }
  env->ReleaseStringUTFChars(lib_path_jstr, lib_path);
  assert(blob != nullptr);

  // This should always be aligned, but just in case...
  void *blob_copy = nullptr;
  if ((((uintptr_t)blob) & (sizeof(void *) - 1)) != 0) {
    android_cat.warning()
      << "Blob with offset " << blobinfo.blob_offset
      << " and size " << blobinfo.blob_size << " is not aligned!\n";

    blob_copy = malloc(blobinfo.blob_size);
    memcpy(blob_copy, blob, blobinfo.blob_size);
    blob = blob_copy;
  }

  assert(blobinfo.num_pointers <= MAX_NUM_POINTERS);
  for (uint32_t i = 0; i < blobinfo.num_pointers; ++i) {
    // Only offset if the pointer is non-NULL.  Except for the first
    // pointer, which may never be NULL and usually (but not always)
    // points to the beginning of the blob.
    if (i == 0 || blobinfo.pointers[i] != nullptr) {
      blobinfo.pointers[i] = (void *)((uintptr_t)blobinfo.pointers[i] + (uintptr_t)blob);
    }
  }

  // Now load the configuration files.
  ConfigPage *page = nullptr;
  ConfigPageManager *cp_mgr;
  const char *prc_data = (char *)blobinfo.pointers[1];
  if (prc_data != nullptr) {
    cp_mgr = ConfigPageManager::get_global_ptr();
    std::istringstream in(prc_data);
    page = cp_mgr->make_explicit_page("builtin");
    page->read_prc(in);
  }

  // Mount the assets directory.
  Filename apk_fn(apk_path);
  PT(VirtualFileMountAndroidAsset) asset_mount;
  asset_mount = new VirtualFileMountAndroidAsset(app->activity->assetManager, apk_fn);
  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();

  //Filename asset_dir(apk_fn.get_dirname(), "assets");
  Filename asset_dir("/android_asset");
  vfs->mount(asset_mount, asset_dir, 0);

  // Release the apk_path.
  env->ReleaseStringUTFChars(code_path, apk_path);

  // Now add the asset directory to the model-path.
  //TODO: prevent it from adding the directory multiple times.
  get_model_path().append_directory(asset_dir);

  // Offset the pointers in the module table using the base mmap address.
  // We did a read-only mmap, so we have to create a copy of this table.
  // First count how many modules there are.
  struct _frozen *src_moddef = (struct _frozen *)blobinfo.pointers[0];
  struct _frozen *dst_moddef;
  struct _frozen *new_modules = nullptr;
  if (blob_copy != nullptr) {
    // We already made a copy, so we can just write to this.
    dst_moddef = src_moddef;
  } else {
    size_t num_modules = 0;
    while (src_moddef->name) {
      num_modules++;
      src_moddef++;
    }

    new_modules = (struct _frozen *)malloc(sizeof(struct _frozen) * (num_modules + 1));
    memcpy(new_modules, blobinfo.pointers[0], sizeof(struct _frozen) * (num_modules + 1));
    dst_moddef = new_modules;
  }
  PyImport_FrozenModules = dst_moddef;

  while (dst_moddef->name) {
    dst_moddef->name = (char *)((uintptr_t)dst_moddef->name + (uintptr_t)blob);
    if (dst_moddef->code != nullptr) {
      dst_moddef->code = (unsigned char *)((uintptr_t)dst_moddef->code + (uintptr_t)blob);
    }
    //__android_log_print(ANDROID_LOG_DEBUG, "Panda3D", "MOD: %s %p %d\n", dst_moddef->name, (void*)dst_moddef->code, dst_moddef->size);
    dst_moddef++;
  }

  PyPreConfig preconfig;
  PyPreConfig_InitIsolatedConfig(&preconfig);
  preconfig.utf8_mode = 1;
  PyStatus status = Py_PreInitialize(&preconfig);
  if (PyStatus_Exception(status)) {
    env->ReleaseStringUTFChars(libdir_jstr, libdir);
    Py_ExitStatusException(status);
    return;
  }

  // Register the android_support module.
  if (PyImport_AppendInittab("android_support", &PyInit_android_support) < 0) {
    android_cat.error()
      << "Failed to register android_support module.\n";
    env->ReleaseStringUTFChars(libdir_jstr, libdir);
    return;
  }

  PyConfig config;
  PyConfig_InitIsolatedConfig(&config);
  config.pathconfig_warnings = 0;   /* Suppress errors from getpath.c */
  config.buffered_stdio = 0;
  config.configure_c_stdio = 0;
  config.write_bytecode = 0;
  config.module_search_paths_set = 1; // Leave sys.path empty
  PyConfig_SetBytesString(&config, &config.platlibdir, libdir);
  env->ReleaseStringUTFChars(libdir_jstr, libdir);

  if (blobinfo.flags & F_python_verbose) {
    config.verbose = 1;
  }

  status = Py_InitializeFromConfig(&config);
  PyConfig_Clear(&config);
  if (PyStatus_Exception(status)) {
      Py_ExitStatusException(status);
      return;
  }

  while (!app->destroyRequested) {
    // Call the main module.  This will not return until the app is done.
    android_cat.info() << "Importing __main__\n";

    int n = PyImport_ImportFrozenModule("__main__");
    if (n == 0) {
      Py_FatalError("__main__ not frozen");
      break;
    }
    if (n < 0) {
      if (!PyErr_ExceptionMatches(PyExc_SystemExit)) {
        PyErr_Print();
      } else {
        PyErr_Clear();
      }
    }

    if (app->destroyRequested) {
      // The app closed responding to a destroy request.
      break;
    }

    // Ask Android to clean up the activity.
    android_cat.info() << "Exited from __main__, finishing activity\n";
    ANativeActivity_finish(activity);

    // We still need to keep an event loop going until Android gives us leave
    // to end the process.
    while (!app->destroyRequested) {
      struct android_poll_source *source = nullptr;
      int ident = ALooper_pollOnce(-1, nullptr, nullptr, (void **)&source);
      if (ident == LOOPER_ID_MAIN) {
        int8_t cmd = android_app_read_cmd(app);
        android_app_pre_exec_cmd(app, cmd);
        android_app_post_exec_cmd(app, cmd);

        // I don't think we can get a resume command after we call finish(),
        // but let's handle it just in case.
        if (cmd == APP_CMD_RESUME || cmd == APP_CMD_DESTROY) {
          break;
        }
      } else if (source != nullptr) {
        source->process(app, source);
      }
    }
  }

  Py_Finalize();

  android_cat.info() << "Destroy requested, exiting from android_main\n";

  vfs->unmount(asset_mount);

  if (page != nullptr) {
    cp_mgr->delete_explicit_page(page);
  }

  PyImport_FrozenModules = nullptr;
  if (new_modules != nullptr) {
    free(new_modules);
  }
  unmap_blob(blob);
  if (blob_copy != nullptr) {
    free(blob_copy);
  }

  // Detach the thread before exiting.
  activity->vm->DetachCurrentThread();

  _exit(0);
}
