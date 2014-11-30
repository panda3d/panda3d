import Pmw

class AboutDialog(Pmw.MessageDialog):
    # Window to display version and contact information.

    # Class members containing resettable 'default' values:
    _version = ''
    _copyright = ''
    _contact = ''

    def __init__(self, parent = None, **kw):

	# Define the megawidget options.
	INITOPT = Pmw.INITOPT
	optiondefs = (
	    ('applicationname',   '',          INITOPT),
	    ('iconpos',           'w',         None),
	    ('icon_bitmap',       'info',      None),
	    ('buttons',           ('Close',),  None),
	    ('defaultbutton',     0,           None),
	)
	self.defineoptions(kw, optiondefs)

	# Initialise the base class (after defining the options).
	Pmw.MessageDialog.__init__(self, parent)

	applicationname = self['applicationname']
        if not kw.has_key('title'):
            self.configure(title = 'About ' + applicationname)

        if not kw.has_key('message_text'):
            text = applicationname + '\n\n'
            if AboutDialog._version != '':
              text = text + 'Version ' + AboutDialog._version + '\n'
            if AboutDialog._copyright != '':
              text = text + AboutDialog._copyright + '\n\n'
            if AboutDialog._contact != '':
              text = text + AboutDialog._contact

            self.configure(message_text=text)

	# Check keywords and initialise options.
	self.initialiseoptions()

def aboutversion(value):
    AboutDialog._version = value

def aboutcopyright(value):
    AboutDialog._copyright = value

def aboutcontact(value):
    AboutDialog._contact = value
