/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file phidsdi.h
 * @author rdb
 * @date 2019-02-05
 */

#ifndef PHIDSDI_H
#define PHIDSDI_H

#if defined(_WIN32) && !defined(CPPPARSER)

// Copy definitions from hidusage.h, until we can drop support for the 7.1 SDK
typedef USHORT USAGE, *PUSAGE;

#define HID_USAGE_PAGE_UNDEFINED       ((USAGE) 0x00)
#define HID_USAGE_PAGE_GENERIC         ((USAGE) 0x01)
#define HID_USAGE_PAGE_SIMULATION      ((USAGE) 0x02)
#define HID_USAGE_PAGE_VR              ((USAGE) 0x03)
#define HID_USAGE_PAGE_SPORT           ((USAGE) 0x04)
#define HID_USAGE_PAGE_GAME            ((USAGE) 0x05)
#define HID_USAGE_PAGE_KEYBOARD        ((USAGE) 0x07)
#define HID_USAGE_PAGE_LED             ((USAGE) 0x08)
#define HID_USAGE_PAGE_BUTTON          ((USAGE) 0x09)

#define HID_USAGE_GENERIC_POINTER      ((USAGE) 0x01)
#define HID_USAGE_GENERIC_MOUSE        ((USAGE) 0x02)
#define HID_USAGE_GENERIC_JOYSTICK     ((USAGE) 0x04)
#define HID_USAGE_GENERIC_GAMEPAD      ((USAGE) 0x05)
#define HID_USAGE_GENERIC_KEYBOARD     ((USAGE) 0x06)
#define HID_USAGE_GENERIC_KEYPAD       ((USAGE) 0x07)
#define HID_USAGE_GENERIC_SYSTEM_CTL   ((USAGE) 0x80)

#define HID_USAGE_GENERIC_X            ((USAGE) 0x30)
#define HID_USAGE_GENERIC_Y            ((USAGE) 0x31)
#define HID_USAGE_GENERIC_Z            ((USAGE) 0x32)
#define HID_USAGE_GENERIC_RX           ((USAGE) 0x33)
#define HID_USAGE_GENERIC_RY           ((USAGE) 0x34)
#define HID_USAGE_GENERIC_RZ           ((USAGE) 0x35)
#define HID_USAGE_GENERIC_SLIDER       ((USAGE) 0x36)
#define HID_USAGE_GENERIC_DIAL         ((USAGE) 0x37)
#define HID_USAGE_GENERIC_WHEEL        ((USAGE) 0x38)
#define HID_USAGE_GENERIC_HATSWITCH    ((USAGE) 0x39)

// Copy definitions from hidpi.h, until we can drop support for the 7.1 SDK
#define HIDP_STATUS_SUCCESS ((NTSTATUS)(0x11 << 16))

typedef enum _HIDP_REPORT_TYPE {
  HidP_Input,
  HidP_Output,
  HidP_Feature
} HIDP_REPORT_TYPE;

typedef struct _HIDP_BUTTON_CAPS {
  USAGE UsagePage;
  UCHAR ReportID;
  BOOLEAN IsAlias;
  USHORT BitField;
  USHORT LinkCollection;
  USAGE LinkUsage;
  USAGE LinkUsagePage;
  BOOLEAN IsRange;
  BOOLEAN IsStringRange;
  BOOLEAN IsDesignatorRange;
  BOOLEAN IsAbsolute;
  ULONG Reserved[10];
  union {
    struct {
      USAGE UsageMin, UsageMax;
      USHORT StringMin, StringMax;
      USHORT DesignatorMin, DesignatorMax;
      USHORT DataIndexMin, DataIndexMax;
    } Range;
    struct  {
      USAGE Usage, Reserved1;
      USHORT StringIndex, Reserved2;
      USHORT DesignatorIndex, Reserved3;
      USHORT DataIndex, Reserved4;
    } NotRange;
  };
} HIDP_BUTTON_CAPS, *PHIDP_BUTTON_CAPS;

typedef struct _HIDP_VALUE_CAPS {
  USAGE UsagePage;
  UCHAR ReportID;
  BOOLEAN IsAlias;
  USHORT BitField;
  USHORT LinkCollection;
  USAGE LinkUsage;
  USAGE LinkUsagePage;
  BOOLEAN IsRange;
  BOOLEAN IsStringRange;
  BOOLEAN IsDesignatorRange;
  BOOLEAN IsAbsolute;
  BOOLEAN HasNull;
  UCHAR Reserved;
  USHORT BitSize;
  USHORT ReportCount;
  USHORT Reserved2[5];
  ULONG UnitsExp;
  ULONG Units;
  LONG LogicalMin, LogicalMax;
  LONG PhysicalMin, PhysicalMax;
  union {
    struct {
      USAGE UsageMin, UsageMax;
      USHORT StringMin, StringMax;
      USHORT DesignatorMin, DesignatorMax;
      USHORT DataIndexMin, DataIndexMax;
    } Range;
    struct {
      USAGE Usage, Reserved1;
      USHORT StringIndex, Reserved2;
      USHORT DesignatorIndex, Reserved3;
      USHORT DataIndex, Reserved4;
    } NotRange;
  };
} HIDP_VALUE_CAPS, *PHIDP_VALUE_CAPS;

typedef PUCHAR PHIDP_REPORT_DESCRIPTOR;
typedef struct _HIDP_PREPARSED_DATA *PHIDP_PREPARSED_DATA;

typedef struct _HIDP_CAPS {
  USAGE Usage;
  USAGE UsagePage;
  USHORT InputReportByteLength;
  USHORT OutputReportByteLength;
  USHORT FeatureReportByteLength;
  USHORT Reserved[17];
  USHORT NumberLinkCollectionNodes;
  USHORT NumberInputButtonCaps;
  USHORT NumberInputValueCaps;
  USHORT NumberInputDataIndices;
  USHORT NumberOutputButtonCaps;
  USHORT NumberOutputValueCaps;
  USHORT NumberOutputDataIndices;
  USHORT NumberFeatureButtonCaps;
  USHORT NumberFeatureValueCaps;
  USHORT NumberFeatureDataIndices;
} HIDP_CAPS, *PHIDP_CAPS;

typedef struct _HIDP_DATA {
  USHORT DataIndex;
  USHORT Reserved;
  union {
    ULONG RawValue;
    BOOLEAN On;
  };
} HIDP_DATA, *PHIDP_DATA;

typedef LONG NTSTATUS;
typedef NTSTATUS (__stdcall *pHidP_GetCaps)(PHIDP_PREPARSED_DATA, PHIDP_CAPS);
typedef NTSTATUS (__stdcall *pHidP_GetButtonCaps)(HIDP_REPORT_TYPE, PHIDP_BUTTON_CAPS, PUSHORT, PHIDP_PREPARSED_DATA);
typedef NTSTATUS (__stdcall *pHidP_GetValueCaps)(HIDP_REPORT_TYPE, PHIDP_VALUE_CAPS, PUSHORT, PHIDP_PREPARSED_DATA);
typedef NTSTATUS (__stdcall *pHidP_GetData)(HIDP_REPORT_TYPE, PHIDP_DATA, PULONG, PHIDP_PREPARSED_DATA, PCHAR, ULONG);
typedef ULONG (__stdcall *pHidP_MaxDataListLength)(HIDP_REPORT_TYPE, PHIDP_PREPARSED_DATA);

#endif

#endif
