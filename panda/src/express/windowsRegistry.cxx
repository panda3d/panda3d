/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file windowsRegistry.cxx
 * @author drose
 * @date 2001-08-06
 */

#include "windowsRegistry.h"
#include "config_express.h"
#include "textEncoder.h"

#if defined(WIN32_VC)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN 1
#endif
#include <windows.h>

using std::string;

/**
 * Sets the registry key to the indicated value as a string.  The supplied
 * string value is automatically converted from whatever encoding is set by
 * TextEncoder::set_default_encoding() and written as a Unicode string.  The
 * registry key must already exist prior to calling this function.
 */
bool WindowsRegistry::
set_string_value(const string &key, const string &name, const string &value,
        WindowsRegistry::RegLevel rl)
{
  TextEncoder encoder;
  std::wstring wvalue = encoder.decode_text(value);

  // Now convert the string to Windows' idea of the correct wide-char
  // encoding, so we can store it in the registry.  This might well be the
  // same string we just decoded from, but it might not.

  // Windows likes to have a null character trailing the string (even though
  // we also pass a length).
  wvalue += (wchar_t)0;
  int mb_result_len =
    WideCharToMultiByte(CP_ACP, 0,
                        wvalue.data(), wvalue.length(),
                        nullptr, 0,
                        nullptr, nullptr);
  if (mb_result_len == 0) {
    express_cat.error()
      << "Unable to convert '" << value
      << "' from Unicode to MultiByte form.\n";
    return false;
  }

  char *mb_result = (char *)alloca(mb_result_len);
  WideCharToMultiByte(CP_ACP, 0,
                      wvalue.data(), wvalue.length(),
                      mb_result, mb_result_len,
                      nullptr, nullptr);

  if (express_cat.is_debug()) {
    express_cat.debug()
      << "Converted '" << value << "' to '" << mb_result
      << "' for storing in registry.\n";
  }

  return do_set(key, name, REG_SZ, mb_result, mb_result_len, rl);
}

/**
 * Sets the registry key to the indicated value as an integer.  The registry
 * key must already exist prior to calling this function.
 */
bool WindowsRegistry::
set_int_value(const string &key, const string &name, int value,
        WindowsRegistry::RegLevel rl)
{
  DWORD dw = value;
  return do_set(key, name, REG_DWORD, &dw, sizeof(dw), rl);
}

/**
 * Returns the type of the indicated key, or T_none if the key is not known or
 * is some unsupported type.
 */
WindowsRegistry::Type WindowsRegistry::
get_key_type(const string &key, const string &name,
        WindowsRegistry::RegLevel rl)
{
  int data_type;
  string data;
  if (!do_get(key, name, data_type, data, rl)) {
    return T_none;
  }

  switch (data_type) {
  case REG_SZ:
    return T_string;

  case REG_DWORD:
    return T_int;

  default:
    return T_none;
  }
}

/**
 * Returns the value associated with the indicated registry key, assuming it
 * is a string value.  The string value is automatically encoded using
 * TextEncoder::get_default_encoding().  If the key is not defined or is not a
 * string type value, default_value is returned instead.
 */
string WindowsRegistry::
get_string_value(const string &key, const string &name,
                 const string &default_value,
                 WindowsRegistry::RegLevel rl)
{
  int data_type;
  string data;
  if (!do_get(key, name, data_type, data, rl)) {
    return default_value;
  }

  if (data_type != REG_SZ) {
    express_cat.warning()
      << "Registry key " << key << " does not contain a string value.\n";
    return default_value;
  }

  // Now we have to decode the MultiByte string to Unicode, and re-encode it
  // according to our own encoding.

  if (data.empty()) {
    return data;
  }

  int wide_result_len =
    MultiByteToWideChar(CP_ACP, 0,
                        data.data(), data.length(),
                        nullptr, 0);
  if (wide_result_len == 0) {
    express_cat.error()
      << "Unable to convert '" << data
      << "' from MultiByte to Unicode form.\n";
    return data;
  }

  wchar_t *wide_result = (wchar_t *)alloca(wide_result_len * sizeof(wchar_t));
  MultiByteToWideChar(CP_ACP, 0,
                      data.data(), data.length(),
                      wide_result, wide_result_len);

  std::wstring wdata(wide_result, wide_result_len);

  TextEncoder encoder;
  string result = encoder.encode_wtext(wdata);

  if (express_cat.is_debug()) {
    express_cat.debug()
      << "Converted '" << data << "' from registry to '" << result << "'\n";
  }

  return result;
}

/**
 * Returns the value associated with the indicated registry key, assuming it
 * is an integer value.  If the key is not defined or is not an integer type
 * value, default_value is returned instead.
 */
int WindowsRegistry::
get_int_value(const string &key, const string &name, int default_value,
        WindowsRegistry::RegLevel rl)
{
  int data_type;
  string data;
  if (!do_get(key, name, data_type, data, rl)) {
    return default_value;
  }

  if (data_type != REG_DWORD) {
    express_cat.warning()
      << "Registry key " << key << " does not contain an integer value.\n";
    return default_value;
  }

  // Now we have a DWORD encoded in a string.
  nassertr(data.length() == sizeof(DWORD), default_value);
  DWORD dw = *(DWORD *)data.data();
  return dw;
}

/**
 * The internal function to actually make all of the appropriate windows calls
 * to set the registry value.
 */
bool WindowsRegistry::
do_set(const string &key, const string &name,
       int data_type, const void *data, int data_length,
       const WindowsRegistry::RegLevel rl)
{
  HKEY hkey, regkey = HKEY_LOCAL_MACHINE;
  LONG error;

  if (rl == rl_user)    // switch to user local settings
      regkey = HKEY_CURRENT_USER;

  error =
    RegOpenKeyEx(regkey, key.c_str(), 0, KEY_SET_VALUE, &hkey);
  if (error != ERROR_SUCCESS) {
    express_cat.error()
      << "Unable to open registry key " << key
      << ": " << format_message(error) << "\n";
    return false;
  }

  bool okflag = true;

  error =
    RegSetValueEx(hkey, name.c_str(), 0, data_type,
                  (CONST BYTE *)data, data_length);
  if (error != ERROR_SUCCESS) {
    express_cat.error()
      << "Unable to set registry key " << key << " name " << name
      << ": " << format_message(error) << "\n";
    okflag = false;
  }

  error = RegCloseKey(hkey);
  if (error != ERROR_SUCCESS) {
    express_cat.warning()
      << "Unable to close opened registry key " << key
      << ": " << format_message(error) << "\n";
  }

  return okflag;
}

/**
 * The internal function to actually make all of the appropriate windows calls
 * to retrieve the registry value.
 */
bool WindowsRegistry::
do_get(const string &key, const string &name, int &data_type, string &data,
       const WindowsRegistry::RegLevel rl)
{
  HKEY hkey, regkey = HKEY_LOCAL_MACHINE;
  LONG error;

  if (rl == rl_user)    // switch to user local settings
      regkey = HKEY_CURRENT_USER;

  error =
    RegOpenKeyEx(regkey, key.c_str(), 0, KEY_QUERY_VALUE, &hkey);
  if (error != ERROR_SUCCESS) {
    express_cat.debug()
      << "Unable to open registry key " << key
      << ": " << format_message(error) << "\n";
    return false;
  }

  bool okflag = true;

  // We start with a 1K buffer; presumably that will be big enough most of the
  // time.
  static const size_t init_buffer_size = 1024;
  char init_buffer[init_buffer_size];
  DWORD buffer_size = init_buffer_size;
  DWORD dw_data_type;

  error =
    RegQueryValueEx(hkey, name.c_str(), 0, &dw_data_type,
                    (BYTE *)init_buffer, &buffer_size);
  if (error == ERROR_SUCCESS) {
    data_type = dw_data_type;
    if (data_type == REG_SZ ||
        data_type == REG_MULTI_SZ ||
        data_type == REG_EXPAND_SZ) {
      // Eliminate the trailing null character for non-zero lengths.
      if (buffer_size > 0)              // if zero, leave it
        buffer_size--;
    }
    data = string(init_buffer, buffer_size);

  } else if (error == ERROR_MORE_DATA) {
    // Huh, 1K wasn't big enough.  Ok, get a bigger buffer.

    // If we were querying HKEY_PERFORMANCE_DATA, we'd have to keep guessing
    // bigger and bigger until we got it.  Since we're querying static data
    // for now, we can just use the size Windows tells us.
    char *new_buffer = (char *)PANDA_MALLOC_ARRAY(buffer_size);
    error =
      RegQueryValueEx(hkey, name.c_str(), 0, &dw_data_type,
                      (BYTE *)new_buffer, &buffer_size);
    if (error == ERROR_SUCCESS) {
      data_type = dw_data_type;
      if (data_type == REG_SZ ||
          data_type == REG_MULTI_SZ ||
          data_type == REG_EXPAND_SZ) {
        // Eliminate the trailing null character for non-zero lengths.
        if (buffer_size > 0)            // if zero, leave it
          buffer_size--;
      }
      data = string(new_buffer, buffer_size);
    }
    PANDA_FREE_ARRAY(new_buffer);
  }

  if (error != ERROR_SUCCESS) {
    express_cat.debug()
      << "Unable to get registry value " << name
      << ": " << format_message(error) << "\n";
    okflag = false;
  }

  error = RegCloseKey(hkey);
  if (error != ERROR_SUCCESS) {
    express_cat.warning()
      << "Unable to close opened registry key " << key
      << ": " << format_message(error) << "\n";
  }

  if (okflag) {
    if (data_type == REG_EXPAND_SZ) {
      // Expand the string.
      DWORD destSize=ExpandEnvironmentStrings(data.c_str(), 0, 0);
      char *dest = (char *)PANDA_MALLOC_ARRAY(destSize);
      ExpandEnvironmentStrings(data.c_str(), dest, destSize);
      data = dest;
      PANDA_FREE_ARRAY(dest);
      data_type = REG_SZ;
    }
  }

  return okflag;
}

/**
 * Returns the Windows error message associated with the given error code.
 */
string WindowsRegistry::
format_message(int error_code) {
  PVOID buffer;
  DWORD length =
    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                  nullptr, error_code, 0, (LPTSTR)&buffer, 0, nullptr);
  if (length == 0) {
    return "Unknown error message";
  }

  const char *text = (const char *)buffer;

  // Strip off \n's and \r's trailing the string.
  while (length > 0 &&
         (text[length - 1] == '\r' || text[length - 1] == '\n')) {
    length--;
  }

  string result((const char *)text, length);
  LocalFree(buffer);
  return result;
}

#endif
