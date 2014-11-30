"""Slices is an interactive text control in which a user types in
commands to be sent to the interpreter.  This particular shell is
based on wxPython's wxStyledTextCtrl.

Sponsored by Orbtech - Your source for Python programming expertise.
Slices is a version of shell modified by David Mashburn."""

__author__ = "David N. Mashburn <david.n.mashburn@gmail.com> / "
__author__ += "Patrick K. O'Brien <pobrien@orbtech.com>"
__cvsid__ = "$Id: sliceshell.py 60100 2009-04-12 02:56:29Z RD $"
__revision__ = "$Revision: 60100 $"[11:-2]

import wx
from wx import stc

import keyword
import os
import sys
import time

from buffer import Buffer
import dispatcher
import editor
import editwindow
import document
import frame
from pseudo import PseudoFileIn
from pseudo import PseudoFileOut
from pseudo import PseudoFileErr
from version import VERSION
from magic import magic
from parse import testForContinuations
from path import ls,cd,pwd,sx


sys.ps3 = '<-- '  # Input prompt.
USE_MAGIC=True
# Force updates from long-running commands after this many seconds
PRINT_UPDATE_MAX_TIME=2

NAVKEYS = (wx.WXK_HOME, wx.WXK_END, wx.WXK_LEFT, wx.WXK_RIGHT,
           wx.WXK_UP, wx.WXK_DOWN, wx.WXK_PRIOR, wx.WXK_NEXT)

GROUPING_SELECTING=0
IO_SELECTING = 1

GROUPING_START = 2
GROUPING_START_FOLDED = 3
GROUPING_MIDDLE = 4
GROUPING_END = 5
INPUT_START = 6
INPUT_START_FOLDED = 7
INPUT_MIDDLE = 8
INPUT_END = 9
OUTPUT_START = 10
OUTPUT_START_FOLDED = 11
OUTPUT_MIDDLE = 12
OUTPUT_END = 13

OUTPUT_BG = 14
READLINE_BG = 15
INPUT_READLINE = 16

# Could add C integration right into the markers...
# Non-editable file marker for auto-loaded files...
# Weave VariableInput = 15
# Weave C code = 16
# C code = 17 (only for use with Pyrex)
# Pyrex / Cython code = 18

GROUPING_MASK = ( 1<<GROUPING_START | 1<<GROUPING_START_FOLDED |
                  1<<GROUPING_MIDDLE | 1<<GROUPING_END )

INPUT_MASK = ( 1<<INPUT_START | 1<<INPUT_START_FOLDED |
               1<<INPUT_MIDDLE | 1<<INPUT_END )
OUTPUT_MASK = ( 1<<OUTPUT_START | 1<<OUTPUT_START_FOLDED |
                1<<OUTPUT_MIDDLE | 1<<OUTPUT_END )
IO_MASK = ( INPUT_MASK | OUTPUT_MASK )

IO_START_MASK = ( 1<<INPUT_START | 1<<OUTPUT_START )
IO_START_FOLDED_MASK = ( 1<<INPUT_START_FOLDED | 1<<OUTPUT_START_FOLDED )
IO_ANY_START_MASK = ( 1<<INPUT_START | 1<<OUTPUT_START |
                      1<<INPUT_START_FOLDED | 1<<OUTPUT_START_FOLDED )
IO_MIDDLE_MASK = ( 1<<INPUT_MIDDLE | 1<<OUTPUT_MIDDLE )
IO_END_MASK = ( 1<<INPUT_END | 1<<OUTPUT_END )

usrBinEnvPythonText = '#!/usr/bin/env python\n'
pyslicesFormatHeaderText = ['#PySlices Save Format Version 1.1 (PySlices v0.9.7.8 and later)\n',
                            '#PySlices Save Format Version 1.2 (PySlices v0.9.8 and later)\n']
groupingStartText = '#PySlices Marker Information -- Begin Grouping Slice\n'
inputStartText = '#PySlices Marker Information -- Begin Input Slice\n'
outputStartText = '#PySlices Marker Information -- Begin Output Slice\n'

tutorialText = """

                            Tutorial!!!
------------------------------------------------------------------------
PySlices is the newest member of the Py suite!
It is a modified version of PyCrust that supports multi-line commands.

Input and output are contained in "Slices" shown as markers in the left margin.
Input Slices have RED margins (active, editable).
Output Slices have BLUE margins (frozen, not editable).

Commands in slices can be on more than one line, as with Sage or Mathematica.
For example, the command:
a=1
b=2
print a+b
will all run in sequence, much like a script.
Try running the above Input Slice by clicking somewhere in its text and
using Ctrl-Return, Shift-Return, or Numpad Enter to execute.
Previous commands (Old Slices) can be re-edited and run again in place.

Slices can also be:
 * selceted (click on the margin, Shift-click for multiple selection)
 * folded (click the margin twice)
 * selected and deleted (hit delete while selected)
 * divided (Ctrl-D)
 * merged (Ctrl-M while selecting adjacent, like-colored slices)

Try deleting the slice above this one by clicking on the red margin.

If you want a more traditional shell feel, try enabling "Shell Mode" in
"Options->Settings->Shell Mode" (or try PyCrust).
In Shell Mode, two returns in a row executes the command, and
    Ctrl-Return and Shift-Return always print newlines.

Saving and opening "sessions" is now supported!  This is a little
different to other shells where the history is saved.  With PySlices, 
the whole document is saved in a simple text format!

To disable this Tutorial on startup, uncheck it in the menu at:
"Options->Startup->Show PySlices tutorial"

PySlices may not be the best thing since sliced bread, but
I hope it makes using Python a little bit sweeter!
"""

class SlicesShellFrame(frame.Frame, frame.ShellFrameMixin):
    """Frame containing the sliceshell component."""

    name = 'SlicesShell Frame'
    revision = __revision__

    def __init__(self, parent=None, id=-1, title='PySlicesShell',
                 pos=wx.DefaultPosition, size=wx.DefaultSize,
                 style=wx.DEFAULT_FRAME_STYLE, locals=None,
                 InterpClass=None,
                 config=None, dataDir=None, filename=None,
                 *args, **kwds):
        """Create SlicesShellFrame instance."""
        frame.Frame.__init__(self, parent, id, title, pos, size, style,shellName='PySlices')
        frame.ShellFrameMixin.__init__(self, config, dataDir)
        
        if size == wx.DefaultSize:
            self.SetSize((750, 525))
        
        intro = 'PySlices %s - The Flakiest Python Shell... Cut Up!' % VERSION
        self.SetStatusText(intro.replace('\n', ', '))
        self.sliceshell = SlicesShell(parent=self, id=-1, introText=intro,
                               locals=locals, InterpClass=InterpClass,
                               startupScript=self.startupScript,
                               execStartupScript=self.execStartupScript,
                               showPySlicesTutorial=self.showPySlicesTutorial,
                               enableShellMode=self.enableShellMode,
                               hideFoldingMargin=self.hideFoldingMargin,
                               *args, **kwds)
        self.buffer = self.sliceshell.buffer

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
        # This isn't working the way I want, but I'll leave it for now.
        #if self.sliceshell.waiting:
        #    if event.CanVeto():
        #        event.Veto(True)
        #else:
        #    # TODO: Add check for saving
        #    self.SaveSettings()
        #    self.sliceshell.destroy()
        #    self.Destroy()

    def OnAbout(self, event):
        """Display an About window."""
        title = 'About PySliceShell'
        text = 'PySliceShell %s\n\n' % VERSION + \
               'Yet another Python shell, only flakier.\n\n' + \
               'Half-baked by Patrick K. O\'Brien,\n' + \
               'the other half is still in the oven.\n\n' + \
               'Shell Revision: %s\n' % self.shell.revision + \
               'Interpreter Revision: %s\n\n' % self.shell.interp.revision + \
               'Platform: %s\n' % sys.platform + \
               'Python Version: %s\n' % sys.version.split()[0] + \
               'wxPython Version: %s\n' % wx.VERSION_STRING + \
               ('\t(%s)\n' % ", ".join(wx.PlatformInfo[1:])) 
        dialog = wx.MessageDialog(self, text, title,
                                  wx.OK | wx.ICON_INFORMATION)
        dialog.ShowModal()
        dialog.Destroy()


    def OnHelp(self, event):
        """Show a help dialog."""
        frame.ShellFrameMixin.OnHelp(self, event)


    def LoadSettings(self):
        if self.config is not None:
            frame.ShellFrameMixin.LoadSettings(self)
            frame.Frame.LoadSettings(self, self.config)
            self.sliceshell.LoadSettings(self.config)

    def SaveSettings(self, force=False):
        if self.config is not None:
            frame.ShellFrameMixin.SaveSettings(self,force)
            if self.autoSaveSettings or force:
                frame.Frame.SaveSettings(self, self.config)
                self.sliceshell.SaveSettings(self.config)

    def DoSaveSettings(self):
        if self.config is not None:
            self.SaveSettings(force=True)
            self.config.Flush()
    
    def OnEnableShellMode(self,event):
        """Change between Slices Mode and Shell Mode"""
        frame.Frame.OnEnableShellMode(self,event)
        self.sliceshell.ToggleShellMode(self.enableShellMode)
    
    def OnHideFoldingMargin(self,event):
        """Change between Slices Mode and Shell Mode"""
        frame.Frame.OnHideFoldingMargin(self,event)
        self.sliceshell.ToggleFoldingMargin(self.hideFoldingMargin)
    # Copied Straight from crustslices.py (update both with any changes...)
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
        self.sliceshell.destroy()
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
        #self.bufferCreate()
        self.clear()
        self.SetTitle( 'PySlices')
        self.sliceshell.NeedsCheckForSave=False
        self.sliceshell.SetSavePoint()
        self.buffer.doc = document.Document()
        self.buffer.name = 'This shell'
        self.buffer.modulename = self.buffer.doc.filebase
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



# TODO : Update the help text
HELP_TEXT = """\
* Key bindings:
Home              Go to the beginning of the line.
End               Go to the end of the line.
Shift+Home        Select to the beginning of the line.
Shift+End         Select to the end of the line.
Ctrl-Home         Jump to the beginning of the slice;
                  If already there, jump to beginning of previous slice
Ctrl-End          Jump to the end of the slice;
                  If already there, jump to end of next slice
Ctrl-PageUp       Jump to the beginning of the shell
Ctrl-PageDown     Jump to the end of the shell
Ctrl+C            Copy selected text, removing prompts.
Ctrl+Shift+C      Copy selected text, retaining prompts.
Alt+C             Copy to the clipboard, including prefixed prompts.
Ctrl+X            Cut selected text.
Ctrl+V            Paste from clipboard.
Ctrl+Shift+V      Paste and run multiple commands from clipboard.
Ctrl+Up Arrow     Retrieve Previous History item.
Alt+P             Retrieve Previous History item.
Ctrl+Down Arrow   Retrieve Next History item.
Alt+N             Retrieve Next History item.
Shift+Up Arrow    Insert Previous History item.
Shift+Down Arrow  Insert Next History item.
F8                Command-completion of History item.
                  (Type a few characters of a previous command and press F8.)
Ctrl+]            Increase font size.
Ctrl+[            Decrease font size.
Ctrl+=            Default font size.

Ctrl-Space        Show Auto Completion.
Ctrl-Shift-Space  Show Call Tip.
Ctrl-Shift-H      Complete Text from History.

Ctrl+F            Search 
Ctrl+G            Search next
F12               on/off "free-edit" mode
                  For testing only -- This does not preserve markers!

In "Slices Mode":
Return            Insert new line
Enter (Numpad)    Run command in slice
Ctrl+Return       ""
Shift+Return      ""

In "Shell Mode":
Return or Enter   Insert a new line
Ctrl+Return       ""
Shift+Return      ""
2 Returns in a row   Run command in slice
"""

class SlicesShellFacade:
    """Simplified interface to all shell-related functionality.

    This is a semi-transparent facade, in that all attributes of other
    are accessible, even though only some are visible to the user."""

    name = 'SlicesShell Interface'
    revision = __revision__

    def __init__(self, other):
        """Create a SlicesShellFacade instance."""
        d = self.__dict__
        d['other'] = other
        d['helpText'] = HELP_TEXT
        d['this'] = other.this

    def help(self):
        """Display some useful information about how to use the slices shell."""
        self.write(self.helpText,type='Output')

    def __getattr__(self, name):
        if hasattr(self.other, name):
            return getattr(self.other, name)
        else:
            raise AttributeError, name

    def __setattr__(self, name, value):
        if self.__dict__.has_key(name):
            self.__dict__[name] = value
        elif hasattr(self.other, name):
            setattr(self.other, name, value)
        else:
            raise AttributeError, name

    def _getAttributeNames(self):
        """Return list of magic attributes to extend introspection."""
        list = [
            'about',
            'ask',
            'autoCallTip',
            'autoComplete',
            'autoCompleteAutoHide',
            'autoCompleteCaseInsensitive',
            'autoCompleteIncludeDouble',
            'autoCompleteIncludeMagic',
            'autoCompleteIncludeSingle',
            'callTipInsert',
            'clear',
            'pause',
            'prompt',
            'quit',
            'redirectStderr',
            'redirectStdin',
            'redirectStdout',
            'run',
            'runfile',
            'wrap',
            'zoom',
            ]
        list.sort()
        return list

DISPLAY_TEXT="""
Author: %r
Py Version: %s
Py Slices Shell Revision: %s
Py Interpreter Revision: %s
Python Version: %s
wxPython Version: %s
wxPython PlatformInfo: %s
Platform: %s"""

class SlicesShell(editwindow.EditWindow):
    """Notebook Shell based on StyledTextCtrl."""

    name = 'SlicesShell'
    revision = __revision__

    def __init__(self, parent, id=-1, pos=wx.DefaultPosition,
                 size=wx.DefaultSize, style=wx.CLIP_CHILDREN,
                 introText='', locals=None, InterpClass=None,
                 startupScript=None, execStartupScript=True,
                 showPySlicesTutorial=True,enableShellMode=False,
                 hideFoldingMargin=False, *args, **kwds):
        """Create Shell instance."""
        editwindow.EditWindow.__init__(self, parent, id, pos, size, style)
        self.wrap()
        if locals is None:
            import __main__
            locals = __main__.__dict__

        # Grab these so they can be restored by self.redirect* methods.
        self.stdin = sys.stdin
        self.stdout = sys.stdout
        self.stderr = sys.stderr

        # Import a default interpreter class if one isn't provided.
        if InterpClass == None:
            from interpreter import Interpreter
        else:
            Interpreter = InterpClass

        # Create a replacement for stdin.
        self.reader = PseudoFileIn(self.readline, self.readlines)
        self.reader.input = ''
        self.reader.isreading = False

        # Set up the interpreter.
        self.interp = Interpreter(locals=locals,
                                  rawin=self.raw_input,
                                  stdin=self.reader,
                                  stdout=PseudoFileOut(self.writeOut),
                                  stderr=PseudoFileErr(self.writeErr),
                                  *args, **kwds)

        # Set up the buffer.
        self.buffer = Buffer()
        self.id = self.GetId()
        self.buffer.addEditor(self)
        self.buffer.name='This shell'
        self.NeedsCheckForSave=False
        
        # Find out for which keycodes the interpreter will autocomplete.
        self.autoCompleteKeys = self.interp.getAutoCompleteKeys()

        # Keep track of the last non-continuation prompt positions.
        # Removed all references to these... solved a lot of odd bugs...
        # self.promptPosStart = 0
        # self.promptPosEnd = 0

        # Keep track of multi-line commands.
        self.more = False
        
        # Use Margins to track input / output / slice number
        self.margins = True
        
        # For use with forced updates during long-running scripts
        self.lastUpdate=None
        
        if self.margins:
            # margin 1 is already defined for the line numbers
            #  may eventually change it back to 0 like it ought to be...
            self.SetMarginType(2, stc.STC_MARGIN_SYMBOL)
            self.SetMarginType(3, stc.STC_MARGIN_SYMBOL)
            self.SetMarginType(4, stc.STC_MARGIN_SYMBOL)
            self.SetMarginWidth(2, 22)
            self.SetMarginWidth(3, 22)
            self.SetMarginWidth(4, 12)
            self.SetMarginSensitive(2,True)
            self.SetMarginSensitive(3,True)
            self.SetMarginSensitive(4,True)
            self.SetProperty("fold", "1")
            # tabs are bad, use spaces
            self.SetProperty("tab.timmy.whinge.level", "4")
            self.SetMargins(0,0)
            
            
            self.SetMarginMask(2, GROUPING_MASK | 1<<GROUPING_SELECTING )
            # Display Markers -24...
            self.SetMarginMask(3, IO_MASK | 1<<IO_SELECTING | 1<<READLINE_BG | 1<<INPUT_READLINE )
            self.SetMarginMask(4, stc.STC_MASK_FOLDERS)
            # Set the mask for the line markers, too...
            self.SetMarginMask(1, 0)
            
            if hideFoldingMargin:
                self.SetMarginWidth(4, 0)
            self.hideFoldingMargin=hideFoldingMargin
            
            sel_color="#E0E0E0"
            grouping_color="black"
            input_color="red"
            output_color="blue"
            
            self.MarkerDefine(GROUPING_SELECTING,    stc.STC_MARK_FULLRECT,
                              sel_color, sel_color)
            self.MarkerDefine(IO_SELECTING,          stc.STC_MARK_FULLRECT,
                              sel_color, sel_color)
            
            self.MarkerDefine(GROUPING_START,        stc.STC_MARK_BOXMINUS,
                              "white", grouping_color)
            self.MarkerDefine(GROUPING_START_FOLDED, stc.STC_MARK_BOXPLUS,
                              "white", grouping_color)
            self.MarkerDefine(GROUPING_MIDDLE,       stc.STC_MARK_VLINE,
                              "white", grouping_color)
            self.MarkerDefine(GROUPING_END,          stc.STC_MARK_LCORNER,
                              "white", grouping_color)
            
            self.MarkerDefine(READLINE_BG, stc.STC_MARK_FULLRECT,
                              wx.Color(191,191,191), wx.Color(191,191,191))
            self.MarkerDefine(INPUT_READLINE, stc.STC_MARK_CHARACTER+ord('<'),
                              input_color, wx.Color(191,191,191))
            
            if enableShellMode:
                self.mode='ShellMode'
            else:
                self.mode='SlicesMode'
            
            self.execOnNextReturn=False
            if self.mode=='SlicesMode':
                self.MarkerDefine(INPUT_START,        stc.STC_MARK_BOXMINUS,
                                  "white", input_color)
                self.MarkerDefine(INPUT_START_FOLDED, stc.STC_MARK_BOXPLUS,
                                  "white", input_color)
                self.MarkerDefine(INPUT_MIDDLE,       stc.STC_MARK_VLINE,
                                  "white", input_color)
                self.MarkerDefine(INPUT_END,          stc.STC_MARK_LCORNER,
                                  "white", input_color)
            elif self.mode=='ShellMode':
                self.MarkerDefine(INPUT_START,        stc.STC_MARK_ARROWS,
                                  input_color, "white")
                self.MarkerDefine(INPUT_START_FOLDED, stc.STC_MARK_BOXPLUS,
                                  "white", input_color)
                self.MarkerDefine(INPUT_MIDDLE,       stc.STC_MARK_DOTDOTDOT,
                                  input_color, "white")
                self.MarkerDefine(INPUT_END,          stc.STC_MARK_DOTDOTDOT,
                                  input_color, "white")
            
            self.MarkerDefine(OUTPUT_START,           stc.STC_MARK_BOXMINUS,
                              "white", output_color)
            self.MarkerDefine(OUTPUT_START_FOLDED,    stc.STC_MARK_BOXPLUS,
                              "white", output_color)
            self.MarkerDefine(OUTPUT_MIDDLE,          stc.STC_MARK_VLINE,
                              "white", output_color)
            self.MarkerDefine(OUTPUT_END,             stc.STC_MARK_LCORNER,
                              "white", output_color)
            
            self.MarkerDefine(OUTPUT_BG,             stc.STC_MARK_BACKGROUND,
                              "white", wx.Color(242,242,255))
            
            # Markers for folding margin...
            self.MarkerDefine(stc.STC_MARKNUM_FOLDEROPEN,    stc.STC_MARK_BOXMINUS,
                              "white", "#808080")
            self.MarkerDefine(stc.STC_MARKNUM_FOLDER,        stc.STC_MARK_BOXPLUS,
                              "white", "#808080")
            self.MarkerDefine(stc.STC_MARKNUM_FOLDERSUB,     stc.STC_MARK_VLINE,
                              "white", "#808080")
            self.MarkerDefine(stc.STC_MARKNUM_FOLDERTAIL,    stc.STC_MARK_LCORNER,
                              "white", "#808080")
            self.MarkerDefine(stc.STC_MARKNUM_FOLDEREND,     stc.STC_MARK_BOXPLUSCONNECTED,
                              "white", "#808080")
            self.MarkerDefine(stc.STC_MARKNUM_FOLDEROPENMID, stc.STC_MARK_BOXMINUSCONNECTED,
                              "white", "#808080")
            self.MarkerDefine(stc.STC_MARKNUM_FOLDERMIDTAIL, stc.STC_MARK_TCORNER,
                              "white", "#808080")

        # Create the command history.  Commands are added into the
        # front of the list (ie. at index 0) as they are entered.
        # self.historyIndex is the current position in the history; it
        # gets incremented as you retrieve the previous command,
        # decremented as you retrieve the next, and reset when you hit
        # Enter.  self.historyIndex == -1 means you're on the current
        # command, not in the history.
        self.history = []
        self.historyIndex = -1

        #DNM -- disable these markers...
        #seb add mode for "free edit"
        self.noteMode = 0
        #self.MarkerDefine(0,stc.STC_MARK_ROUNDRECT)  # marker for hidden
        self.searchTxt = ""

        # Assign handlers for keyboard events.
        self.Bind(wx.EVT_CHAR, self.OnChar)
        self.Bind(wx.EVT_KEY_DOWN, self.OnKeyDown)
        
        self.Bind(wx.stc.EVT_STC_MARGINCLICK, self.OnMarginClick)
        # TODO : Add a general functions to handle mouse clicks in the
        # TODO:  STC window whose sole purpose is to make it so
        # TODO:  that margin selection becomes unselected...

        # Assign handler for the context menu
        self.Bind(wx.EVT_CONTEXT_MENU, self.OnContextMenu)
        self.Bind(wx.EVT_UPDATE_UI, self.OnUpdateUI)
        
        # Assign handlers for edit events
        self.Bind(wx.EVT_MENU, lambda evt: self.Cut(), id=wx.ID_CUT)
        self.Bind(wx.EVT_MENU, lambda evt: self.Copy(), id=wx.ID_COPY)
        self.Bind(wx.EVT_MENU, lambda evt: self.CopyWithPrompts(), id=frame.ID_COPY_PLUS)
        self.Bind(wx.EVT_MENU, lambda evt: self.Paste(), id=wx.ID_PASTE)
        self.Bind(wx.EVT_MENU, lambda evt: self.PasteAndRun(), id=frame.ID_PASTE_PLUS)
        self.Bind(wx.EVT_MENU, lambda evt: self.SelectAll(), id=wx.ID_SELECTALL)
        self.Bind(wx.EVT_MENU, lambda evt: self.Clear(), id=wx.ID_CLEAR)
        self.Bind(wx.EVT_MENU, lambda evt: self.Undo(), id=wx.ID_UNDO)
        self.Bind(wx.EVT_MENU, lambda evt: self.Redo(), id=wx.ID_REDO)
        
        # Assign handler for idle time.
        self.waiting = False
        self.Bind(wx.EVT_IDLE, self.OnIdle)

        # Display the introductory banner information.
        self.showIntro(introText)
        
        outStart,outEnd,inStart,inMiddle,inEnd = [[],[],[],[],[]]
        
        # Make "executed startup script move to the top..."
        if showPySlicesTutorial:
            self.write(tutorialText,'Output')
            tutStart=5
            testStart=16
            outStart=[tutStart,testStart+3]
            outEnd=[tutStart-1,testStart-1]
            inStart=[testStart]
            inMiddle=[testStart+1]
            inEnd=[testStart+2]
        
        # Assign some pseudo keywords to the interpreter's namespace.
        self.setBuiltinKeywords()

        # Add 'shell' to the interpreter's local namespace.
        self.setLocalShell()
        
        # Do this last so the user has complete control over their
        # environment.  They can override anything they want.
        if execStartupScript:
            if startupScript is None:
                startupScript = os.environ.get('PYTHONSTARTUP')
            self.execStartupScript(startupScript)
        else:
            self.prompt()
        
        outStart+=[0]
        outEnd+=[self.GetLineCount()-2]
        inStart+=[self.GetLineCount()-1]
        # Set all the line markers to the proper initial states...
        for i in range(self.GetLineCount()):
            self.clearGroupingMarkers(i)
            self.clearIOMarkers(i)
            if i in outStart:
                self.MarkerAdd(i,GROUPING_START)
                self.MarkerAdd(i,OUTPUT_START)
                # Background color is confusing for tutorial... skip it!
                #self.MarkerAdd(i,OUTPUT_BG)
            elif i in outEnd:
                self.MarkerAdd(i,GROUPING_END)
                self.MarkerAdd(i,OUTPUT_END)
                #self.MarkerAdd(i,OUTPUT_BG)
            elif i in inStart:
                self.MarkerAdd(i,GROUPING_START)
                self.MarkerAdd(i,INPUT_START)
            elif i in inMiddle:
                self.MarkerAdd(i,GROUPING_MIDDLE)
                self.MarkerAdd(i,INPUT_MIDDLE)
            elif i in inEnd:
                self.MarkerAdd(i,GROUPING_END)
                self.MarkerAdd(i,INPUT_END)
            else:
                self.MarkerAdd(i,GROUPING_MIDDLE)
                self.MarkerAdd(i,OUTPUT_MIDDLE)
                #self.MarkerAdd(i,OUTPUT_BG)
        
        self.SliceSelection=False
        self.runningSlice=None
        
        ## NOTE:  See note at bottom of this file...
        ## #seb: File drag and drop
        ## self.SetDropTarget( FileDropTarget(self) )
        
        #ADD UNDO
        # Everywhere "ADD UNDO" appears, there is new code to handle markers
        self.EmptyUndoBuffer()
        
        wx.CallAfter(self.ScrollToLine, 0)

    def ToggleShellMode(self,enableShellMode=None):
        if enableShellMode==None:
            if self.mode=='ShellMode':  self.mode='SlicesMode'
            elif self.mode=='SlicesMode': self.mode='ShellMode'
        elif enableShellMode:
            self.mode='ShellMode'
        else:
            self.mode='SlicesMode'
        
        input_color="red"
        if self.mode=='SlicesMode':
            self.MarkerDefine(INPUT_START,           stc.STC_MARK_BOXMINUS,
                              "white", input_color)
            self.MarkerDefine(INPUT_START_FOLDED,    stc.STC_MARK_BOXPLUS,
                              "white", input_color)
            self.MarkerDefine(INPUT_MIDDLE,          stc.STC_MARK_VLINE,
                              "white", input_color)
            self.MarkerDefine(INPUT_END,             stc.STC_MARK_LCORNER,
                              "white", input_color)
        elif self.mode=='ShellMode':
            self.MarkerDefine(INPUT_START,           stc.STC_MARK_ARROWS,
                              input_color, "white")
            self.MarkerDefine(INPUT_START_FOLDED,    stc.STC_MARK_BOXPLUS,
                              "white", input_color)
            self.MarkerDefine(INPUT_MIDDLE,          stc.STC_MARK_DOTDOTDOT,
                              input_color, "white")
            self.MarkerDefine(INPUT_END,             stc.STC_MARK_DOTDOTDOT,
                              input_color, "white")
    
    def ToggleFoldingMargin(self,hideFoldingMargin=None):
        if hideFoldingMargin==None:
            self.hideFoldingMargin = not self.hideFoldingMargin
        else:
            self.hideFoldingMargin = hideFoldingMargin
        
        if self.hideFoldingMargin:
            self.SetMarginWidth(4, 0)
        else:
            self.SetMarginWidth(4, 12)

    def clearHistory(self):
        self.history = []
        self.historyIndex = -1
        dispatcher.send(signal="SlicesShell.clearHistory")


    def destroy(self):
        del self.interp

    def setFocus(self):
        """Set focus to the slices shell."""
        self.SetFocus()

    def OnIdle(self, event):
        """Free the CPU to do other things."""
        if self.waiting:
            time.sleep(0.05)
        event.Skip()

    def showIntro(self, text=''):
        """Display introductory text in the slices shell."""
        if text:
            self.write(text,type='Output')
        try:
            if self.interp.introText:
                if text and not text.endswith(os.linesep):
                    self.write(os.linesep,type='Output')
                self.write(self.interp.introText,type='Output')
        except AttributeError:
            pass
    
    def setBuiltinKeywords(self):
        """Create pseudo keywords as part of builtins.

        This sets "close", "exit" and "quit" to a helpful string.
        """
        import __builtin__
        __builtin__.close = __builtin__.exit = __builtin__.quit = \
            'Click on the close button to leave the application.'
        __builtin__.cd = cd
        __builtin__.ls = ls
        __builtin__.pwd = pwd
        __builtin__.sx = sx


    def quit(self):
        """Quit the application."""
        # XXX Good enough for now but later we want to send a close event.
        # In the close event handler we can make sure they want to
        # quit.  Other applications, like PythonCard, may choose to
        # hide rather than quit so we should just post the event and
        # let the surrounding app decide what it wants to do.
        self.write('Click on the close button to leave the application.',
                   type='Output')


    def setLocalShell(self):
        """Add 'slicesshell' to locals as reference to ShellFacade instance."""
        self.interp.locals['slicesshell'] = SlicesShellFacade(other=self)


    def execStartupScript(self, startupScript):
        """Execute the user's PYTHONSTARTUP script if they have one."""
        if startupScript and os.path.isfile(startupScript):
            text = 'Startup script executed: ' + startupScript
            self.push('print %r; execfile(%r)' % (text, startupScript))
            self.interp.startupScript = startupScript
        else:
            self.push('')


    def about(self):
        """Display information about Py."""
        text = DISPLAY_TEXT % \
        (__author__, VERSION, self.revision, self.interp.revision,
         sys.version.split()[0], wx.VERSION_STRING, str(wx.PlatformInfo),
         sys.platform)
        self.write(text.strip(),type='Output')
    
    def BreakTextIntoCommands(self,text):
        """Turn a text block into multiple multi-line commands."""
        
        #text = text.lstrip() # This should not be done!
        text = self.fixLineEndings(text)
        text = self.lstripPrompt(text)
        text = text.replace(os.linesep, '\n')
        lines = text.split('\n')
        
        continuations = testForContinuations(text)
        
        if len(continuations)==2: # Error case...
            return None,continuations[1]
        elif len(continuations)==4:
            stringContinuationList,indentationBlockList, \
            lineContinuationList,parentheticalContinuationList = continuations
        
        commands = []
        command = ''
        for j,line in enumerate(lines):
            lstrip = line.lstrip()
            
            # Get the first alnum word:
            first_word=[]
            for i in lstrip:
                if i.isalnum():
                    first_word.append(i)
                else:
                    break
            first_word = ''.join(first_word)
            
            # Continue the command if it is blank, has indentation,
            # starts with else, elif,except, or finally
            # or previous line had a line continuation \
            
            if j==0:
                stringCont = False
                lineCont=False
            else:
                stringCont = stringContinuationList[j-1]
                lineCont = lineContinuationList[j-1]
            
            if line.strip() == '' or lstrip != line or \
               first_word in ['else','elif','except','finally'] or \
               stringCont or lineCont:
                # Multiline command. Add to the command.
                command += '\n'
                command += line
            else:
                # New command.
                if command:
                    # Add the previous command to the list.
                    commands.append(command)
                # Start a new command, which may be multiline.
                command = line
        
        commands.append(command)
        
        return commands
    
    def MarkerSet(self,line,markerBitsSet):
        """MarkerSet is the Set command for MarkerGet"""
        markerBits=self.MarkerGet(line)
        
        numMarkers=14
        for i in range(numMarkers):
            if (markerBitsSet & (1<<i)) and not (markerBits & (1<<i)):
                self.MarkerAdd(line,i)
            elif not (markerBitsSet & (1<<i)) and (markerBits & (1<<i)):
                self.MarkerDelete(line,i)
    def GetGroupingSlice(self,line_num=None):
        """Get the start/stop lines for the slice based on any line in the slice"""
        if line_num==None:
            line_num=self.GetCurrentLine()
        
        num_lines=self.GetLineCount()
        
        for i in range(line_num,-1,-1):
            if self.MarkerGet(i) & (1<<GROUPING_START | 1<<GROUPING_START_FOLDED):
                break
        start_line=i
        
        addition=0
        
        for i in range(line_num,num_lines):
            if self.MarkerGet(i) & 1<<GROUPING_END:
                break
            elif (i>line_num) and ( self.MarkerGet(i)
                         & (1<<GROUPING_START | 1<<GROUPING_START_FOLDED) ):
                addition=-1
                break # the solo case...
        stop_line=i+addition
        
        return start_line,stop_line
    
    def GetIOSlice(self,line_num=None):
        """Get the start/stop lines for the slice based on any line in the slice"""
        if line_num==None:
            line_num=self.GetCurrentLine()
        
        num_lines=self.GetLineCount()
        
        for i in range(line_num,-1,-1):
            if self.MarkerGet(i) & IO_ANY_START_MASK:
                break
        start_line=i
        
        addition=0
        
        for i in range(line_num,num_lines):
            if self.MarkerGet(i) & IO_END_MASK:
                break
            elif (i>line_num) and (self.MarkerGet(i) & IO_ANY_START_MASK):
                addition=-1
                break # the solo case...
        stop_line=i+addition
        
        return start_line,stop_line
    
    def FoldGroupingSlice(self,line_num=None):
        if line_num==None:
            line_num=self.GetCurrentLine()
        
        start,end=self.GetGroupingSlice(line_num)
        self.HideLines(start+1,end)
        marker=self.MarkerGet(start)
        self.clearGroupingMarkers(start)
        self.MarkerAdd(start,GROUPING_START_FOLDED)
        self.clearIOMarkers(start)
        if marker & ( 1<<INPUT_START | 1<<INPUT_START_FOLDED ):
            self.MarkerAdd(start,INPUT_START_FOLDED)
        elif marker & ( 1<<OUTPUT_START | 1<<OUTPUT_START_FOLDED ):
            self.MarkerAdd(start,OUTPUT_START_FOLDED)
            self.MarkerAdd(start,OUTPUT_BG)
        else:
            pass #print 'Bad Markers!!!'
    def FoldIOSlice(self,line_num=None):
        if line_num==None:
            line_num=self.GetCurrentLine()
        
        start,end=self.GetIOSlice(line_num)
        self.HideLines(start+1,end)
        marker=self.MarkerGet(start)
        if (self.MarkerGet(start) & \
               (1<<GROUPING_START | 1<<GROUPING_START_FOLDED )) and \
               (self.MarkerGet(end) & 1<<GROUPING_END):
            self.clearGroupingMarkers(start)
            self.MarkerAdd(start,GROUPING_START_FOLDED)
        self.clearIOMarkers(start)
        if marker & ( 1<<INPUT_START | 1<<INPUT_START_FOLDED ):
            self.MarkerAdd(start,INPUT_START_FOLDED)
        elif marker & ( 1<<OUTPUT_START | 1<<OUTPUT_START_FOLDED ):
            self.MarkerAdd(start,OUTPUT_START_FOLDED)
            self.MarkerAdd(start,OUTPUT_BG)
        else:
            pass #print 'Bad Markers!!!'
    def UnFoldGroupingSlice(self,line_num=None):
        if line_num==None:
            line_num=self.GetCurrentLine()
        
        start,end=self.GetGroupingSlice(line_num)
        self.ShowLines(start+1,end)
        self.clearGroupingMarkers(start)
        self.MarkerAdd(start,GROUPING_START)
        for i in range(start,end):
            marker=self.MarkerGet(i)
            if marker & (1<<INPUT_START | 1<<INPUT_START_FOLDED):
                self.clearIOMarkers(i)
                self.MarkerAdd(i,INPUT_START)
            elif marker & (1<<OUTPUT_START | 1<<OUTPUT_START_FOLDED):
                self.clearIOMarkers(i)
                self.MarkerAdd(i,OUTPUT_START)
                self.MarkerAdd(i,OUTPUT_BG)
        
    def UnFoldIOSlice(self,line_num=None):
        if line_num==None:
            line_num=self.GetCurrentLine()
        
        start,end=self.GetIOSlice(line_num)
        self.ShowLines(start+1,end)
        marker=self.MarkerGet(start)
        if (self.MarkerGet(start) & \
               (1<<GROUPING_START | 1<<GROUPING_START_FOLDED )) and \
               (self.MarkerGet(end) & 1<<GROUPING_END):
            self.clearGroupingMarkers(start)
            self.MarkerAdd(start,GROUPING_START)
        self.clearIOMarkers(start)
        if marker & 1<<INPUT_START_FOLDED:
            self.MarkerAdd(start,INPUT_START)
        elif marker & 1<<OUTPUT_START_FOLDED:
            self.MarkerAdd(start,OUTPUT_START)
            self.MarkerAdd(start,OUTPUT_BG)
    
    def DeleteOutputSlicesAfter(self,line_num=None):
        """Delete all outputs after an input"""
        if line_num==None:
            line_num=self.GetCurrentLine()
        
        num_lines=self.GetLineCount()
        
        if self.MarkerGet(line_num) & OUTPUT_MASK:
            #print 'You can only run "DeleteOutputSlicesAfter" from an Input slice!'
            return
        
        startIn,endIn=self.GetIOSlice(line_num)
        startGrouping,endGrouping=self.GetGroupingSlice(line_num)
        
        if endIn<endGrouping:
            self.SetSelection(self.PositionFromLine(endIn+1),
                              self.PositionFromLine(endGrouping+1))
            self.ReplaceSelection('',sliceDeletion=True)
        
        new_pos=self.GetLineEndPosition(line_num)
        self.SetCurrentPos(new_pos)
        self.SetSelection(new_pos,new_pos)
    
    def SplitSlice(self,line_num=None):
        if line_num==None:
            line_num=self.GetCurrentLine()
        
        start_num,end_num=self.GetIOSlice(line_num)
        
        if self.MarkerGet(line_num) & INPUT_MASK:
            type='Input'
            start=INPUT_START
            end=INPUT_END
            splitGrouping=True
        elif self.MarkerGet(line_num) & OUTPUT_MASK:
            type='Output'
            start=OUTPUT_START
            end=OUTPUT_END
            splitGrouping=False
        
        if start_num==end_num:
            return # Can't split one line!
        elif start_num==line_num:
            self.clearIOMarkers(line_num+1)
            self.MarkerAdd(line_num+1,start)
            if type=='Output': self.MarkerAdd(line_num+1,OUTPUT_BG)
            if splitGrouping:
                self.clearGroupingMarkers(line_num+1)
                self.MarkerAdd(line_num+1,GROUPING_START)
        else:
            self.clearIOMarkers(line_num)
            self.MarkerAdd(line_num,start)
            if type=='Output': self.MarkerAdd(line_num,OUTPUT_BG)
            if splitGrouping:
                self.clearGroupingMarkers(line_num)
                self.MarkerAdd(line_num,GROUPING_START)
            if line_num-1>start_num:
                self.clearIOMarkers(line_num-1)
                self.MarkerAdd(line_num-1,end)
                if type=='Output': self.MarkerAdd(line_num-1,OUTPUT_BG)
                if splitGrouping:
                    self.clearGroupingMarkers(line_num-1)
                    self.MarkerAdd(line_num-1,GROUPING_END)
    
    def BackspaceWMarkers(self,force=False):
        # Warning: This is not good at checking for bad markers!
        c_before=self.GetCharAt(self.GetCurrentPos() - 1)
        c_after=self.GetCharAt(self.GetCurrentPos())
        
        if c_before==0:
            # Disallow deleting the first line or it will destroy the markers...
            return False
        elif c_before in (ord('\n'),ord('\r')):
            line_num=self.GetCurrentLine()
            
            marker=self.MarkerGet(line_num)
            marker_before=self.MarkerGet(line_num-1)
            marker_after=self.MarkerGet(line_num+1)
            if marker_before & ( 1<<GROUPING_END ) :
                return False # Disallow deleting lines between slices...
            elif marker & ( 1<<GROUPING_START | 1<<GROUPING_START_FOLDED ) :
                return False # Disallow deleting lines between slices...
            else:
                if marker_before & ( 1<<GROUPING_START | 1<<GROUPING_START_FOLDED ) :
                    self.clearGroupingMarkers(line_num)
                elif marker & ( 1<<GROUPING_END ) :
                    self.clearGroupingMarkers(line_num-1)
            
            if (marker_before & 1<<INPUT_END) and force:
                # Special case for use in processLine
                self.clearIOMarkers(line_num)
            elif marker_before & (1<<INPUT_END | 1<<OUTPUT_END):
                return False # Disallow deleting lines between slices...
            elif marker & ( 1<<INPUT_START | 1<<INPUT_START_FOLDED ) :
                return False # Disallow deleting lines between slices...
            else:
                if marker_before & (1<<INPUT_START  | 1<<INPUT_START_FOLDED |
                                    1<<OUTPUT_START | 1<<OUTPUT_START_FOLDED):
                    self.clearIOMarkers(line_num)
                elif marker & ( 1<<INPUT_END | 1<<OUTPUT_END ) :
                    self.clearIOMarkers(line_num-1)
        
        return True # If everything went well, return True and do the delete...
    
    def ForwardDeleteWMarkers(self):
        c_before=self.GetCharAt(self.GetCurrentPos() - 1)
        c_after=self.GetCharAt(self.GetCurrentPos())
        if c_after==0:
            # Disallow deleting the first line or it will destroy the markers...
            return False
        elif c_after in (ord('\n'),ord('\r')):
            line_num=self.GetCurrentLine()
            
            marker=self.MarkerGet(line_num)
            marker_before=self.MarkerGet(line_num-1)
            marker_after=self.MarkerGet(line_num+1)
            if marker & ( 1<<GROUPING_END ) :
                return False # Disallow deleting lines between slices...
            elif marker_after & ( 1<<GROUPING_START | 1<<GROUPING_START_FOLDED ) :
                return False # Disallow deleting lines between slices...
            else:
                if marker & ( 1<<GROUPING_START | 1<<GROUPING_START_FOLDED ) :
                    self.clearGroupingMarkers(line_num+1)
                elif marker_after & ( 1<<GROUPING_END ) :
                    self.clearGroupingMarkers(line_num)
            
            if marker & ( 1<<INPUT_END | 1<<OUTPUT_END ) :
                return False # Disallow deleting lines between slices...
            elif marker_after & ( 1<<INPUT_START | 1<<INPUT_START_FOLDED ) :
                return False # Disallow deleting lines between slices...
            else:
                if marker & (1<<INPUT_START  | 1<<INPUT_START_FOLDED |
                             1<<OUTPUT_START | 1<<OUTPUT_START_FOLDED) :
                    self.clearIOMarkers(line_num+1)
                elif marker_after & ( 1<<INPUT_END | 1<<OUTPUT_END ) :
                    self.clearIOMarkers(line_num)
        
        return True
    
    def GetIOSelection(self):
        started=False
        start=0
        end=self.GetLineCount()-1
        type=None
        for i in range(self.GetLineCount()):
            if self.MarkerGet(i) & 1<<IO_SELECTING:
                if started==False:
                    start=i
                    if self.MarkerGet(i) & INPUT_MASK:
                        type='input'
                    elif self.MarkerGet(i) & OUTPUT_MASK:
                        type='output'
                else:
                    if self.MarkerGet(i) & INPUT_MASK:
                        if type=='output':
                            end=i-1
                            break
                    elif self.MarkerGet(i) & OUTPUT_MASK:
                        if type=='input':
                            end=i-1
                            break
                started=True
            elif started==True:
                end=i-1
                break
        
        if started==False:
            #print 'No Selection!!'
            self.SliceSelection=False
        
        return start,end
    
    def MergeAdjacentSlices(self):
        """This function merges all adjacent selected slices.\n""" + \
        """Right now, only IO Merging is allowed."""
        started=False
        start=0
        end=self.GetLineCount()-1
        type=None
        for i in range(self.GetLineCount()):
            if self.MarkerGet(i) & 1<<IO_SELECTING:
                if started==False:
                    start=i
                    if self.MarkerGet(i) & INPUT_MASK:
                        type='input'
                    elif self.MarkerGet(i) & OUTPUT_MASK:
                        type='output'
                else:
                    if self.MarkerGet(i) & INPUT_MASK:
                        if type=='output':
                            end=i-1
                            break
                        else:
                            self.clearIOMarkers(i)
                            self.clearGroupingMarkers(i)
                            self.MarkerAdd(i,INPUT_MIDDLE)
                            self.MarkerAdd(i,GROUPING_MIDDLE)
                    elif self.MarkerGet(i) & OUTPUT_MASK:
                        if type=='input':
                            end=i-1
                            break
                        else:
                            self.clearIOMarkers(i)
                            self.clearGroupingMarkers(i)
                            self.MarkerAdd(i,OUTPUT_MIDDLE)
                            self.MarkerAdd(i,OUTPUT_BG)
                            self.MarkerAdd(i,GROUPING_MIDDLE)
                started=True
            elif started==True:
                end=i-1
                break
        
        if started and end!=start:
            self.clearIOMarkers(end)
            self.clearGroupingMarkers(end)
            if type=='input':
                self.MarkerAdd(end,INPUT_END)
                if end+1<self.GetLineCount():
                    if self.MarkerGet(end+1) & OUTPUT_MASK:
                        self.MarkerAdd(end,GROUPING_MIDDLE)
                    else:
                        self.MarkerAdd(end,GROUPING_END)
                else:
                    self.MarkerAdd(end,GROUPING_END)
            else:
                if self.MarkerGet(start) & 1<<GROUPING_END:
                    self.clearGroupingMarkers(start)
                    self.MarkerAdd(start,GROUPING_MIDDLE)
                self.MarkerAdd(end,OUTPUT_END)
                self.MarkerAdd(end,OUTPUT_BG)
                self.MarkerAdd(end,GROUPING_END)
            
    
    def SliceSelectionDelete(self):
        """Deletion of any selected and possibly discontinuous slices."""
        if not self.SliceSelection:
            return
        
        # collect the line numbers to be deleted...
        selectedSlices=[]
        start,end=None,None
        for i in range(self.GetLineCount()):
            if self.MarkerGet(i) & (1<<GROUPING_SELECTING | 1<<IO_SELECTING):
                if start==None:
                    start=i
                end=i
            elif start!=None:
                selectedSlices.append([start,end])
                start,end=None,None
        if start!=None:
            selectedSlices.append([start,end])
        
        # Unselect everything
        self.MarginUnselectAll()
        self.SliceSelection=False
        
        # Going in reverse, delete the selections, fixing the markers as we go...
        for i in range(len(selectedSlices)-1,-1,-1):
            self.SetSelection(self.PositionFromLine(selectedSlices[i][0]),
                              self.GetLineEndPosition(selectedSlices[i][1])+1)
            
            markerNext = self.MarkerGet(selectedSlices[i][1]+1)
            
            self.ReplaceSelection('',sliceDeletion=True)
            
            cur_line=self.GetCurrentLine()
            
            # If we've made a mess of the grouping markers, clean it up...
            if ((self.MarkerGet(cur_line-1) & 1<<GROUPING_END) and
               (self.MarkerGet(cur_line) & ( 1<<GROUPING_MIDDLE | 1<<GROUPING_END ) )):
                self.clearGroupingMarkers(cur_line)
                self.MarkerAdd(cur_line,GROUPING_START)
            elif (( self.MarkerGet(cur_line-1) & 1<<GROUPING_MIDDLE ) and
                 ( self.MarkerGet(cur_line) &
                      ( 1<<GROUPING_START | 1<<GROUPING_START_FOLDED ) )):
                self.clearGroupingMarkers(cur_line-1)
                self.MarkerAdd(cur_line-1,GROUPING_END)
            
            if markerNext & 1<<OUTPUT_START:
                self.clearIOMarkers(cur_line)
                self.MarkerAdd(cur_line,OUTPUT_START)
                self.MarkerAdd(cur_line,OUTPUT_BG)
            elif markerNext & 1<<OUTPUT_START_FOLDED:
                self.clearIOMarkers(cur_line)
                self.MarkerAdd(cur_line,OUTPUT_START_FOLDED)
                self.MarkerAdd(cur_line,OUTPUT_BG)
        
        return
    
    def OnChar(self, event):
        """Keypress event handler.

        Only receives an event if OnKeyDown calls event.Skip() for the
        corresponding event."""
        
        if self.noteMode:
            event.Skip()
            return

        # Prevent modification of output slices
        if not self.CanEdit():
            return
        key = event.GetKeyCode()
        currpos = self.GetCurrentPos()
        stoppos = self.PositionFromLine(self.GetCurrentLine())
        
        # Return (Enter) needs to be ignored in this handler.
        if key in [wx.WXK_RETURN, wx.WXK_NUMPAD_ENTER]:
            pass
        elif key in self.autoCompleteKeys:
            # Usually the dot (period) key activates auto completion.
            # Get the command between the prompt and the cursor.  Add
            # the autocomplete character to the end of the command.
            if self.AutoCompActive():
                self.AutoCompCancel()
            command = self.GetTextRange(stoppos, currpos) + chr(key)
            
            # write with undo wrapper...
            cpos=self.GetCurrentPos()
            s=chr(key)
            #ADD UNDO
            self.UpdateUndoHistoryBefore('insert',s,cpos,cpos+len(s),
                                         forceNewAction=False)
            self.write(s,type='Input')
            self.UpdateUndoHistoryAfter()
            
            if self.autoComplete:
                self.autoCompleteShow(command)
        elif key == ord('('):
            # The left paren activates a call tip and cancels an
            # active auto completion.
            if self.AutoCompActive():
                self.AutoCompCancel()
            # Get the command between the prompt and the cursor.  Add
            # the '(' to the end of the command.
            self.ReplaceSelection('')
            command = self.GetTextRange(stoppos, currpos) + '('
            
            # write with undo wrapper...
            cpos=self.GetCurrentPos()
            s='('
            self.UpdateUndoHistoryBefore('insert',s,cpos,cpos+len(s),
                                         forceNewAction=True)
            self.undoHistory[self.undoIndex]['allowsAppend']=True
            self.write(s,type='Input')
            self.UpdateUndoHistoryAfter()
            
            self.autoCallTipShow(command,
                                 self.GetCurrentPos() == self.GetTextLength())
        else:
            # Allow the normal event handling to take place.
            # Use undo wrapper
            cpos=self.GetCurrentPos()
            try:
                s=chr(key)
                self.UpdateUndoHistoryBefore('insert',s,cpos,cpos+len(s))
                event.Skip()
                self.UpdateUndoHistoryAfter()
            except:
                event.Skip()
    
    def AutoCompActiveCallback(self):
        numChars=self.GetTextLength()-self.TotalLengthForAutoCompActiveCallback
        if numChars==0:
            self.undoIndex-=1
            del(self.undoHistory[-1])
        else:
            uH=self.undoHistory
            uI=self.undoIndex
            cpos=uH[uI]['posStart']
            s=''.join([chr(self.GetCharAt(cpos+i)) for i in range(numChars)])
            s.replace(os.linesep,'\n')
            self.undoHistory[self.undoIndex]['charList'] = s
            self.undoHistory[self.undoIndex]['posEnd'] = cpos + numChars
            self.undoHistory[self.undoIndex]['numLines'] = len(s.split('\n'))
            self.UpdateUndoHistoryAfter()
    
    def OnKeyDown(self, event):
        """Key down event handler."""

        key = event.GetKeyCode()
        # If the auto-complete window is up let it do its thing.
        if self.AutoCompActive():
            if key in [wx.WXK_RETURN, wx.WXK_NUMPAD_ENTER]:
                cpos=self.GetCurrentPos()
                self.UpdateUndoHistoryBefore('insert','dummy',cpos,cpos+5,
                                             forceNewAction=True)
                self.undoHistory[self.undoIndex]['allowsAppend'] = True
                self.TotalLengthForAutoCompActiveCallback=self.GetTextLength()
                event.Skip()
                wx.CallAfter(self.AutoCompActiveCallback)
            if key in [wx.WXK_DELETE,wx.WXK_BACK]:
                self.AutoCompCancel()
            else:
                event.Skip()
            return
        
        #DNM
        # Prevent modification of output slices
        controlDown = event.ControlDown()
        altDown = event.AltDown()
        shiftDown = event.ShiftDown()
        currpos = self.GetCurrentPos()
        endpos = self.GetTextLength()
        selecting = self.GetSelectionStart() != self.GetSelectionEnd()
        
        if key == wx.WXK_F12: #seb
            if self.noteMode:
                # self.promptPosStart not used anyway - or ? 
##                # We don't need to do this any more!
##                self.promptPosEnd = self.PositionFromLine(self.GetLineCount()-1 ) +
##                                    len(str(sys.ps1))
##                self.GotoLine(self.GetLineCount())
##                self.GotoPos(self.promptPosEnd)
##                self.prompt()  #make sure we have a prompt
                self.SetCaretForeground("black")
                self.SetCaretWidth(1)    #default
                self.SetCaretPeriod(500) #default
            else:
                self.SetCaretForeground("red")
                self.SetCaretWidth(4)
                self.SetCaretPeriod(0) #steady

            self.noteMode = not self.noteMode
            return
        if self.noteMode:
            event.Skip()
            return
        
        doLineBreak=False
        doSubmitCommand=False
        doPass=False
        # Return is used to insert a line break.
        # In Shell Mode, hit Return or Enter twice to submit a command
        if ((not controlDown and not shiftDown and not altDown) and
           key in [wx.WXK_RETURN,]):
            if self.mode=='SlicesMode':
                doLineBreak=True
            elif self.mode=='ShellMode':
                startLine,endLine = self.GetIOSlice()
                startpos = self.PositionFromLine(startLine)
                endpos = self.GetLineEndPosition(endLine)
                command = self.GetTextRange(startpos, endpos)
                strCont,indentBlock,lineCont,parenCont = testForContinuations(command,ignoreErrors=True)
                
                lastLine = command.split('\n')[-1]
                if lastLine.lstrip()=='': # all whitespace...
                    stillIndented=False
                elif lastLine[0]==' ':
                    stillIndented=True
                else:
                    stillIndented=False
                
                if strCont[-1] or indentBlock[-1] or lineCont[-1] or \
                   parenCont[-1]:
                    doLineBreak=True
                elif stillIndented:
                    new_pos=self.GetLineEndPosition(endLine)
                    self.SetCurrentPos(new_pos)
                    self.SetSelection(new_pos,new_pos)
                    doLineBreak=True
                elif self.GetCurrentLine()!=endLine:
                    new_pos=self.GetLineEndPosition(endLine)
                    self.SetCurrentPos(new_pos)
                    self.SetSelection(new_pos,new_pos)
                    doPass = True
                else:
                    doSubmitCommand=True
        # Enter (Shift/Ctrl + Enter/Return) submits a command to the interpreter.
        # In Shell Mode, hit Return or Enter twice to submit a command
        elif ( key in [wx.WXK_NUMPAD_ENTER,] or
            ( (shiftDown or controlDown) and key in [wx.WXK_RETURN,
                                                     wx.WXK_NUMPAD_ENTER] ) ):
            if self.mode=='SlicesMode':
                doSubmitCommand=True
            elif self.mode=='ShellMode':
                doLineBreak=True
        
        #Only relevant in ShellMode...
        
        if doPass:
            pass
        elif doLineBreak or doSubmitCommand:
            if self.CallTipActive():
                self.CallTipCancel()
            elif self.SliceSelection:
                for i in range(self.GetLineCount()):
                    if self.MarkerGet(i) & 1<<GROUPING_SELECTING:
                        self.DoMarginClick(i, 2, shiftDown, controlDown)
                        break
                    elif self.MarkerGet(i) & 1<<IO_SELECTING:
                        self.DoMarginClick(i, 3, shiftDown, controlDown)
                        break
            elif doLineBreak:
                self.insertLineBreak()
                #Only relevant in ShellMode...
            elif doSubmitCommand:
                self.DeleteOutputSlicesAfter()
                self.processLine()
        
        # Let Ctrl-Alt-* get handled normally.
        elif controlDown and altDown:
            event.Skip()
        
        # Clear the current, unexecuted command.
        elif key == wx.WXK_ESCAPE:
            if self.CallTipActive():
                event.Skip()
        # Clear the current command
        elif key == wx.WXK_BACK and controlDown and shiftDown:
            self.clearCommand()

        # Increase font size.
        elif controlDown and key in (ord(']'), wx.WXK_NUMPAD_ADD):
            dispatcher.send(signal='FontIncrease')

        # Decrease font size.
        elif controlDown and key in (ord('['), wx.WXK_NUMPAD_SUBTRACT):
            dispatcher.send(signal='FontDecrease')

        # Default font size.
        elif controlDown and key in (ord('='), wx.WXK_NUMPAD_DIVIDE):
            dispatcher.send(signal='FontDefault')

        # Cut to the clipboard.
        elif (controlDown and key in (ord('X'), ord('x'))) \
                 or (shiftDown and key == wx.WXK_DELETE):
            self.Cut()

        # Copy to the clipboard.
        elif controlDown and not shiftDown \
                 and key in (ord('C'), ord('c'), wx.WXK_INSERT):
            self.Copy()

        # Copy to the clipboard, including prompts.
        elif controlDown and shiftDown \
                 and key in (ord('C'), ord('c'), wx.WXK_INSERT):
            self.CopyWithPrompts()

        # Copy to the clipboard, including prefixed prompts.
        elif altDown and not controlDown \
                 and key in (ord('C'), ord('c'), wx.WXK_INSERT):
            self.CopyWithPromptsPrefixed()
        
        # Home needs to be aware of the prompt.
        elif controlDown and key == wx.WXK_HOME:
            # Go to the beginning of the IO Slice
            curLine = self.GetCurrentLine()
            IOstart = self.GetIOSlice(curLine)[0]
            home = self.PositionFromLine(IOstart)
            if currpos == home and \
               IOstart > 0:
                home = self.PositionFromLine(self.GetIOSlice(curLine-1)[0])
            self.SetCurrentPos(home)
            if not selecting and not shiftDown:
                self.SetAnchor(home)
                self.EnsureCaretVisible()

        elif controlDown and key == wx.WXK_END:
            curLine = self.GetCurrentLine()
            IOend = self.GetIOSlice(curLine)[1]
            end = self.GetLineEndPosition(IOend)
            if currpos == end and \
               IOend < self.GetLineCount()-1:
                end = self.GetLineEndPosition(self.GetIOSlice(curLine+1)[1])
            self.SetCurrentPos(end)
            if not selecting and not shiftDown:
                self.SetAnchor(end)
                self.EnsureCaretVisible()
        
        elif controlDown and key == wx.WXK_PAGEUP:
            pos=0
            if currpos > pos:
                self.SetCurrentPos(pos)
                if not selecting and not shiftDown:
                    self.SetAnchor(pos)
                    self.EnsureCaretVisible()
        
        elif controlDown and key == wx.WXK_PAGEDOWN:
            pos = self.GetLineEndPosition(self.GetLineCount()-1)
            if currpos < pos:
                self.SetCurrentPos(pos)
                if not selecting and not shiftDown:
                    self.SetAnchor(pos)
                    self.EnsureCaretVisible()
        
        elif selecting and key not in NAVKEYS and not self.CanEdit():
            pass

        # Paste from the clipboard.
        elif (controlDown and not shiftDown and key in (ord('V'), ord('v'))) \
                 or (shiftDown and not controlDown and key == wx.WXK_INSERT):
            self.Paste()

        # Paste from the clipboard, run commands.
        elif controlDown and shiftDown and \
                             key in (ord('V'), ord('v')) and self.CanEdit():
            self.PasteAndRun()
            
        # Replace with the previous command from the history buffer.
        elif (controlDown and not shiftDown and key == wx.WXK_UP) \
              or (altDown and key in (ord('P'), ord('p'))) and self.CanEdit():
            self.OnHistoryReplace(step=+1)
            
        # Replace with the next command from the history buffer.
        elif (controlDown and not shiftDown and key == wx.WXK_DOWN) \
              or (altDown and key in (ord('N'), ord('n'))) and self.CanEdit():
            self.OnHistoryReplace(step=-1)
            
        # Insert the previous command from the history buffer.
        elif (controlDown and shiftDown and key == wx.WXK_UP) and \
              self.CanEdit():
            self.OnHistoryInsert(step=+1)
            
        # Insert the next command from the history buffer.
        elif (controlDown and shiftDown and key == wx.WXK_DOWN) and \
              self.CanEdit():
            self.OnHistoryInsert(step=-1)
            
        # Ctrl-Space shows Auto Completion
        # Ctrl-Shift-Space shows CallTips
        elif controlDown and key == wx.WXK_SPACE:
            self.OnCallTipAutoCompleteManually(shiftDown)
        
        # Ctrl+Shift+H is used to complete Text (from already typed words)
        elif controlDown and shiftDown and key in [ord('H')]:
            self.OnShowCompHistory()
        
        # Search up the history for the text in front of the cursor.
        elif key == wx.WXK_F8:
            self.OnHistorySearch()
            
        # Don't backspace over the latest non-continuation prompt.
        elif key == wx.WXK_BACK:
            if self.SliceSelection:
                self.SliceSelectionDelete()
                wx.CallAfter(self.RestoreFirstMarker)
            elif selecting and self.CanEdit():
                self.ReplaceSelection('')
                #event.Skip()
            elif self.CanEdit():
                doDelete=True
                cur_line=self.GetCurrentLine()
                if not cur_line==0 and \
                   self.GetCurrentPos()==self.PositionFromLine(cur_line):
                    if self.MarkerGet(cur_line-1) & OUTPUT_MASK:
                        doDelete=False
                
                if doDelete:
                    cpos=self.GetCurrentPos()
                    s=chr(self.GetCharAt(cpos-1))
                    self.UpdateUndoHistoryBefore('delete',s,cpos-1,cpos)
                    if self.BackspaceWMarkers():
                        event.Skip()
                
                wx.CallAfter(self.RestoreFirstMarker)
        
        elif key == wx.WXK_DELETE:
            if self.SliceSelection:
                self.SliceSelectionDelete()
                wx.CallAfter(self.RestoreFirstMarker)
            elif selecting and self.CanEdit():
                self.ReplaceSelection('')
                #event.Skip()
            elif self.CanEdit():
                doDelete=True
                cur_line=self.GetCurrentLine()
                if not cur_line==self.GetLineCount()-1 and \
                   self.GetCurrentPos()==self.GetLineEndPosition(cur_line):
                    if self.MarkerGet(cur_line+1) & OUTPUT_MASK:
                        doDelete=False
                
                if doDelete:
                    cpos=self.GetCurrentPos()
                    s=chr(self.GetCharAt(cpos))
                    self.UpdateUndoHistoryBefore('delete',s,cpos,cpos+1)
                    if self.ForwardDeleteWMarkers():
                        event.Skip()
                
                wx.CallAfter(self.RestoreFirstMarker)
        
        # Only allow these keys after the latest prompt.
        elif key == wx.WXK_TAB and self.CanEdit():
            # use the same mechanism as with autocmplete...
            cpos=self.GetCurrentPos()
            self.UpdateUndoHistoryBefore('insert','dummy',cpos,cpos+5,
                                         forceNewAction=True)
            self.undoHistory[self.undoIndex]['allowsAppend'] = True
            self.TotalLengthForAutoCompActiveCallback=self.GetTextLength()
            event.Skip()
            wx.CallAfter(self.AutoCompActiveCallback)
        
        # Don't toggle between insert mode and overwrite mode.
        elif key == wx.WXK_INSERT:
            pass
        
        # Don't allow line deletion.
        #elif controlDown and key in (ord('L'), ord('l')):
            # TODO : Allow line deletion eventually ??
            #event.Skip()
        #    pass
        
        # Don't allow line transposition.
        # Toggle Shell Mode / Slices Mode
        elif controlDown and key in (ord('T'), ord('t')):
            self.ToggleShellMode()
        
        #Open and Save now work when using CrustSlicesFrames
        elif controlDown and key in (ord('L'), ord('l')):
            #print 'Load it'
            file=wx.FileSelector("Load File As New Slice")
            if file!=u'':
                fid=open(file,'r')
                self.LoadPyFileAsSlice(fid)
                fid.close()
        
        elif controlDown and key in (ord('D'), ord('d')):
            #Disallow line duplication in favor of divide slices
            if self.MarkerGet(self.GetCurrentLine()) & INPUT_MASK:
                #ADD UNDO
                cpos=self.GetCurrentPos()
                start,end = map(self.PositionFromLine,
                              self.GetGroupingSlice(self.LineFromPosition(cpos)))
                self.UpdateUndoHistoryBefore('marker','',start,end,
                                             forceNewAction=True)
                self.SplitSlice()
                # Turn off selecting
                self.SetSelection(cpos,cpos)
                self.ReplaceSelection('')
                self.UpdateUndoHistoryAfter()
        
        elif controlDown and key in (ord('M'), ord('m')):
            #ADD UNDO
            if self.SliceSelection:
                cpos=self.GetCurrentPos()
                ioSel=self.GetIOSelection()
            if self.SliceSelection:
                start,end = map(self.PositionFromLine,ioSel)
                self.UpdateUndoHistoryBefore('marker','',start,end,
                                             forceNewAction=True)
                self.MergeAdjacentSlices()
                # Turn off selecting
                self.SetSelection(cpos,cpos)
                self.ReplaceSelection('')
                self.UpdateUndoHistoryAfter()
            
        
        # Change arrow keys to allow margin behaviors...
        elif self.SliceSelection and \
             key in [wx.WXK_UP,wx.WXK_DOWN,wx.WXK_RIGHT,wx.WXK_LEFT]:
        # TODO : This is useful, but not optimal!
            if key==wx.WXK_UP:
                for i in range(self.GetLineCount()):
                    if self.MarkerGet(i) & 1<<GROUPING_SELECTING:
                        if i>0:                     #Grouping
                            self.DoMarginClick(i-1, 2, shiftDown, controlDown)
                        break
                    elif self.MarkerGet(i) & 1<<IO_SELECTING:
                        if i>0:                     #IO
                            self.DoMarginClick(i-1, 3, shiftDown, controlDown)
                        break
            elif key==wx.WXK_DOWN:
                for i in range(self.GetLineCount()-1,-1,-1):
                    if self.MarkerGet(i) & 1<<GROUPING_SELECTING:
                        if i<self.GetLineCount()-1: #Grouping
                            self.DoMarginClick(i+1, 2, shiftDown, controlDown)
                        break
                    elif self.MarkerGet(i) & 1<<IO_SELECTING:
                        if i<self.GetLineCount()-1: #IO
                            self.DoMarginClick(i+1, 3, shiftDown, controlDown)
                        break
            elif key==wx.WXK_RIGHT:
                for i in range(self.GetLineCount()):
                    if self.MarkerGet(i) & 1<<GROUPING_SELECTING:
                        self.DoMarginClick(i, 3, shiftDown, controlDown)
                        break
                    elif self.MarkerGet(i) & 1<<IO_SELECTING:
                        self.MarginUnselectAll()
                        # Go to the beginning of the IO Slice
                        self.SetCurrentPos(self.PositionFromLine(i))
                        if not selecting and not shiftDown:
                            self.SetAnchor(self.PositionFromLine(i))
                            self.EnsureCaretVisible()
                        break
            elif key==wx.WXK_LEFT:
                for i in range(self.GetLineCount()):
                    if self.MarkerGet(i) & 1<<GROUPING_SELECTING:
                        break
                    elif self.MarkerGet(i) & 1<<IO_SELECTING:
                        self.DoMarginClick(i, 2, shiftDown, controlDown)
                        break
        # Basic navigation keys should work anywhere.
        elif key in NAVKEYS:
            event.Skip()
        # Protect the readonly portion of the slices shell.
        elif not self.CanEdit():
            pass
        else:
            # Check to see if we're selecting
            if self.GetSelectionEnd()>self.GetSelectionStart():
                # Check to see if a normal input took place
                if not controlDown and not altDown and key<256:
                    self.ReplaceSelection('') # This seems to work...
            event.Skip()
        
        if self.SliceSelection:
            if key not in [wx.WXK_UP,wx.WXK_DOWN,wx.WXK_RIGHT,wx.WXK_LEFT,
                           wx.WXK_ALT,wx.WXK_COMMAND,wx.WXK_CONTROL,wx.WXK_SHIFT]:
                self.MarginUnselectAll()
    
    
    def MarginSelectAll(self):
        num_lines=self.GetLineCount()
        for i in range(num_lines):
            self.MarkerAdd(i,GROUPING_SELECTING)
            self.MarkerDelete(i,IO_SELECTING)
    
    def MarginUnselectAll(self):
        num_lines=self.GetLineCount()
        for i in range(num_lines):
            self.MarkerDelete(i,IO_SELECTING)
            self.MarkerDelete(i,GROUPING_SELECTING)
        self.SliceSelection=False
    
    def DoMarginClick(self, lineClicked, margin, shiftDown, controlDown):
        num_lines=self.GetLineCount()
        
        if margin==1:
            pass # these events are not sent right now...
        if margin==2:
            self.SliceSelection=True
            start,end=self.GetGroupingSlice(lineClicked)
            startPos=self.PositionFromLine(start)
            self.SetCurrentPos(startPos)
            self.SetSelection(startPos,startPos)
            start_marker=self.MarkerGet(start)
            if self.MarkerGet(lineClicked) & 1<<GROUPING_SELECTING:
                toggle=self.MarkerDelete
                if not shiftDown:
                    if start_marker & 1<<GROUPING_START:
                        self.FoldGroupingSlice(lineClicked)
                    elif start_marker & 1<<GROUPING_START_FOLDED:
                        self.UnFoldGroupingSlice(lineClicked)
            else:
                toggle=self.MarkerAdd
            
            if not shiftDown:
                self.MarginUnselectAll()
            
            for i in range(start,end+1):
                toggle(i,GROUPING_SELECTING)
        elif margin==3:
            self.SliceSelection=True
            start,end=self.GetIOSlice(lineClicked)
            startPos=self.PositionFromLine(start)
            self.SetCurrentPos(startPos)
            self.SetSelection(startPos,startPos)
            start_marker=self.MarkerGet(start)
            if self.MarkerGet(lineClicked) & 1<<IO_SELECTING:
                toggle=self.MarkerDelete
                if not shiftDown:
                    if start_marker & IO_START_MASK:
                        self.FoldIOSlice(lineClicked)
                    elif start_marker & IO_START_FOLDED_MASK:
                        self.UnFoldIOSlice(lineClicked)
            else:
                toggle=self.MarkerAdd
            
            if not shiftDown:
                self.MarginUnselectAll()
            
            for i in range(start,end+1):
                toggle(i,IO_SELECTING)
            
            #print start,end
            
        elif margin==4:
            # TODO : Folding ??
            if 1:#self.MarkerGet(lineClicked) & ( 1<<7 | 1<<8 ):
                if shiftDown:
                    self.SetFoldExpanded(lineClicked, True)
                    self.Expand(lineClicked, True, True, 1)
                elif controlDown:
                    if self.GetFoldExpanded(lineClicked):
                        self.SetFoldExpanded(lineClicked, False)
                        self.Expand(lineClicked, False, True, 0)
                    else:
                        self.SetFoldExpanded(lineClicked, True)
                        self.Expand(lineClicked, True, True, 100)
                else:
                    self.ToggleFold(lineClicked)
        else:
            self.MarginUnselectAll()
        if margin in [2,3]:
            if toggle==self.MarkerDelete and not shiftDown:
                self.SliceSelection=False
            else:
                self.SliceSelection=True
    
    def OnMarginClick(self, evt):
        
        # fold and unfold as neededNAVKEYS
        lineClicked = self.LineFromPosition(evt.GetPosition())
        self.DoMarginClick(lineClicked,evt.GetMargin(),evt.GetShift(),evt.GetControl())
        evt.Skip()

    def OnShowCompHistory(self):
        """Show possible autocompletion Words from already typed words."""
        
        #copy from history
        his = self.history[:]
        
        #put together in one string
        joined = " ".join (his)
        import re

        #sort out only "good" words
        newlist = re.split("[ \.\[\]=}(\)\,0-9\"]", joined)
        
        #length > 1 (mix out "trash")
        thlist = []
        for i in newlist:
            if len (i) > 1:
                thlist.append (i)
        
        #unique (no duplicate words
        #oneliner from german python forum => unique list
        unlist = [thlist[i] for i in xrange(len(thlist)) if thlist[i] not in thlist[:i]]
            
        #sort lowercase
        unlist.sort(lambda a, b: cmp(a.lower(), b.lower()))
        
        #this is more convenient, isn't it?
        self.AutoCompSetIgnoreCase(True)
        
        #join again together in a string
        stringlist = " ".join(unlist)

        #pos von 0 noch ausrechnen

        #how big is the offset?
        cpos = self.GetCurrentPos() - 1
        while chr (self.GetCharAt (cpos)).isalnum():
            cpos -= 1
            
        #the most important part
        self.AutoCompShow(self.GetCurrentPos() - cpos -1, stringlist)
    
    def ReplaceSelection(self,text,sliceDeletion=False,*args,**kwds):
        startIO,endIO=self.GetIOSlice()
        startGrouping,endGrouping=self.GetGroupingSlice()
        startSel = self.LineFromPosition(self.GetSelectionStart())
        endSel = self.LineFromPosition(self.GetSelectionEnd())
        
        #ADD UNDO
        cpos=self.GetSelectionStart()
        s=self.GetSelectedText()
        if s!='':
            self.UpdateUndoHistoryBefore('delete',s,cpos,cpos+len(s),
                                         forceNewAction=True)
        editwindow.EditWindow.ReplaceSelection(self,'',*args,**kwds)
        if s!='' and not sliceDeletion:
            self.UpdateUndoHistoryAfter()
        
        if endSel-startSel>0 and not sliceDeletion:
            if endSel==endIO and startIO!=self.GetCurrentLine():
                self.clearIOMarkers()
                self.MarkerAdd(self.GetCurrentLine(),INPUT_END)
            
            if endSel==endGrouping and startGrouping!=self.GetCurrentLine():
                self.clearGroupingMarkers()
                self.MarkerAdd(self.GetCurrentLine(),GROUPING_END)
        
        cpos=self.GetSelectionStart()
        s=text
        if s!='':
            self.UpdateUndoHistoryBefore('insert',s,cpos,cpos+len(s),
                                         forceNewAction=True)
            self.write(text)
            self.UpdateUndoHistoryAfter()
        
        self.ensureSingleGroupingMarker()
        self.ensureSingleIOMarker()
        
    
    def clearCommand(self):
        """Delete the current, unexecuted command."""
        if not self.CanEdit():
            return
        start,end=self.GetIOSlice()
        startpos = self.PositionFromLine(start)
        endpos = self.GetLineEndPosition(end)
        self.SetSelection(startpos, endpos)
        self.ReplaceSelection('')
        self.more = False

    def OnHistoryReplace(self, step):
        """Replace with the previous/next command from the history buffer."""
        if not self.CanEdit():
            return    
        self.clearCommand()
        self.replaceFromHistory(step)

    def replaceFromHistory(self, step):
        """Replace selection with command from the history buffer."""
        if not self.CanEdit():
            return
        self.ReplaceSelection('')
        newindex = self.historyIndex + step
        if -1 <= newindex <= len(self.history):
            self.historyIndex = newindex
        if 0 <= newindex <= len(self.history)-1:
            command = self.history[self.historyIndex]
            command = command.replace('\n', os.linesep)# + ps2)
            self.ReplaceSelection(command)

    def OnHistoryInsert(self, step):
        """Insert the previous/next command from the history buffer."""
        if not self.CanEdit():
            return
        startpos = self.GetCurrentPos()
        self.replaceFromHistory(step)
        endpos = self.GetCurrentPos()
        self.SetSelection(endpos, startpos)
    
    # TODO: Fix Me!
    def OnHistorySearch(self):
        """Search up the history buffer for the text in front of the cursor."""
        if not self.CanEdit():
            return
        startpos = self.GetCurrentPos()
        # The text up to the cursor is what we search for.
        numCharsAfterCursor = self.GetTextLength() - startpos
        searchText = self.getCommand(rstrip=False)
        #print 'history search', startpos,numCharsAfterCursor,searchText
        if numCharsAfterCursor > 0:
            searchText = searchText[:-numCharsAfterCursor]
        if not searchText:
            return
        # Search upwards from the current history position and loop
        # back to the beginning if we don't find anything.
        if (self.historyIndex <= -1) \
        or (self.historyIndex >= len(self.history)-2):
            searchOrder = range(len(self.history))
        else:
            searchOrder = range(self.historyIndex+1, len(self.history)) + \
                          range(self.historyIndex)
        for i in searchOrder:
            command = self.history[i]
            if command[:len(searchText)] == searchText:
                # Replace the current selection with the one we found.
                self.ReplaceSelection(command[len(searchText):])
                endpos = self.GetCurrentPos()
                self.SetSelection(endpos, startpos)
                # We've now warped into middle of the history.
                self.historyIndex = i
                break

    def setStatusText(self, text):
        """Display status information."""

        # This method will likely be replaced by the enclosing app to
        # do something more interesting, like write to a status bar.
        print text

    def insertLineBreak(self):
        """Insert a new line break."""
        if not self.CanEdit():
            return
        elif self.reader.isreading:
            self.processLine()
            return
        
        
        # write with undo wrapper...
        cpos=self.GetCurrentPos()
        s=os.linesep
        self.UpdateUndoHistoryBefore('insert',s,cpos,cpos+1)
        self.write(s,type='Input')
        self.UpdateUndoHistoryAfter()
        
        self.more = True
        self.prompt()

    def processLine(self):
        """Process the line of text at which the user hit Enter or Shift+RETURN."""
        # The user hit ENTER (Shift+RETURN) (Shift+ENTER) and we need to
        # decide what to do. They could be sitting on any line in the slices shell.
        thepos = self.GetCurrentPos()
        cur_line = self.GetCurrentLine()
        marker=self.MarkerGet(cur_line)
        if marker & INPUT_MASK:
            pass
        elif marker & OUTPUT_MASK:
            return
        else:
            pass #print 'BLANK LINE!!'
        
        startline,endline=self.GetIOSlice(cur_line)
        
        if startline==0:
            startpos=0
        else:
            startpos=self.PositionFromLine(startline)
        
        endpos=self.GetLineEndPosition(endline)
        
        # If they hit ENTER inside the current command, execute the command.
        if self.CanEdit():
            self.SetCurrentPos(endpos)
            self.interp.more = False
            command = self.GetTextRange(startpos, endpos)
            lines = command.split(os.linesep)
            lines = [line.rstrip() for line in lines]
            command = '\n'.join(lines)
            if self.reader.isreading:
                if not command:
                    # Match the behavior of the standard Python shell
                    # when the user hits return without entering a value.
                    command = '\n'
                self.reader.input = command
                self.write(os.linesep,'Input')
                self.MarkerSet(self.GetCurrentLine(),READLINE_BG)
                self.MarkerSet(self.GetCurrentLine(),INPUT_READLINE)
            else:
                self.runningSlice = (startline,endline)
                self.push(command,useMultiCommand=True)
                #print 'command: ',command
                wx.FutureCall(1, self.EnsureCaretVisible)
                self.runningSlice=None
        
        skip=self.BackspaceWMarkers(force=True)
        if skip:
            self.DeleteBack()
        
        if self.GetCurrentLine()==self.GetLineCount()-1:
            self.write(os.linesep,type='Input')
            cpos=self.GetCurrentLine()
            if self.MarkerGet(cpos-1) & OUTPUT_MASK:
                self.MarkerAdd(cpos-1,OUTPUT_BG)
            self.SplitSlice()
        else:
            cur_line=self.GetCurrentLine()
            new_pos=self.GetLineEndPosition(cur_line+1)
            self.SetSelection(new_pos,new_pos)
            self.SetCurrentPos(new_pos)
        
        self.EmptyUndoBuffer()
        self.NeedsCheckForSave=True
        if self.hasSyntaxError:
            pos=self.GetLineEndPosition(self.syntaxErrorRealLine)
            self.SetCurrentPos(pos)
            self.SetSelection(pos,pos)
    
    # Not Used!!
    def getMultilineCommand(self, rstrip=True):
        """Extract a multi-line command from the editor.

        The command may not necessarily be valid Python syntax."""
        # DNM
        # XXX Need to extract real prompts here. Need to keep track of
        # the prompt every time a command is issued.
        text = self.GetCurLine()[0]
        line = self.GetCurrentLine()
        # Add Marker testing here...
        while text == '' and line > 0: # Need to add markers handling...
            line -= 1
            self.GotoLine(line)
            text = self.GetCurLine()[0]
        if text=='':
            line = self.GetCurrentLine()
            self.GotoLine(line)
            startpos = self.GetCurrentPos()
            line += 1
            self.GotoLine(line)
            while self.GetCurLine()[0]=='':
                line += 1
                self.GotoLine(line)
            stoppos = self.GetCurrentPos()
            command = self.GetTextRange(startpos, stoppos)
            command = command.replace(os.linesep, '\n')
            command = command.rstrip()
            command = command.replace('\n', os.linesep)
        else:
            command = ''
        if rstrip:
            command = command.rstrip()
        return command

    def getCommand(self, text=None, rstrip=True):
        """Extract a command from text which may include a shell prompt.

        The command may not necessarily be valid Python syntax."""
        if not text:
            text = self.GetCurLine()[0]
        # Strip the prompt off the front leaving just the command.
        command = self.lstripPrompt(text)
        # Change this -- Nothing has prompts!
        #if command == text:
        #    command = ''  # Real commands have prompts.
        if rstrip:
            command = command.rstrip()
        return command

    def lstripPrompt(self, text):
        """Return text without a leading prompt."""
        ps1 = str(sys.ps1)
        ps1size = len(ps1)
        ps2 = str(sys.ps2)
        ps2size = len(ps2)
        # Strip the prompt off the front of text.
        if text[:ps1size] == ps1:
            text = text[ps1size:]
        elif text[:ps2size] == ps2:
            text = text[ps2size:]
        return text

    def push(self, command, silent = False,useMultiCommand=False):
        """Send command to the interpreter for execution."""
        if not silent:
            self.write(os.linesep,type='Output')
        # TODO : What other magic might we insert here?
        # TODO : Is there a good reason not to include magic?
        if USE_MAGIC:
            command=magic(command)
        
        # Allows multi-component commands...
        self.hasSyntaxError=False
        if useMultiCommand:
            result = self.BreakTextIntoCommands(command)
            if result[0] == None:
                commands=[command]
                self.hasSyntaxError=True
                syntaxErrorLine=result[1]+1
                self.syntaxErrorRealLine = self.GetCurrentLine()+result[1]-len(command.split('\n'))
            else:
                commands=result
        else:
            commands=[command]
        
        busy = wx.BusyCursor()
        self.waiting = True
        self.lastUpdate=None
        
        for i in commands:
            if self.hasSyntaxError:
                lineno=syntaxErrorLine
                offset=0 # not sure how to easily recover this information...
                self.write('  File "<input>", line '+str(lineno)+'\n    '+i.split('\n')[lineno-1]+'\n'+' '*offset+'    ^\nSyntaxError: invalid syntax\n','Error')
            else:
                self.more = self.interp.push(i+'\n')
            # (the \n stops many things from bouncing at the interpreter)
            # I could do the following, but I don't really like it!
            #if useMultiCommand:
            #    self.SplitSlice()
        self.lastUpdate=None
        
        if not silent:
            self.MarkerAdd(self.GetIOSlice()[0],OUTPUT_BG)
        
        self.waiting = False
        del busy
        if not self.more: # could loop-add to history, too, but I don't like it!
            self.addHistory(command.rstrip())
        
        if not silent:
            self.prompt()

    def addHistory(self, command):
        """Add command to the command history."""
        # Reset the history position.
        self.historyIndex = -1
        # Insert this command into the history, unless it's a blank
        # line or the same as the last command.
        if command!='' and ( len(self.history)==0 or command!=self.history[0] ):
            self.history.insert(0, command)
            dispatcher.send(signal="SlicesShell.addHistory", command=command)
    
    def clearGroupingMarkers(self,line_num=None):
        if line_num==None:
            line_num=self.GetCurrentLine()
        self.MarkerDelete(line_num,GROUPING_START)
        self.MarkerDelete(line_num,GROUPING_START_FOLDED)
        self.MarkerDelete(line_num,GROUPING_MIDDLE)
        self.MarkerDelete(line_num,GROUPING_END)
    def clearIOMarkers(self,line_num=None):
        if line_num==None:
            line_num=self.GetCurrentLine()
        self.MarkerDelete(line_num,INPUT_START)
        self.MarkerDelete(line_num,INPUT_START_FOLDED)
        self.MarkerDelete(line_num,INPUT_MIDDLE)
        self.MarkerDelete(line_num,INPUT_END)
        self.MarkerDelete(line_num,OUTPUT_START)
        self.MarkerDelete(line_num,OUTPUT_START_FOLDED)
        self.MarkerDelete(line_num,OUTPUT_MIDDLE)
        self.MarkerDelete(line_num,OUTPUT_END)
        self.MarkerDelete(line_num,OUTPUT_BG)
        self.MarkerDelete(line_num,READLINE_BG)
        self.MarkerDelete(line_num,INPUT_READLINE)
    def ensureSingleGroupingMarker(self,line_num=None):
        if line_num==None:
            line_num=self.GetCurrentLine()
        marker=self.MarkerGet(line_num)
        if marker & 1<<GROUPING_START:
            self.MarkerDelete(line_num,GROUPING_START_FOLDED)
            self.MarkerDelete(line_num,GROUPING_MIDDLE)
            self.MarkerDelete(line_num,GROUPING_END)
        elif marker & 1<<GROUPING_START_FOLDED:
            self.MarkerDelete(line_num,GROUPING_MIDDLE)
            self.MarkerDelete(line_num,GROUPING_END)
        elif marker & 1<<GROUPING_MIDDLE:
            self.MarkerDelete(line_num,GROUPING_END)
        elif marker & 1<<GROUPING_END:
            pass
        else:
            #print 'ERROR! NO GROUPING MARKERS!'
            return 1 # Blank marker
        
        return 0
    
    def ensureSingleIOMarker(self,line_num=None):
        if line_num==None:
            line_num=self.GetCurrentLine()
        marker=self.MarkerGet(line_num)
        if marker & INPUT_MASK:
            self.MarkerDelete(line_num,OUTPUT_START)
            self.MarkerDelete(line_num,OUTPUT_START_FOLDED)
            self.MarkerDelete(line_num,OUTPUT_MIDDLE)
            self.MarkerDelete(line_num,OUTPUT_END)
            self.MarkerDelete(line_num,OUTPUT_BG)
            [start,start_folded] = [INPUT_START,INPUT_START_FOLDED]
            [middle,end] = [INPUT_MIDDLE,INPUT_END]
        elif marker & OUTPUT_MASK:
            self.MarkerDelete(line_num,INPUT_START)
            self.MarkerDelete(line_num,INPUT_START_FOLDED)
            self.MarkerDelete(line_num,INPUT_MIDDLE)
            self.MarkerDelete(line_num,INPUT_END)
            [start,start_folded] = [OUTPUT_START,OUTPUT_START_FOLDED]
            [middle,end] = [OUTPUT_MIDDLE,OUTPUT_END]
        else:
            #print 'ERROR! NO IO MARKERS!'
            return 1 # Blank marker
        
        if marker & 1<<start:
            self.MarkerDelete(line_num,start_folded)
            self.MarkerDelete(line_num,middle)
            self.MarkerDelete(line_num,end)
        elif marker & 1<<start_folded:
            self.MarkerDelete(line_num,middle)
            self.MarkerDelete(line_num,end)
        elif marker & 1<<middle:
            self.MarkerDelete(line_num,end)
        elif marker & 1<<end:
            pass
        
        return 0
        
    def RestoreFirstMarker(self):
        first_marker=self.MarkerGet(0)
        self.clearGroupingMarkers(0)
        self.clearIOMarkers(0)
        
        if first_marker & 1<<GROUPING_START :
            self.MarkerAdd(0,GROUPING_START)
        elif first_marker & 1<<GROUPING_START_FOLDED :
            self.MarkerAdd(0,GROUPING_START_FOLDED)
        else:
            self.MarkerAdd(0,GROUPING_START)
        
        if first_marker & 1<<INPUT_START :
            self.MarkerAdd(0,INPUT_START)
        elif first_marker & 1<<INPUT_START_FOLDED :
            self.MarkerAdd(0,INPUT_START_FOLDED)
        elif first_marker & 1<<OUTPUT_START :
            self.MarkerAdd(0,OUTPUT_START)
            #self.MarkerAdd(0,OUTPUT_BG) # More harm than good??
        elif first_marker & 1<<OUTPUT_START_FOLDED :
            self.MarkerAdd(0,OUTPUT_START_FOLDED)
            #self.MarkerAdd(0,OUTPUT_BG) # More harm than good??
        else:
            self.MarkerAdd(0,INPUT_START)
        
        if self.doHistUpdate:
            self.UpdateUndoHistoryAfter()
    
    def IsAllowedPair(self,m1,m2):
        """This testing function ensures that two adjacent markers are valid"""
        i_s = 1<<INPUT_START | 1<<INPUT_START_FOLDED
        o_s = 1<<OUTPUT_START | 1<<OUTPUT_START_FOLDED
        g_s = 1<<GROUPING_START | 1<<GROUPING_START_FOLDED
        i_m,o_m,g_m = 1<<INPUT_MIDDLE, 1<<OUTPUT_MIDDLE, 1<<GROUPING_MIDDLE
        i_e,o_e,g_e = 1<<INPUT_END, 1<<OUTPUT_END, 1<<GROUPING_END
        
        if (m1 & i_s) and (m1 & g_s):     #1
            if   (m2 & i_s) and (m2 & g_s): return True  #1
            elif (m2 & i_m) and (m2 & g_m): return True  #2
            elif (m2 & i_e) and (m2 & g_m): return True  #3
            elif (m2 & i_e) and (m2 & g_e): return True  #4
            elif (m2 & o_s) and (m2 & g_s): return False #5
            elif (m2 & o_s) and (m2 & g_m): return True  #6
            elif (m2 & o_s) and (m2 & g_e): return True  #7
            elif (m2 & o_m) and (m2 & g_m): return False #8
            elif (m2 & o_e) and (m2 & g_e): return False #9
            else: return False
        elif (m1 & i_m) and (m1 & g_m):   #2
            if   (m2 & i_m) and (m2 & g_m): return True  #2
            elif (m2 & i_e) and (m2 & g_m): return True  #3
            elif (m2 & i_e) and (m2 & g_e): return True  #4
            else: return False
        elif (m1 & i_e) and (m1 & g_m):   #3
            if   (m2 & o_s) and (m2 & g_m): return True  #6
            elif (m2 & o_s) and (m2 & g_e): return True  #7
            else: return False
        elif (m1 & i_e) and (m1 & g_e):   #4
            if   (m2 & i_s) and (m2 & g_s): return True  #1
            elif (m2 & o_s) and (m2 & g_s): return True  #5
            else: return False
        elif (m1 & o_s) and (m1 & g_s):   #5
            if   (m2 & i_s) and (m2 & g_s): return True  #1
            elif (m2 & i_m) and (m2 & g_m): return False #2
            elif (m2 & i_e) and (m2 & g_m): return False #3
            elif (m2 & i_e) and (m2 & g_e): return False #4
            elif (m2 & o_s) and (m2 & g_s): return True  #5
            elif (m2 & o_s) and (m2 & g_m): return False #6
            elif (m2 & o_s) and (m2 & g_e): return False #7
            elif (m2 & o_m) and (m2 & g_m): return True  #8
            elif (m2 & o_e) and (m2 & g_e): return True  #9
            else: return False
        elif (m1 & o_s) and (m1 & g_m):   #6
            if   (m2 & o_m) and (m2 & g_m): return True  #8
            elif (m2 & o_e) and (m2 & g_e): return True  #9
            else: return False
        elif (m1 & o_s) and (m1 & g_e):   #7
            if   (m2 & i_s) and (m2 & g_s): return True  #1
            elif (m2 & o_s) and (m2 & g_s): return True  #5
            else: return False
        elif (m1 & o_m) and (m1 & g_m):   #8
            if   (m2 & o_m) and (m2 & g_m): return True  #8
            elif (m2 & o_e) and (m2 & g_e): return True  #9
            else: return False
        elif (m1 & o_e) and (m1 & g_e):   #9
            if   (m2 & i_s) and (m2 & g_s): return True  #1
            elif (m2 & o_s) and (m2 & g_s): return True  #5
            else: return False
        else:
            return False
            
        
    def CleanAllMarkers(self):
        self.RestoreFirstMarker()
        first_marker=self.MarkerGet(0)
        last_line_num=self.GetLineCount()-1
        
        for i in range(1,last_line_num):
            self.ensureSingleGroupingMarker(i)
            self.ensureSingleIOMarker(i)
            
            previous_marker=self.MarkerGet(i-1)
            marker=self.MarkerGet(i)
            
            if not self.IsAllowedPair(previous_marker,marker):
                pass # FIX MARKER!!
            # FIX ME
    
    def write(self, text,type='Input',silent=False):
        """Display text in the slices shell.

        Replace line endings with OS-specific endings."""
        text = self.fixLineEndings(text)
        split=text.split(os.linesep)
        self.AddText(text)
        
        # This part handles all the marker stuff that accompanies
        # adding or removing new lines of text...
        # Get the total number of lines in the Document == last line number
        last_line_num=self.GetLineCount()-1
        # Get the line number we ended on in the write
        end_line_num=self.GetCurrentLine()
        # Get the number of returns we are using == number of lines we pasted -1
        num_new_lines=text.count(os.linesep)
        # So if num_new_lines==0, start_line_num and end_line_num are the same
        start_line_num=end_line_num-num_new_lines+1
        
        # This is a little unnecessary because there will always
        # be a line before if we just inserted a newline!
        if start_line_num == 0:
            previous_line_num=None
        else:
            previous_line_num=start_line_num-1
        
        #However, this is very important...
        if end_line_num == last_line_num:
            next_line_num=None
        else:
            next_line_num=end_line_num+1
        
        if type=='Input':
            start = INPUT_START
            start_folded = INPUT_START_FOLDED
            middle = INPUT_MIDDLE
            end = INPUT_END
            # preparation for more io types...
            opposite_start_mask = 1<<OUTPUT_START
            opposite_start_folded_mask = 1<<OUTPUT_START_FOLDED
            opposite_middle_mask = 1<<OUTPUT_MIDDLE # To test for bad writes...
            opposite_end_mask = 1<<OUTPUT_END # To test for bad writes...
        elif type in ['Output','Error']:
            #self.MarkerAdd(start_line_num,GROUPING_START_FOLDED)
            start=OUTPUT_START
            start_folded=OUTPUT_START_FOLDED
            middle=OUTPUT_MIDDLE
            end=OUTPUT_END
            # preparation for more io types...
            opposite_start_mask = 1<<INPUT_START
            opposite_start_folded_mask = 1<<INPUT_START_FOLDED
            opposite_middle_mask = 1<<INPUT_MIDDLE # To test for bad writes...
            opposite_end_mask = 1<<INPUT_END # To test for bad writes...
        
        if num_new_lines>0: #Do nothing if typing within a line...
            # Update the Grouping Markers
            # For the previous line and the start_line
            # Test to make sure we can write ... but not here ...
            #    test this before we call write or before we add text...
            # So we assume it already obeys the rules 
            
            badMarkers=False
            fixIOEnd=True
            
            if previous_line_num==None:
                # This is an impossible case, here just for completeness...
                self.clearGroupingMarkers(start_line_num)
                self.MarkerAdd(start_line_num,GROUPING_START)
                
                self.clearIOMarkers(start_line_num)
                self.MarkerAdd(start_line_num,start)
                if type in ['Output','Error']: self.MarkerAdd(start_line_num,OUTPUT_BG)
            else:
                previous_marker=self.MarkerGet(previous_line_num)
                if previous_marker & opposite_middle_mask:
                    badMarkers=True
            
            if next_line_num==None:
                self.MarkerAdd(end_line_num,GROUPING_END)
                self.MarkerAdd(end_line_num,end)
                if type in ['Output','Error']: self.MarkerAdd(end_line_num,OUTPUT_BG)
                fixEndMarkers=False
                # May be overwritten below if start_line_num==end_line_num...
            else:
                next_marker=self.MarkerGet(next_line_num)
                fixEndMarkers=True
                if next_marker & ( opposite_middle_mask | opposite_end_mask ):
                    badMarkers=True
            
            if not badMarkers:
                # ensure previous_line only has one marker & turn end into middle
                if previous_line_num!=None:
                    # Adjust previous line appropriately, ensure only one marker
                    # Only print errors if we are on input!
                    blank=False
                    blank=blank or self.ensureSingleGroupingMarker(previous_line_num)
                    blank=blank or self.ensureSingleIOMarker(previous_line_num)
                    
                    if blank:
                        #if type=='Input' and not silent: print 'BLANK LINE!' # BAD CASE
                        pass
                    
                    if previous_marker & 1<<GROUPING_END :
                        # Make GROUPING slice continue unless we hit
                        #  an output end and are starting a new input...
                        if (previous_marker & OUTPUT_MASK) and type=='Input':
                            pass
                        else:
                            self.MarkerDelete(previous_line_num,GROUPING_END)
                            # ONLY CHANGING CASE
                            self.MarkerAdd(previous_line_num,GROUPING_MIDDLE)
                    
                    if previous_marker & 1<<end :
                        self.MarkerDelete(previous_line_num,end)
                        self.MarkerAdd(previous_line_num,middle) # ONLY CHANGING CASE
                        if type in ['Output','Error']: self.MarkerAdd(previous_line_num,OUTPUT_BG)
                    elif previous_marker & opposite_middle_mask :
                         # BAD CASE
                        if type=='Input' and not silent:
                            #print 'Should have been a bad marker!'
                            pass
                    
                    # We can only add input to an input slice
                    # And can only add output to an output slice
                    
                    if previous_marker & ( opposite_start_mask |
                                           opposite_start_folded_mask |
                                           opposite_end_mask ):
                        if type=='Input':
                            self.clearGroupingMarkers(start_line_num)
                            self.MarkerAdd(start_line_num,GROUPING_START)
                            if start_line_num==end_line_num:
                                fixEndMarkers=False
                        else:
                            if start_line_num==end_line_num:
                                fixIOEnd=False
                        self.clearIOMarkers(start_line_num)
                        self.MarkerAdd(start_line_num,start)
                        if type in ['Output','Error']: self.MarkerAdd(start_line_num,OUTPUT_BG)
                    else:
                        if next_line_num!=None:
                            self.clearGroupingMarkers(start_line_num)
                            self.clearIOMarkers(start_line_num)
                            self.MarkerAdd(start_line_num,GROUPING_MIDDLE)
                            self.MarkerAdd(start_line_num,middle)
                            if type in ['Output','Error']: self.MarkerAdd(start_line_num,OUTPUT_BG)
                            # This may be overwritten if start_line_num==end_line_num
                
                # Take care of all the middle lines...
                # Does nothing for only one line...
                for i in range(start_line_num,end_line_num):
                    self.clearGroupingMarkers(i)
                    self.MarkerAdd(i,GROUPING_MIDDLE)
                    
                    self.clearIOMarkers(i)
                    self.MarkerAdd(i,middle)
                    if type in ['Output','Error']: self.MarkerAdd(i,OUTPUT_BG)
                
                if fixEndMarkers:
                    # Take care of the end_line if we haven't already done so...
                    blank=False
                    blank=blank or self.ensureSingleGroupingMarker(next_line_num)
                    blank=blank or self.ensureSingleIOMarker(next_line_num)
                    
                    if blank:
                        if type=='Input' and not silent:
                            #print 'BLANK LINE!' # BAD CASE
                            pass
                    
                    self.clearGroupingMarkers(end_line_num)
                    if fixIOEnd:
                        self.clearIOMarkers(end_line_num)
                    
                    if next_marker & ( 1<<GROUPING_START | 1<<GROUPING_START_FOLDED ) :
                        self.MarkerAdd(end_line_num,GROUPING_END)
                    elif next_marker & ( 1<<GROUPING_MIDDLE | 1<<GROUPING_END ) :
                        self.MarkerAdd(end_line_num,GROUPING_MIDDLE)
                    
                    if fixIOEnd: 
                        if next_marker & ( 1<<start | 1<<start_folded ) :
                            self.MarkerAdd(end_line_num,end)
                            if type in ['Output','Error']: self.MarkerAdd(end_line_num,OUTPUT_BG)
                        elif next_marker & ( 1<<middle | 1<<end ) :
                            self.MarkerAdd(end_line_num,middle)
                            if type in ['Output','Error']: self.MarkerAdd(end_line_num,OUTPUT_BG)
                        elif next_marker & ( opposite_start_mask |
                                             opposite_start_folded_mask ):
                            self.MarkerAdd(end_line_num,end)
                            if type in ['Output','Error']: self.MarkerAdd(end_line_num,OUTPUT_BG)
                        else:
                            self.MarkerAdd(end_line_num,start_folded)
                            if type in ['Output','Error']: self.MarkerAdd(end_line_num,OUTPUT_BG)
                            if type=='Input' and not silent:
                                #print 'BAD MARKERS!'
                                pass
            else:
                if type=='Input' and not silent:
                    #print 'BAD MARKERS!!!'
                    pass
        
        self.EnsureCaretVisible()
        
        if self.waiting:
            if self.lastUpdate==None:
                self.lastUpdate=time.time()
            if time.time()-self.lastUpdate > PRINT_UPDATE_MAX_TIME:
                self.Update()
                self.lastUpdate=time.time()
    
    def fixLineEndings(self, text):
        """Return text with line endings replaced by OS-specific endings."""
        lines = text.split('\r\n')
        for l in range(len(lines)):
            chunks = lines[l].split('\r')
            for c in range(len(chunks)):
                chunks[c] = os.linesep.join(chunks[c].split('\n'))
            lines[l] = os.linesep.join(chunks)
        text = os.linesep.join(lines)
        return text

    def prompt(self): # Autoindent added!!!
        """Display proper prompt for the context: ps1, ps2 or ps3.
        
        If this is a continuation line, autoindent as necessary."""
        # TODO : How much of this can I do away with now without prompts??
        
        isreading = self.reader.isreading
        
        skip = True
        if isreading:
            prompt = str(sys.ps3)
        elif self.more:
            prompt = str(sys.ps2)
        else:
            prompt = str(sys.ps1)
        pos = self.GetCurLine()[1]
        if pos > 0:
            if isreading:
                skip = True
            else:
                self.write(os.linesep,type='Input')
        if not self.more:
            # Not needed anymore! # self.promptPosStart = self.GetCurrentPos()
            pass
        if not skip:
            self.write(prompt,type='Input')
        if not self.more:
            # Not needed anymore! # self.promptPosEnd = self.GetCurrentPos()
            # Clear the undo history after running a command.
            self.EmptyUndoBuffer()
        
        #DNM/CP
        # Autoindent magic
        # Match the indent of the line above
        # UNLESS the line above ends in a colon...then add four spaces
        # (after valid keywords (if, else, etc...) only)
        if self.more:
            line_num=self.GetCurrentLine()
            currentLine=self.GetLine(line_num)
            previousLine=self.GetLine(line_num-1)
            pstrip=previousLine.strip()
            lstrip=previousLine.lstrip()
            
            if pstrip == '':
                # because it is all whitespace!
                indent=previousLine.strip('\n').strip('\r')
            else:
                indent=previousLine[:(len(previousLine)-len(lstrip))]
                if testForContinuations(previousLine,ignoreErrors=True)[1][0]:
                    indent+=' '*4
            
            #ADD UNDO
            cpos=self.GetCurrentPos()
            s=indent
            self.UpdateUndoHistoryBefore('insert',s,cpos,cpos+len(s))
            self.write(s,type='Input')
            self.UpdateUndoHistoryAfter()
            
            
        self.EnsureCaretVisible()
        self.ScrollToColumn(0)

    def readline(self):
        """Replacement for stdin.readline()."""
        input = ''
        reader = self.reader
        reader.isreading = True
        self.prompt()
        
        # Ensure that we get a new line and that it's got an input marker...
        # Also need to temporarily block any other action...
        cLine = self.GetCurrentLine()
        self.clearIOMarkers(cLine)
        self.MarkerAdd(cLine,INPUT_START)
        self.MarkerAdd(cLine,READLINE_BG)
        self.MarkerAdd(cLine,INPUT_READLINE)
        
        try:
            while not reader.input:
                wx.YieldIfNeeded()
            input = reader.input
        finally:
            start,end = self.GetIOSlice()
            start = self.runningSlice[1] + 1
            for i in range(start,end+1):
                self.clearIOMarkers(i)
                self.clearGroupingMarkers(i)
                self.MarkerAdd(i,OUTPUT_BG)
                if i == start:    self.MarkerAdd(i,OUTPUT_START)
                elif i==end:      self.MarkerAdd(i,OUTPUT_END)
                else:             self.MarkerAdd(i,OUTPUT_MIDDLE)
                
                if i==end:        self.MarkerAdd(i,GROUPING_END)
                else:             self.MarkerAdd(i,GROUPING_MIDDLE)
            reader.input = ''
            reader.isreading = False
        input = str(input)  # In case of Unicode.
        return input

    def readlines(self):
        """Replacement for stdin.readlines()."""
        lines = []
        while lines[-1:] != ['\n']:
            lines.append(self.readline())
        return lines

    def raw_input(self, prompt=''):
        """Return string based on user input."""
        if prompt:
            self.write(prompt,type='Output')
        return self.readline()

    def ask(self, prompt='Please enter your response:'):
        """Get response from the user using a dialog box."""
        dialog = wx.TextEntryDialog(None, prompt,
                                    'Input Dialog (Raw)', '')
        try:
            if dialog.ShowModal() == wx.ID_OK:
                text = dialog.GetValue()
                return text
        finally:
            dialog.Destroy()
        return ''

    def pause(self):
        """Halt execution pending a response from the user."""
        self.ask('Press enter to continue:')

    def clear(self):
        """Delete all text from the slices shell."""
        self.ClearAll()
        self.MarkerAdd(0,GROUPING_START)
        self.MarkerAdd(0,INPUT_START)

    def run(self, command, prompt=True, verbose=True):
        """Execute command as if it was typed in directly.
        >>> shell.run('print "this"')
        >>> print "this"
        this
        >>>
        """
        # Go to the very bottom of the text.
        endpos = self.GetTextLength()
        self.SetCurrentPos(endpos)
        command = command.rstrip()
        if prompt: self.prompt()
        if verbose: self.write(command,type='Input')
        self.push(command)

    # TODO : Will have to fix this to handle other kinds of errors mentioned before...
    def runfile(self, filename):
        """Execute all commands in file as if they were typed into the shell."""
        file = open(filename)
        try:
            self.prompt()
            for command in file.readlines():
                if command[:6] == 'shell.':
                    # Run shell methods silently.
                    self.run(command, prompt=False, verbose=False)
                else:
                    self.run(command, prompt=False, verbose=True)
        finally:
            file.close()

    def autoCompleteShow(self, command, offset = 0):
        """Display auto-completion popup list."""
        self.AutoCompSetAutoHide(self.autoCompleteAutoHide)
        self.AutoCompSetIgnoreCase(self.autoCompleteCaseInsensitive)
        list = self.interp.getAutoCompleteList(command,
                    includeMagic=self.autoCompleteIncludeMagic,
                    includeSingle=self.autoCompleteIncludeSingle,
                    includeDouble=self.autoCompleteIncludeDouble)
        if list:
            options = ' '.join(list)
            #offset = 0
            self.AutoCompShow(offset, options)

    def autoCallTipShow(self, command, insertcalltip = True, forceCallTip = False):
        """Display argument spec and docstring in a popup window."""
        if self.CallTipActive():
            self.CallTipCancel()
        (name, argspec, tip) = self.interp.getCallTip(command)
        if tip:
            dispatcher.send(signal='SlicesShell.calltip', sender=self, calltip=tip)
        if not self.autoCallTip and not forceCallTip:
            return
        startpos = self.GetCurrentPos()
        if argspec and insertcalltip and self.callTipInsert:
            # write with undo history...
            cpos=self.GetCurrentPos()
            s=argspec + ')'
            self.UpdateUndoHistoryBefore('insert',s,cpos,cpos+len(s))
            self.write(s,type='Input')
            self.UpdateUndoHistoryAfter()
            
            endpos = self.GetCurrentPos()
            self.SetSelection(startpos, endpos)
        if tip:
            tippos = startpos - (len(name) + 1)
            fallback = startpos - self.GetColumn(startpos)
            # In case there isn't enough room, only go back to the fallback.
            tippos = max(tippos, fallback)
            self.CallTipShow(tippos, tip)
    
    def OnCallTipAutoCompleteManually (self, shiftDown):
        """AutoComplete and Calltips manually."""
        if self.AutoCompActive():
            self.AutoCompCancel()
        currpos = self.GetCurrentPos()
        stoppos = self.PositionFromLine(self.GetIOSlice()[0])

        cpos = currpos
        #go back until '.' is found
        pointavailpos = -1
        while cpos >= stoppos:
            if self.GetCharAt(cpos) == ord ('.'):
                pointavailpos = cpos
                break
            cpos -= 1

        #word from non whitespace until '.'
        if pointavailpos != -1:
            #look backward for first whitespace char
            textbehind = self.GetTextRange (pointavailpos + 1, currpos)
            pointavailpos += 1

            if not shiftDown:
                #call AutoComplete
                stoppos = self.PositionFromLine(self.GetIOSlice()[0])
                textbefore = self.GetTextRange(stoppos, pointavailpos)
                self.autoCompleteShow(textbefore, len (textbehind))
            else:
                #call CallTips
                cpos = pointavailpos
                begpos = -1
                while cpos > stoppos:
                    if chr(self.GetCharAt(cpos)).isspace():
                        begpos = cpos
                        break
                    cpos -= 1
                if begpos == -1:
                    begpos = cpos
                ctips = self.GetTextRange (begpos, currpos)
                ctindex = ctips.find ('(')
                if ctindex != -1 and not self.CallTipActive():
                    #insert calltip, if current pos is '(', otherwise show it only
                    self.autoCallTipShow( ctips[:ctindex + 1], 
                          self.GetCharAt(currpos - 1) == ord('(') and
                              self.GetCurrentPos() == self.GetTextLength(),
                          True )
                

    def writeOut(self, text):
        """Replacement for stdout."""
        self.write(text,type='Output')
        # TODO : FLUSH?? How to make this update real-time...

    def writeErr(self, text):
        """Replacement for stderr."""
        self.write(text,type='Error')

    def redirectStdin(self, redirect=True):
        """If redirect is true then sys.stdin will come from the shell."""
        if redirect:
            sys.stdin = self.reader
        else:
            sys.stdin = self.stdin

    def redirectStdout(self, redirect=True):
        """If redirect is true then sys.stdout will go to the shell."""
        if redirect:
            sys.stdout = PseudoFileOut(self.writeOut)
        else:
            sys.stdout = self.stdout

    def redirectStderr(self, redirect=True):
        """If redirect is true then sys.stderr will go to the shell."""
        if redirect:
            sys.stderr = PseudoFileErr(self.writeErr)
        else:
            sys.stderr = self.stderr
    
    # Take a spashot of the WHOLE grouping slice (or slices)
    # The argument s is either what got added or deleted
    def UpdateUndoHistoryBefore(self,actionType,s,posStart,posEnd,
                                forceNewAction=False):
        uH=self.undoHistory
        uI=self.undoIndex
        
        s=s.replace(os.linesep,'\n')
        startLine=self.LineFromPosition(posStart)
        
        if actionType=='marker':
            numLines = self.LineFromPosition(posEnd) - startLine
        else:
            numLines=s.count('\n')
        
        makeNewAction=forceNewAction
        
        if forceNewAction:
            makeNewAction=True
        elif self.undoIndex==-1:
            makeNewAction=True
        elif not uH[uI]['allowsAppend']:
            makeNewAction=True
        elif actionType!=uH[uI]['actionType']:
            makeNewAction=True
        elif actionType=='insert':
            if posStart!=uH[uI]['posEnd']:
                makeNewAction=True
            else: # This is a continuation of the previous insert
                uH[uI]['charList'] = uH[uI]['charList']+s
                uH[uI]['posEnd']   = posEnd # posStart cannot move
                uH[uI]['numLines'] = uH[uI]['numLines']+numLines
        elif actionType=='delete':
            # This is a forward continuation of the previous delete
            if posStart==uH[uI]['posStart']:
                uH[uI]['charList'] = uH[uI]['charList']+s
                uH[uI]['posEnd'] = posEnd
                uH[uI]['numLines'] = uH[uI]['numLines']+numLines
            # This is a backward continuation of the previous delete
            elif posEnd==uH[uI]['posStart']:
                uH[uI]['charList'] = s+uH[uI]['charList']
                uH[uI]['posStart'] = posStart
                uH[uI]['startLine'] = startLine
                uH[uI]['numLines'] = uH[uI]['numLines']+numLines
            else:
                makeNewAction=True
            
        elif actionType=='marker':
            makeNewAction=True
        else:
            pass #print 'Unsupported Action Type!!'
        
        if makeNewAction:
            del(self.undoHistory[uI+1:]) # remove actions after undoIndex
            
            uH.append({
              'actionType' : actionType,    # Action type ('insert','delete','marker')
              'allowsAppend': not forceNewAction, # Can action be joined with others?
              'charList' : s,        # Character list
              'posStart' : posStart, # Cursor poition at the start of the action
              'posEnd' : posEnd,     # Cursor position at the end of the action
              'startLine' : startLine,  # Start line number,
              'numLines' : numLines,  # Number of newlines involved
              'mBStart' : None,       # Starting line for markers BEFORE action
              'mAStart' : None,       # Starting line for markers AFTER action
              'markersBefore' : None, # [markers BEFORE action]
              'markersAfter' : None   # [markers AFTER action]
             })
            
            self.undoIndex+=1
            
            # Only update the before when starting a new action
            start = startLine
            if actionType=='insert':
                end = start
            else:
                end = start + numLines
            
            # Update Marker Info
            newStart=self.GetGroupingSlice(start)[0]
            newEnd=self.GetGroupingSlice(end)[1]
            self.undoHistory[self.undoIndex]['markersBefore'] = \
                 [self.MarkerGet(i) for i in range(newStart,newEnd+1)]
            self.undoHistory[self.undoIndex]['mBStart']=newStart
        
        self.doHistUpdate=True
    
    def UpdateUndoHistoryAfter(self): # s is either what got added or deleted
        start = self.undoHistory[self.undoIndex]['startLine']
        if self.undoHistory[self.undoIndex]['actionType']=='delete':
            end = start
        else:
            end = start + self.undoHistory[self.undoIndex]['numLines']
        
        newStart=min(self.GetGroupingSlice(start)[0]-1, 0)
        newEnd=max(self.GetGroupingSlice(end)[1]+1, self.GetLineCount()-1)
        self.undoHistory[self.undoIndex]['markersAfter'] = \
             [self.MarkerGet(i) for i in range(newStart,newEnd+1)]
        self.undoHistory[self.undoIndex]['mAStart']=newStart
        
        self.doHistUpdate=False
    
    def Undo(self):
        #ADD UNDO
        #Skip undo if there are no actions...
        if self.undoIndex==-1:
            return
        
        uHI=self.undoHistory[self.undoIndex]
        
        if uHI['actionType'] in ['insert','delete']:
            # This will perform the opposite of the action given
            editwindow.EditWindow.Undo(self)
        elif uHI['actionType']=='marker': # No text changed, don't pass to STC
            pass
        else:
            #print 'Unsupported actionType in undoHistory!!'
            return
        
        numLines=len(uHI['markersBefore'])
        for i in range(numLines):
            self.MarkerSet( uHI['mBStart']+i , uHI['markersBefore'][i] )
        
        self.undoIndex-=1
    
    def Redo(self):
        #ADD UNDO
        # First check to see if there are any redo operations available
        # Note that for redo, undoIndex=-1 is a valid input
        if self.undoIndex >= len(self.undoHistory)-1:
            return
        self.undoIndex+=1
        uHI=self.undoHistory[self.undoIndex]
        
        if uHI['actionType'] in ['insert','delete']:
            # This will re-perform the given action
            editwindow.EditWindow.Redo(self)
        elif uHI['actionType']=='marker': # No text changed, don't pass to STC
            pass
        else:
            #print 'Unsupported actionType in undoHistory!!'
            return
        
        numLines=len(uHI['markersAfter'])
        for i in range(numLines):
            self.MarkerSet( uHI['mAStart']+i , uHI['markersAfter'][i] )
    
    def EmptyUndoBuffer(self):
        editwindow.EditWindow.EmptyUndoBuffer(self)
        self.undoIndex=-1
        self.undoHistory=[]
        self.doHistUpdate=False
    
    def CanCut(self):
        return self.CanEdit() and \
               (self.GetSelectionStart() != self.GetSelectionEnd())
    
    def CanPaste(self):
        """Return true if a paste should succeed."""
        if self.CanEdit() and editwindow.EditWindow.CanPaste(self):
            return True
        else:
            return False

    def CanEdit(self):
        """Return true if editing should succeed."""
        marker=self.MarkerGet(self.GetCurrentLine())
        
        if marker & OUTPUT_MASK:
            return False
        elif marker & INPUT_MASK:
            if self.reader.isreading and not \
                    (self.MarkerGet(self.GetCurrentLine()) & 1<<INPUT_READLINE ):
                return False
            start,end=self.GetIOSlice()
            sliceStartPos=self.PositionFromLine(start)
            sliceEndPos=self.GetLineEndPosition(end)
            """Return true if text is selected and can be cut."""
            if self.GetSelectionStart() == self.GetSelectionEnd():
                return True
            elif self.GetSelectionStart() != self.GetSelectionEnd() \
                   and self.GetSelectionStart() >= sliceStartPos \
                   and self.GetSelectionEnd() >= sliceStartPos \
                   and self.GetSelectionStart() <= sliceEndPos \
                   and self.GetSelectionEnd() <= sliceEndPos:
                return True
            else:
                return False

    def Cut(self):
        """Remove selection and place it on the clipboard."""
        if self.CanCut() and self.CanCopy():
            if self.AutoCompActive():
                self.AutoCompCancel()
            if self.CallTipActive():
                self.CallTipCancel()
            self.Copy()
            self.ReplaceSelection('')

    def Copy(self):
        """Copy selection and place it on the clipboard."""
        if self.CanCopy():
            ps1 = str(sys.ps1)
            ps2 = str(sys.ps2)
            command = self.GetSelectedText()
            command = command.replace(os.linesep + ps2, os.linesep)
            command = command.replace(os.linesep + ps1, os.linesep)
            command = self.lstripPrompt(text=command)
            data = wx.TextDataObject(command)
            self._clip(data)

    def CopyWithPrompts(self):
        """Copy selection, including prompts, and place it on the clipboard."""
        if self.CanCopy():
            command = self.GetSelectedText()
            data = wx.TextDataObject(command)
            self._clip(data)

    def CopyWithPromptsPrefixed(self):
        """Copy selection, including prompts prefixed with four
        spaces, and place it on the clipboard."""
        if self.CanCopy():
            command = self.GetSelectedText()
            spaces = ' ' * 4
            command = spaces + command.replace(os.linesep,
                                               os.linesep + spaces)
            data = wx.TextDataObject(command)
            self._clip(data)

    def _clip(self, data):
        if wx.TheClipboard.Open():
            wx.TheClipboard.UsePrimarySelection(False)
            wx.TheClipboard.SetData(data)
            wx.TheClipboard.Flush()
            wx.TheClipboard.Close()

    def Paste(self):
        """Replace selection with clipboard contents."""
        
        #ADD UNDO
        if self.CanPaste() and wx.TheClipboard.Open():
            ps2 = str(sys.ps2)
            if wx.TheClipboard.IsSupported(wx.DataFormat(wx.DF_TEXT)):
                data = wx.TextDataObject()
                if wx.TheClipboard.GetData(data):
                    self.ReplaceSelection('')
                    command = data.GetText()
                    command = command.rstrip()
                    command = self.fixLineEndings(command)
                    command = self.lstripPrompt(text=command)
                    # TODO : This is still useful... Add it back other places?
                    command = command.replace(os.linesep + ps2, '\n')
                    command = command.replace(os.linesep, '\n')
                    #DNM--Don't use '... '
                    command = command.replace('\n', os.linesep)# + ps2)
                    
                    cpos=self.GetCurrentPos()
                    s=command
                    self.UpdateUndoHistoryBefore('insert', s, cpos,
                                            cpos+len(s), forceNewAction=True)
                    self.write(s,type='Input')
                    self.UpdateUndoHistoryAfter()
                    
                    # Makes paste -> type -> undo consistent with other STC apps
                    self.ReplaceSelection('')
            wx.TheClipboard.Close()
    
    
    def PasteAndRun(self):
        """Replace selection with clipboard contents, run commands."""
        text = ''
        if wx.TheClipboard.Open():
            if wx.TheClipboard.IsSupported(wx.DataFormat(wx.DF_TEXT)):
                data = wx.TextDataObject()
                if wx.TheClipboard.GetData(data):
                    text = data.GetText()
            wx.TheClipboard.Close()
        if text:
            self.Execute(text)
            
    
    def Execute(self, text):
        """Replace selection with text and run commands."""
        start,end=self.GetIOSlice()
        startpos=self.PositionFromLine(start)
        endpos=self.GetLineEndPosition(end)
        
        self.SetCurrentPos(endpos)
        self.SetSelection(startpos, endpos)
        self.ReplaceSelection('')
        
        hasSyntaxError=False
        result = self.BreakTextIntoCommands(command)
        if result[0] == None:
            commands=[command]
            hasSyntaxError=True
        else:
            commands=result
        
        for command in commands:
            command = command.replace('\n', os.linesep)
            self.write(command)
            self.processLine()

    def wrap(self, wrap=True):
        """Sets whether text is word wrapped."""
        try:
            self.SetWrapMode(wrap)
        except AttributeError:
            return 'Wrapping is not available in this version.'

    def zoom(self, points=0):
        """Set the zoom level.

        This number of points is added to the size of all fonts.  It
        may be positive to magnify or negative to reduce."""
        self.SetZoom(points)

    def LoadSettings(self, config):
        self.autoComplete = \
                    config.ReadBool('Options/AutoComplete', True)
        self.autoCompleteIncludeMagic = \
                    config.ReadBool('Options/AutoCompleteIncludeMagic', True)
        self.autoCompleteIncludeSingle = \
                    config.ReadBool('Options/AutoCompleteIncludeSingle', True)
        self.autoCompleteIncludeDouble = \
                    config.ReadBool('Options/AutoCompleteIncludeDouble', True)
        self.autoCallTip = \
                    config.ReadBool('Options/AutoCallTip', True)
        self.callTipInsert = \
                    config.ReadBool('Options/CallTipInsert', True)
        
        self.SetWrapMode(config.ReadBool('View/WrapMode', True))

        self.lineNumbers = \
                    config.ReadBool('View/ShowLineNumbers', True)
        self.setDisplayLineNumbers (self.lineNumbers)
        zoom = config.ReadInt('View/Zoom/Shell', -99)
        if zoom != -99:
            self.SetZoom(zoom)


    
    def SaveSettings(self, config):
        config.WriteBool('Options/AutoComplete', self.autoComplete)
        config.WriteBool('Options/AutoCompleteIncludeMagic',
                                            self.autoCompleteIncludeMagic)
        config.WriteBool('Options/AutoCompleteIncludeSingle',
                                            self.autoCompleteIncludeSingle)
        config.WriteBool('Options/AutoCompleteIncludeDouble',
                                            self.autoCompleteIncludeDouble)
        config.WriteBool('Options/AutoCallTip', self.autoCallTip)
        config.WriteBool('Options/CallTipInsert', self.callTipInsert)
        config.WriteBool('View/WrapMode', self.GetWrapMode())
        config.WriteBool('View/ShowLineNumbers', self.lineNumbers)
        config.WriteInt('View/Zoom/Shell', self.GetZoom())

    def GetContextMenu(self):
        """
            Create and return a context menu for the slices shell.
            This is used instead of the scintilla default menu
            in order to correctly respect our immutable buffer.
        """
        menu = wx.Menu()
        menu.Append(wx.ID_UNDO, "Undo")
        menu.Append(wx.ID_REDO, "Redo")
        
        menu.AppendSeparator()
        
        menu.Append(wx.ID_CUT, "Cut")
        menu.Append(wx.ID_COPY, "Copy")
        menu.Append(frame.ID_COPY_PLUS, "Copy Plus")
        menu.Append(wx.ID_PASTE, "Paste")
        menu.Append(frame.ID_PASTE_PLUS, "Paste Plus")
        menu.Append(wx.ID_CLEAR, "Clear")
        
        menu.AppendSeparator()
        
        menu.Append(wx.ID_SELECTALL, "Select All")
        return menu
        
    def OnContextMenu(self, evt):
        menu = self.GetContextMenu()
        self.PopupMenu(menu)
        
    def OnUpdateUI(self, evt):
        id = evt.Id
        if id in (wx.ID_CUT, wx.ID_CLEAR):
            evt.Enable(self.CanCut())
        elif id in (wx.ID_COPY, frame.ID_COPY_PLUS):
            evt.Enable(self.CanCopy())
        elif id in (wx.ID_PASTE, frame.ID_PASTE_PLUS):
            evt.Enable(self.CanPaste())
        elif id == wx.ID_UNDO:
            evt.Enable(self.CanUndo())
        elif id == wx.ID_REDO:
            evt.Enable(self.CanRedo())

    def LoadPySlicesFile(self,fid):
        invalidFileString = 'Not a valid input format'
        lineCount=0
        groupingStartLines=[0]
        ioStartLines=[0]
        ioStartTypes=[]
        removeComment=False
        
        # Read the initial three (or four) lines that have version and marker information
        line=fid.readline()
        if line == usrBinEnvPythonText:
            line=fid.readline() # Add the option to place #!/usr/bin/env python at the top
        if line not in pyslicesFormatHeaderText:  print invalidFileString ; return
        line=fid.readline()
        if line != groupingStartText:  print invalidFileString ; return
        line=fid.readline()
        if line == inputStartText:      ioStartTypes.append('input');removeComment=False
        elif line == outputStartText:   ioStartTypes.append('output');removeComment=True
        else:  print invalidFileString ; return
        
        self.ClearAll()
        
        # Write the file's text to the text area
        # Capture Marker information to
        for i in fid:
            if i==groupingStartText:
                groupingStartLines.append(lineCount)
            elif i==inputStartText:
                ioStartLines.append(lineCount)
                ioStartTypes.append('input')
                removeComment=False
            elif i==outputStartText:
                ioStartLines.append(lineCount)
                ioStartTypes.append('output')
                removeComment=True
            else:
                if removeComment:   w=i[1:].replace(os.linesep,'\n')
                else:               w=i.replace(os.linesep,'\n')
                self.write(w,'Input',silent=True)
                lineCount+=1
        
        if w[-1]=='\n':
            lineCount+=1
        
        for i in range(lineCount+1):
            self.clearGroupingMarkers(i)
            self.clearIOMarkers(i)
            
            doMiddle=False
            doEnd=False
            if groupingStartLines!=[]:
                if i == groupingStartLines[0]:
                    self.MarkerAdd(i,GROUPING_START)
                    del groupingStartLines[0]
                elif i+1 == groupingStartLines[0]:
                    doEnd=True
                else:
                    doMiddle=True
            elif i==lineCount-1:
                doEnd=True
            else:
                doMiddle=True
            
            if doMiddle:
                self.MarkerAdd(i,GROUPING_MIDDLE)
            elif doEnd:
                self.MarkerAdd(i,GROUPING_END)
            
            doMiddle=False
            doEnd=False
            if ioStartLines!=[]:
                if i == ioStartLines[0]:
                    # Delete the old ioStartTypes (keep the current copy for later use)
                    if i>0: del ioStartTypes[0]
                    
                    if ioStartTypes[0]=='input':
                        self.MarkerAdd(i,INPUT_START)
                    elif ioStartTypes[0]=='output':
                        self.MarkerAdd(i,OUTPUT_START)
                        self.MarkerAdd(i,OUTPUT_BG)
                    else:
                        #print 'Invalid Type!';
                        return
                    
                    # Only delete markers we are totally finished with...
                    # Keep one more "StartTypes" than "StartLines"
                    del ioStartLines[0]
                elif i+1 == ioStartLines[0]:
                    doEnd=True
                else:
                    doMiddle=True
            elif i==lineCount-1:
                doEnd=True
            else:
                doMiddle=True
            
            if doMiddle:
                if ioStartTypes[0]=='input':
                    self.MarkerAdd(i,INPUT_MIDDLE)
                elif ioStartTypes[0]=='output':
                    self.MarkerAdd(i,OUTPUT_MIDDLE)
                    self.MarkerAdd(i,OUTPUT_BG)
                else:
                    #print 'Invalid Type!';
                    return
            elif doEnd:
                if ioStartTypes[0]=='input':
                    self.MarkerAdd(i,INPUT_END)
                elif ioStartTypes[0]=='output':
                    self.MarkerAdd(i,OUTPUT_END)
                    self.MarkerAdd(i,OUTPUT_BG)
                else:
                    #print 'Invalid Type!';
                    return
                
        self.EmptyUndoBuffer() # maybe not?

    
    def SavePySlicesFile(self,fid):
        addComment=False
        fid.write(usrBinEnvPythonText.replace('\n',os.linesep))
        fid.write(pyslicesFormatHeaderText[-1].replace('\n',os.linesep))
        for i in range(self.GetLineCount()):
            markers=self.MarkerGet(i)
            if markers & ( 1<<GROUPING_START | 1<<GROUPING_START_FOLDED ):
                fid.write(groupingStartText.replace('\n',os.linesep))
            if markers & ( 1<<INPUT_START | 1<<INPUT_START_FOLDED ):
                fid.write(inputStartText.replace('\n',os.linesep))
                addComment=False
            if markers & ( 1<<OUTPUT_START | 1<<OUTPUT_START_FOLDED ):
                fid.write(outputStartText.replace('\n',os.linesep))
                addComment=True
            if addComment: fid.write('#')
            fid.write(self.GetLine(i).replace('\n',os.linesep))
    
    # FIX ME!!
    def LoadPyFileAsSlice(self,fid):
        curpos=self.GetCurrentPos()
        start,end = self.GetGroupingSlice()
        
        endpos=self.GetLineEndPosition(end)
        self.SetCurrentPos(endpos)
        self.SetSelection(endpos, endpos)
        
        text='\n'+fid.read()
        self.write(text,'Input')
        newpos=self.GetCurrentPos()
        
        self.SetCurrentPos(curpos)
        self.SetSelection(curpos,curpos)
        self.SplitSlice()
        #self.SetCurrentPos(newpos)
        #self.SetSelection(newpos,newpos)
    
    def hasChanged(self):
        """Return True if contents have changed."""
        return self.GetModify() or self.NeedsCheckForSave
        
        
        
## NOTE: The DnD of file names is disabled until we can figure out how
## best to still allow DnD of text.

## #seb : File drag and drop
## class FileDropTarget(wx.FileDropTarget):
##     def __init__(self, obj):
##         wx.FileDropTarget.__init__(self)
##         self.obj = obj
##     def OnDropFiles(self, x, y, filenames):
##         if len(filenames) == 1:
##             txt = 'r\"%s\"' % filenames[0]
##         else:
##             txt = '( '
##             for f in filenames:
##                 txt += 'r\"%s\" , ' % f
##             txt += ')'
##         self.obj.AppendText(txt)
##         pos = self.obj.GetCurrentPos()
##         self.obj.SetCurrentPos( pos )
##         self.obj.SetSelection( pos, pos )



## class TextAndFileDropTarget(wx.DropTarget):
##     def __init__(self, sliceshell):
##         wx.DropTarget.__init__(self)
##         self.sliceshell = sliceshell
##         self.compdo = wx.DataObjectComposite()
##         self.textdo = wx.TextDataObject()
##         self.filedo = wx.FileDataObject()
##         self.compdo.Add(self.textdo)
##         self.compdo.Add(self.filedo, True)
        
##         self.SetDataObject(self.compdo)
                
##     def OnDrop(self, x, y):
##         return True
    
##     def OnData(self, x, y, result):
##         self.GetData()
##         if self.textdo.GetTextLength() > 1:
##             text = self.textdo.GetText()
##             # *** Do somethign with the dragged text here...
##             self.textdo.SetText('')
##         else:
##             filenames = str(self.filename.GetFilenames())
##             if len(filenames) == 1:
##                 txt = 'r\"%s\"' % filenames[0]
##             else:
##                 txt = '( '
##                 for f in filenames:
##                     txt += 'r\"%s\" , ' % f
##                 txt += ')'
##             self.sliceshell.AppendText(txt)
##             pos = self.sliceshell.GetCurrentPos()
##             self.sliceshell.SetCurrentPos( pos )
##             self.sliceshell.SetSelection( pos, pos )

##         return result
