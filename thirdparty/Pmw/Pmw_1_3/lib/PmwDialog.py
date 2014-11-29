# Based on iwidgets2.2.0/dialog.itk and iwidgets2.2.0/dialogshell.itk code.

# Convention:
#   Each dialog window should have one of these as the rightmost button:
#     Close         Close a window which only displays information.
#     Cancel        Close a window which may be used to change the state of
#                   the application.

import sys
import types
import Tkinter
import Pmw

# A Toplevel with a ButtonBox and child site.

class Dialog(Pmw.MegaToplevel):
    def __init__(self, parent = None, **kw):

	# Define the megawidget options.
	INITOPT = Pmw.INITOPT
	optiondefs = (
	    ('buttonbox_hull_borderwidth',   1,         None),
	    ('buttonbox_hull_relief',        'raised',  None),
	    ('buttonboxpos',                 's',       INITOPT),
	    ('buttons',                      ('OK',),   self._buttons),
	    ('command',                      None,      None),
	    ('dialogchildsite_borderwidth',  1,         None),
	    ('dialogchildsite_relief',       'raised',  None),
	    ('defaultbutton',                None,      self._defaultButton),
            ('master',                       'parent',  None),
	    ('separatorwidth',               0,         INITOPT),
	)
	self.defineoptions(kw, optiondefs)

	# Initialise the base class (after defining the options).
	Pmw.MegaToplevel.__init__(self, parent)

	# Create the components.

	oldInterior = Pmw.MegaToplevel.interior(self)

	# Set up pack options according to the position of the button box.
        pos = self['buttonboxpos']
	if pos not in 'nsew':
	    raise ValueError, \
	        'bad buttonboxpos option "%s":  should be n, s, e, or w' \
		    % pos

	if pos in 'ns':
	    orient = 'horizontal'
	    fill = 'x'
	    if pos == 'n':
	        side = 'top'
	    else:
	        side = 'bottom'
	else:
	    orient = 'vertical'
	    fill = 'y'
	    if pos == 'w':
	        side = 'left'
	    else:
	        side = 'right'

	# Create the button box.
	self._buttonBox = self.createcomponent('buttonbox',
		(), None,
		Pmw.ButtonBox, (oldInterior,), orient = orient)
	self._buttonBox.pack(side = side, fill = fill)

	# Create the separating line.
	width = self['separatorwidth']
	if width > 0:
	    self._separator = self.createcomponent('separator',
		    (), None,
		    Tkinter.Frame, (oldInterior,), relief = 'sunken',
		    height = width, width = width, borderwidth = width / 2)
	    self._separator.pack(side = side, fill = fill)
	
	# Create the child site.
	self.__dialogChildSite = self.createcomponent('dialogchildsite',
		(), None,
		Tkinter.Frame, (oldInterior,))
	self.__dialogChildSite.pack(side=side, fill='both', expand=1)

	self.oldButtons = ()
	self.oldDefault = None

	self.bind('<Return>', self._invokeDefault)
        self.userdeletefunc(self._doCommand)
        self.usermodaldeletefunc(self._doCommand)
	
	# Check keywords and initialise options.
	self.initialiseoptions()

    def interior(self):
	return self.__dialogChildSite

    def invoke(self, index = Pmw.DEFAULT):
	return self._buttonBox.invoke(index)

    def _invokeDefault(self, event):
	try:
	    self._buttonBox.index(Pmw.DEFAULT)
	except ValueError:
	    return
	self._buttonBox.invoke()

    def _doCommand(self, name = None):
        if name is not None and self.active() and \
                Pmw.grabstacktopwindow() != self.component('hull'):
            # This is a modal dialog but is not on the top of the grab
            # stack (ie:  should not have the grab), so ignore this
            # event.  This seems to be a bug in Tk and may occur in
            # nested modal dialogs.
            #
            # An example is the PromptDialog demonstration.  To
            # trigger the problem, start the demo, then move the mouse
            # to the main window, hit <TAB> and then <TAB> again.  The
            # highlight border of the "Show prompt dialog" button
            # should now be displayed.  Now hit <SPACE>, <RETURN>,
            # <RETURN> rapidly several times.  Eventually, hitting the
            # return key invokes the password dialog "OK" button even
            # though the confirm dialog is active (and therefore
            # should have the keyboard focus).  Observed under Solaris
            # 2.5.1, python 1.5.2 and Tk8.0.

            # TODO:  Give focus to the window on top of the grabstack.
            return

	command = self['command']
	if callable(command):
	    return command(name)
	else:
	    if self.active():
	        self.deactivate(name)
	    else:
	        self.withdraw()

    def _buttons(self):
	buttons = self['buttons']
	if type(buttons) != types.TupleType and type(buttons) != types.ListType:
	    raise ValueError, \
	        'bad buttons option "%s": should be a tuple' % str(buttons)
	if self.oldButtons == buttons:
	  return

	self.oldButtons = buttons

	for index in range(self._buttonBox.numbuttons()):
	    self._buttonBox.delete(0)
	for name in buttons:
	    self._buttonBox.add(name,
		command=lambda self=self, name=name: self._doCommand(name))

	if len(buttons) > 0:
	    defaultbutton = self['defaultbutton']
	    if defaultbutton is None:
		self._buttonBox.setdefault(None)
	    else:
		try:
		    self._buttonBox.index(defaultbutton)
		except ValueError:
		    pass
		else:
		    self._buttonBox.setdefault(defaultbutton)
	self._buttonBox.alignbuttons()

    def _defaultButton(self):
	defaultbutton = self['defaultbutton']
	if self.oldDefault == defaultbutton:
	  return

	self.oldDefault = defaultbutton

	if len(self['buttons']) > 0:
	    if defaultbutton is None:
		self._buttonBox.setdefault(None)
	    else:
		try:
		    self._buttonBox.index(defaultbutton)
		except ValueError:
		    pass
		else:
		    self._buttonBox.setdefault(defaultbutton)
