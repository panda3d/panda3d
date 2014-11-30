"""PySlices combines the slices and filling into one control."""

__author__ = "David N. Mashburn <david.n.mashburn@gmail.com> / "
__author__ += "Patrick K. O'Brien <pobrien@orbtech.com>"
__cvsid__ = "$Id: crustslices.py 44235 2007-01-17 23:05:14Z RD $"
__revision__ = "$Revision: 44235 $"[11:-2]

import wx

import os
import pprint
import re
import sys

import dispatcher
import crust
import document
import editwindow
import editor
from filling import Filling
import frame
from sliceshell import SlicesShell
from version import VERSION


class CrustSlices(crust.Crust):
    """Slices based on SplitterWindow."""

    name = 'Slices'
    revision = __revision__
    sashoffset = 300

    def __init__(self, parent, id=-1, pos=wx.DefaultPosition,
                 size=wx.DefaultSize, style=wx.SP_3D|wx.SP_LIVE_UPDATE,
                 name='Slices Window', rootObject=None, rootLabel=None,
                 rootIsNamespace=True, intro='', locals=None,
                 InterpClass=None,
                 startupScript=None, execStartupScript=True,
                 showPySlicesTutorial=True,
                 enableShellMode=False, hideFoldingMargin=False,
                 *args, **kwds):
        """Create CrustSlices instance."""
        wx.SplitterWindow.__init__(self, parent, id, pos, size, style, name)

        # Turn off the tab-traversal style that is automatically
        # turned on by wx.SplitterWindow.  We do this because on
        # Windows the event for Ctrl-Enter is stolen and used as a
        # navigation key, but the SlicesShell window uses it to insert lines.
        style = self.GetWindowStyle()
        self.SetWindowStyle(style & ~wx.TAB_TRAVERSAL)
        
        self.sliceshell = SlicesShell(parent=self, introText=intro,
                                 locals=locals, InterpClass=InterpClass,
                                 startupScript=startupScript,
                                 execStartupScript=execStartupScript,
                                 showPySlicesTutorial=showPySlicesTutorial,
                                 enableShellMode=enableShellMode,
                                 hideFoldingMargin=hideFoldingMargin,
                                 *args, **kwds)
        
        self.editor = self.sliceshell
        self.shell = self.sliceshell
        if rootObject is None:
            rootObject = self.sliceshell.interp.locals
        self.notebook = wx.Notebook(parent=self, id=-1)
        self.sliceshell.interp.locals['notebook'] = self.notebook
        self.filling = Filling(parent=self.notebook,
                               rootObject=rootObject,
                               rootLabel=rootLabel,
                               rootIsNamespace=rootIsNamespace)
        # Add 'filling' to the interpreter's locals.
        self.sliceshell.interp.locals['filling'] = self.filling
        self.notebook.AddPage(page=self.filling, text='Namespace', select=True)
        
        self.display = crust.Display(parent=self.notebook)
        self.notebook.AddPage(page=self.display, text='Display')
        # Add 'pp' (pretty print) to the interpreter's locals.
        self.sliceshell.interp.locals['pp'] = self.display.setItem
        self.display.nbTab = self.notebook.GetPageCount()-1
        
        self.calltip = crust.Calltip(parent=self.notebook,ShellClassName='SlicesShell')
        self.notebook.AddPage(page=self.calltip, text='Calltip')
        
        self.sessionlisting = crust.SessionListing(parent=self.notebook,ShellClassName='SlicesShell')
        self.notebook.AddPage(page=self.sessionlisting, text='History')
        
        self.dispatcherlisting = crust.DispatcherListing(parent=self.notebook)
        self.notebook.AddPage(page=self.dispatcherlisting, text='Dispatcher')

        
        # Initialize in an unsplit mode, and check later after loading
        # settings if we should split or not.
        self.sliceshell.Hide()
        self.notebook.Hide()
        self.Initialize(self.sliceshell)
        self._shouldsplit = True
        wx.CallAfter(self._CheckShouldSplit)
        self.SetMinimumPaneSize(100)

        self.Bind(wx.EVT_SIZE, self.SplitterOnSize)
        self.Bind(wx.EVT_SPLITTER_SASH_POS_CHANGED, self.OnChanged)
        self.Bind(wx.EVT_SPLITTER_DCLICK, self.OnSashDClick)

class CrustSlicesFrame(crust.CrustFrame):
    """Frame containing all the PySlices components."""

    name = 'SliceFrame'
    revision = __revision__


    def __init__(self, parent=None, id=-1, title='PySlices',
                 pos=wx.DefaultPosition, size=wx.DefaultSize,
                 style=wx.DEFAULT_FRAME_STYLE,
                 rootObject=None, rootLabel=None, rootIsNamespace=True,
                 locals=None, InterpClass=None,
                 config=None, dataDir=None, filename=None,
                 *args, **kwds):
        """Create CrustFrame instance."""
        frame.Frame.__init__(self, parent, id, title, pos, size, style,
                             shellName='PySlices')
        frame.ShellFrameMixin.__init__(self, config, dataDir)
        
        if size == wx.DefaultSize:
            self.SetSize((800, 600))
        
        intro = 'PySlices %s - The Flakiest Python Shell... Cut up!' % VERSION
        
        self.SetStatusText(intro.replace('\n', ', '))
        self.crust = CrustSlices(parent=self, intro=intro,
                                 rootObject=rootObject,
                                 rootLabel=rootLabel,
                                 rootIsNamespace=rootIsNamespace,
                                 locals=locals,
                                 InterpClass=InterpClass,
                                 startupScript=self.startupScript,
                                 execStartupScript=self.execStartupScript,
                                 showPySlicesTutorial=self.showPySlicesTutorial,
                                 enableShellMode=self.enableShellMode,
                                 hideFoldingMargin=self.hideFoldingMargin,
                                 *args, **kwds)
        self.sliceshell = self.crust.sliceshell
        self.buffer = self.sliceshell.buffer
        # Override the filling so that status messages go to the status bar.
        self.crust.filling.tree.setStatusText = self.SetStatusText
        
        # Override the shell so that status messages go to the status bar.
        self.sliceshell.setStatusText = self.SetStatusText
        
        self.sliceshell.SetFocus()
        self.LoadSettings()
        
        self.currentDirectory = os.path.expanduser('~')
        
        if filename!=None:
            self.bufferOpen(filename)
        
        self.Bind(wx.EVT_IDLE, self.OnIdle)

    def OnClose(self, event):
        """Event handler for closing."""
        self.bufferClose()
    
    def OnAbout(self, event):
        """Display an About window."""
        title = 'About PySlices'
        text = 'PySlices %s\n\n' % VERSION + \
               'Yet another Python shell, only flakier.\n\n' + \
               'Half-baked by Patrick K. O\'Brien,\n' + \
               'the other half is still in the oven.\n\n' + \
               'Shell Revision: %s\n' % self.sliceshell.revision + \
               'Interpreter Revision: %s\n\n' % self.sliceshell.interp.revision + \
               'Platform: %s\n' % sys.platform + \
               'Python Version: %s\n' % sys.version.split()[0] + \
               'wxPython Version: %s\n' % wx.VERSION_STRING + \
               ('\t(%s)\n' % ", ".join(wx.PlatformInfo[1:]))
        dialog = wx.MessageDialog(self, text, title,
                                  wx.OK | wx.ICON_INFORMATION)
        dialog.ShowModal()
        dialog.Destroy()

    def OnEnableShellMode(self,event):
        """Change between Slices Mode and Shell Mode"""
        frame.Frame.OnEnableShellMode(self,event)
        self.sliceshell.ToggleShellMode(self.enableShellMode)
    
    def OnHideFoldingMargin(self,event):
        """Change between Slices Mode and Shell Mode"""
        frame.Frame.OnHideFoldingMargin(self,event)
        self.sliceshell.ToggleFoldingMargin(self.hideFoldingMargin)

    # Stolen Straight from editor.EditorFrame
    # Modified a little... :)
    # ||
    # \/
    def OnIdle(self, event):
        """Event handler for idle time."""
        self._updateTitle()
        event.Skip()
    
    def _updateTitle(self):
        """Show current title information."""
        title = self.GetTitle()
        if self.bufferHasChanged():
            if title.startswith('* '):
                pass
            else:
                self.SetTitle('* ' + title)
        else:
            if title.startswith('* '):
                self.SetTitle(title[2:])
    
    def hasBuffer(self):
        """Return True if there is a current buffer."""
        if self.buffer:
            return True
        else:
            return False

    def bufferClose(self):
        """Close buffer."""
        if self.buffer.hasChanged():
            cancel = self.bufferSuggestSave()
            if cancel:
                #event.Veto()
                return cancel
        self.SaveSettings()
        self.crust.sliceshell.destroy()
        self.bufferDestroy()
        self.Destroy()
        
        return False

    def bufferCreate(self, filename=None):
        """Create new buffer."""
        self.bufferDestroy()
        buffer = Buffer()
        self.panel = panel = wx.Panel(parent=self, id=-1)
        panel.Bind (wx.EVT_ERASE_BACKGROUND, lambda x: x)        
        editor = Editor(parent=panel)
        panel.editor = editor
        sizer = wx.BoxSizer(wx.VERTICAL)
        sizer.Add(editor.window, 1, wx.EXPAND)
        panel.SetSizer(sizer)
        panel.SetAutoLayout(True)
        sizer.Layout()
        buffer.addEditor(editor)
        buffer.open(filename)
        self.setEditor(editor)
        self.editor.setFocus()
        self.SendSizeEvent()
        

    def bufferDestroy(self):
        """Destroy the current buffer."""
        if self.buffer:
            self.editor = None
            self.buffer = None


    def bufferHasChanged(self):
        """Return True if buffer has changed since last save."""
        if self.buffer:
            return self.buffer.hasChanged()
        else:
            return False

    def bufferNew(self):
        """Create new buffer."""
        cancel = self.bufferSuggestSave()
        if cancel:
            return cancel
        self.sliceshell.clear()
        self.SetTitle( 'PySlices')
        self.sliceshell.NeedsCheckForSave=False
        self.sliceshell.SetSavePoint()
        self.buffer.doc = document.Document()
        self.buffer.name = 'This shell'
        self.buffer.modulename = self.buffer.doc.filebase
        #self.bufferCreate()
        cancel = False
        return cancel

    def bufferOpen(self,file=None):
        """Open file in buffer."""
        if self.bufferHasChanged():
            cancel = self.bufferSuggestSave()
            if cancel:
                return cancel
        
        if file==None:
            file=wx.FileSelector('Open a PySlices File',
                                 wildcard='*.pyslices',
                                 default_path=self.currentDirectory)
        if file!=None and file!=u'':
            fid=open(file,'r')
            self.sliceshell.LoadPySlicesFile(fid)
            fid.close()
            self.currentDirectory = os.path.split(file)[0]
            self.SetTitle( os.path.split(file)[1] + ' - PySlices')
            self.sliceshell.NeedsCheckForSave=False
            self.sliceshell.SetSavePoint()
            self.buffer.doc = document.Document(file)
            self.buffer.name = self.buffer.doc.filename
            self.buffer.modulename = self.buffer.doc.filebase
            self.sliceshell.ScrollToLine(0)
        return
    
##     def bufferPrint(self):
##         """Print buffer."""
##         pass

##     def bufferRevert(self):
##         """Revert buffer to version of file on disk."""
##         pass
    
    # was self.buffer.save(self): # """Save buffer."""
    def simpleSave(self,confirmed=False):
        filepath = self.buffer.doc.filepath
        self.buffer.confirmed = confirmed
        if not filepath:
            return  # XXX Get filename
        if not os.path.exists(filepath):
            self.buffer.confirmed = True
        if not self.buffer.confirmed:
            self.buffer.confirmed = self.buffer.overwriteConfirm(filepath)
        if self.buffer.confirmed:
            try:
                fid = open(filepath, 'wb')
                self.sliceshell.SavePySlicesFile(fid)
            finally:
                if fid:
                    fid.close()
            self.sliceshell.SetSavePoint()
            self.SetTitle( os.path.split(filepath)[1] + ' - PySlices')
            self.sliceshell.NeedsCheckForSave=False
    
    def bufferSave(self):
        """Save buffer to its file."""
        if self.buffer.doc.filepath:
            # self.buffer.save()
            self.simpleSave(confirmed=True)
            cancel = False
        else:
            cancel = self.bufferSaveAs()
        return cancel

    def bufferSaveAs(self):
        """Save buffer to a new filename."""
        if self.bufferHasChanged() and self.buffer.doc.filepath:
            cancel = self.bufferSuggestSave()
            if cancel:
                return cancel
        filedir = ''
        if self.buffer and self.buffer.doc.filedir:
            filedir = self.buffer.doc.filedir
        result = editor.saveSingle(title='Save PySlices File',directory=filedir,
                                   wildcard='PySlices Files (*.pyslices)|*.pyslices')
        if result.path not in ['',None]:
            if result.path[-9:]!=".pyslices":
                result.path+=".pyslices"
            
            self.buffer.doc = document.Document(result.path)
            self.buffer.name = self.buffer.doc.filename
            self.buffer.modulename = self.buffer.doc.filebase
            self.simpleSave(confirmed=True) # allow overwrite
            cancel = False
        else:
            cancel = True
        return cancel
    
    def bufferSaveACopy(self):
        """Save buffer to a new filename."""
        filedir = ''
        if self.buffer and self.buffer.doc.filedir:
            filedir = self.buffer.doc.filedir
        result = editor.saveSingle(title='Save a Copy of PySlices File',directory=filedir,
                                   wildcard='PySlices Files (*.pyslices)|*.pyslices')
        
        if result.path not in ['',None]:
            if result.path[-9:]!=".pyslices":
                result.path+=".pyslices"
            
            # if not os.path.exists(result.path):
            try: # Allow overwrite...
                fid = open(result.path, 'wb')
                self.sliceshell.SavePySlicesFile(fid)
            finally:
                if fid:
                    fid.close()
                
            cancel = False
        else:
            cancel = True
        return cancel
    
    def bufferSuggestSave(self):
        """Suggest saving changes.  Return True if user selected Cancel."""
        result = editor.messageDialog(parent=None,
                               message='%s has changed.\n'
                                       'Would you like to save it first'
                                       '?' % self.buffer.name,
                               title='Save current file?',
                               style=wx.YES_NO | wx.CANCEL | wx.NO_DEFAULT |
                                     wx.CENTRE | wx.ICON_QUESTION )
        if result.positive:
            cancel = self.bufferSave()
        else:
            cancel = result.text == 'Cancel'
        return cancel

    def updateNamespace(self):
        """Update the buffer namespace for autocompletion and calltips."""
        if self.buffer.updateNamespace():
            self.SetStatusText('Namespace updated')
        else:
            self.SetStatusText('Error executing, unable to update namespace')
