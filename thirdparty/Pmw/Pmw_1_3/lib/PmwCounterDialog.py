import Pmw

# A Dialog with a counter

class CounterDialog(Pmw.Dialog):

    def __init__(self, parent = None, **kw):

	# Define the megawidget options.
	INITOPT = Pmw.INITOPT
	optiondefs = (
	    ('borderx',    20,  INITOPT),
	    ('bordery',    20,  INITOPT),
	)
	self.defineoptions(kw, optiondefs)

	# Initialise the base class (after defining the options).
	Pmw.Dialog.__init__(self, parent)

	# Create the components.
	interior = self.interior()

	# Create the counter.
	aliases = (
	    ('entryfield', 'counter_entryfield'),
	    ('entry', 'counter_entryfield_entry'),
	    ('label', 'counter_label')
	)
	self._cdCounter = self.createcomponent('counter',
		aliases, None,
		Pmw.Counter, (interior,))
	self._cdCounter.pack(fill='x', expand=1,
		padx = self['borderx'], pady = self['bordery'])
	
        if not kw.has_key('activatecommand'):
            # Whenever this dialog is activated, set the focus to the
            # Counter's entry widget.
            tkentry = self.component('entry')
            self.configure(activatecommand = tkentry.focus_set)

	# Check keywords and initialise options.
	self.initialiseoptions()

    # Supply aliases to some of the entry component methods.
    def insertentry(self, index, text):
	self._cdCounter.insert(index, text)

    def deleteentry(self, first, last=None):
	self._cdCounter.delete(first, last)

    def indexentry(self, index):
	return self._cdCounter.index(index)

Pmw.forwardmethods(CounterDialog, Pmw.Counter, '_cdCounter')
