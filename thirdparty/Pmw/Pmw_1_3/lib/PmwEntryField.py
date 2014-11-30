# Based on iwidgets2.2.0/entryfield.itk code.

import re
import string
import types
import Tkinter
import Pmw

# Possible return values of validation functions.
OK = 1
ERROR = 0
PARTIAL = -1

class EntryField(Pmw.MegaWidget):
    _classBindingsDefinedFor = 0

    def __init__(self, parent = None, **kw):

	# Define the megawidget options.
	INITOPT = Pmw.INITOPT
	optiondefs = (
	    ('command',           None,        None),
	    ('errorbackground',   'pink',      None),
	    ('invalidcommand',    self.bell,   None),
	    ('labelmargin',       0,           INITOPT),
	    ('labelpos',          None,        INITOPT),
	    ('modifiedcommand',   None,        None),
	    ('sticky',            'ew',        INITOPT),
	    ('validate',          None,        self._validate),
	    ('extravalidators',   {},          None),
	    ('value',             '',          INITOPT),
	)
	self.defineoptions(kw, optiondefs)

	# Initialise the base class (after defining the options).
	Pmw.MegaWidget.__init__(self, parent)

	# Create the components.
	interior = self.interior()
	self._entryFieldEntry = self.createcomponent('entry',
		(), None,
		Tkinter.Entry, (interior,))
	self._entryFieldEntry.grid(column=2, row=2, sticky=self['sticky'])
	if self['value'] != '':
	    self.__setEntry(self['value'])
	interior.grid_columnconfigure(2, weight=1)
	interior.grid_rowconfigure(2, weight=1)

	self.createlabel(interior)

	# Initialise instance variables.

	self.normalBackground = None
        self._previousText = None

	# Initialise instance.

	_registerEntryField(self._entryFieldEntry, self)

        # Establish the special class bindings if not already done.
        # Also create bindings if the Tkinter default interpreter has
        # changed.  Use Tkinter._default_root to create class
        # bindings, so that a reference to root is created by
        # bind_class rather than a reference to self, which would
        # prevent object cleanup.
        if EntryField._classBindingsDefinedFor != Tkinter._default_root:
	    tagList = self._entryFieldEntry.bindtags()
            root  = Tkinter._default_root
	    	    
	    allSequences = {}
	    for tag in tagList:

                sequences = root.bind_class(tag)
                if type(sequences) is types.StringType:
                    # In old versions of Tkinter, bind_class returns a string
                    sequences = root.tk.splitlist(sequences)

		for sequence in sequences:
		    allSequences[sequence] = None
	    for sequence in allSequences.keys():
		root.bind_class('EntryFieldPre', sequence, _preProcess)
		root.bind_class('EntryFieldPost', sequence, _postProcess)

	    EntryField._classBindingsDefinedFor = root

	self._entryFieldEntry.bindtags(('EntryFieldPre',) +
		self._entryFieldEntry.bindtags() + ('EntryFieldPost',))
	self._entryFieldEntry.bind('<Return>', self._executeCommand)

	# Check keywords and initialise options.
	self.initialiseoptions()

    def destroy(self):
	_deregisterEntryField(self._entryFieldEntry)
        Pmw.MegaWidget.destroy(self)

    def _getValidatorFunc(self, validator, index):
	# Search the extra and standard validator lists for the
	# given 'validator'.  If 'validator' is an alias, then
	# continue the search using the alias.  Make sure that
	# self-referencial aliases do not cause infinite loops.

	extraValidators = self['extravalidators']
	traversedValidators = []

	while 1:
	    traversedValidators.append(validator)
	    if extraValidators.has_key(validator):
		validator = extraValidators[validator][index]
	    elif _standardValidators.has_key(validator):
		validator = _standardValidators[validator][index]
	    else:
		return validator
	    if validator in traversedValidators:
		return validator

    def _validate(self):
	dict = {
	    'validator' : None,
	    'min' : None,
	    'max' : None,
	    'minstrict' : 1,
	    'maxstrict' : 1,
	}
	opt = self['validate']
	if type(opt) is types.DictionaryType:
	    dict.update(opt)
	else:
	    dict['validator'] = opt

	# Look up validator maps and replace 'validator' field with
	# the corresponding function.
	validator = dict['validator']
	valFunction = self._getValidatorFunc(validator, 0)
	self._checkValidateFunction(valFunction, 'validate', validator)
	dict['validator'] = valFunction

	# Look up validator maps and replace 'stringtovalue' field
	# with the corresponding function.
	if dict.has_key('stringtovalue'):
	    stringtovalue = dict['stringtovalue'] 
	    strFunction = self._getValidatorFunc(stringtovalue, 1)
	    self._checkValidateFunction(
		    strFunction, 'stringtovalue', stringtovalue)
	else:
	    strFunction = self._getValidatorFunc(validator, 1)
	    if strFunction == validator:
		strFunction = len
	dict['stringtovalue'] = strFunction

	self._validationInfo = dict
	args = dict.copy()
	del args['validator']
	del args['min']
	del args['max']
	del args['minstrict']
	del args['maxstrict']
	del args['stringtovalue']
	self._validationArgs = args
        self._previousText = None

	if type(dict['min']) == types.StringType and strFunction is not None:
	    dict['min'] = apply(strFunction, (dict['min'],), args)
	if type(dict['max']) == types.StringType and strFunction is not None:
	    dict['max'] = apply(strFunction, (dict['max'],), args)

	self._checkValidity()

    def _checkValidateFunction(self, function, option, validator):
	# Raise an error if 'function' is not a function or None.

	if function is not None and not callable(function):
	    extraValidators = self['extravalidators']
	    extra = extraValidators.keys()
	    extra.sort()
	    extra = tuple(extra)
	    standard = _standardValidators.keys()
	    standard.sort()
	    standard = tuple(standard)
	    msg = 'bad %s value "%s":  must be a function or one of ' \
		'the standard validators %s or extra validators %s'
	    raise ValueError, msg % (option, validator, standard, extra)

    def _executeCommand(self, event = None):
	cmd = self['command']
	if callable(cmd):
            if event is None:
                # Return result of command for invoke() method.
                return cmd()
            else:
                cmd()
	    
    def _preProcess(self):

        self._previousText = self._entryFieldEntry.get()
        self._previousICursor = self._entryFieldEntry.index('insert')
        self._previousXview = self._entryFieldEntry.index('@0')
	if self._entryFieldEntry.selection_present():
	    self._previousSel= (self._entryFieldEntry.index('sel.first'),
		self._entryFieldEntry.index('sel.last'))
	else:
	    self._previousSel = None

    def _postProcess(self):

	# No need to check if text has not changed.
	previousText = self._previousText
	if previousText == self._entryFieldEntry.get():
	    return self.valid()

	valid = self._checkValidity()
        if self.hulldestroyed():
            # The invalidcommand called by _checkValidity() destroyed us.
            return valid

	cmd = self['modifiedcommand']
	if callable(cmd) and previousText != self._entryFieldEntry.get():
	    cmd()
	return valid
	    
    def checkentry(self):
	# If there is a variable specified by the entry_textvariable
	# option, checkentry() should be called after the set() method
	# of the variable is called.

	self._previousText = None
	return self._postProcess()

    def _getValidity(self):
	text = self._entryFieldEntry.get()
	dict = self._validationInfo
	args = self._validationArgs

	if dict['validator'] is not None:
	    status = apply(dict['validator'], (text,), args)
	    if status != OK:
		return status

	# Check for out of (min, max) range.
	if dict['stringtovalue'] is not None:
	    min = dict['min']
	    max = dict['max']
	    if min is None and max is None:
		return OK
	    val = apply(dict['stringtovalue'], (text,), args)
	    if min is not None and val < min:
		if dict['minstrict']:
		    return ERROR
		else:
		    return PARTIAL
	    if max is not None and val > max:
		if dict['maxstrict']:
		    return ERROR
		else:
		    return PARTIAL
	return OK

    def _checkValidity(self):
	valid = self._getValidity()
	oldValidity = valid

	if valid == ERROR:
	    # The entry is invalid.
	    cmd = self['invalidcommand']
	    if callable(cmd):
		cmd()
            if self.hulldestroyed():
                # The invalidcommand destroyed us.
                return oldValidity

	    # Restore the entry to its previous value.
	    if self._previousText is not None:
		self.__setEntry(self._previousText)
		self._entryFieldEntry.icursor(self._previousICursor)
		self._entryFieldEntry.xview(self._previousXview)
		if self._previousSel is not None:
		    self._entryFieldEntry.selection_range(self._previousSel[0],
			self._previousSel[1])

		# Check if the saved text is valid as well.
		valid = self._getValidity()

	self._valid = valid

        if self.hulldestroyed():
            # The validator or stringtovalue commands called by
            # _checkValidity() destroyed us.
            return oldValidity

	if valid == OK:
	    if self.normalBackground is not None:
		self._entryFieldEntry.configure(
			background = self.normalBackground)
		self.normalBackground = None
	else:
	    if self.normalBackground is None:
		self.normalBackground = self._entryFieldEntry.cget('background')
		self._entryFieldEntry.configure(
			background = self['errorbackground'])

        return oldValidity

    def invoke(self):
	return self._executeCommand()

    def valid(self):
        return self._valid == OK

    def clear(self):
        self.setentry('')

    def __setEntry(self, text):
	oldState = str(self._entryFieldEntry.cget('state'))
	if oldState != 'normal':
	    self._entryFieldEntry.configure(state='normal')
	self._entryFieldEntry.delete(0, 'end')
	self._entryFieldEntry.insert(0, text)
	if oldState != 'normal':
	    self._entryFieldEntry.configure(state=oldState)

    def setentry(self, text):
	self._preProcess()
        self.__setEntry(text)
	return self._postProcess()

    def getvalue(self):
        return self._entryFieldEntry.get()

    def setvalue(self, text):
        return self.setentry(text)

Pmw.forwardmethods(EntryField, Tkinter.Entry, '_entryFieldEntry')

# ======================================================================


# Entry field validation functions

_numericregex = re.compile('^[0-9]*$')
_alphabeticregex = re.compile('^[a-z]*$', re.IGNORECASE)
_alphanumericregex = re.compile('^[0-9a-z]*$', re.IGNORECASE)

def numericvalidator(text):
    if text == '':
        return PARTIAL
    else:
	if _numericregex.match(text) is None:
	    return ERROR
	else:
	    return OK
    
def integervalidator(text):
    if text in ('', '-', '+'):
        return PARTIAL
    try:
	string.atol(text)
	return OK
    except ValueError:
	return ERROR
    
def alphabeticvalidator(text):
    if _alphabeticregex.match(text) is None:
	return ERROR
    else:
	return OK
    
def alphanumericvalidator(text):
    if _alphanumericregex.match(text) is None:
	return ERROR
    else:
	return OK
    
def hexadecimalvalidator(text):
    if text in ('', '0x', '0X', '+', '+0x', '+0X', '-', '-0x', '-0X'):
        return PARTIAL
    try:
	string.atol(text, 16)
	return OK
    except ValueError:
	return ERROR
    
def realvalidator(text, separator = '.'):
    if separator != '.':
	if string.find(text, '.') >= 0:
	    return ERROR
	index = string.find(text, separator)
	if index >= 0:
	    text = text[:index] + '.' + text[index + 1:]
    try:
	string.atof(text)
	return OK
    except ValueError:
	# Check if the string could be made valid by appending a digit
	# eg ('-', '+', '.', '-.', '+.', '1.23e', '1E-').
	if len(text) == 0:
	    return PARTIAL
	if text[-1] in string.digits:
	    return ERROR
	try:
	    string.atof(text + '0')
	    return PARTIAL
	except ValueError:
	    return ERROR
    
def timevalidator(text, separator = ':'):
    try:
	Pmw.timestringtoseconds(text, separator)
	return OK
    except ValueError:
	if len(text) > 0 and text[0] in ('+', '-'):
	    text = text[1:]
	if re.search('[^0-9' + separator + ']', text) is not None:
	    return ERROR
	return PARTIAL

def datevalidator(text, format = 'ymd', separator = '/'):
    try:
	Pmw.datestringtojdn(text, format, separator)
	return OK
    except ValueError:
	if re.search('[^0-9' + separator + ']', text) is not None:
	    return ERROR
	return PARTIAL

_standardValidators = {
    'numeric'      : (numericvalidator,      string.atol),
    'integer'      : (integervalidator,      string.atol),
    'hexadecimal'  : (hexadecimalvalidator,  lambda s: string.atol(s, 16)),
    'real'         : (realvalidator,         Pmw.stringtoreal),
    'alphabetic'   : (alphabeticvalidator,   len),
    'alphanumeric' : (alphanumericvalidator, len),
    'time'         : (timevalidator,         Pmw.timestringtoseconds),
    'date'         : (datevalidator,         Pmw.datestringtojdn),
}

_entryCache = {}

def _registerEntryField(entry, entryField):
    # Register an EntryField widget for an Entry widget

    _entryCache[entry] = entryField

def _deregisterEntryField(entry):
    # Deregister an Entry widget
    del _entryCache[entry]

def _preProcess(event):
    # Forward preprocess events for an Entry to it's EntryField

    _entryCache[event.widget]._preProcess()

def _postProcess(event):
    # Forward postprocess events for an Entry to it's EntryField

    # The function specified by the 'command' option may have destroyed
    # the megawidget in a binding earlier in bindtags, so need to check.
    if _entryCache.has_key(event.widget):
        _entryCache[event.widget]._postProcess()
