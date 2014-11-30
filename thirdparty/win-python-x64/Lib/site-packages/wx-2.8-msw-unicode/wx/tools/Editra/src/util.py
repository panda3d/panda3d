###############################################################################
# Name: util.py                                                               #
# Purpose: Misc utility functions used through out Editra                     #
# Author: Cody Precord <cprecord@editra.org>                                  #
# Copyright: (c) 2008 Cody Precord <staff@editra.org>                         #
# License: wxWindows License                                                  #
###############################################################################

"""
This file contains various helper functions and utilities that the program uses.

"""

__author__ = "Cody Precord <cprecord@editra.org>"
__svnid__ = "$Id: util.py 67367 2011-03-31 19:27:05Z CJP $"
__revision__ = "$Revision: 67367 $"

#--------------------------------------------------------------------------#
# Imports
import os
import sys
import mimetypes
import encodings
import codecs
import urllib2
import wx

# Editra Libraries
import ed_glob
import ed_event
import ed_crypt
import dev_tool
import syntax.syntax as syntax
import syntax.synglob as synglob
import ebmlib

_ = wx.GetTranslation
#--------------------------------------------------------------------------#

class DropTargetFT(wx.PyDropTarget):
    """Drop target capable of accepting dropped files and text
    @todo: has some issues with the clipboard on windows under certain
           conditions. They arent fatal but need fixing.

    """
    def __init__(self, window, textcallback=None, filecallback=None):
        """Initializes the Drop target
        @param window: window to recieve drop objects

        """
        super(DropTargetFT, self).__init__()

        # Attributes
        self.window = window
        self._data = dict(data=None, fdata=None, tdata=None,
                          tcallb=textcallback, fcallb=filecallback)
        self._tmp = None
        self._lastp = None

        # Setup
        self.InitObjects()

    def CreateDragString(self, txt):
        """Creates a bitmap of the text that is being dragged
        @todo: possibly set colors to match highlighting of text
        @todo: generalize this to be usable by other widgets besides stc

        """
        if not issubclass(self.window.__class__, wx.stc.StyledTextCtrl):
            return

        stc = self.window
        txt = txt.split(stc.GetEOLChar())
        longest = (0, 0)
        for line in txt:
            ext = stc.GetTextExtent(line)
            if ext[0] > longest[0]:
                longest = ext

        cords = [ (0, x * longest[1]) for x in xrange(len(txt)) ]
        try:
            mdc = wx.MemoryDC(wx.EmptyBitmap(longest[0] + 5,
                                             longest[1] * len(txt), 32))
            mdc.SetBackgroundMode(wx.TRANSPARENT)
            mdc.SetTextForeground(stc.GetDefaultForeColour())
            mdc.SetFont(stc.GetDefaultFont())
            mdc.DrawTextList(txt, cords)
            self._tmp = wx.DragImage(mdc.GetAsBitmap())
        except wx.PyAssertionError, msg:
            Log("[droptargetft][err] %s" % str(msg))

    def InitObjects(self):
        """Initializes the text and file data objects
        @postcondition: all data objects are initialized

        """
        self._data['data'] = wx.DataObjectComposite()
        self._data['tdata'] = wx.TextDataObject()
        self._data['fdata'] = wx.FileDataObject()
        self._data['data'].Add(self._data['tdata'], True)
        self._data['data'].Add(self._data['fdata'], False)
        self.SetDataObject(self._data['data'])

    def OnEnter(self, x_cord, y_cord, drag_result):
        """Called when a drag starts
        @keyword x_cord: x cord of enter point
        @keyword y_cord: y cord of enter point
        @return: result of drop object entering window

        """
        # GetData seems to happen automatically on msw, calling it again
        # causes this to fail the first time.
        if wx.Platform in ['__WXGTK__', '__WXMSW__']:
            return wx.DragCopy

        if wx.Platform == '__WXMAC__':
            try:
                self.GetData()
            except wx.PyAssertionError:
                return wx.DragError

        self._lastp = (x_cord, y_cord)
        files = self._data['fdata'].GetFilenames()
        text = self._data['tdata'].GetText()
        if len(files):
            self.window.SetCursor(wx.StockCursor(wx.CURSOR_COPY_ARROW))
        else:
            self.CreateDragString(text)
        return drag_result

    def OnDrop(self, x_cord=0, y_cord=0):
        """Gets the drop cords
        @keyword x_cord: x cord of drop object
        @keyword y_cord: y cord of drop object
        @todo: implement snapback when drop is out of range

        """
        self._tmp = None
        self._lastp = None
        return True

    def OnDragOver(self, x_cord, y_cord, drag_result):
        """Called when the cursor is moved during a drag action
        @keyword x_cord: x cord of mouse
        @keyword y_cord: y cord of mouse
        @return: result of drag over
        @todo: For some reason the caret position changes which can be seen
               by the brackets getting highlighted. However the actual caret
               is not moved.

        """
        stc = self.window
        if self._tmp is None:
            if hasattr(stc, 'DoDragOver'):
                val = stc.DoDragOver(x_cord, y_cord, drag_result)
                self.ScrollBuffer(stc, x_cord, y_cord)
            drag_result = wx.DragCopy
        else:
            # A drag image was created
            if hasattr(stc, 'DoDragOver'):
                point = wx.Point(x_cord, y_cord)
                self._tmp.BeginDrag(point - self._lastp, stc)
                self._tmp.Hide()
                stc.DoDragOver(x_cord, y_cord, drag_result)
                self._tmp.Move(point)
                self._tmp.Show()
                self._tmp.RedrawImage(self._lastp, point, True, True)
                self._lastp = point
                self.ScrollBuffer(stc, x_cord, y_cord)
            drag_result = wx.DragCopy

        return drag_result

    def OnData(self, x_cord, y_cord, drag_result):
        """Gets and processes the dropped data
        @postcondition: dropped data is processed

        """
        self.window.SetCursor(wx.StockCursor(wx.CURSOR_ARROW))
        if self.window.HasCapture():
            self.window.ReleaseMouse()

        try:
            data = self.GetData()
        except wx.PyAssertionError:
            wx.PostEvent(self.window.GetTopLevelParent(), \
                        ed_event.StatusEvent(ed_event.edEVT_STATUS, -1,
                                             _("Unable to accept dropped file "
                                               "or text")))
            data = False
            drag_result = wx.DragCancel

        if data:
            files = self._data['fdata'].GetFilenames()
            text = self._data['tdata'].GetText()
            if len(files) > 0 and self._data['fcallb'] is not None:
                self._data['fcallb'](files)
            elif len(text) > 0:
                if self._data['tcallb'] is not None:
                    self._data['tcallb'](text)
                elif hasattr(self.window, 'DoDropText'):
                    self.window.DoDropText(x_cord, y_cord, text)
        self.InitObjects()
        return drag_result

    def OnLeave(self):
        """Handles the event of when the drag object leaves the window
        @postcondition: Cursor is set back to normal state

        """
        self.window.SetCursor(wx.StockCursor(wx.CURSOR_ARROW))
        if self.window.HasCapture():
            self.window.ReleaseMouse()

        if self._tmp is not None:
            try:
                self._tmp.EndDrag()
            except wx.PyAssertionError, msg:
                Log("[droptargetft][err] %s" % str(msg))

    @staticmethod
    def ScrollBuffer(stc, x_cord, y_cord):
        """Scroll the buffer as the dragged text is moved towards the
        ends.
        @param stc: StyledTextCtrl
        @param x_cord: int (x position)
        @param y_cord: int (y position)
        @note: currenly does not work on wxMac

        """
        cline = stc.PositionFromPoint(wx.Point(x_cord, y_cord))
        if cline != wx.stc.STC_INVALID_POSITION:
            cline = stc.LineFromPosition(cline)
            fline = stc.GetFirstVisibleLine()
            lline = stc.GetLastVisibleLine()
            if (cline - fline) < 2:
                stc.ScrollLines(-1)
            elif lline - cline < 2:
                stc.ScrollLines(1)
            else:
                pass

#---- End FileDropTarget ----#

class EdClipboard(ebmlib.CycleCache):
    """Local clipboard object
    @todo: make into a singleton

    """
    def GetNext(self):
        """Get the next item in the cache"""
        # Initialize the clipboard if it hasn't been loaded yet and
        # there is something in the system clipboard
        if self.GetCurrentSize() == 0:
            txt = GetClipboardText()
            if txt is not None:
                self.Put(txt)

        return super(EdClipboard, self).GetNext()

    def IsAtIndex(self, txt):
        """Is the passed in phrase at the current cycle index in the
        cache. Used to check if index should be reset or to continue in
        the cycle.
        @param txt: selected text

        """
        pre = self.PeekPrev()
        next = self.PeekNext()
        if txt in (pre, next):
            return True
        else:
            return False

    def Put(self, txt):
        """Put some text in the clipboard"""
        pre = self.PeekPrev()
        next = self.PeekNext()
        if len(txt) and txt not in (pre, next):
            self.PutItem(txt)

#---- Misc Common Function Library ----#
# Used for holding the primary selection on mac/msw
FAKE_CLIPBOARD = None

def GetClipboardText(primary=False):
    """Get the primary selection from the clipboard if there is one
    @return: str or None

    """
    if primary and wx.Platform == '__WXGTK__':
        wx.TheClipboard.UsePrimarySelection(True)
    elif primary:
        # Fake the primary selection on mac/msw
        global FAKE_CLIPBOARD
        return FAKE_CLIPBOARD
    else:
        pass

    text_obj = wx.TextDataObject()
    rtxt = None
    
    if wx.TheClipboard.IsOpened() or wx.TheClipboard.Open():
        if wx.TheClipboard.GetData(text_obj):
            rtxt = text_obj.GetText()
        wx.TheClipboard.Close()

    if primary and wx.Platform == '__WXGTK__':
        wx.TheClipboard.UsePrimarySelection(False)
    return rtxt

def SetClipboardText(txt, primary=False):
    """Copies text to the clipboard
    @param txt: text to put in clipboard
    @keyword primary: Set txt as primary selection (x11)

    """
    # Check if using primary selection
    if primary and wx.Platform == '__WXGTK__':
        wx.TheClipboard.UsePrimarySelection(True)
    elif primary:
        # Fake the primary selection on mac/msw
        global FAKE_CLIPBOARD
        FAKE_CLIPBOARD = txt
        return True
    else:
        pass

    data_o = wx.TextDataObject()
    data_o.SetText(txt)
    if wx.TheClipboard.IsOpened() or wx.TheClipboard.Open():
        wx.TheClipboard.SetData(data_o)
        wx.TheClipboard.Close()
        if primary and wx.Platform == '__WXGTK__':
            wx.TheClipboard.UsePrimarySelection(False)
        return True
    else:
        return False

def FilterFiles(file_list):
    """Filters a list of paths and returns a list of paths
    that can probably be opened in the editor.
    @param file_list: list of files/folders to filter for good files in

    """
    good = list()
    checker = ebmlib.FileTypeChecker()
    for path in file_list:
        if not checker.IsBinary(path):
            good.append(path)
    return good

def GetFileType(fname):
    """Get what the type of the file is as Editra sees it
    in a formatted string.
    @param fname: file path
    @return: string (formatted/translated filetype)

    """
    if os.path.isdir(fname):
        return _("Folder")

    eguess = syntax.GetTypeFromExt(fname.split('.')[-1])
    if eguess == synglob.LANG_TXT and fname.split('.')[-1] == 'txt':
        return _("Text Document")
    elif eguess == synglob.LANG_TXT:
        mtype = mimetypes.guess_type(fname)[0]
        if mtype is not None:
            return mtype
        else:
            return _("Unknown")
    else:
        return _("%s Source File") % eguess

def GetFileReader(file_name, enc='utf-8'):
    """Returns a file stream reader object for reading the
    supplied file name. It returns a file reader using the encoding
    (enc) which defaults to utf-8. If lookup of the reader fails on
    the host system it will return an ascii reader.
    If there is an error in creating the file reader the function
    will return a negative number.
    @param file_name: name of file to get a reader for
    @keyword enc: encoding to use for reading the file
    @return file reader, or int if error.

    """
    try:
        file_h = file(file_name, "rb")
    except (IOError, OSError):
        dev_tool.DEBUGP("[file_reader] Failed to open file %s" % file_name)
        return -1

    try:
        reader = codecs.getreader(enc)(file_h)
    except (LookupError, IndexError, ValueError):
        dev_tool.DEBUGP('[file_reader] Failed to get %s Reader' % enc)
        reader = file_h
    return reader

def GetFileWriter(file_name, enc='utf-8'):
    """Returns a file stream writer object for reading the
    supplied file name. It returns a file writer in the supplied
    encoding if the host system supports it other wise it will return
    an ascii reader. The default will try and return a utf-8 reader.
    If there is an error in creating the file reader the function
    will return a negative number.
    @param file_name: path of file to get writer for
    @keyword enc: encoding to write text to file with

    """
    try:
        file_h = open(file_name, "wb")
    except IOError:
        dev_tool.DEBUGP("[file_writer][err] Failed to open file %s" % file_name)
        return -1
    try:
        writer = codecs.getwriter(enc)(file_h)
    except (LookupError, IndexError, ValueError):
        dev_tool.DEBUGP('[file_writer][err] Failed to get %s Writer' % enc)
        writer = file_h
    return writer

def GetFileManagerCmd():
    """Get the file manager open command for the current os. Under linux
    it will check for xdg-open, nautilus, konqueror, and Thunar, it will then
    return which one it finds first or 'nautilus' it finds nothing.
    @return: string

    """
    if wx.Platform == '__WXMAC__':
        return 'open'
    elif wx.Platform == '__WXMSW__':
        return 'explorer'
    else:
        # Check for common linux filemanagers returning first one found
        #          Gnome/ubuntu KDE/kubuntu  xubuntu
        for cmd in ('xdg-open', 'nautilus', 'konqueror', 'Thunar'):
            result = os.system("which %s > /dev/null" % cmd)
            if result == 0:
                return cmd
        else:
            return 'nautilus'

def GetUserConfigBase():
    """Get the base user configuration directory path"""
    cbase = ed_glob.CONFIG['CONFIG_BASE']
    if cbase is None:
        cbase = wx.StandardPaths_Get().GetUserDataDir()
        if wx.Platform == '__WXGTK__':
            if u'.config' not in cbase and not os.path.exists(cbase):
                # If no existing configuration return xdg config path
                base, cfgdir = os.path.split(cbase)
                tmp_path = os.path.join(base, '.config')
                if os.path.exists(tmp_path):
                    cbase = os.path.join(tmp_path, cfgdir.lstrip(u'.'))
    return cbase + os.sep

def HasConfigDir(loc=u""):
    """ Checks if the user has a config directory and returns True
    if the config directory exists or False if it does not.
    @return: whether config dir in question exists on an expected path

    """
    cbase = GetUserConfigBase()
    to_check = os.path.join(cbase, loc)
    return os.path.exists(to_check)

def MakeConfigDir(name):
    """Makes a user config directory
    @param name: name of config directory to make in user config dir

    """
    cbase = GetUserConfigBase()
    try:
        os.mkdir(cbase + name)
    except (OSError, IOError):
        pass

def RepairConfigState(path):
    """Repair the state of profile path, updating and creating it
    it does not exist.
    @param path: path of profile

    """
    if os.path.isabs(path) and os.path.exists(path):
        return path
    else:
        # Need to fix some stuff up
        CreateConfigDir()
        import profiler
        return profiler.Profile_Get("MYPROFILE")

def CreateConfigDir():
    """ Creates the user config directory its default sub
    directories and any of the default config files.
    @postcondition: all default configuration files/folders are created

    """
    #---- Resolve Paths ----#
    config_dir = GetUserConfigBase()
    profile_dir = os.path.join(config_dir, u"profiles")
    dest_file = os.path.join(profile_dir, u"default.ppb")
    ext_cfg = [u"cache", u"styles", u"plugins"]

    #---- Create Directories ----#
    if not os.path.exists(config_dir):
        os.mkdir(config_dir)

    if not os.path.exists(profile_dir):
        os.mkdir(profile_dir)

    for cfg in ext_cfg:
        if not HasConfigDir(cfg):
            MakeConfigDir(cfg)

    import profiler
    profiler.TheProfile.LoadDefaults()
    profiler.Profile_Set("MYPROFILE", dest_file)
    profiler.TheProfile.Write(dest_file)
    profiler.UpdateProfileLoader()

def ResolvConfigDir(config_dir, sys_only=False):
    """Checks for a user config directory and if it is not
    found it then resolves the absolute path of the executables
    directory from the relative execution path. This is then used
    to find the location of the specified directory as it relates
    to the executable directory, and returns that path as a
    string.
    @param config_dir: name of config directory to resolve
    @keyword sys_only: only get paths of system config directory or user one
    @note: This method is probably much more complex than it needs to be but
           the code has proven itself.

    """
    # Try to get a User config directory
    if not sys_only:
        user_config = GetUserConfigBase()
        user_config = os.path.join(user_config, config_dir)

        if os.path.exists(user_config):
            return user_config + os.sep

    # Check if the system install path has already been resolved once before
    if ed_glob.CONFIG['INSTALL_DIR'] != u"":
        tmp = os.path.join(ed_glob.CONFIG['INSTALL_DIR'], config_dir)
        tmp = os.path.normpath(tmp) + os.sep
        if os.path.exists(tmp):
            return tmp
        else:
            del tmp

    # The following lines are used only when Editra is being run as a
    # source package. If the found path does not exist then Editra is
    # running as as a built package.
    if not hasattr(sys, 'frozen'):
        path = __file__
        path = os.sep.join(path.split(os.sep)[:-2])
        path =  path + os.sep + config_dir + os.sep
        if os.path.exists(path):
            if not ebmlib.IsUnicode(path):
                path = unicode(path, sys.getfilesystemencoding())
            return path

    # If we get here we need to do some platform dependant lookup
    # to find everything.
    path = sys.argv[0]
    if not ebmlib.IsUnicode(path):
        path = unicode(path, sys.getfilesystemencoding())

    # If it is a link get the real path
    if os.path.islink(path):
        path = os.path.realpath(path)

    # Tokenize path
    pieces = path.split(os.sep)

    if wx.Platform == u'__WXMSW__':
        # On Windows the exe is in same dir as config directories
        pro_path = os.sep.join(pieces[:-1])

        if os.path.isabs(pro_path):
            pass
        elif pro_path == u"":
            pro_path = os.getcwd()
            pieces = pro_path.split(os.sep)
            pro_path = os.sep.join(pieces[:-1])
        else:
            pro_path = os.path.abspath(pro_path)
    elif wx.Platform == u'__WXMAC__':
        # On OS X the config directories are in the applet under Resources
        stdpath = wx.StandardPaths_Get()
        pro_path = stdpath.GetResourcesDir()
        pro_path = os.path.join(pro_path, config_dir)
    else:
        pro_path = os.sep.join(pieces[:-2])
        if pro_path.startswith(os.sep):
            pass
        elif pro_path == u"":
            pro_path = os.getcwd()
            pieces = pro_path.split(os.sep)
            if pieces[-1] not in [ed_glob.PROG_NAME.lower(), ed_glob.PROG_NAME]:
                pro_path = os.sep.join(pieces[:-1])
        else:
            pro_path = os.path.abspath(pro_path)

    if wx.Platform != u'__WXMAC__':
        pro_path = pro_path + os.sep + config_dir + os.sep

    path = os.path.normpath(pro_path) + os.sep

    # Make sure path is unicode
    if not ebmlib.IsUnicode(path):
        path = unicode(path, sys.getdefaultencoding())

    return path

def GetResources(resource):
    """Returns a list of resource directories from a given toplevel config dir
    @param resource: config directory name
    @return: list of resource directory that exist under the given resource path

    """
    rec_dir = ResolvConfigDir(resource)
    if os.path.exists(rec_dir):
        rec_lst = [ rec.title() for rec in os.listdir(rec_dir)
                    if os.path.isdir(rec_dir + rec) and rec[0] != u"." ]
        return rec_lst
    else:
        return -1

def GetResourceFiles(resource, trim=True, get_all=False,
                     suffix=None, title=True):
    """Gets a list of resource files from a directory and trims the
    file extentions from the names if trim is set to True (default).
    If the get_all parameter is set to True the function will return
    a set of unique items by looking up both the user and system level
    files and combining them, the default behavior returns the user
    level files if they exist or the system level files if the
    user ones do not exist.
    @param resource: name of config directory to look in (i.e cache)
    @keyword trim: trim file extensions or not
    @keyword get_all: get a set of both system/user files or just user level
    @keyword suffix: Get files that have the specified suffix or all (default)
    @keyword title: Titlize the results

    """
    rec_dir = ResolvConfigDir(resource)
    if get_all:
        rec_dir2 = ResolvConfigDir(resource, True)
    rec_list = list()
    if not os.path.exists(rec_dir):
        return -1
    else:
        recs = os.listdir(rec_dir)
        if get_all and os.path.exists(rec_dir2):
            recs.extend(os.listdir(rec_dir2))

        for rec in recs:
            if os.path.isfile(rec_dir + rec) or \
              (get_all and os.path.isfile(rec_dir2 + rec)):

                # If a suffix was specified only keep files that match
                if suffix is not None:
                    if not rec.endswith(suffix):
                        continue

                # Trim the last part of an extension if one exists
                if trim:
                    rec = ".".join(rec.split(u".")[:-1]).strip()

                # Make the resource name a title if requested
                if title and len(rec):
                    rec = rec[0].upper() + rec[1:]

                if len(rec):
                    rec_list.append(rec)
        rec_list.sort()
        
        return list(set(rec_list))

def GetAllEncodings():
    """Get all encodings found on the system
    @return: list of strings

    """
    elist = encodings.aliases.aliases.values()
    elist = list(set(elist))
    elist.sort()
    elist = [ enc for enc in elist if not enc.endswith('codec') ]
    return elist

def Log(msg):
    """Push the message to the apps log
    @param msg: message string to log

    """
    wx.GetApp().GetLog()(msg)

def GetProxyOpener(proxy_set):
    """Get a urlopener for use with a proxy
    @param proxy_set: proxy settings to use

    """
    Log("[util][info] Making proxy opener with %s" % str(proxy_set))
    proxy_info = dict(proxy_set)
    auth_str = "%(uname)s:%(passwd)s@%(url)s"
    url = proxy_info['url']
    if url.startswith('http://'):
        auth_str = "http://" + auth_str
        proxy_info['url'] = url.replace('http://', '')
    else:
        pass

    if len(proxy_info.get('port', '')):
        auth_str = auth_str + ":%(port)s"

    proxy_info['passwd'] = ed_crypt.Decrypt(proxy_info['passwd'],
                                            proxy_info['pid'])
    Log("[util][info] Formatted proxy request: %s" % \
        (auth_str.replace('%(passwd)s', '****') % proxy_info))
    proxy = urllib2.ProxyHandler({"http" : auth_str % proxy_info})
    opener = urllib2.build_opener(proxy, urllib2.HTTPHandler)
    return opener

#---- GUI helper functions ----#

def SetWindowIcon(window):
    """Sets the given windows icon to be the programs
    application icon.
    @param window: window to set app icon for

    """
    try:
        if wx.Platform == "__WXMSW__":
            ed_icon = ed_glob.CONFIG['SYSPIX_DIR'] + u"editra.ico"
            window.SetIcon(wx.Icon(ed_icon, wx.BITMAP_TYPE_ICO))
        else:
            ed_icon = ed_glob.CONFIG['SYSPIX_DIR'] + u"editra.png"
            window.SetIcon(wx.Icon(ed_icon, wx.BITMAP_TYPE_PNG))
    finally:
        pass

#-----------------------------------------------------------------------------#

class IntValidator(wx.PyValidator):
    """A Generic integer validator"""
    def __init__(self, min_=0, max_=0):
        """Initialize the validator
        @keyword min: min value to accept
        @keyword max: max value to accept

        """
        wx.PyValidator.__init__(self)
        self._min = min_
        self._max = max_

        # Event managment
        self.Bind(wx.EVT_CHAR, self.OnChar)

    def Clone(self):
        """Clones the current validator
        @return: clone of this object

        """
        return IntValidator(self._min, self._max)

    def Validate(self, win):
        """Validate an window value
        @param win: window to validate

        """
        val = win.GetValue()
        return val.isdigit()

    def OnChar(self, event):
        """Process values as they are entered into the control
        @param event: event that called this handler

        """
        key = event.GetKeyCode()
        if key < wx.WXK_SPACE or key == wx.WXK_DELETE or \
           key > 255 or chr(key) in '0123456789':
            event.Skip()
            return

        if not wx.Validator_IsSilent():
            wx.Bell()

        return
