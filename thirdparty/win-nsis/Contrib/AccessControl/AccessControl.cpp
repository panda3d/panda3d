
/* AccessControl plugin for NSIS
 * Copyright (C) 2003 Mathias Hasselmann <mathias@taschenorakel.de>
 *
 * This software is provided 'as-is', without any express or implied 
 * warranty. In no event will the authors be held liable for any damages 
 * arising from the use of this software. 
 *
 * Permission is granted to anyone to use this software for any purpose, 
 * including commercial applications, and to alter it and redistribute it 
 * freely, subject to the following restrictions:
 *
 *   1. The origin of this software must not be misrepresented; you must not 
 *      claim that you wrote the original software. If you use this software 
 *      in a product, an acknowledgment in the product documentation would be
 *      appreciated but is not required.
 *
 *   2. Altered source versions must be plainly marked as such, and must not 
 *      be misrepresented as being the original software.
 *
 *   3. This notice may not be removed or altered from any source 
 *      distribution.
 */

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include "nsis/pluginapi.h"
#pragma comment(lib, "nsis/pluginapi.lib")
#include <aclapi.h>
#include <sddl.h>

/*****************************************************************************
 GLOBAL VARIABLES
 *****************************************************************************/

//#pragma message("Warning: Where is g_output?")
//extern FILE * g_output;
static HINSTANCE g_hInstance = NULL;
/*static HWND g_hwndParent = NULL;
static HWND g_hwndDialog = NULL;
static HWND g_hwndProgress = NULL;*/
int g_string_size = 1024;

/*****************************************************************************
 TYPE DEFINITIONS
 *****************************************************************************/

typedef struct
{
  const TCHAR * name;
  SE_OBJECT_TYPE type;
  BYTE defaultInheritance;
  const TCHAR ** const permissionNames;
  const DWORD * const permissionFlags;
  const int permissionCount;
}
SchemeType;

typedef enum
{
  ChangeMode_Owner,
  ChangeMode_Group
}
ChangeMode;

/*****************************************************************************
 PLUG-IN HANDLING
 *****************************************************************************/

#define PUBLIC_FUNCTION(Name) \
extern "C" void __declspec(dllexport) Name(HWND hwndParent, int string_size, \
                TCHAR * variables, stack_t ** stacktop) \
{ \
  EXDLL_INIT(); \
  g_string_size = string_size;/* \
  g_hwndParent = hwndParent; \
  g_hwndDialog = FindWindowEx(g_hwndParent, NULL, WC_DIALOG, ""); \
  g_hwndProgress = GetDlgItem(g_hwndDialog, 1006);*/

#define PUBLIC_FUNCTION_END \
}

void showerror_s(const TCHAR * fmt, TCHAR * arg)
{
  TCHAR * msg = (TCHAR *)LocalAlloc(LPTR, g_string_size*sizeof(TCHAR));
  wsprintf(msg, fmt, arg);
  pushstring(msg);
  LocalFree(msg);
}

void showerror_d(const TCHAR * fmt, DWORD arg)
{
  TCHAR * msg = (TCHAR *)LocalAlloc(LPTR, g_string_size*sizeof(TCHAR));
  wsprintf(msg, fmt, arg);
  pushstring(msg);
  LocalFree(msg);
}

#define ABORT_s(x, y) \
    do { showerror_s(_T(x), y); goto cleanup; } while(0)
#define ABORT_d(x, y) \
    do { showerror_d(_T(x), y); goto cleanup; } while(0)
#define ABORT(x) \
    do { PushStringA(x); goto cleanup; } while(0)

BOOL WINAPI DllMain(HINSTANCE hInst, ULONG ul_reason_for_call, LPVOID lpReserved)
{
  g_hInstance = (HINSTANCE)hInst;
	return TRUE;
}

/*****************************************************************************
 ARRAY UTILITIES
 *****************************************************************************/

#define SIZE_OF_ARRAY(Array) \
  (sizeof((Array)) / sizeof(*(Array)))

#define ARRAY_CONTAINS(Array, Index) \
  ArrayContainsImpl(Index, SIZE_OF_ARRAY(Array))

/** Checks if an array of a given size contains a given index.
 ** Used by the ARRAY_CONTAINS macro.
 **/
static int ArrayContainsImpl(int index, int size)
{
  return index >= 0 && index < size;
}

/*****************************************************************************
 STRING UTILITIES
 *****************************************************************************/

/*char *my_strchr(char needle, char *haystack)
{
  char *ret = NULL;
  char *cp = NULL;

  if (haystack != NULL)
  {
    cp = haystack;
    while((*cp != '\0') && (ret == NULL))
    {
      if (*cp == needle) ret = cp;
      cp++;
    }
  }
  return ret;
}*/

/*
 * Based on function from http://www.codeguru.com/cpp/w-p/system/security/article.php/c5659/
 */
/*PSID GetBinarySid(LPCTSTR szSid)
{
  // This function is based off the KB article Q198907.
  // This function is the same as ConvertStringSidToSid(),
  // except that function is only on Windows 2000 and newer.
  // The calling function must free the returned SID with FreeSid.

  //_ASSERTE(szSid);
  //_ASSERTE(lstrlen(szSid));

  PSID pSid = NULL;

  char * szSidCopy = (char *)LocalAlloc(LPTR, lstrlen(szSid) + 1);

  try
  {
    int i;
    char * ptr, * ptr1;
    SID_IDENTIFIER_AUTHORITY sia;// ZeroMemory(&sia, sizeof(sia));
    BYTE nByteAuthorityCount = 0;
    DWORD dwSubAuthority[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    lstrcpy(szSidCopy, szSid);

    // S-SID_REVISION- + IdentifierAuthority- + subauthorities- + NULL

    // Skip 'S'
    if(!(ptr = my_strchr('-', szSidCopy)))
      return NULL;

    // Skip '-'
    ptr++;

    // Skip SID_REVISION
    if(!(ptr = my_strchr('-', ptr)))
      return NULL;

    // Skip '-'
    ptr++;

    // Skip IdentifierAuthority
    if(!(ptr1 = my_strchr('-', ptr)))
      return NULL;
    *ptr1 = 0;

    if((*ptr == '0') && (*(ptr + 1) == 'x'))
    {
      wsprintf(ptr, "0x%02hx%02hx%02hx%02hx%02hx%02hx",
        &sia.Value[0],
        &sia.Value[1],
        &sia.Value[2],
        &sia.Value[3],
        &sia.Value[4],
        &sia.Value[5]);
    }
    else
    {
      DWORD dwValue;
      wsprintf(ptr, "%lu", &dwValue);

      sia.Value[5] = (BYTE)(dwValue & 0x000000FF);
      sia.Value[4] = (BYTE)(dwValue & 0x0000FF00) >> 8;
      sia.Value[3] = (BYTE)(dwValue & 0x00FF0000) >> 16;
      sia.Value[2] = (BYTE)(dwValue & 0xFF000000) >> 24;
    }

    // Skip '-'
    *ptr1 = '-';
    ptr = ptr1;
    ptr1++;

    for(i = 0; i < 8; i++)
    {
      // Get subauthority
      if(!(ptr = my_strchr('-', ptr)))
      break;
      *ptr = 0;
      ptr++;
      nByteAuthorityCount++;
    }

    for(i = 0; i < nByteAuthorityCount; i++)
    {
      // Get subauthority
      wsprintf(ptr1, "%lu", &dwSubAuthority[i]);
      ptr1 += lstrlen(ptr1) + 1;
    }
    LocalFree(szSidCopy);
    szSidCopy = NULL;

    if(!AllocateAndInitializeSid(&sia,
      nByteAuthorityCount,
      dwSubAuthority[0],
      dwSubAuthority[1],
      dwSubAuthority[2],
      dwSubAuthority[3],
      dwSubAuthority[4],
      dwSubAuthority[5],
      dwSubAuthority[6],
      dwSubAuthority[7],
      &pSid))
    {
      pSid = NULL;
    }
  }
  catch(...)
  {
    LocalFree(szSidCopy);
    pSid = NULL;
  }
  return pSid;
}*/

/** Converts a string into an enumeration index. If the enumeration
 ** contains the string the index of the string within the enumeration
 ** is return. On error you'll receive -1.
 **/
static int ParseEnum(const TCHAR * keywords[], const TCHAR * str)
{
  const TCHAR ** key;

  for(key = keywords; *key; ++key)
    if (!lstrcmpi(str, *key))
      return (int)(key - keywords);

  return -1;
}

/** Parses a trustee string. If enclosed in brackets the string contains
 ** a string SID. Otherwise it's assumed that the string contains a
 ** trustee name.
 **/
static TCHAR * ParseTrustee(TCHAR * trustee, DWORD * trusteeForm)
{
  TCHAR * strend = trustee + lstrlen(trustee) - 1;

  if ('(' == *trustee && ')' == *strend)
  {
    PSID pSid = NULL;

    *strend = '\0'; 
    trustee++;

    //pSid = GetBinarySid(trustee);
    if (!ConvertStringSidToSid(trustee, &pSid))
      pSid = NULL;

    *trusteeForm = TRUSTEE_IS_SID;
		return (TCHAR *)pSid;
  }

  *trusteeForm = TRUSTEE_IS_NAME;
  TCHAR * ret = (TCHAR *)LocalAlloc(LPTR, g_string_size*sizeof(TCHAR));
  lstrcpy(ret, trustee);
  return ret;
}

static char * ParseSid(TCHAR * trustee)
{
  PSID pSid = NULL;
  TCHAR * strend = trustee + lstrlen(trustee) - 1;

  if ('(' == *trustee && ')' == *strend)
  {
    *strend = '\0'; 
    ++trustee;

    //pSid = GetBinarySid(trustee);
    if (!ConvertStringSidToSid(trustee, &pSid))
      pSid = NULL;
  }
  else
  {
    DWORD sidLen = 0;
    DWORD domLen = 0;
    TCHAR * domain = NULL;
    SID_NAME_USE use;

    if ((LookupAccountName(NULL, trustee, 
       NULL, &sidLen, NULL, &domLen, &use) ||
       ERROR_INSUFFICIENT_BUFFER == GetLastError()) &&
        NULL != (domain = (TCHAR *)LocalAlloc(LPTR, domLen*sizeof(TCHAR))) &&
      NULL != (pSid = (PSID)LocalAlloc(LPTR, sidLen)))
    {
      if (!LookupAccountName(NULL, trustee, 
        pSid, &sidLen, domain, &domLen, &use))
      {
        LocalFree(pSid);
        pSid = NULL;
      }
    }

    LocalFree(domain);
  }

  return (char *)pSid;
}

/* i know: this function is far to generious in accepting strings. 
 * but hey: this all is about code size, isn't it? 
 * so i can live with that pretty well.
 */
static DWORD ParsePermissions(const SchemeType * scheme, TCHAR * str)
{
  DWORD perms = 0;
  TCHAR * first, * last;

  for(first = str; *first; first = last)
  {
    int token;

    while(*first && *first <= ' ') ++first;
    for(last = first; *last && *last > ' ' && *last != '|' && 
      *last != '+'; ++last);
    if (*last) *last++ = '\0';

    token = ParseEnum(scheme->permissionNames, first);
    if (token >= 0 && token < scheme->permissionCount)
      perms|= scheme->permissionFlags[token];
  }

  return perms;
}

/*****************************************************************************
 SYMBOL TABLES
 *****************************************************************************/

static const TCHAR * g_filePermissionNames[] =
{
  _T("ReadData"), _T("WriteData"), _T("AppendData"), 
  _T("ReadEA"), _T("WriteEA"), _T("Execute"), _T("ReadAttributes"), _T("WriteAttributes"), 
  _T("Delete"), _T("ReadControl"), _T("WriteDAC"), _T("WriteOwner"), _T("Synchronize"),
  _T("FullAccess"), _T("GenericRead"), _T("GenericWrite"), _T("GenericExecute"), NULL
};

static const DWORD g_filePermissionFlags[] =
{
  FILE_READ_DATA, FILE_WRITE_DATA, FILE_APPEND_DATA,
  FILE_READ_EA, FILE_WRITE_EA, FILE_EXECUTE, FILE_READ_ATTRIBUTES, FILE_WRITE_ATTRIBUTES,
  DELETE, READ_CONTROL, WRITE_DAC, WRITE_OWNER, SYNCHRONIZE,
  FILE_ALL_ACCESS, FILE_GENERIC_READ, FILE_GENERIC_WRITE, FILE_GENERIC_EXECUTE
};

static const TCHAR * g_directoryPermissionNames[] =
{
  _T("ListDirectory"), _T("AddFile"), _T("AddSubdirectory"), 
  _T("ReadEA"), _T("WriteEA"), _T("Traverse"), _T("DeleteChild"), 
  _T("ReadAttributes"), _T("WriteAttributes"), 
  _T("Delete"), _T("ReadControl"), _T("WriteDAC"), _T("WriteOwner"), _T("Synchronize"),
  _T("FullAccess"), _T("GenericRead"), _T("GenericWrite"), _T("GenericExecute"), NULL
};

static const DWORD g_directoryPermissionFlags[] =
{
  FILE_LIST_DIRECTORY, FILE_ADD_FILE, FILE_ADD_SUBDIRECTORY,
  FILE_READ_EA, FILE_WRITE_EA, FILE_TRAVERSE, FILE_DELETE_CHILD,
  FILE_READ_ATTRIBUTES, FILE_WRITE_ATTRIBUTES,
  DELETE, READ_CONTROL, WRITE_DAC, WRITE_OWNER, SYNCHRONIZE,
  FILE_ALL_ACCESS, FILE_GENERIC_READ, FILE_GENERIC_WRITE, FILE_GENERIC_EXECUTE
};

static const TCHAR * g_registryPermissionNames[] =
{
  _T("QueryValue"), _T("SetValue"), _T("CreateSubKey"), 
  _T("EnumerateSubKeys"), _T("Notify"), _T("CreateLink"), 
  _T("Delete"), _T("ReadControl"), _T("WriteDAC"), _T("WriteOwner"), _T("Synchronize"),
  _T("GenericRead"), _T("GenericWrite"), _T("GenericExecute"), _T("FullAccess"), NULL
};

static const DWORD g_registryPermissionFlags[] =
{
  KEY_QUERY_VALUE, KEY_SET_VALUE, KEY_CREATE_SUB_KEY, 
  KEY_ENUMERATE_SUB_KEYS, KEY_NOTIFY, KEY_CREATE_LINK, 
  DELETE, READ_CONTROL, WRITE_DAC, WRITE_OWNER, SYNCHRONIZE,
  KEY_READ, KEY_WRITE, KEY_EXECUTE, KEY_ALL_ACCESS
};

static const SchemeType g_fileScheme[] =
{
  _T("file"), SE_FILE_OBJECT,
  OBJECT_INHERIT_ACE,
  g_filePermissionNames, 
  g_filePermissionFlags,
  SIZE_OF_ARRAY(g_filePermissionFlags)
};

static const SchemeType g_directoryScheme[] =
{
  _T("directory"), SE_FILE_OBJECT,
  OBJECT_INHERIT_ACE | CONTAINER_INHERIT_ACE,
  g_directoryPermissionNames, 
  g_directoryPermissionFlags,
  SIZE_OF_ARRAY(g_directoryPermissionFlags)
};

static const SchemeType g_registryScheme[] =
{ 
  _T("registry"), SE_REGISTRY_KEY,
  CONTAINER_INHERIT_ACE,
  g_registryPermissionNames, 
  g_registryPermissionFlags,
  SIZE_OF_ARRAY(g_registryPermissionFlags)
};

static const TCHAR * g_rootKeyNames[] =
{
  _T("HKCR"), _T("HKCU"), _T("HKLM"), _T("HKU"), NULL
};

static const TCHAR * g_rootKeyPrefixes[] =
{
  _T("CLASSES_ROOT\\"), _T("CURRENT_USER\\"), _T("MACHINE\\"), _T("USERS\\")
};

/*****************************************************************************
 GENERIC ACL HANDLING
 *****************************************************************************/

static void ChangeDACL(const SchemeType * scheme, TCHAR * path, DWORD mode, BOOL noinherit)
{
  TCHAR * param = (TCHAR *)LocalAlloc(LPTR, g_string_size*sizeof(TCHAR));
	TCHAR * trusteeName = NULL;
  PSID pSid = NULL;
  DWORD trusteeForm = TRUSTEE_IS_NAME;
  DWORD permissions = 0;

  PACL pOldAcl = NULL;
  PACL pNewAcl = NULL;
  EXPLICIT_ACCESS access;

  DWORD ret = 0;

  if (popstring(param))
    ABORT("Trustee is missing");

  if (NULL == (trusteeName = ParseTrustee(param, &trusteeForm)))
    ABORT_s("Bad trustee (%s)", param);

  if (popstring(param))
    ABORT("Permission flags are missing");

  if (0 == (permissions = ParsePermissions(scheme, param)))
    ABORT_s("Bad permission flags (%s)", param);

  ret = GetNamedSecurityInfo(path, scheme->type,
          DACL_SECURITY_INFORMATION,
          NULL, NULL, &pOldAcl, NULL, NULL);
  if (ret != ERROR_SUCCESS)
    ABORT_d("Cannot read access control list. Error code: %d", ret);

  BuildExplicitAccessWithName(&access, _T(""), permissions, (ACCESS_MODE)mode, 
    scheme->defaultInheritance);

  access.Trustee.TrusteeForm = (TRUSTEE_FORM)trusteeForm;
  access.Trustee.ptstrName = trusteeName;
  if (noinherit)
    access.grfInheritance = NO_INHERITANCE;

  ret = SetEntriesInAcl(1, &access, pOldAcl, &pNewAcl);
  if (ret != ERROR_SUCCESS)
    ABORT_d("Cannot build new access control list. Error code: %d", ret);

  ret = SetNamedSecurityInfo(path, scheme->type,
          DACL_SECURITY_INFORMATION,
          NULL, NULL, pNewAcl, NULL);
  if (ret != ERROR_SUCCESS)
    ABORT_d("Cannot apply new access control list. Error code: %d", ret);

cleanup:
  if (NULL != pNewAcl)
    LocalFree(pNewAcl);
  if (NULL != pOldAcl)
    LocalFree(pOldAcl);

  LocalFree(trusteeName);
  LocalFree(param);
}

static void ChangeInheritance(const SchemeType * scheme, TCHAR * path, BOOL inherit)
{
  PACL pOldAcl = NULL;
  DWORD ret = 0;

  ret = GetNamedSecurityInfo(path, scheme->type,
          DACL_SECURITY_INFORMATION,
          NULL, NULL, &pOldAcl, NULL, NULL);
  if (ret != ERROR_SUCCESS)
    ABORT_d("Cannot read access control list. Error code: %d", ret);

  ret = SetNamedSecurityInfo(path, scheme->type,
          DACL_SECURITY_INFORMATION | (inherit ? UNPROTECTED_DACL_SECURITY_INFORMATION : PROTECTED_DACL_SECURITY_INFORMATION),
          NULL, NULL, pOldAcl, NULL);
  if (ret != ERROR_SUCCESS)
    ABORT_d("Cannot change access control list inheritance. Error code: %d", ret);

cleanup:
  LocalFree(pOldAcl);
}

BOOL SetPrivilege(HANDLE hToken, LPCTSTR lpszPrivilege, BOOL bEnablePrivilege) 
{
  TOKEN_PRIVILEGES tp;
  LUID luid;

  if (!LookupPrivilegeValue(NULL, lpszPrivilege, &luid))
    return FALSE;

  tp.PrivilegeCount = 1;
  tp.Privileges[0].Luid = luid;
  tp.Privileges[0].Attributes = (bEnablePrivilege ? SE_PRIVILEGE_ENABLED : 0);

  if (!AdjustTokenPrivileges(
         hToken,
         FALSE,
         &tp,
         sizeof(TOKEN_PRIVILEGES),
         (PTOKEN_PRIVILEGES)NULL,
         (PDWORD)NULL))
    return FALSE;

  if (GetLastError() == ERROR_NOT_ALL_ASSIGNED)
    return FALSE;

  return TRUE;
}

static void ChangeOwner(const SchemeType * scheme, TCHAR * path, ChangeMode mode)
{
  TCHAR * param = (TCHAR *)LocalAlloc(LPTR, g_string_size*sizeof(TCHAR));
  SECURITY_INFORMATION what;
  PSID pSidOwner = NULL;
  PSID pSidGroup = NULL;
  PSID pSid = NULL;

  DWORD ret = 0;

  HANDLE hToken;

  if (popstring(param))
    ABORT("Trustee is missing");

  if (NULL == (pSid = ParseSid(param)))
    ABORT_s("Bad trustee (%s)", param);

  switch(mode)
  {
  case ChangeMode_Owner:
    what = OWNER_SECURITY_INFORMATION;
    pSidOwner = pSid;
    break;

  case ChangeMode_Group:
    what = GROUP_SECURITY_INFORMATION;
    pSidGroup = pSid;
    break;

  default:
    ABORT_d("Bug: Unsupported change mode: %d", mode);
  }

  if (!OpenProcessToken(GetCurrentProcess(),
    TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
    ABORT_d("Cannot open process token. Error code: %d", GetLastError());

  if (!SetPrivilege(hToken, SE_RESTORE_NAME, TRUE))
    ABORT("Unable to give SE_RESTORE_NAME privilege.");
  ret = SetNamedSecurityInfo(path, scheme->type, 
          what, pSidOwner, pSidGroup, NULL, NULL);
  if (ret != ERROR_SUCCESS)
    ABORT_d("Cannot apply new ownership. Error code: %d", ret);

cleanup:
  SetPrivilege(hToken, SE_RESTORE_NAME, FALSE);
  CloseHandle(hToken);
  LocalFree(param);
}

static void GetOwner(const SchemeType * scheme, TCHAR * path, ChangeMode mode, TCHAR * owner)
{
  SECURITY_INFORMATION what;
  PSID pSidOwner = NULL;
  PSID pSidGroup = NULL;
  PSID pSid = NULL;
  SID_NAME_USE eUse = SidTypeUnknown;
  DWORD dwOwner = g_string_size;
  TCHAR * domain = (TCHAR *)LocalAlloc(LPTR, g_string_size*sizeof(TCHAR));
  DWORD dwDomain = g_string_size;

  DWORD ret = 0;

  switch(mode)
  {
  case ChangeMode_Owner:
    what = OWNER_SECURITY_INFORMATION;
    break;

  case ChangeMode_Group:
    what = GROUP_SECURITY_INFORMATION;
    break;

  default:
    ABORT_d("Bug: Unsupported change mode: %d", mode);
  }

  ret = GetNamedSecurityInfo(path, scheme->type, 
          what, &pSidOwner, &pSidGroup, NULL, NULL, NULL);
  if (ret != ERROR_SUCCESS)
    ABORT_d("Cannot get current ownership. Error code: %d", ret);

  if (!LookupAccountSid(NULL, (pSidOwner ? pSidOwner : pSidGroup),
          owner, &dwOwner, domain, &dwDomain, &eUse))
    ABORT_d("Cannot look up owner. Error code: %d", GetLastError());

cleanup:
  LocalFree(domain);
}

static void ClearACL(const SchemeType * scheme, TCHAR * path, BOOL noinherit)
{
  TCHAR * param = (TCHAR *)LocalAlloc(LPTR, g_string_size*sizeof(TCHAR));
  TCHAR * trusteeName = NULL;
  PSID pSid = NULL;
  DWORD trusteeForm = TRUSTEE_IS_NAME;
  DWORD permissions = 0;

  PACL pNewAcl = NULL;
  EXPLICIT_ACCESS access;

  DWORD ret = 0;

  if (popstring(param))
    ABORT("Trustee is missing");

  if (NULL == (trusteeName = ParseTrustee(param, &trusteeForm)))
    ABORT_s("Bad trustee (%s)", param);

  if (popstring(param))
    ABORT("Permission flags are missing");

  if (0 == (permissions = ParsePermissions(scheme, param)))
    ABORT_s("Bad permission flags (%s)", param);

  BuildExplicitAccessWithName(&access, _T(""), permissions, SET_ACCESS, 
    scheme->defaultInheritance);

  access.Trustee.TrusteeForm = (TRUSTEE_FORM)trusteeForm;
  access.Trustee.ptstrName = trusteeName;
  if (noinherit)
    access.grfInheritance = NO_INHERITANCE;

  ret = SetEntriesInAcl(1, &access, NULL, &pNewAcl);
  if (ret != ERROR_SUCCESS)
    ABORT_d("Cannot build new access control list. Error code: %d", ret);

  ret = SetNamedSecurityInfo(path, scheme->type,
          DACL_SECURITY_INFORMATION,
          NULL, NULL, pNewAcl, NULL);
  if (ret != ERROR_SUCCESS)
    ABORT_d("Cannot change access control list inheritance. Error code: %d", ret);

cleanup:
  if (NULL != pNewAcl)
    LocalFree(pNewAcl);
  LocalFree(trusteeName);
  LocalFree(param);
}

/*****************************************************************************
 FILESYSTEM BASED ACL HANDLING
 *****************************************************************************/

static const SchemeType * PopFileArgs(TCHAR * path, BOOL *options)
{
  if (popstring(path) == 0)
  {
    DWORD attr;
    *options = FALSE;
    if (lstrcmpi(path, _T("/noinherit")) == 0)
    {
      *options = TRUE;
      popstring(path);
    }
    attr = GetFileAttributes(path);

    if (INVALID_FILE_ATTRIBUTES != attr)
      return FILE_ATTRIBUTE_DIRECTORY & attr
        ? g_directoryScheme
        : g_fileScheme;
    else
      ABORT("Invalid filesystem path missing");
  }
  else
    ABORT("Filesystem path missing");

cleanup:
  return NULL;
}

static void ChangeFileInheritance(BOOL inherit)
{
  TCHAR * path = (TCHAR *)LocalAlloc(GPTR, g_string_size*sizeof(TCHAR));

  if (NULL != path)
  {
    BOOL options = FALSE;
    const SchemeType * scheme;

    if (NULL != (scheme = PopFileArgs(path, &options)))
      ChangeInheritance(scheme, path, inherit);

    LocalFree(path);
  }
}

static void ChangeFileDACL(DWORD mode)
{
  TCHAR * path = (TCHAR *)LocalAlloc(GPTR, g_string_size*sizeof(TCHAR));

  if (NULL != path)
  {
    BOOL noinherit = FALSE;
    const SchemeType * scheme;

    if (NULL != (scheme = PopFileArgs(path, &noinherit)))
      ChangeDACL(scheme, path, mode, noinherit);

    LocalFree(path);
  }
}

static void ChangeFileOwner(ChangeMode mode)
{
  TCHAR * path = (TCHAR *)LocalAlloc(GPTR, g_string_size*sizeof(TCHAR));

  if (NULL != path)
  {
    BOOL noinherit = FALSE;
    const SchemeType * scheme;

    if (NULL != (scheme = PopFileArgs(path, &noinherit)))
      ChangeOwner(scheme, path, mode);

    LocalFree(path);
  }
}

static void PushFileOwner(ChangeMode mode)
{
  TCHAR * path = (TCHAR *)LocalAlloc(GPTR, g_string_size*sizeof(TCHAR));

  if (NULL != path)
  {
    BOOL noinherit = FALSE;
    const SchemeType * scheme;
    TCHAR * owner = (TCHAR*)LocalAlloc(LPTR, g_string_size*sizeof(TCHAR));

    if (NULL != owner)
    {
      if (NULL != (scheme = PopFileArgs(path, &noinherit)))
      {
        GetOwner(scheme, path, mode, owner);
        pushstring(owner);
      }
      LocalFree(owner);
    }

    LocalFree(path);
  }
}

/*****************************************************************************
 REGISTRY BASED ACL HANDLING
 *****************************************************************************/

static BOOL PopRegKeyArgs(TCHAR * path, BOOL *options)
{
  TCHAR * param = (TCHAR *)GlobalAlloc(GPTR, g_string_size*sizeof(TCHAR));
  int iRootKey;
  BOOL success = FALSE;

  if (!popstring(param))
  {
    *options = FALSE;
    if (lstrcmpi(param, _T("/noinherit")) == 0)
    {
      *options = TRUE;
      popstring(param);
    }
  }
  else
    ABORT("Root key name missing");

  iRootKey = ParseEnum(g_rootKeyNames, param);
  if (!ARRAY_CONTAINS(g_rootKeyPrefixes, iRootKey))
    ABORT_s("Bad root key name (%s)", param);

  if (popstring(param))
    ABORT("Registry key name missing");

  path[0] = 0;
  lstrcat(path, g_rootKeyPrefixes[iRootKey]);
  lstrcat(path, param);

  success = TRUE;

cleanup:
  GlobalFree(param);
  return success;
}

static void ChangeRegKeyInheritance(BOOL inherit)
{
  TCHAR * path = (TCHAR *)LocalAlloc(GPTR, g_string_size*sizeof(TCHAR));

  if (NULL != path)
  {
    BOOL noinherit = FALSE;

    if (PopRegKeyArgs(path, &noinherit))
      ChangeInheritance(g_registryScheme, path, inherit);

    LocalFree(path);
  }
}

static void ChangeRegKeyDACL(DWORD mode)
{
  TCHAR * path = (TCHAR *)LocalAlloc(GPTR, g_string_size*sizeof(TCHAR));

  if (NULL != path)
  {
    BOOL noinherit = FALSE;

    if (PopRegKeyArgs(path, &noinherit))
      ChangeDACL(g_registryScheme, path, mode, 0);

    LocalFree(path);
  }
}

static void ChangeRegKeyOwner(ChangeMode mode)
{
  TCHAR * path = (TCHAR *)LocalAlloc(GPTR, g_string_size*sizeof(TCHAR));

  if (NULL != path)
  {
    BOOL noinherit = FALSE;

    if (PopRegKeyArgs(path, &noinherit))
      ChangeOwner(g_registryScheme, path, mode);

    LocalFree(path);
  }
}

static void PushRegKeyOwner(ChangeMode mode)
{
  BOOL noinherit = FALSE;
  TCHAR * path = (TCHAR *)LocalAlloc(GPTR, g_string_size*sizeof(TCHAR));

  if (NULL != path)
  {
    TCHAR * owner = (TCHAR*)LocalAlloc(LPTR, g_string_size*sizeof(TCHAR));

    if (NULL != owner)
    {

      if (PopRegKeyArgs(path, &noinherit))
      {
        GetOwner(g_registryScheme, path, mode, owner);
        pushstring(owner);
      }
      else
        pushstring(_T(""));

      LocalFree(owner);
    }
    LocalFree(path);
  }
}

/*****************************************************************************
 PUBLIC FILE RELATED FUNCTIONS
 *****************************************************************************/

PUBLIC_FUNCTION(EnableFileInheritance)
  ChangeFileInheritance(TRUE);
PUBLIC_FUNCTION_END

PUBLIC_FUNCTION(DisableFileInheritance)
  ChangeFileInheritance(FALSE);
PUBLIC_FUNCTION_END

PUBLIC_FUNCTION(GrantOnFile)
  ChangeFileDACL(GRANT_ACCESS);
PUBLIC_FUNCTION_END

PUBLIC_FUNCTION(SetOnFile)
  ChangeFileDACL(SET_ACCESS);
PUBLIC_FUNCTION_END

PUBLIC_FUNCTION(DenyOnFile)
  ChangeFileDACL(DENY_ACCESS);
PUBLIC_FUNCTION_END

PUBLIC_FUNCTION(RevokeOnFile)
  ChangeFileDACL(REVOKE_ACCESS);
PUBLIC_FUNCTION_END

PUBLIC_FUNCTION(SetFileOwner)
  ChangeFileOwner(ChangeMode_Owner);
PUBLIC_FUNCTION_END

PUBLIC_FUNCTION(SetFileGroup)
  ChangeFileOwner(ChangeMode_Group);
PUBLIC_FUNCTION_END

PUBLIC_FUNCTION(GetFileOwner)
  PushFileOwner(ChangeMode_Owner);
PUBLIC_FUNCTION_END

PUBLIC_FUNCTION(GetFileGroup)
  PushFileOwner(ChangeMode_Group);
PUBLIC_FUNCTION_END

PUBLIC_FUNCTION(ClearOnFile)
{
  TCHAR * path = (TCHAR *)LocalAlloc(GPTR, string_size*sizeof(TCHAR));

  if (NULL != path)
  {
    BOOL noinherit = FALSE;
    const SchemeType * scheme;

    if (NULL != (scheme = PopFileArgs(path, &noinherit)))
      ClearACL(scheme, path, noinherit);

    LocalFree(path);
  }
}
PUBLIC_FUNCTION_END

/*****************************************************************************
 PUBLIC REGISTRY RELATED FUNCTIONS
 *****************************************************************************/

PUBLIC_FUNCTION(EnableRegKeyInheritance)
  ChangeRegKeyInheritance(TRUE);
PUBLIC_FUNCTION_END

PUBLIC_FUNCTION(DisableRegKeyInheritance)
  ChangeRegKeyInheritance(FALSE);
PUBLIC_FUNCTION_END

PUBLIC_FUNCTION(GrantOnRegKey)
  ChangeRegKeyDACL(GRANT_ACCESS);
PUBLIC_FUNCTION_END

PUBLIC_FUNCTION(SetOnRegKey)
  ChangeRegKeyDACL(SET_ACCESS);
PUBLIC_FUNCTION_END

PUBLIC_FUNCTION(DenyOnRegKey)
  ChangeRegKeyDACL(DENY_ACCESS);
PUBLIC_FUNCTION_END

PUBLIC_FUNCTION(RevokeOnRegKey)
  ChangeRegKeyDACL(REVOKE_ACCESS);
PUBLIC_FUNCTION_END

PUBLIC_FUNCTION(SetRegKeyOwner)
  ChangeRegKeyOwner(ChangeMode_Owner);
PUBLIC_FUNCTION_END

PUBLIC_FUNCTION(SetRegKeyGroup)
  ChangeRegKeyOwner(ChangeMode_Group);
PUBLIC_FUNCTION_END

PUBLIC_FUNCTION(GetRegKeyOwner)
  PushRegKeyOwner(ChangeMode_Owner);
PUBLIC_FUNCTION_END

PUBLIC_FUNCTION(GetRegKeyGroup)
  PushRegKeyOwner(ChangeMode_Group);
PUBLIC_FUNCTION_END

PUBLIC_FUNCTION(ClearOnRegKey)
{
  TCHAR * path = (TCHAR *)LocalAlloc(GPTR, string_size*sizeof(TCHAR));

  if (NULL != path)
  {
    BOOL noinherit = FALSE;

    if (PopRegKeyArgs(path, &noinherit))
      ClearACL(g_registryScheme, path, noinherit);

    LocalFree(path);
  }
}
PUBLIC_FUNCTION_END

/*****************************************************************************
 OTHER ACCOUNT RELATED FUNCTIONS
 *****************************************************************************/

PUBLIC_FUNCTION(SidToName)
{
  TCHAR * param = (TCHAR *)LocalAlloc(LPTR, string_size*sizeof(TCHAR));
  PSID pSid;
  SID_NAME_USE eUse;
  TCHAR * name = (TCHAR *)LocalAlloc(LPTR, string_size*sizeof(TCHAR));
  DWORD dwName = string_size;
  TCHAR * domain = (TCHAR *)LocalAlloc(LPTR, string_size*sizeof(TCHAR));
  DWORD dwDomain = string_size;

  popstring(param);
  //pSid = GetBinarySid(param);
  if (!ConvertStringSidToSid(param, &pSid))
    pSid = NULL;

  if (NULL != pSid &&
      !LookupAccountSid(NULL, pSid,
        name, &dwName,
        domain, &dwDomain,
        &eUse))
	  ABORT_d("Cannot look up owner. Error code: %d", GetLastError());

  pushstring(name);
  pushstring(domain);

cleanup:
  LocalFree(domain);
  LocalFree(name);
  LocalFree(param);
}
PUBLIC_FUNCTION_END

PUBLIC_FUNCTION(GetCurrentUserName)
{
  TCHAR * name = (TCHAR *)LocalAlloc(LPTR, string_size*sizeof(TCHAR));
  DWORD dwName = string_size;
  GetUserName(name, &dwName);
  pushstring(name);
  LocalFree(name);
}
PUBLIC_FUNCTION_END

PUBLIC_FUNCTION(IsUserTheAdministrator)
{
  TCHAR * name = (TCHAR *)LocalAlloc(LPTR, string_size*sizeof(TCHAR));
  TCHAR * sidstr = NULL;
  DWORD dwName = string_size;
  PSID pSid = NULL;
  DWORD sidLen = 0;
  DWORD domLen = 0;
  TCHAR * domain = NULL;
  SID_NAME_USE use;

  if (popstring(name))
    ABORT("Missing user name plug-in parameter.");

  if ((LookupAccountName(NULL, name, 
      NULL, &sidLen, NULL, &domLen, &use) ||
      ERROR_INSUFFICIENT_BUFFER == GetLastError()) &&
      NULL != (domain = (TCHAR *)LocalAlloc(LPTR, domLen*sizeof(TCHAR))) &&
    NULL != (pSid = (PSID)LocalAlloc(LPTR, sidLen)))
  {
    if (!LookupAccountName(NULL, name, 
      pSid, &sidLen, domain, &domLen, &use))
    {
      LocalFree(pSid);
      pSid = NULL;
      ABORT_d("Couldn't lookup current user name. Error code %d: ", GetLastError());
    }

    int uid;
    if (500 == (uid = *GetSidSubAuthority(pSid, *GetSidSubAuthorityCount(pSid) - 1)))
      pushstring(_T("yes"));
    else
      pushstring(_T("no"));

    sidstr = (TCHAR *)LocalAlloc(LPTR, string_size*sizeof(TCHAR));
    ConvertSidToStringSid(pSid, &sidstr);

    int len = lstrlen(sidstr);
    TCHAR * strend = sidstr + len - 1;
    TCHAR * strstart = sidstr;
    while (*strend != '-' && len >= 0)
    {
      strend--;
      len--;
    }
    *strend = '\0';
    lstrcat(strend, _T("-500"));

    pushstring(sidstr);
  }

cleanup:
  if (NULL != sidstr)
    LocalFree(sidstr);
  LocalFree(name);
}
PUBLIC_FUNCTION_END