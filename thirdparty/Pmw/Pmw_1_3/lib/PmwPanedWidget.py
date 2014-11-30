# PanedWidget
# a frame which may contain several resizable sub-frames

import string
import sys
import types
import Tkinter
import Pmw

class PanedWidget(Pmw.MegaWidget):

    def __init__(self, parent = None, **kw):

	# Define the megawidget options.
	INITOPT = Pmw.INITOPT
	optiondefs = (
            ('command',            None,         None),
            ('orient',             'vertical',   INITOPT),
            ('separatorrelief',    'sunken',     INITOPT),
            ('separatorthickness', 2,            INITOPT),
            ('handlesize',         8,            INITOPT),
            ('hull_width',         400,          None),
            ('hull_height',        400,          None),
	)
	self.defineoptions(kw, optiondefs,
                dynamicGroups = ('Frame', 'Separator', 'Handle'))

	# Initialise the base class (after defining the options).
	Pmw.MegaWidget.__init__(self, parent)

	self.bind('<Configure>', self._handleConfigure)

	if self['orient'] not in ('horizontal', 'vertical'):
	    raise ValueError, 'bad orient option ' + repr(self['orient']) + \
		': must be either \'horizontal\' or \'vertical\''

        self._separatorThickness = self['separatorthickness']
        self._handleSize = self['handlesize']
	self._paneNames = []            # List of pane names
	self._paneAttrs = {}            # Map from pane name to pane info

	self._timerId = None
	self._frame = {}
	self._separator = []
	self._button = []
	self._totalSize = 0
	self._movePending = 0
	self._relsize = {}
	self._relmin = {}
	self._relmax = {}
	self._size = {}
	self._min = {}
	self._max = {}
	self._rootp = None
	self._curSize = None
	self._beforeLimit = None
	self._afterLimit = None
	self._buttonIsDown = 0
	self._majorSize = 100
	self._minorSize = 100

	# Check keywords and initialise options.
	self.initialiseoptions()

    def insert(self, name, before = 0, **kw):
	# Parse <kw> for options.
        self._initPaneOptions(name)
	self._parsePaneOptions(name, kw)

	insertPos = self._nameToIndex(before)
	atEnd = (insertPos == len(self._paneNames))

	# Add the frame.
	self._paneNames[insertPos:insertPos] = [name]
	self._frame[name] = self.createcomponent(name,
		(), 'Frame',
		Tkinter.Frame, (self.interior(),))

	# Add separator, if necessary.
	if len(self._paneNames) > 1:
	    self._addSeparator()
	else:
	    self._separator.append(None)
	    self._button.append(None)

	# Add the new frame and adjust the PanedWidget
	if atEnd:
	    size = self._size[name]
	    if size > 0 or self._relsize[name] is not None:
		if self['orient'] == 'vertical':
		    self._frame[name].place(x=0, relwidth=1,
					    height=size, y=self._totalSize)
		else:
		    self._frame[name].place(y=0, relheight=1,
					    width=size, x=self._totalSize)
	    else:
		if self['orient'] == 'vertical':
		    self._frame[name].place(x=0, relwidth=1,
					    y=self._totalSize)
		else:
		    self._frame[name].place(y=0, relheight=1,
					    x=self._totalSize)
	else:
	    self._updateSizes()

	self._totalSize = self._totalSize + self._size[name]
	return self._frame[name]

    def add(self, name, **kw):
        return apply(self.insert, (name, len(self._paneNames)), kw)

    def delete(self, name):
	deletePos = self._nameToIndex(name)
	name = self._paneNames[deletePos]
	self.destroycomponent(name)
	del self._paneNames[deletePos]
	del self._frame[name]
	del self._size[name]
	del self._min[name]
	del self._max[name]
	del self._relsize[name]
	del self._relmin[name]
	del self._relmax[name]

	last = len(self._paneNames)
	del self._separator[last]
	del self._button[last]
        if last > 0:
            self.destroycomponent(self._sepName(last))
            self.destroycomponent(self._buttonName(last))

	self._plotHandles()

    def setnaturalsize(self):
        self.update_idletasks()
        totalWidth = 0
        totalHeight = 0
        maxWidth = 0
        maxHeight = 0
	for name in self._paneNames:
            frame = self._frame[name]
            w = frame.winfo_reqwidth()
            h = frame.winfo_reqheight()
            totalWidth = totalWidth + w
            totalHeight = totalHeight + h
            if maxWidth < w:
                maxWidth = w
            if maxHeight < h:
                maxHeight = h

        # Note that, since the hull is a frame, the width and height
        # options specify the geometry *outside* the borderwidth and
        # highlightthickness.
        bw = string.atoi(str(self.cget('hull_borderwidth')))
        hl = string.atoi(str(self.cget('hull_highlightthickness')))
        extra = (bw + hl) * 2
        if str(self.cget('orient')) == 'horizontal':
            totalWidth = totalWidth + extra
            maxHeight = maxHeight + extra
            self.configure(hull_width = totalWidth, hull_height = maxHeight)
        else:
            totalHeight = (totalHeight + extra +
                    (len(self._paneNames) - 1) * self._separatorThickness)
            maxWidth = maxWidth + extra
            self.configure(hull_width = maxWidth, hull_height = totalHeight)

    def move(self, name, newPos, newPosOffset = 0):

        # see if we can spare ourselves some work
        numPanes = len(self._paneNames)
        if numPanes < 2:
            return

        newPos = self._nameToIndex(newPos) + newPosOffset
        if newPos < 0 or newPos >=numPanes:
            return

        deletePos = self._nameToIndex(name)

        if deletePos == newPos:
            # inserting over ourself is a no-op
            return

        # delete name from old position in list
        name = self._paneNames[deletePos]
        del self._paneNames[deletePos]

        # place in new position
        self._paneNames[newPos:newPos] = [name]

        # force everything to redraw
        self._plotHandles()
        self._updateSizes()

    def _nameToIndex(self, nameOrIndex):
	try:
	    pos = self._paneNames.index(nameOrIndex)
	except ValueError:
	    pos = nameOrIndex

	return pos

    def _initPaneOptions(self, name):
	# Set defaults.
	self._size[name] = 0
	self._relsize[name] = None
	self._min[name] = 0
	self._relmin[name] = None
	self._max[name] = 100000
	self._relmax[name] = None

    def _parsePaneOptions(self, name, args):
	# Parse <args> for options.
	for arg, value in args.items():
	    if type(value) == types.FloatType:
		relvalue = value
		value = self._absSize(relvalue)
	    else:
		relvalue = None

	    if arg == 'size':
		self._size[name], self._relsize[name] = value, relvalue
	    elif arg == 'min':
		self._min[name], self._relmin[name] = value, relvalue
	    elif arg == 'max':
		self._max[name], self._relmax[name] = value, relvalue
	    else:
		raise ValueError, 'keyword must be "size", "min", or "max"'

    def _absSize(self, relvalue):
	return int(round(relvalue * self._majorSize))

    def _sepName(self, n):
	return 'separator-%d' % n

    def _buttonName(self, n):
	return 'handle-%d' % n

    def _addSeparator(self):
	n = len(self._paneNames) - 1

	downFunc = lambda event, s = self, num=n: s._btnDown(event, num)
	upFunc = lambda event, s = self, num=n: s._btnUp(event, num)
	moveFunc = lambda event, s = self, num=n: s._btnMove(event, num)

	# Create the line dividing the panes.
	sep = self.createcomponent(self._sepName(n),
		(), 'Separator',
		Tkinter.Frame, (self.interior(),),
		borderwidth = 1,
		relief = self['separatorrelief'])
	self._separator.append(sep)

	sep.bind('<ButtonPress-1>', downFunc)
	sep.bind('<Any-ButtonRelease-1>', upFunc)
	sep.bind('<B1-Motion>', moveFunc)

	if self['orient'] == 'vertical':
	    cursor = 'sb_v_double_arrow'
	    sep.configure(height = self._separatorThickness,
                    width = 10000, cursor = cursor)
	else:
	    cursor = 'sb_h_double_arrow'
	    sep.configure(width = self._separatorThickness,
                    height = 10000, cursor = cursor)

	self._totalSize = self._totalSize + self._separatorThickness

	# Create the handle on the dividing line.
	handle = self.createcomponent(self._buttonName(n),
		(), 'Handle',
		Tkinter.Frame, (self.interior(),),
		    relief = 'raised',
		    borderwidth = 1,
		    width = self._handleSize,
		    height = self._handleSize,
		    cursor = cursor,
		)
	self._button.append(handle)

	handle.bind('<ButtonPress-1>', downFunc)
	handle.bind('<Any-ButtonRelease-1>', upFunc)
	handle.bind('<B1-Motion>', moveFunc)

	self._plotHandles()

	for i in range(1, len(self._paneNames)):
	    self._separator[i].tkraise()
	for i in range(1, len(self._paneNames)):
	    self._button[i].tkraise()

    def _btnUp(self, event, item):
	self._buttonIsDown = 0
	self._updateSizes()
	try:
	    self._button[item].configure(relief='raised')
	except:
	    pass

    def _btnDown(self, event, item):
	self._button[item].configure(relief='sunken')
	self._getMotionLimit(item)
	self._buttonIsDown = 1
	self._movePending = 0

    def _handleConfigure(self, event = None):
	self._getNaturalSizes()
	if self._totalSize == 0:
	    return

	iterRange = list(self._paneNames)
	iterRange.reverse()
	if self._majorSize > self._totalSize:
	    n = self._majorSize - self._totalSize
	    self._iterate(iterRange, self._grow, n)
	elif self._majorSize < self._totalSize:
	    n = self._totalSize - self._majorSize
	    self._iterate(iterRange, self._shrink, n)

	self._plotHandles()
	self._updateSizes()

    def _getNaturalSizes(self):
	# Must call this in order to get correct winfo_width, winfo_height
	self.update_idletasks()

	self._totalSize = 0

	if self['orient'] == 'vertical':
	    self._majorSize = self.winfo_height()
	    self._minorSize = self.winfo_width()
	    majorspec = Tkinter.Frame.winfo_reqheight
	else:
	    self._majorSize = self.winfo_width()
	    self._minorSize = self.winfo_height()
	    majorspec = Tkinter.Frame.winfo_reqwidth

        bw = string.atoi(str(self.cget('hull_borderwidth')))
        hl = string.atoi(str(self.cget('hull_highlightthickness')))
        extra = (bw + hl) * 2
        self._majorSize = self._majorSize - extra
        self._minorSize = self._minorSize - extra

	if self._majorSize < 0:
	    self._majorSize = 0
	if self._minorSize < 0:
	    self._minorSize = 0

	for name in self._paneNames:
	    # adjust the absolute sizes first...
	    if self._relsize[name] is None:
		#special case
		if self._size[name] == 0:
		    self._size[name] = apply(majorspec, (self._frame[name],))
		    self._setrel(name)
	    else:
		self._size[name] = self._absSize(self._relsize[name])

	    if self._relmin[name] is not None:
		self._min[name] = self._absSize(self._relmin[name])
	    if self._relmax[name] is not None:
		self._max[name] = self._absSize(self._relmax[name])

	    # now adjust sizes
	    if self._size[name] < self._min[name]:
		self._size[name] = self._min[name]
		self._setrel(name)

	    if self._size[name] > self._max[name]:
		self._size[name] = self._max[name]
		self._setrel(name)

	    self._totalSize = self._totalSize + self._size[name]

	# adjust for separators
	self._totalSize = (self._totalSize +
                (len(self._paneNames) - 1) * self._separatorThickness)

    def _setrel(self, name):
	if self._relsize[name] is not None:
	    if self._majorSize != 0:
		self._relsize[name] = round(self._size[name]) / self._majorSize

    def _iterate(self, names, proc, n):
	for i in names:
	    n = apply(proc, (i, n))
	    if n == 0:
		break

    def _grow(self, name, n):
	canGrow = self._max[name] - self._size[name]

	if canGrow > n:
	    self._size[name] = self._size[name] + n
	    self._setrel(name)
	    return 0
	elif canGrow > 0:
	    self._size[name] = self._max[name]
	    self._setrel(name)
	    n = n - canGrow

	return n

    def _shrink(self, name, n):
	canShrink = self._size[name] - self._min[name]

	if canShrink > n:
	    self._size[name] = self._size[name] - n
	    self._setrel(name)
	    return 0
	elif canShrink > 0:
	    self._size[name] = self._min[name]
	    self._setrel(name)
	    n = n - canShrink

	return n

    def _updateSizes(self):
	totalSize = 0

	for name in self._paneNames:
	    size = self._size[name]
	    if self['orient'] == 'vertical':
		self._frame[name].place(x = 0, relwidth = 1,
					y = totalSize,
					height = size)
	    else:
		self._frame[name].place(y = 0, relheight = 1,
					x = totalSize,
					width = size)

	    totalSize = totalSize + size + self._separatorThickness

	# Invoke the callback command
	cmd = self['command']
	if callable(cmd):
	    cmd(map(lambda x, s = self: s._size[x], self._paneNames))

    def _plotHandles(self):
	if len(self._paneNames) == 0:
	    return

	if self['orient'] == 'vertical':
	    btnp = self._minorSize - 13
	else:
	    h = self._minorSize

	    if h > 18:
		btnp = 9
	    else:
		btnp = h - 9

	firstPane = self._paneNames[0]
	totalSize = self._size[firstPane]

	first = 1
	last = len(self._paneNames) - 1

	# loop from first to last, inclusive
	for i in range(1, last + 1):

	    handlepos = totalSize - 3
	    prevSize = self._size[self._paneNames[i - 1]]
	    nextSize = self._size[self._paneNames[i]]

	    offset1 = 0

	    if i == first:
		if prevSize < 4:
		    offset1 = 4 - prevSize
	    else:
		if prevSize < 8:
		    offset1 = (8 - prevSize) / 2

	    offset2 = 0

	    if i == last:
		if nextSize < 4:
		    offset2 = nextSize - 4
	    else:
		if nextSize < 8:
		    offset2 = (nextSize - 8) / 2

	    handlepos = handlepos + offset1

	    if self['orient'] == 'vertical':
		height = 8 - offset1 + offset2

		if height > 1:
		    self._button[i].configure(height = height)
		    self._button[i].place(x = btnp, y = handlepos)
		else:
		    self._button[i].place_forget()

		self._separator[i].place(x = 0, y = totalSize,
					 relwidth = 1)
	    else:
		width = 8 - offset1 + offset2

		if width > 1:
		    self._button[i].configure(width = width)
		    self._button[i].place(y = btnp, x = handlepos)
		else:
		    self._button[i].place_forget()

		self._separator[i].place(y = 0, x = totalSize,
					 relheight = 1)

	    totalSize = totalSize + nextSize + self._separatorThickness

    def pane(self, name):
	return self._frame[self._paneNames[self._nameToIndex(name)]]

    # Return the name of all panes
    def panes(self):
	return list(self._paneNames)

    def configurepane(self, name, **kw):
	name = self._paneNames[self._nameToIndex(name)]
	self._parsePaneOptions(name, kw)
	self._handleConfigure()

    def updatelayout(self):
	self._handleConfigure()

    def _getMotionLimit(self, item):
	curBefore = (item - 1) * self._separatorThickness
	minBefore, maxBefore = curBefore, curBefore

	for name in self._paneNames[:item]:
	    curBefore = curBefore + self._size[name]
	    minBefore = minBefore + self._min[name]
	    maxBefore = maxBefore + self._max[name]

	curAfter = (len(self._paneNames) - item) * self._separatorThickness
	minAfter, maxAfter = curAfter, curAfter
	for name in self._paneNames[item:]:
	    curAfter = curAfter + self._size[name]
	    minAfter = minAfter + self._min[name]
	    maxAfter = maxAfter + self._max[name]

	beforeToGo = min(curBefore - minBefore, maxAfter - curAfter)
	afterToGo = min(curAfter - minAfter, maxBefore - curBefore)

	self._beforeLimit = curBefore - beforeToGo
	self._afterLimit = curBefore + afterToGo
	self._curSize = curBefore

	self._plotHandles()

    # Compress the motion so that update is quick even on slow machines
    #
    # theRootp = root position (either rootx or rooty)
    def _btnMove(self, event, item):
	self._rootp = event

	if self._movePending == 0:
	    self._timerId = self.after_idle(
		    lambda s = self, i = item: s._btnMoveCompressed(i))
	    self._movePending = 1

    def destroy(self):
        if self._timerId is not None:
          self.after_cancel(self._timerId)
	  self._timerId = None
        Pmw.MegaWidget.destroy(self)

    def _btnMoveCompressed(self, item):
	if not self._buttonIsDown:
	    return

	if self['orient'] == 'vertical':
	    p = self._rootp.y_root - self.winfo_rooty()
	else:
	    p = self._rootp.x_root - self.winfo_rootx()

	if p == self._curSize:
	    self._movePending = 0
	    return

	if p < self._beforeLimit:
	    p = self._beforeLimit

	if p >= self._afterLimit:
	    p = self._afterLimit

	self._calculateChange(item, p)
	self.update_idletasks()
	self._movePending = 0

    # Calculate the change in response to mouse motions
    def _calculateChange(self, item, p):

	if p < self._curSize:
	    self._moveBefore(item, p)
	elif p > self._curSize:
	    self._moveAfter(item, p)

	self._plotHandles()

    def _moveBefore(self, item, p):
	n = self._curSize - p

	# Shrink the frames before
	iterRange = list(self._paneNames[:item])
	iterRange.reverse()
	self._iterate(iterRange, self._shrink, n)

	# Adjust the frames after
	iterRange = self._paneNames[item:]
	self._iterate(iterRange, self._grow, n)

	self._curSize = p

    def _moveAfter(self, item, p):
	n = p - self._curSize

	# Shrink the frames after
	iterRange = self._paneNames[item:]
	self._iterate(iterRange, self._shrink, n)

	# Adjust the frames before
	iterRange = list(self._paneNames[:item])
	iterRange.reverse()
	self._iterate(iterRange, self._grow, n)

	self._curSize = p
