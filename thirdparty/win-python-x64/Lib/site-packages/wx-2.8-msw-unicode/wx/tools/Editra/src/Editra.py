#!/usr/bin/env python
###############################################################################
# Name: Editra.py                                                             #
# Purpose: Implements Editras App object and the Main method                  #
# Author: Cody Precord <cprecord@editra.org>                                  #
# Copyright: (c) 2008 Cody Precord <staff@editra.org>                         #
# License: wxWindows License                                                  #
###############################################################################

"""
This module defines the Editra Application object and the Main method for
running Editra.

@summary: Editra's main application object and MainLoop

"""

__author__ = "Cody Precord <cprecord@editra.org>"
__svnid__ = "$Id: Editra.py 67432 2011-04-11 00:02:30Z CJP $"
__revision__ = "$Revision: 67432 $"

#--------------------------------------------------------------------------#
# Dependencies
import os
import sys

# Due to some methods that were added in 2.8.3 being used in a large number
# of places Editra has become incompatible with wxPython 2.8.1.1 and earlier.
# So ensure correct version of wxPython can be loaded
if not hasattr(sys, 'frozen') and 'wx' not in sys.modules:
    import wxversion
    wxversion.ensureMinimal('2.8')

import codecs
import base64
import locale
import getopt
import shutil
import wx

# The event handler mixin is now part of wxPython proper, but there hasn't
# been an official release with it yet, so try to import the official module
# but fallback to our own copy if it fails.
try:
    import wx.lib.eventStack as events
except:
    import extern.events as events

# Try and import a system installed version of pkg_resources else fallback to
# the one bundled with Editra's source.
try:
    from pkg_resources import resource_filename
except ImportError:
    from extern.pkg_resources import resource_filename

# Editra Libraries
import ed_glob
import ed_i18n
import profiler
import util
import dev_tool
import ed_main
import ed_art
import ed_txt
import ed_event
import updater
import plugin
import ed_ipc
import ebmlib
from syntax import synglob

#--------------------------------------------------------------------------#
# Global Variables
ID_UPDATE_CHECK = wx.NewId()

_ = wx.GetTranslation
#--------------------------------------------------------------------------#

class Editra(wx.App, events.AppEventHandlerMixin):
    """The Editra Application Object
    @deprecated: L{GetMainWindow}

    """
    def __init__(self, *args, **kargs):
        """Initialize that main app and its attributes
        @postcondition: application is created and ready to be run in mainloop

        """
        wx.App.__init__(self, *args, **kargs)
        events.AppEventHandlerMixin.__init__(self)

        # Attributes
        self._log = dev_tool.DEBUGP
        self._lock = False
        self._windows = dict()

        # Disable debug popups
        wx.Log.EnableLogging(False)
        # XXX: Temporary - disable assertions on OSX to work around 
        #      upstream bug in drawing code "couldnt draw the rotated text"
        if wx.Platform == '__WXMAC__':
            self.SetAssertMode(wx.PYAPP_ASSERT_SUPPRESS)

        # Purge old logs
        logfile = dev_tool.EdLogFile()
        logfile.PurgeOldLogs(7)

        if ed_glob.SINGLE:
            # Setup the instance checker
            instance_name = u"%s-%s" % (self.GetAppName(), wx.GetUserId())
            self._instance = wx.SingleInstanceChecker(instance_name)
            if self._instance.IsAnotherRunning():
                try:
                    opts, args = ProcessCommandLine()
                except getopt.GetoptError, msg:
                    self._log("[app][err] %s" % str(msg))
                    args = list()
                    opts = dict()

                exml = ed_ipc.IPCCommand()
                if len(args):
                    nargs = list()
                    for p in args:
                        try:
                            p = ebmlib.GetAbsPath(p)
                        except:
                            pass
                        fxml = ed_ipc.IPCFile()
                        fxml.value = p
                        nargs.append(fxml)
                    exml.filelist = nargs
                arglist = list()
                for arg, val in opts.items():
                    axml = ed_ipc.IPCArg()
                    axml.name = arg
                    axml.value = val
                    arglist.append(axml)
                exml.arglist = arglist

                # TODO: need to process other command line options as well i.e) -g
                self._log("[app][info] Sending: %s" % exml.Xml)
                rval = ed_ipc.SendCommands(exml, profiler.Profile_Get('SESSION_KEY'))
                # If sending the command failed then let the editor startup
                # a new instance
                if not rval:
                    self._isfirst = True
            else:
                self._log("[app][info] Starting Ipc server...")
                # Set the session key and save it to the users profile so
                # that other instances can access the server
                key = unicode(base64.b64encode(os.urandom(8), 'zZ'))
                key = wx.GetUserName() + key
                profiler.Profile_Set('SESSION_KEY', key)
                profiler.Profile_Set('ISBINARY', hasattr(sys, 'frozen'))
                path = profiler.Profile_Get('MYPROFILE')
                profiler.TheProfile.Write(path)
                try:
                    self._server = ed_ipc.EdIpcServer(self, profiler.Profile_Get('SESSION_KEY'))
                    self._server.start()
                except Exception, msg:
                    self._log("[app][err] Failed to start ipc server")
                    self._log("[app][err] %s" % str(msg))
                    self._server = None
                self._isfirst = True
        else:
            self._isfirst = True

        # Setup Plugins after locale as they may have resource that need to
        # be loaded.
        if self._isfirst:
            self._pluginmgr = plugin.PluginManager()

            self._log("[app][info] Registering Editra's ArtProvider")
            wx.ArtProvider.PushProvider(ed_art.EditraArt())

        # Check if libenchant has been loaded or need to be
        import extern.stcspellcheck as stcspellcheck
        checker = stcspellcheck.STCSpellCheck
        if not checker.isEnchantOk():
            spref = profiler.Profile_Get('SPELLCHECK', default=dict())
            libpath = spref.get('epath', u'')
            checker.reloadEnchant(libpath)
            # TODO: log if load fails here

    def AddMessageCatalog(self, name, path):
        """Add a catalog lookup path to the app
        @param name: name of catalog (i.e 'projects')
        @param path: catalog lookup path

        """
        if self.locale is not None:
            path = resource_filename(path, 'locale')
            self.locale.AddCatalogLookupPathPrefix(path)
            self.locale.AddCatalog(name)

    def OnInit(self):
        """Initialize the Editor
        @note: this gets called before __init__
        @postcondition: custom artprovider and plugins are loaded

        """
        self.SetAppName(ed_glob.PROG_NAME)

        self._log = dev_tool.DEBUGP
        self._log("[app][info] Editra is Initializing")

        # Load user preferences
        self.profile_updated = InitConfig()
        self._isfirst = False # Is the first instance
        self._instance = None

        # Setup Locale
        locale.setlocale(locale.LC_ALL, '')
        self.locale = wx.Locale(ed_i18n.GetLangId(profiler.Profile_Get('LANG')))
        if self.locale.GetCanonicalName() in ed_i18n.GetAvailLocales():
            self.locale.AddCatalogLookupPathPrefix(ed_glob.CONFIG['LANG_DIR'])
            self.locale.AddCatalog(ed_glob.PROG_NAME)
        else:
            del self.locale
            self.locale = None

        # Check and set encoding if necessary
        d_enc = profiler.Profile_Get('ENCODING')
        if not d_enc:
            profiler.Profile_Set('ENCODING', ed_txt.DEFAULT_ENCODING)
        else:
            # Ensure the default encoding is valid
            # Fixes up older installs on some systems that may have an
            # invalid encoding set.
            try:
                codecs.lookup(d_enc)
            except (LookupError, TypeError):
                self._log("[app][err] Resetting bad encoding: %s" % d_enc)
                profiler.Profile_Set('ENCODING', ed_txt.DEFAULT_ENCODING)

        # Setup the Error Reporter
        if profiler.Profile_Get('REPORTER', 'bool', True):
            sys.excepthook = dev_tool.ExceptionHook

        #---- Bind Events ----#
        self.Bind(wx.EVT_ACTIVATE_APP, self.OnActivate)
        self.Bind(wx.EVT_MENU, self.OnNewWindow, id=ed_glob.ID_NEW_WINDOW)
        self.Bind(wx.EVT_MENU, self.OnCloseWindow)
        self.Bind(ed_event.EVT_NOTIFY, self.OnNotify)
        self.Bind(ed_ipc.EVT_COMMAND_RECV, self.OnCommandReceived)

        # Splash a warning if version is not a final version
        if profiler.Profile_Get('APPSPLASH'):
            import edimage
            splash_img = edimage.splashwarn.GetBitmap()
            self.splash = wx.SplashScreen(splash_img, wx.SPLASH_CENTRE_ON_PARENT | \
                                          wx.SPLASH_NO_TIMEOUT, 0, None, wx.ID_ANY)
            self.splash.Show()

        return True

    def Destroy(self):
        """Destroy the application"""
        try:
            # Cleanup the instance checker
            del self._instance
        except AttributeError:
            pass
        wx.App.Destroy(self)

    def DestroySplash(self):
        """Destroy the splash screen"""
        # If is created and not dead already
        if getattr(self, 'splash', None) is not None and \
           isinstance(self.splash, wx.SplashScreen):
            self.splash.Destroy()
            self.splash = None

    def Exit(self, force=False):
        """Exit the program
        @postcondition: If no toplevel windows are present program will exit.
        @postcondition: Program may remain open if an open window is locking.

        """
        self._pluginmgr.WritePluginConfig()
        profiler.TheProfile.Write(profiler.Profile_Get('MYPROFILE'))
        if not self._lock or force:
            if hasattr(self, '_server'):
                self._server.Shutdown()

            try:
                # Cleanup the instance checker
                del self._instance
            except AttributeError:
                pass

            # Exit the app
            wx.App.ExitMainLoop(self)

    def GetLocaleObject(self):
        """Get the locale object owned by this app. Use this method to add
        extra catalogs for lookup.
        @return: wx.Locale or None

        """
        return self.locale

    def GetLog(self):
        """Returns the logging function used by the app
        @return: the logging function of this program instance

        """
        return self._log

    def GetMainWindow(self):
        """Returns reference to the instance of the MainWindow
        that is running if available, and None if not.
        @return: the L{MainWindow} of this app if it is open

        """
        self._log("[app][warn] Editra::GetMainWindow is deprecated")
        for window in self._windows:
            if not hasattr(self._windows[window][0], '__name__'):
                continue

            if self._windows[window][0].__name__ == "MainWindow":
                return self._windows[window][0]
        return None

    def GetActiveWindow(self):
        """Returns the active main window if there is one else it will
        just return some main window or none if there are no main windows
        @return: frame instance or None

        """
        awin = None
        for win in self.GetMainWindows():
            if win.IsActive():
                awin = win
                break

        if awin is None:
            awin = self.GetTopWindow()
            if not isinstance(awin, ed_main.MainWindow):
                if len(self.GetMainWindows()):
                    awin = self.GetMainWindows()[0]

        return awin

    def GetCurrentBuffer(self):
        """Get the current buffer from the active window or None
        @return: EditraStc

        """
        win = self.GetTopWindow()
        if not isinstance(win, ed_main.MainWindow):
            win = self.GetActiveWindow()
            if win is None:
                return None # UI dead?

        if isinstance(win, ed_main.MainWindow):
            nbook = win.GetNotebook()
            if nbook:
                return nbook.GetCurrentCtrl()
        return None

    def GetMainWindows(self):
        """Returns a list of all open main windows
        @return: list of L{MainWindow} instances of this app (list may be empty)

        """
        mainw = list()
        for window in self._windows:
            try:
                if self._windows[window][0].__name__ == "MainWindow":
                    mainw.append(self._windows[window][0])
            except AttributeError:
                continue
        return mainw

    def GetOpenWindows(self):
        """Returns a list of open windows
        @return: list of all open windows owned by app

        """
        return self._windows

    def GetPluginManager(self):
        """Returns the plugin manager used by this application
        @return: Apps plugin manager
        @see: L{plugin}

        """
        return self._pluginmgr

    def GetProfileUpdated(self):
        """Was the profile updated 
        @return: bool

        """
        return self.profile_updated

    def GetWindowInstance(self, wintype):
        """Get an instance of an open window if one exists
        @param wintype: Class type of window to look for
        @precondition: Window must have called L{RegisterWindow}
        @return: Instance of window or None

        """
        for win in self._windows:
            if isinstance(self._windows[win][0], wintype):
                return self._windows[win][0]
        return None

    def IsLocked(self):
        """Returns whether the application is locked or not
        @return: whether a window has locked the app from closing or not

        """
        return self._lock

    def IsOnlyInstance(self):
        """Check if this app is the the first instance that is running
        @return: bool

        """
        return self._isfirst

    def Lock(self):
        """Locks the app from exiting
        @postcondition: program is locked from exiting

        """
        self._lock = True

    def OpenFile(self, filename, line=-1):
        """Open a file in the currently active window
        @param filename: file path
        @keyword line: int

        """
        window = self.GetTopWindow()
        if not isinstance(window, ed_main.MainWindow):
            window = None

        try:
            encoding = sys.getfilesystemencoding()
            fname = ed_txt.DecodeString(filename, encoding)

            if profiler.Profile_Get('OPEN_NW', default=False):
                self.OpenNewWindow(fname, window)
            elif window:
                window.DoOpen(ed_glob.ID_COMMAND_LINE_OPEN, fname, line)

                # Make sure the window is brought to the front
                if window.IsIconized():
                    window.Iconize(False)
                window.Raise()
            else:
                # Some unlikely error condition
                self._log("[app][err] OpenFile unknown error: %s" % filename)
                
        except Exception, msg:
            self._log("[app][err] Failed to open file: %s" % str(msg))

    def MacNewFile(self):
        """Stub for future use"""
        pass

    def MacOpenFile(self, filename):
        """Macintosh Specific code for opening files that are associated
        with the editor and double clicked on after the editor is already
        running.
        @param filename: file path string
        @postcondition: if L{MainWindow} is open file will be opened in notebook

        """
        self._log("[app][info] MacOpenFile Fired")
        self.OpenFile(filename, line=-1)

    def MacPrintFile(self, filename):
        """Stub for future use
        @param filename: file to print

        """
        pass

    def MacReopenApp(self):
        """Handle kAEReopenApplication when dock icons is clicked on"""
        frame = self.GetTopWindow()
        if frame is not None:
            if frame.IsIconized():
                frame.Iconize(False)
            frame.Raise()

    def OnActivate(self, evt):
        """Activation Event Handler
        @param evt: event that called this handler
        @type evt: wx.ActivateEvent

        """
        if evt.GetActive():
            self._log("[app][info] I'm Awake!!")
            # Refresh Clipboard Ring
            ed_main.MainWindow.UpdateClipboardRing()

#            frame = self.GetTopWindow()
#            if frame is not None:
#                if frame.IsIconized():
#                    frame.Iconize(False)
#                frame.Raise()
        else:
            self._log("[app][info] Going to sleep")
        evt.Skip()

    def OnExit(self, evt=None, force=False):
        """Handle application exit request
        @param evt: event that called this handler

        """
        e_id = -1
        if evt:
            e_id = evt.GetId()

        if e_id == ed_glob.ID_EXIT:
            # First loop is to ensure current top window is
            # closed first
            for win in self.GetMainWindows():
                if win.IsActive():
                    result = win.Close()
                    if result:
                        break
                    return
            for win in self.GetMainWindows():
                win.Raise()
                result = win.Close()
                if not result:
                    break
            self.Exit(force)
        else:
            if evt:
                evt.Skip()

    def OnNewWindow(self, evt):
        """Create a new editing window
        @param evt: wx.EVT_MENU

        """
        if evt.GetId() == ed_glob.ID_NEW_WINDOW:
            frame = evt.GetEventObject().GetMenuBar().GetFrame()
            self.OpenNewWindow(caller=frame)
        else:
            evt.Skip()

    def OnCommandReceived(self, evt):
        """Receive commands from the IPC server
        @todo: move command processing into own module

        """
        self._log("[app][info] IN OnCommandReceived")
        cmds = evt.GetCommands()
        if isinstance(cmds, ed_ipc.IPCCommand):
            self._log("[app][info] OnCommandReceived %s" % cmds.Xml)
            if not len(cmds.filelist):
                self.OpenNewWindow()
            else:
                # TODO: change goto line handling to require one
                #       arg per file specified on the command line
                #       i.e) -g 23,44,100
                line = -1
                for argobj in cmds.arglist:
                    arg = argobj.name
                    if arg == '-g':
                        line = int(argobj.value)
                        if line > 0:
                            line -= 1
                        break

                for fname in cmds.filelist:
                    self.OpenFile(fname.value, line)

    def OnCloseWindow(self, evt):
        """Close the currently active window
        @param evt: wx.MenuEvent

        """
        if evt.GetId() in [ed_glob.ID_CLOSE, ed_glob.ID_CLOSE_WINDOW]:
            for window in wx.GetTopLevelWindows():
                if hasattr(window, 'IsActive') and window.IsActive():
                    if hasattr(window, 'Close'):
                        window.Close()
                    break
        else:
            evt.Skip()

    def OpenNewWindow(self, fname=u'', caller=None):
        """Open a new window
        @keyword fname: Open a file in the new window
        @return: the new window

        """
        frame = ed_main.MainWindow(None, wx.ID_ANY,
                                   profiler.Profile_Get('WSIZE'),
                                   ed_glob.PROG_NAME)
        if caller:
            pos = caller.GetPosition()
            frame.SetPosition((pos.x + 22, pos.y + 22))

        self.RegisterWindow(repr(frame), frame, True)
        self.SetTopWindow(frame)
        if isinstance(fname, basestring) and fname != u'':
            frame.DoOpen(ed_glob.ID_COMMAND_LINE_OPEN, fname)
        frame.Show(True)

        # Ensure frame gets an Activate event when shown
        # this doesn't happen automatically on windows
        if wx.Platform == '__WXMSW__':
            wx.PostEvent(frame, wx.ActivateEvent(wx.wxEVT_ACTIVATE, True))
        return frame

    def OnNotify(self, evt):
        """Handle notification events
        @param evt: L{ed_event.NotificationEvent}

        """
        e_val = evt.GetValue()
        if evt.GetId() == ID_UPDATE_CHECK and \
           isinstance(e_val, tuple) and e_val[0]:
            self.DestroySplash()
            mdlg = wx.MessageDialog(self.GetActiveWindow(),
                                    _("An updated version of Editra is available\n"
                                      "Would you like to download Editra %s now?") %\
                                      e_val[1], _("Update Available"),
                                    wx.YES_NO|wx.YES_DEFAULT|wx.CENTER|wx.ICON_INFORMATION)
            if mdlg.ShowModal() == wx.ID_YES:
                dl_dlg = updater.DownloadDialog(None, wx.ID_ANY,
                                                _("Downloading Update"))
                dp_sz = wx.GetDisplaySize()
                dl_dlg.SetPosition(((dp_sz[0] - (dl_dlg.GetSize()[0] + 5)), 25))
                dl_dlg.Show()
            mdlg.Destroy()
        else:
            evt.Skip()

    def RegisterWindow(self, name, window, can_lock=False):
        """Registers winows with the app. The name should be the
        repr of window. The can_lock parameter is a boolean stating
        whether the window can keep the main app running after the
        main frame has exited.
        @param name: name of window
        @param window: reference to window object
        @keyword can_lock: whether window can lock exit or not

        """
        self._windows[name] = (window, can_lock)

    def ReloadArtProvider(self):
        """Reloads the custom art provider onto the artprovider stack
        @postcondition: artprovider is removed and reloaded

        """
        try:
            wx.ArtProvider.PopProvider()
        finally:
            wx.ArtProvider.PushProvider(ed_art.EditraArt())

    def UnLock(self):
        """Unlocks the application
        @postcondition: application is unlocked so it can exit

        """
        self._lock = False

    def UnRegisterWindow(self, name):
        """Unregisters a named window with the app if the window
        was the top window and if other windows that can lock are
        registered in the window stack it will promote the next one
        it finds to be the top window. If no windows that fit this
        criteria are found it will close the application.
        @param name: name of window to unregister

        """
        if name in self._windows:
            self._windows.pop(name)
            
            if not len(self._windows):
                self._log("[app][info] No more open windows shutting down")
                self.Exit()
                return

            # TODO: WXBUG? calling GetTopWindow when there are no more top
            #       level windows causes a crash under MSW. Moving this line
            #       above the previous check can reproduce the error.
            cur_top = self.GetTopWindow()
            if name == repr(cur_top):
                found = False
                for key in self._windows:
                    if self._windows[key][1]:
                        self._log("[app][info] Promoting %s to top" % key)
                        try:
                            self.SetTopWindow(self._windows[key][0])
                        except Exception:
                            continue
                        found = True
                        break

                if not found:
                    self._log("[app][info] No more top windows exiting app")
                    self.UnLock()
                    self.Exit()
            else:
                self._log("[app][info] UnRegistered %s" % name)
        else:
            self._log("[app][warn] The window %s is not registered" % name)

    def WindowCanLock(self, winname):
        """Checks if a named window can lock the application or
        not. The window must have been previously registered with
        a call to RegisterWindow for this function to have any
        real usefullness.
        @param winname: name of window to query

        """
        if winname in self._windows:
            return self._windows[winname][1]
        else:
            self._log("[app][warn] the window %s has "
                      "not been registered" % winname)
            return False

#--------------------------------------------------------------------------#

def InitConfig():
    """Initializes the configuration data
    @postcondition: all configuration data is set

    """
    # Check if a custom config directory was specified on the commandline
    if ed_glob.CONFIG['CONFIG_BASE'] is not None:
        # TODO: there is a bug when the first time the config is created
        #       where the settings will not be saved until second launching.
        config_base = os.path.abspath(ed_glob.CONFIG['CONFIG_BASE'])
    else:
        # Look for a profile directory on the system level. If this directory
        # exists Use it instead of the user one. This will allow for running
        # Editra from a portable drive or for system administrators to enforce
        # settings on a system installed version.
        config_base = util.ResolvConfigDir(u'.Editra', True) 

    if os.path.exists(config_base):
        ed_glob.CONFIG['CONFIG_BASE'] = config_base
        ed_glob.CONFIG['PROFILE_DIR'] = os.path.join(config_base, u"profiles")
        ed_glob.CONFIG['PROFILE_DIR'] += os.sep
        ed_glob.CONFIG['ISLOCAL'] = True
    else:
        config_base = wx.StandardPaths.Get().GetUserDataDir()
        ed_glob.CONFIG['PROFILE_DIR'] = util.ResolvConfigDir(u"profiles")

    # Check for if config directory exists and if profile is from the current
    # running version of Editra.
    profile_updated = False
    if util.HasConfigDir() and os.path.exists(ed_glob.CONFIG['PROFILE_DIR']):
        if profiler.ProfileIsCurrent():
            pstr = profiler.GetProfileStr()
            # If using local(portable) config the profile string is stored
            # as a relative path that just names the config file.
            if ed_glob.CONFIG['ISLOCAL']:
                pstr = os.path.join(ed_glob.CONFIG['PROFILE_DIR'], pstr)
            pstr = util.RepairConfigState(pstr)
            profiler.TheProfile.Load(pstr)
        else:
            dev_tool.DEBUGP("[InitConfig][info] Updating Profile to current version")

            # Load and update profile
            pstr = profiler.GetProfileStr()
            pstr = util.RepairConfigState(pstr)
            profiler.TheProfile.Load(pstr)
            profiler.TheProfile.Update()

            #---- Temporary Profile Adaptions ----#

            # Added after 0.5.32
            mconfig = profiler.Profile_Get('LEXERMENU', default=None)
            if mconfig is None:
                mconfig = [ synglob.LANG_C, synglob.LANG_CPP,
                            synglob.LANG_BASH, synglob.LANG_CSS,
                            synglob.LANG_HTML, synglob.LANG_JAVA,
                            synglob.LANG_LISP, synglob.LANG_PERL,
                            synglob.LANG_PHP, synglob.LANG_PYTHON,
                            synglob.LANG_RUBY, synglob.LANG_SQL,
                            synglob.LANG_XML]
                mconfig.sort()
                profiler.Profile_Set('LEXERMENU', mconfig)

            # GUI_DEBUG mode removed in 0.2.5
            mode = profiler.Profile_Get('MODE')
            if mode == 'GUI_DEBUG':
                profiler.Profile_Set('MODE', 'DEBUG')

            # This key has been removed so clean it from old profiles
            profiler.Profile_Del('LASTCHECK')

            # Print modes don't use strings anymore
            if isinstance(profiler.Profile_Get('PRINT_MODE'), basestring):
                profiler.Profile_Set('PRINT_MODE', ed_glob.PRINT_BLACK_WHITE)

            # Simplifications to eol mode persistence (0.4.28)
            # Keep for now till plugins are updated
            #profiler.Profile_Del('EOL') # changed to EOL_MODE

            # After 0.4.65 LAST_SESSION now points a session file and not
            # to a list of files to open.
            sess = profiler.Profile_Get('LAST_SESSION')
            if isinstance(sess, list):
                profiler.Profile_Set('LAST_SESSION', u'')

            #---- End Temporary Profile Adaptions ----#

            # Write out updated profile
            profiler.TheProfile.Write(pstr)

            # When upgrading from an older version make sure all
            # config directories are available.
            for cfg in ("cache", "styles", "plugins", "profiles", "sessions"):
                if not util.HasConfigDir(cfg):
                    util.MakeConfigDir(cfg)

            profile_updated = True
    else:
        # Fresh install
        util.CreateConfigDir()

        # Check and upgrade installs from old location
        success = True
        try:
            success = UpgradeOldInstall()
        except Exception, msg:
            dev_tool.DEBUGP("[InitConfig][err] %s" % msg)
            success = False

        if not success:
            old_cdir = u"%s%s.%s%s" % (wx.GetHomeDir(), os.sep,
                                       ed_glob.PROG_NAME, os.sep)
            msg = ("Failed to upgrade your old installation\n"
                   "To retain your old settings you may need to copy some files:\n"
                   "\nFrom: %s\n\nTo: %s") % (old_cdir, config_base)
            wx.MessageBox(msg, "Upgrade Failed", style=wx.ICON_WARNING|wx.OK)

        # Set default eol for windows
        if wx.Platform == '__WXMSW__':
            profiler.Profile_Set('EOL_MODE', ed_glob.EOL_MODE_CRLF)
            profiler.Profile_Set('ICONSZ', (16, 16))
        elif wx.Platform == '__WXMAC__':
            # Default to 32x32 toolbar icons on OSX
            profiler.Profile_Set('ICONSZ', (32, 32))

    #---- Profile Loaded / Installed ----#

    # Set debug mode
    emode = profiler.Profile_Get('MODE')
    if 'DEBUG' in emode:
        ed_glob.DEBUG = True
        if emode.startswith('VERBOSE'):
            ed_glob.VDEBUG = True

    # Resolve resource locations
    ed_glob.CONFIG['CONFIG_DIR'] = util.ResolvConfigDir(u"")
    ed_glob.CONFIG['INSTALL_DIR'] = util.ResolvConfigDir(u"", True)
    ed_glob.CONFIG['KEYPROF_DIR'] = util.ResolvConfigDir(u"ekeys", True)
    ed_glob.CONFIG['SYSPIX_DIR'] = util.ResolvConfigDir(u"pixmaps", True)
    ed_glob.CONFIG['PLUGIN_DIR'] = util.ResolvConfigDir(u"plugins")
    ed_glob.CONFIG['THEME_DIR'] = util.ResolvConfigDir(os.path.join(u"pixmaps", u"theme"))
    ed_glob.CONFIG['LANG_DIR'] = util.ResolvConfigDir(u"locale", True)
    ed_glob.CONFIG['STYLES_DIR'] = util.ResolvConfigDir(u"styles")
    ed_glob.CONFIG['SYS_PLUGIN_DIR'] = util.ResolvConfigDir(u"plugins", True)
    ed_glob.CONFIG['SYS_STYLES_DIR'] = util.ResolvConfigDir(u"styles", True)
    ed_glob.CONFIG['TEST_DIR'] = util.ResolvConfigDir(os.path.join(u"tests", u"syntax"), True)

    # Make sure all standard config directories are there
    for cfg in ("cache", "styles", "plugins", "profiles", "sessions"):
        if not util.HasConfigDir(cfg):
            util.MakeConfigDir(cfg)
    ed_glob.CONFIG['CACHE_DIR'] = util.ResolvConfigDir(u"cache")
    ed_glob.CONFIG['SESSION_DIR'] = util.ResolvConfigDir(u"sessions")

    return profile_updated

#--------------------------------------------------------------------------#

def UpgradeOldInstall():
    """Upgrade an old installation and transfer all files if they exist
    @note: FOR INTERNAL USE ONLY
    @return: bool (True if success, False if failure)

    """
    old_cdir = u"%s%s.%s%s" % (wx.GetHomeDir(), os.sep,
                               ed_glob.PROG_NAME, os.sep)
    base = ed_glob.CONFIG['CONFIG_BASE']
    if base is None:
        base = wx.StandardPaths.Get().GetUserDataDir() + os.sep

    err = 0
    if os.path.exists(old_cdir) and \
       base.lower().rstrip(os.sep) != old_cdir.lower().rstrip(os.sep):
        for item in os.listdir(old_cdir):
            try:
                dest = os.path.join(base, item)
                item = os.path.join(old_cdir, item)
                if os.path.exists(dest):
                    if os.path.isdir(dest):
                        shutil.rmtree(dest, True)
                    else:
                        os.remove(dest)

                shutil.move(item, dest)
            except Exception, msg:
                util.Log("[Upgrade][err] %s" % msg)
                err += 1
                continue

        os.rmdir(old_cdir)

        # Load the copied over profile
        pstr = profiler.GetProfileStr()
        prof = os.path.basename(pstr)
        pstr = os.path.join(base, u"profiles", prof)
        if os.path.exists(pstr):
            profiler.TheProfile.Load(pstr)
            profiler.TheProfile.Update()
            profiler.UpdateProfileLoader()

        if not err:
            wx.MessageBox(_("Your profile has been updated to the latest "
                "version") + u"\n" + \
              _("Please check the preferences dialog to check "
                "your preferences"),
              _("Profile Updated"))

    return not err

#--------------------------------------------------------------------------#

def PrintHelp(err=None):
    """Print command line help
    @postcondition: Help is printed and program exits

    """
    if err is not None:
        sys.stderr.write(err + os.linesep)

    print(("Editra - %s - Developers Text Editor\n"
       "Cody Precord (2005-2010)\n\n"
       "usage: Editra [arguments] [files... ]\n\n"
       "Short Arguments:\n"
       "  -c    Set custom configuration directory at runtime\n"
       "  -d    Turn on console debugging (-dd for verbose debug)\n"
       "  -D    Turn off console debugging (overrides preferences)\n"
       "  -g    Open file to line (i.e Editra -g 10 file.txt)\n"
       "  -h    Show this help message\n"
       "  -p    Run Editra in the profiler (outputs to editra.prof).\n"
       "  -v    Print version number and exit\n"
       "  -S    Disable single instance checker\n"
       "\nLong Arguments:\n"
       "  --confdir arg     Set custom configuration directory at runtime\n"
       "  --debug           Turn on console debugging\n"
       "  --help            Show this help message\n"
       "  --auth            Print the ipc server info\n"
       "  --version         Print version number and exit\n"
       "  --profileOut arg  Run Editra in the profiler (arg is output file)\n"
      ) % ed_glob.VERSION)

    if err is None:
        os._exit(0)
    else:
        os._exit(1)

#--------------------------------------------------------------------------#

def ProcessCommandLine():
    """Process the command line switches
    @return: tuple ({switches,}, [args,])

    """
    try:
        items, args = getopt.getopt(sys.argv[1:], "dg:hp:vDSc:",
                                   ['debug', 'help', 'version', 'auth',
                                    'confdir=', 'profileOut='])
    except getopt.GetoptError, msg:
        # Raise error to console and exit
        PrintHelp(str(msg))
    
    # Process command line options
    opts = dict(items)
    for opt, value in dict(opts).items():
        if opt in ['-h', '--help']:
            PrintHelp()
        elif opt in ['-v', '--version']:
            print(ed_glob.VERSION)
            os._exit(0)
        elif opt in ['-d', '--debug'] and '-D' not in opts.keys():
            # If the debug flag is set more than once go into verbose mode
            if ed_glob.DEBUG:
                ed_glob.VDEBUG = True
            ed_glob.DEBUG = True
            opts.pop(opt)
        elif opt == '-D':
            ed_glob.DEBUG = False
            ed_glob.VDEBUG = False
            opts.pop('-D')
        elif opt == '-S':
            # Disable single instance checker
            ed_glob.SINGLE = False
            opts.pop(opt)
        elif opt in ['-c', '--confdir']:
            ed_glob.CONFIG['CONFIG_BASE'] = value
            opts.pop(opt)
        elif opt == '--profileOut':
            opts['-p'] = value
            opts.pop('--profileOut')
        elif opt == '-g':
            # Validate argument passed to -g
            if not value.isdigit():
                PrintHelp("error: -g requires a number as an argument!")
        else:
            pass

    # Return any unprocessed arguments
    return opts, args

#--------------------------------------------------------------------------#

def Main():
    """Configures and Runs an instance of Editra
    @summary: Parses command line options, loads the user profile, creates
              an instance of Editra and starts the main loop.

    """
    opts, args = ProcessCommandLine()

    if '-p' in opts:
        p_file = opts['-p']
        opts.pop('-p')

        if not len(p_file):
            # Fall back to default output file
            p_file = "editra.prof"
            
        import hotshot
        prof = hotshot.Profile(p_file)
        prof.runcall(_Main, opts, args)
        prof.close()
    else:
        _Main(opts, args)

def _Main(opts, args):
    """Main method
    @param opts: Commandline options
    @param args: Commandline arguments

    """
    # Put extern subpackage on path so that bundled external dependencies
    # can be found if needed.
    if not hasattr(sys, 'frozen'):
        epath = os.path.join(os.path.dirname(__file__), 'extern')
        if os.path.exists(epath):
            sys.path.append(epath)

    # Create Application
    dev_tool.DEBUGP("[main][app] Initializing application...")
    editra_app = Editra(False)

    # Print ipc server authentication info
    if '--auth' in opts:
        opts.pop('--auth')
        print "port=%d,key=%s" % (ed_ipc.EDPORT,
                                  profiler.Profile_Get('SESSION_KEY'))

    # Check if this is the only instance, if its not exit since
    # any of the opening commands have already been passed to the
    # master instance
    if not editra_app.IsOnlyInstance():
        dev_tool.DEBUGP("[main][info] Second instance exiting...")
        editra_app.Destroy()
        os._exit(0)

    # Set the timeout on destroying the splash screen
    wx.CallLater(2300, editra_app.DestroySplash)

    if profiler.Profile_Get('SET_WSIZE'):
        wsize = profiler.Profile_Get('WSIZE')
    else:
        wsize = (700, 450)
    frame = ed_main.MainWindow(None, wx.ID_ANY, wsize, ed_glob.PROG_NAME)
    frame.Maximize(profiler.Profile_Get('MAXIMIZED'))
    editra_app.RegisterWindow(repr(frame), frame, True)
    editra_app.SetTopWindow(frame)
    frame.Show(True)

    # Load Session Data
    # But not if there are command line args for files to open
    if profiler.Profile_Get('SAVE_SESSION', 'bool', False) and not len(args):
        session = profiler.Profile_Get('LAST_SESSION', default=u'')
        if isinstance(session, list):
            # Check for format conversion from previous versions
            profiler.Profile_Set('LAST_SESSION', u'')
        else:
            frame.GetNotebook().LoadSessionFile(session)
        del session

    # Unlike wxMac/wxGTK Windows doesn't post an activate event when a window
    # is first shown, so do it manually to make sure all event handlers get
    # pushed.
    if wx.Platform == '__WXMSW__':
        wx.PostEvent(frame, wx.ActivateEvent(wx.wxEVT_ACTIVATE, True))

    # Do update check if preferences say its ok to do so
    isadmin = os.access(ed_glob.CONFIG['INSTALL_DIR'], os.R_OK|os.W_OK)
    if isadmin and profiler.Profile_Get('CHECKUPDATE', default=True):
        uthread = updater.UpdateThread(editra_app, ID_UPDATE_CHECK)
        uthread.start()

    if len(args):
        line = -1
        if '-g' in opts:
            line = max(0, int(opts.pop('-g')) - 1)

        # TODO: should line arg only be applied to the first file name or all?
        #       currently apply to all.
        for arg in args:
            try:
                fname = ed_txt.DecodeString(arg, sys.getfilesystemencoding())
                fname = ebmlib.GetAbsPath(fname)
                frame.DoOpen(ed_glob.ID_COMMAND_LINE_OPEN, fname, line)
            except IndexError:
                dev_tool.DEBUGP("[main][err] IndexError on commandline args")

    # Notify that profile was updated
    if editra_app.GetProfileUpdated():
        editra_app.DestroySplash()
        # Make sure window iniliazes to default position
        profiler.Profile_Del('WPOS')
        wx.MessageBox(_("Your profile has been updated to the latest "
                        "version") + u"\n" + \
                      _("Please check the preferences dialog to verify "
                        "your preferences"),
                      _("Profile Updated"))

    # 3. Start Applications Main Loop
    dev_tool.DEBUGP("[main][info] Starting MainLoop...")
    wx.CallAfter(frame.Raise)

    # Install handlers to exit app if os is shutting down/restarting
    ebmlib.InstallTermHandler(editra_app.Exit, force=True)

    editra_app.MainLoop()
    dev_tool.DEBUGP("[main][info] MainLoop finished exiting application")
    os._exit(0)

#-----------------------------------------------------------------------------#
if __name__ == '__main__':
    Main()
