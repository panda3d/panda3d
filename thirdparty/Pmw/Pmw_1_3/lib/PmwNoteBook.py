import string
import types
import Tkinter
import Pmw

class NoteBook(Pmw.MegaArchetype):

    def __init__(self, parent = None, **kw):

	# Define the megawidget options.
	INITOPT = Pmw.INITOPT
        optiondefs = (
	    ('hull_highlightthickness',  0,           None),
	    ('hull_borderwidth',         0,           None),
            ('arrownavigation',          1,           INITOPT),
            ('borderwidth',              2,           INITOPT),
            ('createcommand',            None,        None),
            ('lowercommand',             None,        None),
            ('pagemargin',               4,           INITOPT),
            ('raisecommand',             None,        None),
	    ('tabpos',                   'n',         INITOPT),
        )
	self.defineoptions(kw, optiondefs, dynamicGroups = ('Page', 'Tab'))

	# Initialise the base class (after defining the options).
	Pmw.MegaArchetype.__init__(self, parent, Tkinter.Canvas)

        self.bind('<Map>', self._handleMap)
        self.bind('<Configure>', self._handleConfigure)

        tabpos = self['tabpos']
	if tabpos is not None and tabpos != 'n':
            raise ValueError, \
                'bad tabpos option %s:  should be n or None' % repr(tabpos)
        self._withTabs = (tabpos is not None)
        self._pageMargin = self['pagemargin']
        self._borderWidth = self['borderwidth']

        # Use a dictionary as a set of bits indicating what needs to
        # be redisplayed the next time _layout() is called.  If
        # dictionary contains 'topPage' key, the value is the new top
        # page to be displayed.  None indicates that all pages have
        # been deleted and that _layout() should draw a border under where
        # the tabs should be.
        self._pending = {}
        self._pending['size'] = 1
        self._pending['borderColor'] = 1
        self._pending['topPage'] = None
        if self._withTabs:
            self._pending['tabs'] = 1

        self._canvasSize = None       # This gets set by <Configure> events

        # Set initial height of space for tabs
        if self._withTabs:
            self.tabBottom = 35
        else:
            self.tabBottom = 0

        self._lightBorderColor, self._darkBorderColor = \
                Pmw.Color.bordercolors(self, self['hull_background'])

	self._pageNames   = []        # List of page names

        # Map from page name to page info.  Each item is itself a
        # dictionary containing the following items:
        #   page           the Tkinter.Frame widget for the page
        #   created        set to true the first time the page is raised
        #   tabbutton      the Tkinter.Button widget for the button (if any)
        #   tabreqwidth    requested width of the tab
        #   tabreqheight   requested height of the tab
        #   tabitems       the canvas items for the button: the button
        #                  window item, the lightshadow and the darkshadow
        #   left           the left and right canvas coordinates of the tab
        #   right
	self._pageAttrs   = {}

        # Name of page currently on top (actually displayed, using
        # create_window, not pending).  Ignored if current top page
        # has been deleted or new top page is pending.  None indicates
        # no pages in notebook.
	self._topPageName = None

        # Canvas items used:
        #   Per tab: 
        #       top and left shadow
        #       right shadow
        #       button
        #   Per notebook: 
        #       page
        #       top page
        #       left shadow
        #       bottom and right shadow
        #       top (one or two items)

        # Canvas tags used:
        #   lighttag      - top and left shadows of tabs and page
        #   darktag       - bottom and right shadows of tabs and page
        #                   (if no tabs then these are reversed)
        #                   (used to color the borders by recolorborders)

        # Create page border shadows.
        if self._withTabs:
            self._pageLeftBorder = self.create_polygon(0, 0, 0, 0, 0, 0,
                fill = self._lightBorderColor, tags = 'lighttag')
            self._pageBottomRightBorder = self.create_polygon(0, 0, 0, 0, 0, 0,
                fill = self._darkBorderColor, tags = 'darktag')
            self._pageTop1Border = self.create_polygon(0, 0, 0, 0, 0, 0,
                fill = self._darkBorderColor, tags = 'lighttag')
            self._pageTop2Border = self.create_polygon(0, 0, 0, 0, 0, 0,
                fill = self._darkBorderColor, tags = 'lighttag')
        else:
            self._pageLeftBorder = self.create_polygon(0, 0, 0, 0, 0, 0,
                fill = self._darkBorderColor, tags = 'darktag')
            self._pageBottomRightBorder = self.create_polygon(0, 0, 0, 0, 0, 0,
                fill = self._lightBorderColor, tags = 'lighttag')
            self._pageTopBorder = self.create_polygon(0, 0, 0, 0, 0, 0,
                fill = self._darkBorderColor, tags = 'darktag')

	# Check keywords and initialise options.
	self.initialiseoptions()

    def insert(self, pageName, before = 0, **kw):
	if self._pageAttrs.has_key(pageName):
	    msg = 'Page "%s" already exists.' % pageName
	    raise ValueError, msg

        # Do this early to catch bad <before> spec before creating any items.
	beforeIndex = self.index(before, 1)

        pageOptions = {}
        if self._withTabs:
            # Default tab button options.
            tabOptions = {
                'text' : pageName,
                'borderwidth' : 0,
            }

        # Divide the keyword options into the 'page_' and 'tab_' options.
        for key in kw.keys():
            if key[:5] == 'page_':
                pageOptions[key[5:]] = kw[key]
                del kw[key]
            elif self._withTabs and key[:4] == 'tab_':
                tabOptions[key[4:]] = kw[key]
                del kw[key]
            else:
		raise KeyError, 'Unknown option "' + key + '"'

        # Create the frame to contain the page.
	page = apply(self.createcomponent, (pageName,
		(), 'Page',
		Tkinter.Frame, self._hull), pageOptions)

        attributes = {}
        attributes['page'] = page
        attributes['created'] = 0

        if self._withTabs:
            # Create the button for the tab.
            def raiseThisPage(self = self, pageName = pageName):
                self.selectpage(pageName)
            tabOptions['command'] = raiseThisPage
            tab = apply(self.createcomponent, (pageName + '-tab',
                    (), 'Tab',
                    Tkinter.Button, self._hull), tabOptions)

            if self['arrownavigation']:
                # Allow the use of the arrow keys for Tab navigation:
                def next(event, self = self, pageName = pageName):
                    self.nextpage(pageName)
                def prev(event, self = self, pageName = pageName):
                    self.previouspage(pageName)
                tab.bind('<Left>', prev)
                tab.bind('<Right>', next)

            attributes['tabbutton'] = tab
            attributes['tabreqwidth'] = tab.winfo_reqwidth()
            attributes['tabreqheight'] = tab.winfo_reqheight()

            # Create the canvas item to manage the tab's button and the items
            # for the tab's shadow.
            windowitem = self.create_window(0, 0, window = tab, anchor = 'nw')
            lightshadow = self.create_polygon(0, 0, 0, 0, 0, 0,
                tags = 'lighttag', fill = self._lightBorderColor)
            darkshadow = self.create_polygon(0, 0, 0, 0, 0, 0,
                tags = 'darktag', fill = self._darkBorderColor)
            attributes['tabitems'] = (windowitem, lightshadow, darkshadow)
            self._pending['tabs'] = 1

        self._pageAttrs[pageName] = attributes
	self._pageNames.insert(beforeIndex, pageName)

        # If this is the first page added, make it the new top page
        # and call the create and raise callbacks.
        if self.getcurselection() is None:
            self._pending['topPage'] = pageName
            self._raiseNewTop(pageName)

        self._layout()
        return page
  		
    def add(self, pageName, **kw):
        return apply(self.insert, (pageName, len(self._pageNames)), kw)

    def delete(self, *pageNames):
        newTopPage = 0
        for page in pageNames:
            pageIndex = self.index(page)
            pageName = self._pageNames[pageIndex]
            pageInfo = self._pageAttrs[pageName]

            if self.getcurselection() == pageName:
                if len(self._pageNames) == 1:
                    newTopPage = 0
                    self._pending['topPage'] = None
                elif pageIndex == len(self._pageNames) - 1:
                    newTopPage = 1
                    self._pending['topPage'] = self._pageNames[pageIndex - 1]
                else:
                    newTopPage = 1
                    self._pending['topPage'] = self._pageNames[pageIndex + 1]

            if self._topPageName == pageName:
                self._hull.delete(self._topPageItem)
                self._topPageName = None
                                
            if self._withTabs:
                self.destroycomponent(pageName + '-tab')
                apply(self._hull.delete, pageInfo['tabitems'])
            self.destroycomponent(pageName)
            del self._pageAttrs[pageName]
            del self._pageNames[pageIndex]

        # If the old top page was deleted and there are still pages
        # left in the notebook, call the create and raise callbacks.
        if newTopPage:
            pageName = self._pending['topPage']
            self._raiseNewTop(pageName)

        if self._withTabs:
            self._pending['tabs'] = 1
        self._layout()

    def page(self, pageIndex):
        pageName = self._pageNames[self.index(pageIndex)]
	return self._pageAttrs[pageName]['page']

    def pagenames(self):
	return list(self._pageNames)

    def getcurselection(self):
        if self._pending.has_key('topPage'):
            return self._pending['topPage']
        else:
            return self._topPageName

    def tab(self, pageIndex):
        if self._withTabs:
            pageName = self._pageNames[self.index(pageIndex)]
            return self._pageAttrs[pageName]['tabbutton']
        else:
            return None

    def index(self, index, forInsert = 0):
	listLength = len(self._pageNames)
	if type(index) == types.IntType:
	    if forInsert and index <= listLength:
		return index
	    elif not forInsert and index < listLength:
		return index
	    else:
		raise ValueError, 'index "%s" is out of range' % index
	elif index is Pmw.END:
	    if forInsert:
		return listLength
	    elif listLength > 0:
		return listLength - 1
	    else:
		raise ValueError, 'NoteBook has no pages'
	elif index is Pmw.SELECT:
	    if listLength == 0:
		raise ValueError, 'NoteBook has no pages'
            return self._pageNames.index(self.getcurselection())
	else:
            if index in self._pageNames:
                return self._pageNames.index(index)
	    validValues = 'a name, a number, Pmw.END or Pmw.SELECT'
	    raise ValueError, \
                'bad index "%s": must be %s' % (index, validValues)

    def selectpage(self, page):
        pageName = self._pageNames[self.index(page)]
        oldTopPage = self.getcurselection()
        if pageName != oldTopPage:
            self._pending['topPage'] = pageName
            if oldTopPage == self._topPageName:
                self._hull.delete(self._topPageItem)
            cmd = self['lowercommand']
            if cmd is not None:
                cmd(oldTopPage)
            self._raiseNewTop(pageName)

            self._layout()

        # Set focus to the tab of new top page:
        if self._withTabs and self['arrownavigation']:
            self._pageAttrs[pageName]['tabbutton'].focus_set()

    def previouspage(self, pageIndex = None):
        if pageIndex is None:
            curpage = self.index(Pmw.SELECT)
        else:
            curpage = self.index(pageIndex)
	if curpage > 0:
	    self.selectpage(curpage - 1)

    def nextpage(self, pageIndex = None):
        if pageIndex is None:
            curpage = self.index(Pmw.SELECT)
        else:
            curpage = self.index(pageIndex)
	if curpage < len(self._pageNames) - 1:
	    self.selectpage(curpage + 1)

    def setnaturalsize(self, pageNames = None):
        self.update_idletasks()
        maxPageWidth = 1
        maxPageHeight = 1
        if pageNames is None:
            pageNames = self.pagenames()
        for pageName in pageNames:
            pageInfo = self._pageAttrs[pageName]
            page = pageInfo['page']
            w = page.winfo_reqwidth()
            h = page.winfo_reqheight()
            if maxPageWidth < w:
                maxPageWidth = w
            if maxPageHeight < h:
                maxPageHeight = h
        pageBorder = self._borderWidth + self._pageMargin
        width = maxPageWidth + pageBorder * 2
        height = maxPageHeight + pageBorder * 2

        if self._withTabs:
            maxTabHeight = 0
            for pageInfo in self._pageAttrs.values():
                if maxTabHeight < pageInfo['tabreqheight']:
                    maxTabHeight = pageInfo['tabreqheight']
            height = height + maxTabHeight + self._borderWidth * 1.5

        # Note that, since the hull is a canvas, the width and height
        # options specify the geometry *inside* the borderwidth and
        # highlightthickness.
        self.configure(hull_width = width, hull_height = height)

    def recolorborders(self):
        self._pending['borderColor'] = 1
        self._layout()

    def _handleMap(self, event):
        self._layout()

    def _handleConfigure(self, event):
        self._canvasSize = (event.width, event.height)
        self._pending['size'] = 1
        self._layout()

    def _raiseNewTop(self, pageName):
        if not self._pageAttrs[pageName]['created']:
            self._pageAttrs[pageName]['created'] = 1
            cmd = self['createcommand']
            if cmd is not None:
                cmd(pageName)
        cmd = self['raisecommand']
        if cmd is not None:
            cmd(pageName)

    # This is the vertical layout of the notebook, from top (assuming
    # tabpos is 'n'):
    #     hull highlightthickness (top)
    #     hull borderwidth (top)
    #     borderwidth (top border of tabs)
    #     borderwidth * 0.5 (space for bevel)
    #     tab button (maximum of requested height of all tab buttons)
    #     borderwidth (border between tabs and page)
    #     pagemargin (top)
    #     the page itself
    #     pagemargin (bottom)
    #     borderwidth (border below page)
    #     hull borderwidth (bottom)
    #     hull highlightthickness (bottom)
    #
    # canvasBorder is sum of top two elements.
    # tabBottom is sum of top five elements.
    #
    # Horizontal layout (and also vertical layout when tabpos is None):
    #     hull highlightthickness
    #     hull borderwidth
    #     borderwidth
    #     pagemargin
    #     the page itself
    #     pagemargin
    #     borderwidth
    #     hull borderwidth
    #     hull highlightthickness
    #
    def _layout(self):
        if not self.winfo_ismapped() or self._canvasSize is None:
            # Don't layout if the window is not displayed, or we
            # haven't yet received a <Configure> event.
            return

        hullWidth, hullHeight = self._canvasSize
        borderWidth = self._borderWidth
        canvasBorder = string.atoi(self._hull['borderwidth']) + \
            string.atoi(self._hull['highlightthickness'])
        if not self._withTabs:
            self.tabBottom = canvasBorder
        oldTabBottom = self.tabBottom

        if self._pending.has_key('borderColor'):
            self._lightBorderColor, self._darkBorderColor = \
                    Pmw.Color.bordercolors(self, self['hull_background'])

        # Draw all the tabs.
        if self._withTabs and (self._pending.has_key('tabs') or
                self._pending.has_key('size')):
            # Find total requested width and maximum requested height
            # of tabs.
            sumTabReqWidth = 0
            maxTabHeight = 0
            for pageInfo in self._pageAttrs.values():
                sumTabReqWidth = sumTabReqWidth + pageInfo['tabreqwidth']
                if maxTabHeight < pageInfo['tabreqheight']:
                    maxTabHeight = pageInfo['tabreqheight']
            if maxTabHeight != 0:
                # Add the top tab border plus a bit for the angled corners
                self.tabBottom = canvasBorder + maxTabHeight + borderWidth * 1.5

            # Prepare for drawing the border around each tab button.
            tabTop = canvasBorder
            tabTop2 = tabTop + borderWidth
            tabTop3 = tabTop + borderWidth * 1.5
            tabBottom2 = self.tabBottom
            tabBottom = self.tabBottom + borderWidth

            numTabs = len(self._pageNames)
            availableWidth = hullWidth - 2 * canvasBorder - \
                numTabs * 2 * borderWidth
            x = canvasBorder
            cumTabReqWidth = 0
            cumTabWidth = 0

            # Position all the tabs.
            for pageName in self._pageNames:
                pageInfo = self._pageAttrs[pageName]
                (windowitem, lightshadow, darkshadow) = pageInfo['tabitems']
                if sumTabReqWidth <= availableWidth:
                    tabwidth = pageInfo['tabreqwidth']
                else:
                    # This ugly calculation ensures that, when the
                    # notebook is not wide enough for the requested
                    # widths of the tabs, the total width given to
                    # the tabs exactly equals the available width,
                    # without rounding errors.
                    cumTabReqWidth = cumTabReqWidth + pageInfo['tabreqwidth']
                    tmp = (2*cumTabReqWidth*availableWidth + sumTabReqWidth) \
                            / (2 * sumTabReqWidth)
                    tabwidth = tmp - cumTabWidth
                    cumTabWidth = tmp

                # Position the tab's button canvas item.
                self.coords(windowitem, x + borderWidth, tabTop3)
                self.itemconfigure(windowitem,
                    width = tabwidth, height = maxTabHeight)

                # Make a beautiful border around the tab.
                left = x
                left2 = left + borderWidth
                left3 = left + borderWidth * 1.5
                right = left + tabwidth + 2 * borderWidth
                right2 = left + tabwidth + borderWidth
                right3 = left + tabwidth + borderWidth * 0.5

                self.coords(lightshadow, 
                    left, tabBottom2, left, tabTop2, left2, tabTop,
                    right2, tabTop, right3, tabTop2, left3, tabTop2,
                    left2, tabTop3, left2, tabBottom,
                    )
                self.coords(darkshadow, 
                    right2, tabTop, right, tabTop2, right, tabBottom2,
                    right2, tabBottom, right2, tabTop3, right3, tabTop2,
                    )
                pageInfo['left'] = left
                pageInfo['right'] = right

                x = x + tabwidth + 2 * borderWidth

        # Redraw shadow under tabs so that it appears that tab for old
        # top page is lowered and that tab for new top page is raised.
        if self._withTabs and (self._pending.has_key('topPage') or
                self._pending.has_key('tabs') or self._pending.has_key('size')):

            if self.getcurselection() is None:
                # No pages, so draw line across top of page area.
                self.coords(self._pageTop1Border,
                    canvasBorder, self.tabBottom,
                    hullWidth - canvasBorder, self.tabBottom,
                    hullWidth - canvasBorder - borderWidth,
                        self.tabBottom + borderWidth,
                    borderWidth + canvasBorder, self.tabBottom + borderWidth,
                    )

                # Ignore second top border.
                self.coords(self._pageTop2Border, 0, 0, 0, 0, 0, 0)
            else:
                # Draw two lines, one on each side of the tab for the
                # top page, so that the tab appears to be raised.
                pageInfo = self._pageAttrs[self.getcurselection()]
                left = pageInfo['left']
                right = pageInfo['right']
                self.coords(self._pageTop1Border,
                    canvasBorder, self.tabBottom,
                    left, self.tabBottom,
                    left + borderWidth, self.tabBottom + borderWidth,
                    canvasBorder + borderWidth, self.tabBottom + borderWidth,
                    )

                self.coords(self._pageTop2Border,
                    right, self.tabBottom,
                    hullWidth - canvasBorder, self.tabBottom,
                    hullWidth - canvasBorder - borderWidth,
                        self.tabBottom + borderWidth,
                    right - borderWidth, self.tabBottom + borderWidth,
                    )

            # Prevent bottom of dark border of tabs appearing over
            # page top border.
            self.tag_raise(self._pageTop1Border)
            self.tag_raise(self._pageTop2Border)

        # Position the page border shadows.
        if self._pending.has_key('size') or oldTabBottom != self.tabBottom:

            self.coords(self._pageLeftBorder,
                canvasBorder, self.tabBottom,
                borderWidth + canvasBorder,
                    self.tabBottom + borderWidth,
                borderWidth + canvasBorder,
                    hullHeight - canvasBorder - borderWidth,
                canvasBorder, hullHeight - canvasBorder,
                )

            self.coords(self._pageBottomRightBorder,
                hullWidth - canvasBorder, self.tabBottom,
                hullWidth - canvasBorder, hullHeight - canvasBorder,
                canvasBorder, hullHeight - canvasBorder,
                borderWidth + canvasBorder,
                    hullHeight - canvasBorder - borderWidth,
                hullWidth - canvasBorder - borderWidth,
                    hullHeight - canvasBorder - borderWidth,
                hullWidth - canvasBorder - borderWidth,
                    self.tabBottom + borderWidth,
                )

            if not self._withTabs:
                self.coords(self._pageTopBorder,
                    canvasBorder, self.tabBottom,
                    hullWidth - canvasBorder, self.tabBottom,
                    hullWidth - canvasBorder - borderWidth,
                        self.tabBottom + borderWidth,
                    borderWidth + canvasBorder, self.tabBottom + borderWidth,
                    )

        # Color borders.
        if self._pending.has_key('borderColor'):
            self.itemconfigure('lighttag', fill = self._lightBorderColor)
            self.itemconfigure('darktag', fill = self._darkBorderColor)

        newTopPage = self._pending.get('topPage')
        pageBorder = borderWidth + self._pageMargin

        # Raise new top page.
        if newTopPage is not None:
            self._topPageName = newTopPage
            self._topPageItem = self.create_window(
                pageBorder + canvasBorder, self.tabBottom + pageBorder,
                window = self._pageAttrs[newTopPage]['page'],
                anchor = 'nw',
                )

        # Change position of top page if tab height has changed.
        if self._topPageName is not None and oldTabBottom != self.tabBottom:
            self.coords(self._topPageItem,
                    pageBorder + canvasBorder, self.tabBottom + pageBorder)

        # Change size of top page if,
        #   1) there is a new top page.
        #   2) canvas size has changed, but not if there is no top
        #      page (eg:  initially or when all pages deleted).
        #   3) tab height has changed, due to difference in the height of a tab
        if (newTopPage is not None or \
                self._pending.has_key('size') and self._topPageName is not None
                or oldTabBottom != self.tabBottom):
            self.itemconfigure(self._topPageItem,
                width = hullWidth - 2 * canvasBorder - pageBorder * 2,
                height = hullHeight - 2 * canvasBorder - pageBorder * 2 -
                    (self.tabBottom - canvasBorder),
                )

        self._pending = {}

# Need to do forwarding to get the pack, grid, etc methods. 
# Unfortunately this means that all the other canvas methods are also
# forwarded.
Pmw.forwardmethods(NoteBook, Tkinter.Canvas, '_hull')
