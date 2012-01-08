""" This module implements a Panda3D window that can be embedded
within a wx.Frame.  The window itself is either an embedded window,
which is a wx.Window with the Panda window attached to it, or it is a
wxgl.GLCanvas, with Panda directed to draw into it, depending on the
platform.  In either case, you may simply embed this window into a
wx.Frame of your choosing, using sizers or whatever you like. """

import wx
import platform

try:
    import wx.glcanvas as wxgl
except ImportError:
    wxgl = None

from panda3d.core import *

__all__ = ['WxPandaWindow']

if platform.system() != 'Darwin':
    class EmbeddedPandaWindow(wx.Window):
        """ This class implements a Panda3D window that is directly
        embedded within the frame.  It is fully supported on Windows,
        partially supported on Linux, and not at all on OSX. """

        def __init__(self, *args, **kw):
            wx.Window.__init__(self, *args, **kw)

            wp = WindowProperties.getDefault()
            wp.setParentWindow(self.GetHandle())

            if base.win:
                self.win = base.openWindow(props = wp)
            else:
                base.openDefaultWindow(props = wp)
                self.win = base.win

            self.Bind(wx.EVT_SIZE, self.__resized)

        def __resized(self, event):
            wp = WindowProperties()
            wp.setOrigin(0, 0)
            wp.setSize(*self.GetClientSizeTuple())
            self.win.requestProperties(wp)

if hasattr(wxgl, 'GLCanvas'):
    class OpenGLPandaWindow(wxgl.GLCanvas):
        """ This class implements a Panda3D "window" that actually draws
        within the wx GLCanvas object.  It is supported whenever OpenGL is
        Panda's rendering engine, and GLCanvas is available in wx. """

        Keymap = {
            wx.WXK_BACK : KeyboardButton.backspace(),
            wx.WXK_TAB : KeyboardButton.tab(),
            wx.WXK_RETURN : KeyboardButton.enter(),
            wx.WXK_ESCAPE : KeyboardButton.escape(),
            wx.WXK_DELETE : KeyboardButton._del(),  # del is a Python keyword
            wx.WXK_SHIFT : KeyboardButton.shift(),
            wx.WXK_ALT : KeyboardButton.alt(),
            wx.WXK_CONTROL : KeyboardButton.control(),
            wx.WXK_MENU : KeyboardButton.meta(),
            wx.WXK_PAUSE : KeyboardButton.pause(),
            wx.WXK_END : KeyboardButton.end(),
            wx.WXK_HOME : KeyboardButton.home(),
            wx.WXK_LEFT : KeyboardButton.left(),
            wx.WXK_UP : KeyboardButton.up(),
            wx.WXK_RIGHT : KeyboardButton.right(),
            wx.WXK_DOWN : KeyboardButton.down(),
            wx.WXK_PRINT : KeyboardButton.printScreen(),
            wx.WXK_INSERT : KeyboardButton.insert(),
            wx.WXK_F1 : KeyboardButton.f1(),
            wx.WXK_F2 : KeyboardButton.f2(),
            wx.WXK_F3 : KeyboardButton.f3(),
            wx.WXK_F4 : KeyboardButton.f4(),
            wx.WXK_F5 : KeyboardButton.f5(),
            wx.WXK_F6 : KeyboardButton.f6(),
            wx.WXK_F7 : KeyboardButton.f7(),
            wx.WXK_F8 : KeyboardButton.f8(),
            wx.WXK_F9 : KeyboardButton.f9(),
            wx.WXK_F10 : KeyboardButton.f10(),
            wx.WXK_F11 : KeyboardButton.f11(),
            wx.WXK_F12 : KeyboardButton.f12(),
            wx.WXK_NUMLOCK : KeyboardButton.numLock(),
            wx.WXK_SCROLL : KeyboardButton.scrollLock(),
            wx.WXK_PAGEUP : KeyboardButton.pageUp(),
            wx.WXK_PAGEDOWN : KeyboardButton.pageDown(),
            wx.WXK_COMMAND : KeyboardButton.meta(),
            }

        def __init__(self, *args, **kw):
            wxgl.GLCanvas.__init__(self, *args, **kw)

            callbackWindowDict = {
                'Events' : self.__eventsCallback,
                'Properties' : self.__propertiesCallback,
                'Render' : self.__renderCallback,
                }

            # Make sure we have an OpenGL GraphicsPipe.
            if not base.pipe:
                base.makeDefaultPipe()
            pipe = base.pipe
            if pipe.getInterfaceName() != 'OpenGL':
                base.makeAllPipes()
                for pipe in base.pipeList:
                    if pipe.getInterfaceName() == 'OpenGL':
                        break

            if pipe.getInterfaceName() != 'OpenGL':
                raise StandardError, "Couldn't get an OpenGL pipe."

            self.SetCurrent()
            if base.win:
                self.win = base.openWindow(callbackWindowDict = callbackWindowDict, pipe = pipe)
            else:
                base.openDefaultWindow(callbackWindowDict = callbackWindowDict, pipe = pipe)
                self.win = base.win

            self.inputDevice = self.win.getInputDevice(0)

            self.Bind(wx.EVT_SIZE, self.__resized)
            self.Bind(wx.EVT_LEFT_DOWN, lambda event: self.__buttonDown(MouseButton.one()))
            self.Bind(wx.EVT_LEFT_UP, lambda event: self.__buttonUp(MouseButton.one()))
            self.Bind(wx.EVT_MIDDLE_DOWN, lambda event: self.__buttonDown(MouseButton.two()))
            self.Bind(wx.EVT_MIDDLE_UP, lambda event: self.__buttonUp(MouseButton.two()))
            self.Bind(wx.EVT_RIGHT_DOWN, lambda event: self.__buttonDown(MouseButton.three()))
            self.Bind(wx.EVT_RIGHT_UP, lambda event: self.__buttonUp(MouseButton.three()))
            self.Bind(wx.EVT_MOTION, self.__mouseMotion)
            self.Bind(wx.EVT_LEAVE_WINDOW, self.__mouseLeaveWindow)
            self.Bind(wx.EVT_KEY_DOWN, self.__keyDown)
            self.Bind(wx.EVT_KEY_UP, self.__keyUp)
            self.Bind(wx.EVT_CHAR, self.__keystroke)

        def __buttonDown(self, button):
            self.inputDevice.buttonDown(button)

        def __buttonUp(self, button):
            self.inputDevice.buttonUp(button)

        def __mouseMotion(self, event):
            self.inputDevice.setPointerInWindow(*event.GetPosition())

        def __mouseLeaveWindow(self, event):
            self.inputDevice.setPointerOutOfWindow()

        def __keyDown(self, event):
            key = self.__getkey(event)
            if key:
                self.inputDevice.buttonDown(key)

        def __keyUp(self, event):
            key = self.__getkey(event)
            if key:
                self.inputDevice.buttonUp(key)

        def __getkey(self, event):
            code = event.GetKeyCode()
            key = self.Keymap.get(code, None)
            if key is not None:
                return key

            if code >= 0x41 and code <= 0x5a:
                # wxWidgets returns uppercase letters, but Panda expects
                # lowercase letters.
                return KeyboardButton.asciiKey(code + 0x20)
            if code >= 0x20 and code <= 0x80:
                # Other ASCII keys are treated the same way in wx and Panda.
                return KeyboardButton.asciiKey(code)

            return None

        def __keystroke(self, event):
            if hasattr(event, 'GetUnicodeKey'):
                code = event.GetUnicodeKey()
            else:
                code = event.GetKeyCode()
                if code < 0x20 or code >= 0x80:
                    return

            self.inputDevice.keystroke(code)

        def __eventsCallback(self, data):
            data.upcall()

        def __propertiesCallback(self, data):
            data.upcall()

        def __renderCallback(self, data):
            cbType = data.getCallbackType()
            if cbType == CallbackGraphicsWindow.RCTBeginFrame:
                self.SetCurrent()
            elif cbType == CallbackGraphicsWindow.RCTEndFlip:
                self.SwapBuffers()

            data.upcall()

        def __resized(self, event):
            wp = WindowProperties()
            wp.setSize(*self.GetClientSizeTuple())
            self.win.requestProperties(wp)

# Choose the best implementation of WxPandaWindow for the platform.
if platform.system() == 'Darwin':
    WxPandaWindow = OpenGLPandaWindow
else:
    WxPandaWindow = EmbeddedPandaWindow
