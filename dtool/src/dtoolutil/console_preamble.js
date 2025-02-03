/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file console_preamble.js
 * @author rdb
 * @date 2025-02-03
 */

if (ENVIRONMENT_IS_NODE) {
  Module["preInit"] = Module["preInit"] || [];
  Module["preInit"].push(function() {
    if (typeof process === "object" && typeof process.env === "object") {
      // These are made up by emscripten if we don't set them to undefined
      ENV['USER'] = undefined;
      ENV['LOGNAME'] = undefined;
      ENV['PATH'] = undefined;
      ENV['PWD'] = undefined;
      ENV['HOME'] = undefined;
      ENV['LANG'] = undefined;
      ENV['_'] = undefined;
      for (var variable in process.env) {
        ENV[variable] = process.env[variable];
      }
    }

    addOnPreMain(function preloadNodeEnv() {
      var sp = stackSave();
      var set_binary_name = wasmExports["_set_binary_name"];
      if (set_binary_name && typeof __filename === "string") {
        set_binary_name(stringToUTF8OnStack(__filename));
      }

      var set_env_var = wasmExports["_set_env_var"];
      if (set_env_var) {
        for (var variable in ENV) {
          var value = ENV[variable];
          if (value !== undefined) {
            set_env_var(stringToUTF8OnStack(variable), stringToUTF8OnStack(value));
          }
        }
      }
      stackRestore(sp);
    });
  });
}
