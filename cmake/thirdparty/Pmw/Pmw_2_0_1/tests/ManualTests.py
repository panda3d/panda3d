#!/usr/bin/env python

# This is a rough collection of tests that can not be automated.
# To add a new test, create a function with name ending in '_test'.

import os
import string
import time
import sys
import Test
import tkinter
import Pmw

# ----------------------------------------------------------------------

def scrolledframeflashing_test():
    # Script which demonstrates continuous flashing of dynamic scrollbars
    # in Pmw.ScrolledFrame.
    #
    # When this script is run, the two scrollbars will be continuously
    # mapped and unmapped and the window will continuously change size.

    frame = tkinter.Frame(root)
    frame.pack(fill = 'both', expand = 1)

    sf = Pmw.ScrolledFrame(frame, borderframe = 0)
    sf.pack(fill = 'both', expand = 1)

    inner = tkinter.Frame(sf.interior(),
            width = 401,
            height = 300,
            borderwidth = 0,
            highlightthickness = 0,
    )
    inner.pack(fill = 'both', expand = 1)

# ----------------------------------------------------------------------

def scrolledlistboxflashing_test():
    # Script which demonstrates continuous flashing of dynamic scrollbars
    # in Pmw.ScrolledListBox.
    #
    # When this script is run, the two scrollbars will be continuously
    # mapped and unmapped and the window will continuously change size.

    frame = tkinter.Frame(root)
    frame.pack(fill = 'both', expand = 1)

    sf = Pmw.ScrolledListBox(frame,
            listbox_width = 20,
            listbox_height = 10
    )
    sf.pack(fill = 'both', expand = 1)
    for i in range(11):
        sf.insert('end', '2' * 20)

# ----------------------------------------------------------------------

def scrolledlistboxflashing2_test():
    # Another script which demonstrates continuous flashing of dynamic
    # scrollbars in Pmw.ScrolledListBox under Pmw.0.8.
    #
    # When this script is run, the two scrollbars will be continuously
    # mapped and unmapped and the window will continuously change size.
    #
    # (This did not display error when tried with Pmw.0.8, 99/8/3)

    def insert():
        sectionList = ['1', '2', '3', '4', '5', '6', '7', '8', '9',
            '123456789012345678901']
        for counter in sectionList: 
          slb.insert('end', counter)
          
    def clear():
        slb.delete(0, 'end')
        
    global slb
    slb = Pmw.ScrolledListBox(root)
    slb.pack()

    root.after(2000,insert)
    root.after(3000,clear) 
    root.after(4000,insert)

    root.geometry('400x400')

# ----------------------------------------------------------------------

def scrolledtextflashing_test():
    # Script which demonstrates continuous flashing of dynamic scrollbars
    # in Pmw.ScrolledText.
    #
    # When this script is run, the two scrollbars will be continuously
    # mapped and unmapped and the window will continuously change size.

    frame = tkinter.Frame(root)
    frame.pack(fill = 'both', expand = 1)

    sf = Pmw.ScrolledText(frame,
            text_width = 20,
            text_height = 10,
            text_wrap = 'none',
            borderframe = 0
    )
    sf.pack(fill = 'both', expand = 1)
    for i in range(11):
        sf.insert('end', '2' * 20)
        if i != 10:
            sf.insert('end', '\n')

# ----------------------------------------------------------------------

def scrolledcanvasflashing_test():
    # Script which demonstrates continuous flashing of dynamic scrollbars
    # in Pmw.ScrolledCanvas.
    #
    # When this script is run, the two scrollbars will be continuously
    # mapped and unmapped and the window will continuously change size.

    frame = tkinter.Frame(root)
    frame.pack(fill = 'both', expand = 1)

    sf = Pmw.ScrolledCanvas(frame,
            canvas_scrollregion = (0, 0, 301, 200),
            canvas_width=300,
            canvas_height=200,
            borderframe = 0
    )
    sf.pack(fill = 'both', expand = 1)

# ----------------------------------------------------------------------

def scrolledframeflashing2_test():
    # The two scrollbars will be continuously mapped and unmapped, but
    # the toplevel window will remain the same size.

    root.geometry('550x500')

    frame = tkinter.Frame()
    frame.pack()

    sf = Pmw.ScrolledFrame(frame, borderframe = 0)
    sf.pack(fill = 'both')

    inner = tkinter.Frame(sf.interior(),
            width = 401,
            height = 300,
            borderwidth = 0,
            highlightthickness = 0,
    )
    inner.pack()

# ----------------------------------------------------------------------

def reinitialise_test():
    global text
    text = """
    Demonstrates bug in Pmw.0.8.1 and earlier.
    Click on this button, click on OK in the dialog, then Exit below.
    When this window appears again, clicking on this button gives
    an error:
    TclError: can't invoke "wm" command:  application has been destroyed
    """
    class test:
        def __init__(self):
            root = tkinter.Tk()
            Pmw.initialise(root)
            self.messagedialog = Pmw.MessageDialog(message_text = 'Testing')
            self.messagedialog.withdraw()
            button = tkinter.Button(
                    text = text, command = self.messagedialog.activate)
            button.pack(pady = 20)
            exit = tkinter.Button(text = 'Exit', command = root.destroy)
            exit.pack(pady = 20)
            root.mainloop()

    test()
    test()

# ----------------------------------------------------------------------

def componentgroup_test():
    def addbutton(bb):
        bb.configure(Button_background = 'yellow')
        bb.add('Apples')
        bb.after(3000, lambda bb = bb:
                bb.configure(Button_background = 'green'))

    bb = Pmw.ButtonBox(Button_background = 'red')
    bb.add('Bananas')
    bb.pack()

    mb = Pmw.MenuBar(Button_background = 'red')
    mb.configure(Button_background = 'yellow')
    mb.pack()

    pw = Pmw.PanedWidget(Frame_background = 'red')
    pw.configure(Frame_background = 'yellow')
    pw.pack()

    rs = Pmw.RadioSelect(Button_background = 'red')
    rs.configure(Button_background = 'yellow')
    rs.pack()

    bb.after(3000, lambda bb = bb, addbutton = addbutton: addbutton(bb))

# ----------------------------------------------------------------------

def balloon_test():

    # TODO

    # Test that the balloon does not reappear if the mouse button is
    # pressed down inside a widget and then, while the mouse button is
    # being held down, the mouse is moved outside of the widget and
    # then moved back over the widget.

    # Test that when a widget is destroyed while a balloon is being
    # displayed for it then the balloon is withdrawn.

    # Test that when a widget is destroyed while a balloon is being
    # displayed for another widget then the balloon is not withdrawn.

    # Test that there is no eror when a widget is destroyed during the
    # initwait period (between when the mouse enters the widget and
    # when the initwait timer goes off).

    # Test that if unbind() is called on a widget that triggered the
    # balloon to be displayed then the balloon is withdrawn.  Also
    # test that if another widget triggered the balloon then the
    # balloon is not withdrawn.

    # Test that if tagunbind() is called on a canvas or text item that
    # triggered the balloon to be displayed then the balloon is
    # withdrawn.  Also test that if another widget or item triggered
    # the balloon then the balloon is not withdrawn.

    pass

# ----------------------------------------------------------------------

# A class which prints out a message when an instance is deleted.
class MyToplevel(tkinter.Toplevel):

    def __init__(self):
        tkinter.Toplevel.__init__(self)

    def __del__(self):
        print('Window deleted')

def _runMemoryLeakTest():
    global top
    top = MyToplevel()
    Pmw.MegaToplevel(top)
    Pmw.AboutDialog(top)
    Pmw.ComboBoxDialog(top)
    Pmw.CounterDialog(top)
    Pmw.Dialog(top)
    Pmw.MessageDialog(top)
    Pmw.PromptDialog(top)
    Pmw.SelectionDialog(top)
    Pmw.TextDialog(top)

    Pmw.ButtonBox(top).pack()
    Pmw.ComboBox(top).pack()
    Pmw.Counter(top).pack()
    Pmw.EntryField(top).pack()
    Pmw.Group(top).pack()
    Pmw.LabeledWidget(top).pack()
    Pmw.MenuBar(top).pack()
    Pmw.MessageBar(top).pack()
    Pmw.NoteBook(top).pack()
    Pmw.OptionMenu(top).pack()
    Pmw.PanedWidget(top).pack()
    Pmw.RadioSelect(top).pack()
    Pmw.ScrolledCanvas(top).pack()
    Pmw.ScrolledField(top).pack()
    Pmw.ScrolledFrame(top).pack()
    Pmw.ScrolledListBox(top).pack()
    Pmw.ScrolledText(top).pack()
    Pmw.TimeCounter(top).pack()

def _killMemoryLeakTest():
    global top
    top.destroy()
    del top

memoryLeakMessage = """
Click on the "Run test" button to create instances of
all Pmw megawidgets. Then click on the "Destroy" button.
The message "Window deleted" should be printed to
standard output.
"""
def memoryleak_test():
    label = tkinter.Label(text = memoryLeakMessage)
    label.pack()
    run = tkinter.Button(text = 'Run test', command = _runMemoryLeakTest)
    run.pack()
    kill = tkinter.Button(text = 'Destroy', command = _killMemoryLeakTest)
    kill.pack()

# ----------------------------------------------------------------------

def memoryleak2_test():

    print('This test continuously creates and deletes megawidgets and')
    print('their components.  It calls the "top" program, so')
    print('may not work on non-Unix operating systems. Run it for a long,')
    print('long time and check that the process memory size does not')
    print('continue to increase.  Kill with <Control-C>.')

    pid = os.getpid()

    label = tkinter.Label()
    label.pack()

    # Setup each test:

    # 1. Create/delete all megawidgets:
    megawidgets = (
        Pmw.AboutDialog, Pmw.Balloon, Pmw.ButtonBox, Pmw.ComboBox,
        Pmw.ComboBoxDialog, Pmw.Counter, Pmw.CounterDialog, Pmw.Dialog,
        Pmw.EntryField, Pmw.Group, Pmw.HistoryText, Pmw.LabeledWidget,
        Pmw.MainMenuBar, Pmw.MenuBar, Pmw.MessageBar, Pmw.MessageDialog,
        Pmw.NoteBook, Pmw.OptionMenu, Pmw.PanedWidget, Pmw.PromptDialog,
        Pmw.RadioSelect, Pmw.ScrolledCanvas, Pmw.ScrolledField,
        Pmw.ScrolledFrame, Pmw.ScrolledListBox, Pmw.ScrolledText,
        Pmw.SelectionDialog, Pmw.TextDialog, Pmw.TimeCounter,
    )

    # 2. Balloon binding:
    toplevel = tkinter.Toplevel()
    balloon = Pmw.Balloon(toplevel)
    button = tkinter.Button(toplevel)
    button.pack()
    canvas = tkinter.Canvas(toplevel)
    item = canvas.create_rectangle(0, 0, 100, 100)
    canvas.pack()

    # 3. Adding and deleting menu:
    toplevel = tkinter.Toplevel()
    mainmenu = Pmw.MainMenuBar(toplevel)
    mainmenu.addmenu('Foo', 'help')
    toplevel.configure(menu = mainmenu)

    # 4. Adding and deleting notebook page:
    toplevel = tkinter.Toplevel()
    notebook = Pmw.NoteBook(toplevel)
    notebook.pack()

    # 5. Adding and deleting panedwidget pane:
    toplevel = tkinter.Toplevel()
    panedwidget = Pmw.PanedWidget(toplevel)
    panedwidget.pack()
    panedwidget.insert('Foo', size = 100)

    # 6. Adding and deleting MenuBar menu:
    toplevel = tkinter.Toplevel()
    menubar = Pmw.MenuBar(toplevel)
    menubar.pack()

    # 7. Setting OptionMenu items:
    toplevel = tkinter.Toplevel()
    optionmenu = Pmw.OptionMenu(toplevel, items = ('XXX', 'YYY', 'ZZZ'))
    optionmenu.pack()

    # 8. Setting tkinter.Canvas scrollcommand option:
    toplevel = tkinter.Toplevel()
    scrollcanvas = Pmw.ScrolledCanvas(toplevel)
    scrollcanvas.pack()

    global prevSize
    prevSize = -1

    # Loop and run each test:
    count = 0
    while 1:
        count = count + 1
        label.configure(text = count)

        # 1. Create/delete all megawidgets:
        for widgetClass in megawidgets:
            widget = widgetClass()
            if widgetClass == Pmw.MainMenuBar:
                root.configure(menu = widget)
            elif hasattr(widgetClass, 'pack'):
                widget.pack()
            root.update()
            widget.destroy()

        # 2. Balloon binding:
        balloon.bind(button, 'help')
        balloon.tagbind(canvas, item, 'help')
            # tagbind leaks due to a bug in tkinter (v1.127) Canvas - it adds
            # bindings to self._tagcommands but does not delete them.
        root.update()

        # 3. Adding and deleting MainMenuBar menu:
        mainmenu.addmenu('File', 'help')
        root.update()
        mainmenu.deletemenu('File')
        root.update()

        # 4. Adding and deleting notebook page:
        notebook.insert('File')
        root.update()
        notebook.delete('File')
        root.update()

        # 5. Adding and deleting panedwidget pane:
        panedwidget.insert('File', size = 100)
        root.update()
        panedwidget.delete('File')
        root.update()

        # 6. Adding and deleting MenuBar menu:
        menubar.addmenu('File', 'help')
        root.update()
        menubar.deletemenu('File')
        root.update()

        # 7. Setting OptionMenu items:
        optionmenu.setitems(('aaa', 'bbb', 'ccc'))
        root.update()

        # 8. Setting tkinter.Canvas scrollcommand option:
        scrollcanvas.configure(hscrollmode = 'static')
        scrollcanvas.configure(hscrollmode = 'dynamic')

        # Check memory usage:
        # lines = os.popen('top').readlines()
        lines = os.popen('top -b -n 1 -p %d' % pid).readlines()
        for line in lines:
            # if string.find(line, 'python1.5.2') > 0:
            if string.find(line, '^ *%d' % pid) > 0:
                break
        # size = string.atoi(string.lstrip(line[27:32]))
        size = string.atoi(string.lstrip(line[22:29]))
        if prevSize != size:
            print(time.strftime('%H:%M:%S', time.localtime(time.time())),)
            print(line[:-1])
            prevSize = size

# ----------------------------------------------------------------------

def usageExit():
    print('Usage:', sys.argv[0], '<test>')
    print('  where <test> is one of:')
    for test in tests:
        print('   ', test)
    sys.exit()

tests = []
for name in list(locals().keys()):
    if name[-5:] == '_test':
        tests.append(name)
tests.sort()

if len(sys.argv) != 2:
    usageExit()

testName = sys.argv[1]
if testName not in tests:
    print('Unknown test "' + testName + '"')
    usageExit()

if testName == 'reinitialise_test':
    # Run this by itself, since it calls tkinter.Tk, mainloop, etc.
    reinitialise_test()
    sys.exit()

# Use Pmw version in this distribution:
Test.initialise()
root = Test.root
root.deiconify()

# To use a different version of Pmw, comment out the three above lines
# and the "import Test" line and uncomment these three:
#   root = tkinter.Tk()
#   Pmw.setversion('1.0')
#   Pmw.initialise(root)

testFunction = locals()[testName]
testFunction()

if testName != 'memoryleak2_test':
    # This does not use mainloop.
    root.mainloop()
