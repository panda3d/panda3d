#!/usr/bin/env python

# ------------------------------------------------------------------
# Display a splash screen as quickly as possible (before importing
# modules and initialising Pmw).

import tkinter
root = tkinter.Tk(className = 'Demo')
root.withdraw()

splash = tkinter.Toplevel()
splash.withdraw()
splash.title('Welcome to the Pmw demos')
text = tkinter.Label(splash,
    font=('Helvetica', 16, 'bold'),
    relief = 'raised',
    borderwidth = 2,
    padx=50, pady=50,
    text =
    'Welcome to the Pmw megawidgets demo.\n'
    '\n'
    'In a moment the main window will appear.\n'
    'Please enjoy yourself while you wait.\n'
    'You may be interested to know that splash screens\n'
    '(as this window is called) were first devised to draw\n'
    'attention away from the fact the certain applications\n'
    'are slow to start. They are normally flashier and more\n'
    'entertaining than this one. This is a budget model.'
)
text.pack(fill = 'both', expand = 1)
splash.update_idletasks()

width = splash.winfo_reqwidth()
height = splash.winfo_reqheight()
x = (root.winfo_screenwidth() - width) / 2 - root.winfo_vrootx()
y = (root.winfo_screenheight() - height) / 3 - root.winfo_vrooty()
if x < 0:
    x = 0
if y < 0:
    y = 0
geometry = '%dx%d+%d+%d' % (width, height, x, y)

splash.geometry(geometry)
splash.update_idletasks()
splash.deiconify()
root.update()

# ------------------------------------------------------------------

# Now crank up the application windows.

import imp
import os
import re
import string
import sys
import types
import tkinter
import DemoVersion
import Args

# Find where the other scripts are, so they can be listed.
if __name__ == '__main__':
    script_name = sys.argv[0]
else:
    script_name = imp.find_module('DemoVersion')[1]

script_name = os.path.normpath(script_name)
script_name = DemoVersion.expandLinks(script_name)
script_dir = os.path.dirname(script_name)
script_dir = DemoVersion.expandLinks(script_dir)

# Add the '../../..' directory to the path.
package_dir = os.path.dirname(script_dir)
package_dir = DemoVersion.expandLinks(package_dir)
package_dir = os.path.dirname(package_dir)
package_dir = DemoVersion.expandLinks(package_dir)
package_dir = os.path.dirname(package_dir)
package_dir = DemoVersion.expandLinks(package_dir)
sys.path[:0] = [package_dir]

# Import Pmw after modifying sys.path (it may not be in the default path).
import Pmw
DemoVersion.setPmwVersion()

class Demo(Pmw.MegaWidget):

    #these demos are blocked for various reasons
    #Blt demos are blocked because of segfault with BLT2.4z and Tcl8.5
    blocked_demos = ['BltGraph', 'BltTabset']

    def __init__(self, parent=None, **kw):
        # Define the megawidget options.
        optiondefs = ()
        self.defineoptions(kw, optiondefs)

        # Initialise the base class (after defining the options).
        Pmw.MegaWidget.__init__(self, parent)

        # Create the contents.
        top = self.interior()

        panes = Pmw.PanedWidget(top, orient = 'horizontal')
        panes.pack(fill = 'both', expand = 1)

        panes.add('widgetlist')
        self._widgetlist = Pmw.ScrolledListBox(panes.pane('widgetlist'),
                selectioncommand = Pmw.busycallback(self.startDemo),
                label_text = 'Select a widget:',
                labelpos = 'nw',
                vscrollmode = 'dynamic',
                hscrollmode = 'none',
                listbox_exportselection = 0)
        self._widgetlist.pack(fill = 'both', expand = 1, padx = 8)

        panes.add('info')
        self._status = tkinter.Label(panes.pane('info'))
        self._status.pack(padx = 8, anchor = 'w')

        self._example = tkinter.Frame(panes.pane('info'),
                borderwidth = 2,
                relief = 'sunken',
                background = 'white')
        self._example.pack(fill = 'both', expand = 1, padx = 8)

        self.buttonBox = Pmw.ButtonBox(top)
        self.buttonBox.pack(fill = 'x')

        # Add the buttons and make them all the same width.
        self._traceText = 'Trace tk calls'
        self._stopTraceText = 'Stop trace'
        self.buttonBox.add('Trace', text = self._traceText,
                command = self.trace)
        self.buttonBox.add('Code', text = 'Show code', command = self.showCode)
        self.buttonBox.add('Exit', text = 'Exit', command = root.destroy)
        self.buttonBox.alignbuttons()

        # Create the window to display the python code.
        self.codeWindow = Pmw.TextDialog(parent,
            title = 'Python source',
            buttons = ('Dismiss',),
            scrolledtext_labelpos = 'n',
            label_text = 'Source')
        self.codeWindow.withdraw()
        self.codeWindow.insert('end', '')

        self.demoName = None
        self._loadDemos()

        # Check keywords and initialise options.
        self.initialiseoptions()

    def startDemo(self):
        # Import the selected module and create and instance of the module's
        # Demo class.

        sels = self._widgetlist.getcurselection()
        if len(sels) == 0:
            print('No demonstrations to display')
            return
        demoName = sels[0]

        # Ignore if this if it is a sub title.
        if demoName[0] != ' ':
            self._widgetlist.bell()
            return

        # Strip the leading two spaces.
        demoName = demoName[2:]

        # Ignore if this demo is already being shown.
        if self.demoName == demoName:
            return

        #ignore if in the blocked demos
        if demoName in self.blocked_demos:
            return

        self.demoName = demoName

        self.showStatus('Loading ' + demoName)
        # Busy cursor
        self.update_idletasks()

        for window in self._example.winfo_children():
            window.destroy()

        frame = tkinter.Frame(self._example)
        frame.pack(expand = 1)
        exec('import ' + demoName)
        # Need to keep a reference to the widget, so that variables, etc
        # are not deleted.
        self.widget = eval(demoName + '.Demo(frame)')
        title = eval(demoName + '.title')
        self.showStatus(title)

        if self.codeWindow.state() == 'normal':
            self.insertCode()

    def showStatus(self, text):
        self._status.configure(text = text)

    def showCode(self):
        if self.codeWindow.state() != 'normal':
            if self.demoName is None:
                print('No demonstration selected')
                return
            self.insertCode()

        self.codeWindow.show()

    def insertCode(self):
        self.codeWindow.clear()
        fileName = os.path.join(script_dir, self.demoName + '.py')
        self.codeWindow.importfile(fileName)
        self.codeWindow.configure(label_text = self.demoName + ' source')

    def trace(self):
        text = self.buttonBox.component('Trace').cget('text')
        if text == self._traceText:
            self.buttonBox.configure(Trace_text = self._stopTraceText)
            Pmw.tracetk(root, 1)
            self.showStatus('Trace will appear on standard output')
        else:
            self.buttonBox.configure(Trace_text = self._traceText)
            Pmw.tracetk(root, 0)
            self.showStatus('Tk call tracing stopped')

    def _loadDemos(self):
        files = os.listdir(script_dir)
        files.sort()
        megawidgets = []
        others = []
        for file in files:
            if re.search('.py$', file) is not None and \
                    file not in ['All.py', 'DemoVersion.py', 'Args.py']:
                demoName = file[:-3]
                if demoName in self.blocked_demos:
                    continue
                index = demoName.find('_')
                if index < 0:
                    testattr = demoName
                else:
                    testattr = demoName[:index]
                if hasattr(Pmw, testattr):
                    megawidgets.append(demoName)
                else:
                    others.append(demoName)

        self._widgetlist.insert('end', 'Megawidget demos:')
        for name in megawidgets:
            self._widgetlist.insert('end', '  ' + name)
        self._widgetlist.insert('end', 'Other demos:')
        for name in others:
            self._widgetlist.insert('end', '  ' + name)
        self._widgetlist.select_set(1)

class StdOut:
    def __init__(self, displayCommand):
        self.displayCommand = displayCommand
        self.text = '\n'

    def write(self, text):
        if self.text[-1] == '\n':
            self.text = text
        else:
            self.text = self.text + text
        if self.text[-1] == '\n':
            text = self.text[:-1]
        else:
            text = self.text
        self.displayCommand(text)
    def flush(self):
        pass

if os.name == 'nt':
    defaultFontSize = 16
else:
    defaultFontSize = 12

commandLineArgSpecs = (
    ('fontscheme', 0, 'scheme',  'fonts to use [eg pmw2] (Tk defaults)'),
    ('fontsize',   0, 'num',     'size of fonts to use with fontscheme', defaultFontSize),
    ('stdout',     0, Args.Bool, 'print messages rather than display in label'),
)

program = 'All.py'
msg = Args.parseArgs(program, sys.argv, commandLineArgSpecs, 0)
if msg is not None:
    print(msg)
    sys.exit()

size = Args.get('fontsize')
fontScheme = Args.get('fontscheme')
Pmw.initialise(root, size = size, fontScheme = fontScheme, useTkOptionDb = 1)

root.title('Pmw ' + Pmw.version() + ' megawidget demonstration')
if size < 18:
    geometry = '800x550'
else:
    geometry = '1000x700'
root.geometry(geometry)

demo = Demo(root)
demo.pack(fill = 'both', expand = 1)
demo.focus()

# Redirect standard output from demos to status line (unless -stdout
# option given on command line).
if not Args.get('stdout'):
    sys.stdout = StdOut(demo.showStatus)

# Start the first demo.
demo.startDemo()

# Get rid of the splash screen
root.deiconify()
root.update()
splash.destroy()

root.mainloop()
