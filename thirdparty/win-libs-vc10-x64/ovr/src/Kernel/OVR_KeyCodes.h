/************************************************************************************

PublicHeader:   OVR.h
Filename    :   OVR_KeyCodes.h
Content     :   Common keyboard constants
Created     :   September 19, 2012

Copyright   :   Copyright 2012 Oculus VR, Inc. All Rights reserved.

Use of this software is subject to the terms of the Oculus license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

************************************************************************************/

#ifndef OVR_KeyCodes_h
#define OVR_KeyCodes_h

namespace OVR {

//-----------------------------------------------------------------------------------
// ***** KeyCode

// KeyCode enumeration defines platform-independent keyboard key constants.
// Note that Key_A through Key_Z are mapped to capital ascii constants.

enum KeyCode
{
    // Key_None indicates that no key was specified.
    Key_None            = 0, 

    // A through Z and numbers 0 through 9.
    Key_A               = 65,
    Key_B,
    Key_C,
    Key_D,
    Key_E,
    Key_F,
    Key_G,
    Key_H,
    Key_I,
    Key_J,
    Key_K,
    Key_L,
    Key_M,
    Key_N,
    Key_O,
    Key_P,
    Key_Q,
    Key_R,
    Key_S,
    Key_T,
    Key_U,
    Key_V,
    Key_W,
    Key_X,
    Key_Y,
    Key_Z,
    Key_Num0            = 48,
    Key_Num1,
    Key_Num2,
    Key_Num3,
    Key_Num4,
    Key_Num5,
    Key_Num6,
    Key_Num7,
    Key_Num8,
    Key_Num9,

    // Numeric keypad.
    Key_KP_0            = 0xa0,
    Key_KP_1,
    Key_KP_2,
    Key_KP_3,
    Key_KP_4,
    Key_KP_5,
    Key_KP_6,
    Key_KP_7,
    Key_KP_8,
    Key_KP_9,
    Key_KP_Multiply,
    Key_KP_Add,
    Key_KP_Enter,
    Key_KP_Subtract,
    Key_KP_Decimal,
    Key_KP_Divide,
    
    // Function keys.
    Key_F1              = 0xb0,
    Key_F2,
    Key_F3,
    Key_F4,
    Key_F5,
    Key_F6,
    Key_F7,
    Key_F8,
    Key_F9,
    Key_F10,
    Key_F11,
    Key_F12,
    Key_F13,
    Key_F14,
    Key_F15,
    
    // Other keys.
    Key_Backspace       = 8,
    Key_Tab,
    Key_Clear           = 12,
    Key_Return,
    Key_Shift           = 16,
    Key_Control,
    Key_Alt,
    Key_Pause,
    Key_CapsLock        = 20, // Toggle
    Key_Escape          = 27,
    Key_Space           = 32,
    Key_Quote           = 39,
    Key_PageUp          = 0xc0,
    Key_PageDown,
    Key_End,
    Key_Home,
    Key_Left,
    Key_Up,
    Key_Right,
    Key_Down,
    Key_Insert,
    Key_Delete,
    Key_Help,
    
    Key_Comma           = 44,
    Key_Minus,
    Key_Slash           = 47,
    Key_Period,
    Key_NumLock         = 144, // Toggle
    Key_ScrollLock      = 145, // Toggle
    
    Key_Semicolon       = 59,
    Key_Equal           = 61,
    Key_Bar             = 192,
    Key_BracketLeft     = 91,
    Key_Backslash,
    Key_BracketRight,

    Key_OEM_AX          = 0xE1,  //  'AX' key on Japanese AX keyboard
    Key_OEM_102         = 0xE2,  //  "<>" or "\|" on RT 102-key keyboard.
    Key_ICO_HELP        = 0xE3,  //  Help key on ICO
    Key_ICO_00          = 0xE4,  //  00 key on ICO

    Key_Meta,

    // Total number of keys.
    Key_CodeCount
};


//-----------------------------------------------------------------------------------

class KeyModifiers 
{
public:
    enum
    {
        Key_ShiftPressed    = 0x01,
        Key_CtrlPressed     = 0x02,
        Key_AltPressed      = 0x04,
        Key_MetaPressed     = 0x08,
        Key_CapsToggled     = 0x10,
        Key_NumToggled      = 0x20,
        Key_ScrollToggled   = 0x40,

        Initialized_Bit     = 0x80,
        Initialized_Mask    = 0xFF
    };
    unsigned char States;

    KeyModifiers() : States(0) { }
        KeyModifiers(unsigned char st) : States((unsigned char)(st | Initialized_Bit)) { }

    void Reset() { States = 0; }

    bool IsShiftPressed() const { return (States & Key_ShiftPressed) != 0; }
    bool IsCtrlPressed() const  { return (States & Key_CtrlPressed) != 0; }
    bool IsAltPressed() const   { return (States & Key_AltPressed) != 0; }
    bool IsMetaPressed() const  { return (States & Key_MetaPressed) != 0; }
    bool IsCapsToggled() const  { return (States & Key_CapsToggled) != 0; }
    bool IsNumToggled() const   { return (States & Key_NumToggled) != 0; }
    bool IsScrollToggled() const{ return (States & Key_ScrollToggled) != 0; }

    void SetShiftPressed(bool v = true)  { (v) ? States |= Key_ShiftPressed : States &= ~Key_ShiftPressed; }
    void SetCtrlPressed(bool v = true)   { (v) ? States |= Key_CtrlPressed  : States &= ~Key_CtrlPressed; }
    void SetAltPressed(bool v = true)    { (v) ? States |= Key_AltPressed   : States &= ~Key_AltPressed; }
    void SetMetaPressed(bool v = true)   { (v) ? States |= Key_MetaPressed  : States &= ~Key_MetaPressed; }
    void SetCapsToggled(bool v = true)   { (v) ? States |= Key_CapsToggled  : States &= ~Key_CapsToggled; }
    void SetNumToggled(bool v = true)    { (v) ? States |= Key_NumToggled   : States &= ~Key_NumToggled; }
    void SetScrollToggled(bool v = true) { (v) ? States |= Key_ScrollToggled: States &= ~Key_ScrollToggled; }

    bool IsInitialized() const { return (States & Initialized_Mask) != 0; }
};


//-----------------------------------------------------------------------------------

/*
enum PadKeyCode
{
    Pad_None, // Indicates absence of key code.
    Pad_Back,
    Pad_Start,
    Pad_A,
    Pad_B,
    Pad_X,
    Pad_Y,
    Pad_R1,  // RightShoulder;
    Pad_L1,  // LeftShoulder;
    Pad_R2,  // RightTrigger;
    Pad_L2,  // LeftTrigger;
    Pad_Up,
    Pad_Down,
    Pad_Right,
    Pad_Left,
    Pad_Plus,
    Pad_Minus,
    Pad_1,
    Pad_2,
    Pad_H,
    Pad_C,
    Pad_Z,
    Pad_O,
    Pad_T,
    Pad_S,
    Pad_Select,
    Pad_Home,
    Pad_RT,  // RightThumb;
    Pad_LT   // LeftThumb;
};
*/

} // OVR

#endif
