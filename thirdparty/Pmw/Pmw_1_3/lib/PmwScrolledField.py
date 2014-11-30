import Tkinter
import Pmw

class ScrolledField(Pmw.MegaWidget):
    def __init__(self, parent = None, **kw):

	# Define the megawidget options.
	INITOPT = Pmw.INITOPT
	optiondefs = (
	    ('labelmargin',   0,      INITOPT),
	    ('labelpos',      None,   INITOPT),
	    ('sticky',        'ew',   INITOPT),
	    ('text',          '',     self._text),
	)
	self.defineoptions(kw, optiondefs)

	# Initialise the base class (after defining the options).
	Pmw.MegaWidget.__init__(self, parent)

	# Create the components.
	interior = self.interior()
	self._scrolledFieldEntry = self.createcomponent('entry',
		(), None,
		Tkinter.Entry, (interior,))

        # Can't always use 'disabled', since this greys out text in Tk 8.4.2
        try:
            self._scrolledFieldEntry.configure(state = 'readonly')
        except Tkinter.TclError:
            self._scrolledFieldEntry.configure(state = 'disabled')

	self._scrolledFieldEntry.grid(column=2, row=2, sticky=self['sticky'])
	interior.grid_columnconfigure(2, weight=1)
	interior.grid_rowconfigure(2, weight=1)

	self.createlabel(interior)

	# Check keywords and initialise options.
	self.initialiseoptions()

    def _text(self):
        text = self['text']
        self._scrolledFieldEntry.configure(state = 'normal')
        self._scrolledFieldEntry.delete(0, 'end')
        self._scrolledFieldEntry.insert('end', text)

        # Can't always use 'disabled', since this greys out text in Tk 8.4.2
        try:
            self._scrolledFieldEntry.configure(state = 'readonly')
        except Tkinter.TclError:
            self._scrolledFieldEntry.configure(state = 'disabled')

Pmw.forwardmethods(ScrolledField, Tkinter.Entry, '_scrolledFieldEntry')
