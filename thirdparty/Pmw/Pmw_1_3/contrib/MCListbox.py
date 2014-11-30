#
#  FILE: MCListbox.py
#
#  DESCRIPTION:
#    This file provides a generic Multi-Column Listbox widget.  It is derived
#    from a heavily hacked version of Pmw.ScrolledFrame
#
#  This program is free software; you can redistribute  it and/or modify it
#  under  the terms of  the GNU General  Public License as published by the
#  Free Software Foundation;  either version 2 of the  License, or (at your
#  option) any later version.
#
#  THIS  SOFTWARE  IS PROVIDED   ``AS  IS'' AND   ANY  EXPRESS OR IMPLIED
#  WARRANTIES,   INCLUDING, BUT NOT  LIMITED  TO, THE IMPLIED WARRANTIES OF
#  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN
#  NO  EVENT  SHALL   THE AUTHOR  BE    LIABLE FOR ANY   DIRECT, INDIRECT,
#  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
#  NOT LIMITED   TO, PROCUREMENT OF  SUBSTITUTE GOODS  OR SERVICES; LOSS OF
#  USE, DATA,  OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
#  ANY THEORY OF LIABILITY, WHETHER IN  CONTRACT, STRICT LIABILITY, OR TORT
#  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
#  THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
#  You should have received a copy of the  GNU General Public License along
#  with this program; if not, write  to the Free Software Foundation, Inc.,
#  675 Mass Ave, Cambridge, MA 02139, USA.
#

import string
import Tkinter
import Pmw

class MultiColumnListbox(Pmw.MegaWidget):
    def __init__(self, parent = None, **kw):
        colors = Pmw.Color.getdefaultpalette(parent)

        # Define the megawidget options.
        INITOPT = Pmw.INITOPT
        optiondefs = (
            #('borderframe',      1,                          INITOPT),
            ('horizflex',        'fixed',                    self._horizflex),
            ('horizfraction',    0.05,                       INITOPT),
            ('hscrollmode',      'dynamic',                  self._hscrollMode),
            ('labelmargin',      0,                          INITOPT),
            ('labelpos',         None,                       INITOPT),
            ('scrollmargin',     2,                          INITOPT),
            ('usehullsize',      0,                          INITOPT),
            ('vertflex',         'fixed',                    self._vertflex),
            ('vertfraction',     0.05,                       INITOPT),
            ('vscrollmode',      'dynamic',                  self._vscrollMode),
            ('labellist',        None,                       INITOPT),
            ('selectbackground', colors['selectBackground'], INITOPT),
            ('selectforeground', colors['selectForeground'], INITOPT),
            ('background',       colors['background'],       INITOPT),
            ('foreground',       colors['foreground'],       INITOPT),
            ('command',          None,                       None),
            ('dblclickcommand',  None,                       None),
        )
        self.defineoptions(kw, optiondefs)

        # Initialise the base class (after defining the options).
        Pmw.MegaWidget.__init__(self, parent)

        self._numcolumns = len(self['labellist'])
        self._columnlabels = self['labellist']
        self._lineid = 0
        self._numrows = 0
        self._lineitemframes = []
        self._lineitems = []
        self._lineitemdata = {}
        self._labelframe = {}
        self._cursel = []
        
        # Create the components.
        self.origInterior = Pmw.MegaWidget.interior(self)

        if self['usehullsize']:
            self.origInterior.grid_propagate(0)

        # Create a frame widget to act as the border of the clipper. 
        self._borderframe = self.createcomponent('borderframe',
                                                 (), None,
                                                 Tkinter.Frame,
                                                 (self.origInterior,),
                                                 relief = 'sunken',
                                                 borderwidth = 2,
                                                 )
        self._borderframe.grid(row = 2, column = 2,
                               rowspan = 2, sticky = 'news')

        # Create the clipping windows.
        self._hclipper = self.createcomponent('hclipper',
                                              (), None,
                                              Tkinter.Frame,
                                              (self._borderframe,),
                                              width = 400,
                                              height = 300,
                                              )
        self._hclipper.pack(fill = 'both', expand = 1)

        self._hsframe = self.createcomponent('hsframe', (), None,
                                             Tkinter.Frame,
                                             (self._hclipper,),
                                             )
                                             

        self._vclipper = self.createcomponent('vclipper',
                                              (), None,
                                              Tkinter.Frame,
                                              (self._hsframe,),
                                              #width = 400,
                                              #height = 300,
                                              highlightthickness = 0,
                                              borderwidth = 0,
                                              )

        self._vclipper.grid(row = 1, column = 0,
                            columnspan = self._numcolumns,
                            sticky = 'news')#, expand = 1)
        self._hsframe.grid_rowconfigure(1, weight = 1)#, minsize = 300)
        

        gridcolumn = 0
        for labeltext in self._columnlabels:
            lframe = self.createcomponent(labeltext+'frame', (), None,
                                          Tkinter.Frame,
                                          (self._hsframe,),
                                          borderwidth = 1,
                                          relief = 'raised',
                                          )
            label = self.createcomponent(labeltext, (), None,
                                         Tkinter.Label,
                                         (lframe,),
                                         text = labeltext,
                                         )
            label.pack(expand = 0, fill = 'y', side = 'left')
            lframe.grid(row = 0, column = gridcolumn, sticky = 'ews')
            self._labelframe[labeltext] = lframe
            #lframe.update()
            #print lframe.winfo_reqwidth()
            self._hsframe.grid_columnconfigure(gridcolumn, weight = 1)
            gridcolumn = gridcolumn + 1

        lframe.update()
        self._labelheight = lframe.winfo_reqheight()
        self.origInterior.grid_rowconfigure(2, minsize = self._labelheight + 2)

        self.origInterior.grid_rowconfigure(3, weight = 1, minsize = 0)
        self.origInterior.grid_columnconfigure(2, weight = 1, minsize = 0)
        
        # Create the horizontal scrollbar
        self._horizScrollbar = self.createcomponent('horizscrollbar',
                                                    (), 'Scrollbar',
                                                    Tkinter.Scrollbar,
                                                    (self.origInterior,),
                                                    orient='horizontal',
                                                    command=self._xview
                                                    )

        # Create the vertical scrollbar
        self._vertScrollbar = self.createcomponent('vertscrollbar',
                                                   (), 'Scrollbar',
                                                   Tkinter.Scrollbar,
                                                   (self.origInterior,),
                                                   #(self._hclipper,),
                                                   orient='vertical',
                                                   command=self._yview
                                                   )

        self.createlabel(self.origInterior, childCols = 3, childRows = 4)

        # Initialise instance variables.
        self._horizScrollbarOn = 0
        self._vertScrollbarOn = 0
        self.scrollTimer = None
        self._scrollRecurse = 0
        self._horizScrollbarNeeded = 0
        self._vertScrollbarNeeded = 0
        self.startX = 0
        self.startY = 0
        self._flexoptions = ('fixed', 'expand', 'shrink', 'elastic')

        # Create a frame in the clipper to contain the widgets to be
        # scrolled.
        self._vsframe = self.createcomponent('vsframe',
                                             (), None,
                                             Tkinter.Frame,
                                             (self._vclipper,),
                                             #height = 300,
                                             #borderwidth = 4,
                                             #relief = 'groove',
                                             )

        # Whenever the clipping window or scrolled frame change size,
        # update the scrollbars.
        self._hsframe.bind('<Configure>', self._reposition)
        self._vsframe.bind('<Configure>', self._reposition)
        self._hclipper.bind('<Configure>', self._reposition)
        self._vclipper.bind('<Configure>', self._reposition)

        #elf._vsframe.bind('<Button-1>', self._vsframeselect)
        
        # Check keywords and initialise options.
        self.initialiseoptions()

    def destroy(self):
        if self.scrollTimer is not None:
            self.after_cancel(self.scrollTimer)
            self.scrollTimer = None
        Pmw.MegaWidget.destroy(self)

    # ======================================================================

    # Public methods.

    def interior(self):
        return self._vsframe

    # Set timer to call real reposition method, so that it is not
    # called multiple times when many things are reconfigured at the
    # same time.
    def reposition(self):
        if self.scrollTimer is None:
            self.scrollTimer = self.after_idle(self._scrollBothNow)



    def insertrow(self, index, rowdata):
        #if len(rowdata) != self._numcolumns:
        #    raise ValueError, 'Number of items in rowdata does not match number of columns.'
        if index > self._numrows:
            index = self._numrows

        rowframes = {}
        for columnlabel in self._columnlabels:
            celldata = rowdata.get(columnlabel)
            cellframe = self.createcomponent(('cellframeid.%d.%s'%(self._lineid,
                                                                   columnlabel)),
                                             (), ('Cellframerowid.%d'%self._lineid),
                                             Tkinter.Frame,
                                             (self._vsframe,),
                                             background = self['background'],
                                             #borderwidth = 1,
                                             #relief = 'flat'
                                             )

            cellframe.bind('<Double-Button-1>', self._cellframedblclick)
            cellframe.bind('<Button-1>', self._cellframeselect)
                                             
            if celldata:
                cell = self.createcomponent(('cellid.%d.%s'%(self._lineid,
                                                             columnlabel)),
                                            (), ('Cellrowid.%d'%self._lineid),
                                            Tkinter.Label,
                                            (cellframe,),
                                            background = self['background'],
                                            foreground = self['foreground'],
                                            text = celldata,
                                            )

                cell.bind('<Double-Button-1>', self._celldblclick)
                cell.bind('<Button-1>', self._cellselect)

                cell.pack(expand = 0, fill = 'y', side = 'left', padx = 1, pady = 1)
            rowframes[columnlabel] = cellframe

        self._lineitemdata[self._lineid] = rowdata
        self._lineitems.insert(index, self._lineid)
        self._lineitemframes.insert(index, rowframes)
        self._numrows = self._numrows + 1
        self._lineid = self._lineid + 1

        self._placedata(index)

    def _placedata(self, index = 0):
        gridy = index
        for rowframes in self._lineitemframes[index:]:
            gridx = 0
            for columnlabel in self._columnlabels:
                rowframes[columnlabel].grid(row = gridy,
                                           column = gridx,
                                           sticky = 'news')
                gridx = gridx + 1
            gridy = gridy + 1
                


    def addrow(self, rowdata):
        self.insertrow(self._numrows, rowdata)

    def delrow(self, index):
        rowframes = self._lineitemframes.pop(index)
        for columnlabel in self._columnlabels:
            rowframes[columnlabel].destroy()
        self._placedata(index)
        self._numrows = self._numrows - 1
        del self._lineitems[index]
        if index in self._cursel:
            self._cursel.remove(index)
        
        
    def curselection(self):
        # Return a tuple of just one element as this will probably be the
        # interface used in a future implementation when multiple rows can
        # be selected at once.
        return tuple(self._cursel)

    def getcurselection(self):
        # Return a tuple of just one row as this will probably be the
        # interface used in a future implementation when multiple rows can
        # be selected at once.
        sellist = []
        for sel in self._cursel:
            sellist.append(self._lineitemdata[self._lineitems[sel]])
        return tuple(sellist)

    # ======================================================================

    # Configuration methods.

    def _hscrollMode(self):
        # The horizontal scroll mode has been configured.

        mode = self['hscrollmode']


        if mode == 'static':
            if not self._horizScrollbarOn:
                self._toggleHorizScrollbar()
        elif mode == 'dynamic':
            if self._horizScrollbarNeeded != self._horizScrollbarOn:
                self._toggleHorizScrollbar()
        elif mode == 'none':
            if self._horizScrollbarOn:
                self._toggleHorizScrollbar()
        else:
            message = 'bad hscrollmode option "%s": should be static, dynamic, or none' % mode
            raise ValueError, message

    def _vscrollMode(self):
        # The vertical scroll mode has been configured.

        mode = self['vscrollmode']

        if mode == 'static':
            if not self._vertScrollbarOn:
                self._toggleVertScrollbar()
        elif mode == 'dynamic':
            if self._vertScrollbarNeeded != self._vertScrollbarOn:
                self._toggleVertScrollbar()
        elif mode == 'none':
            if self._vertScrollbarOn:
                self._toggleVertScrollbar()
        else:
            message = 'bad vscrollmode option "%s": should be static, dynamic, or none' % mode
            raise ValueError, message

    def _horizflex(self):
        # The horizontal flex mode has been configured.

        flex = self['horizflex']

        if flex not in self._flexoptions:
            message = 'bad horizflex option "%s": should be one of %s' % \
                    mode, str(self._flexoptions)
            raise ValueError, message

        self.reposition()

    def _vertflex(self):
        # The vertical flex mode has been configured.

        flex = self['vertflex']

        if flex not in self._flexoptions:
            message = 'bad vertflex option "%s": should be one of %s' % \
                    mode, str(self._flexoptions)
            raise ValueError, message

        self.reposition()



    # ======================================================================

    # Private methods.

    def _reposition(self, event):
        gridx = 0
        for col in self._columnlabels:
            maxwidth = self._labelframe[col].winfo_reqwidth()
            for row in self._lineitemframes:
                cellwidth = row[col].winfo_reqwidth()
                if cellwidth > maxwidth:
                    maxwidth = cellwidth
            self._hsframe.grid_columnconfigure(gridx, minsize = maxwidth)
            gridwidth = self._hsframe.grid_bbox(column = gridx, row = 0)[2]
            if self['horizflex'] in ('expand', 'elastic') and gridwidth > maxwidth:
                maxwidth = gridwidth
            self._vsframe.grid_columnconfigure(gridx, minsize = maxwidth)
            gridx = gridx + 1
            
            

        self._vclipper.configure(height = self._hclipper.winfo_height() - self._labelheight)
        
        self.reposition()

    # Called when the user clicks in the horizontal scrollbar. 
    # Calculates new position of frame then calls reposition() to
    # update the frame and the scrollbar.
    def _xview(self, mode, value, units = None):

        if mode == 'moveto':
            frameWidth = self._hsframe.winfo_reqwidth()
            self.startX = string.atof(value) * float(frameWidth)
        else:
            clipperWidth = self._hclipper.winfo_width()
            if units == 'units':
                jump = int(clipperWidth * self['horizfraction'])
            else:
                jump = clipperWidth

            if value == '1':
                self.startX = self.startX + jump
            else:
                self.startX = self.startX - jump

        self.reposition()

    # Called when the user clicks in the vertical scrollbar. 
    # Calculates new position of frame then calls reposition() to
    # update the frame and the scrollbar.
    def _yview(self, mode, value, units = None):

        if mode == 'moveto':
            frameHeight = self._vsframe.winfo_reqheight()
            self.startY = string.atof(value) * float(frameHeight)
        else:
            clipperHeight = self._vclipper.winfo_height()
            if units == 'units':
                jump = int(clipperHeight * self['vertfraction'])
            else:
                jump = clipperHeight

            if value == '1':
                self.startY = self.startY + jump
            else:
                self.startY = self.startY - jump

        self.reposition()

    def _getxview(self):

        # Horizontal dimension.
        clipperWidth = self._hclipper.winfo_width()
        frameWidth = self._hsframe.winfo_reqwidth()
        if frameWidth <= clipperWidth:
            # The scrolled frame is smaller than the clipping window.

            self.startX = 0
            endScrollX = 1.0

            if self['horizflex'] in ('expand', 'elastic'):
                relwidth = 1
            else:
                relwidth = ''
        else:
            # The scrolled frame is larger than the clipping window.

            if self['horizflex'] in ('shrink', 'elastic'):
                self.startX = 0
                endScrollX = 1.0
                relwidth = 1
            else:
                if self.startX + clipperWidth > frameWidth:
                    self.startX = frameWidth - clipperWidth
                    endScrollX = 1.0
                else:
                    if self.startX < 0:
                        self.startX = 0
                    endScrollX = (self.startX + clipperWidth) / float(frameWidth)
                relwidth = ''

        # Position frame relative to clipper.
        self._hsframe.place(x = -self.startX, relwidth = relwidth)
        return (self.startX / float(frameWidth), endScrollX)

    def _getyview(self):

        # Vertical dimension.
        clipperHeight = self._vclipper.winfo_height()
        frameHeight = self._vsframe.winfo_reqheight()
        if frameHeight <= clipperHeight:
            # The scrolled frame is smaller than the clipping window.

            self.startY = 0
            endScrollY = 1.0

            if self['vertflex'] in ('expand', 'elastic'):
                relheight = 1
            else:
                relheight = ''
        else:
            # The scrolled frame is larger than the clipping window.

            if self['vertflex'] in ('shrink', 'elastic'):
                self.startY = 0
                endScrollY = 1.0
                relheight = 1
            else:
                if self.startY + clipperHeight > frameHeight:
                    self.startY = frameHeight - clipperHeight
                    endScrollY = 1.0
                else:
                    if self.startY < 0:
                        self.startY = 0
                    endScrollY = (self.startY + clipperHeight) / float(frameHeight)
                relheight = ''

        # Position frame relative to clipper.
        self._vsframe.place(y = -self.startY, relheight = relheight)
        return (self.startY / float(frameHeight), endScrollY)

    # According to the relative geometries of the frame and the
    # clipper, reposition the frame within the clipper and reset the
    # scrollbars.
    def _scrollBothNow(self):
        self.scrollTimer = None

        # Call update_idletasks to make sure that the containing frame
        # has been resized before we attempt to set the scrollbars. 
        # Otherwise the scrollbars may be mapped/unmapped continuously.
        self._scrollRecurse = self._scrollRecurse + 1
        self.update_idletasks()
        self._scrollRecurse = self._scrollRecurse - 1
        if self._scrollRecurse != 0:
            return

        xview = self._getxview()
        yview = self._getyview()
        self._horizScrollbar.set(xview[0], xview[1])
        self._vertScrollbar.set(yview[0], yview[1])

        self._horizScrollbarNeeded = (xview != (0.0, 1.0))
        self._vertScrollbarNeeded = (yview != (0.0, 1.0))

        # If both horizontal and vertical scrollmodes are dynamic and
        # currently only one scrollbar is mapped and both should be
        # toggled, then unmap the mapped scrollbar.  This prevents a
        # continuous mapping and unmapping of the scrollbars. 
        if (self['hscrollmode'] == self['vscrollmode'] == 'dynamic' and
                self._horizScrollbarNeeded != self._horizScrollbarOn and
                self._vertScrollbarNeeded != self._vertScrollbarOn and
                self._vertScrollbarOn != self._horizScrollbarOn):
            if self._horizScrollbarOn:
                self._toggleHorizScrollbar()
            else:
                self._toggleVertScrollbar()
            return

        if self['hscrollmode'] == 'dynamic':
            if self._horizScrollbarNeeded != self._horizScrollbarOn:
                self._toggleHorizScrollbar()

        if self['vscrollmode'] == 'dynamic':
            if self._vertScrollbarNeeded != self._vertScrollbarOn:
                self._toggleVertScrollbar()

    def _toggleHorizScrollbar(self):

        self._horizScrollbarOn = not self._horizScrollbarOn

        interior = self.origInterior
        if self._horizScrollbarOn:
            self._horizScrollbar.grid(row = 5, column = 2, sticky = 'news')
            interior.grid_rowconfigure(4, minsize = self['scrollmargin'])
        else:
            self._horizScrollbar.grid_forget()
            interior.grid_rowconfigure(4, minsize = 0)

    def _toggleVertScrollbar(self):

        self._vertScrollbarOn = not self._vertScrollbarOn

        interior = self.origInterior
        if self._vertScrollbarOn:
            self._vertScrollbar.grid(row = 3, column = 4, sticky = 'news')
            interior.grid_columnconfigure(3, minsize = self['scrollmargin'])
        else:
            self._vertScrollbar.grid_forget()
            interior.grid_columnconfigure(3, minsize = 0)

    # ======================================================================

    # Selection methods.

    #def _vsframeselect(self, event):
    #    print 'vsframe event x: %d  y: %d'%(event.x, event.y)
    #    col, row = self._vsframe.grid_location(event.x, event.y)
    #    self._select(col, row)

    def _cellframeselect(self, event):
        #print 'cellframe event x: %d  y: %d'%(event.x, event.y)
        x = event.widget.winfo_x()
        y = event.widget.winfo_y()
        #col, row = self._vsframe.grid_location(x + event.x, y + event.y)
        self._select(x + event.x, y + event.y)#(col, row)

    def _cellselect(self, event):
        #print 'cell event x: %d  y: %d'%(event.x, event.y)
        lx = event.widget.winfo_x()
        ly = event.widget.winfo_y()
        parent = event.widget.pack_info()['in']
        fx = parent.winfo_x()
        fy = parent.winfo_y()
        #col, row = self._vsframe.grid_location(fx + lx + event.x, fy + ly + event.y)
        self._select(fx + lx + event.x, fy + ly + event.y)#(col, row)

    def _select(self, x, y):
        col, row = self._vsframe.grid_location(x, y)
        #print 'Clicked on col: %d  row: %d'%(col,row)
        cfg = {}
        lineid = self._lineitems[row]
        cfg['Cellrowid.%d_foreground'%lineid] = self['selectforeground']
        cfg['Cellrowid.%d_background'%lineid] = self['selectbackground']
        cfg['Cellframerowid.%d_background'%lineid] = self['selectbackground']
        #cfg['Cellframerowid%d_relief'%row] = 'raised'

        if self._cursel != []:
            cursel = self._cursel[0]
            lineid = self._lineitems[cursel]
            if cursel != None and cursel != row:
                cfg['Cellrowid.%d_foreground'%lineid] = self['foreground']
                cfg['Cellrowid.%d_background'%lineid] = self['background']
                cfg['Cellframerowid.%d_background'%lineid] = self['background']
                #cfg['Cellframerowid%d_relief'%cursel] = 'flat'
            
        apply(self.configure, (), cfg)
        self._cursel = [row]

        cmd = self['command']
        if callable(cmd):
            cmd()



    def _cellframedblclick(self, event):
        #print 'double click cell frame'
        cmd = self['dblclickcommand']
        if callable(cmd):
            cmd()

    def _celldblclick(self, event):
        #print 'double click cell'
        cmd = self['dblclickcommand']
        if callable(cmd):
            cmd()

if __name__ == '__main__':

    rootWin = Tkinter.Tk()

    Pmw.initialise()

    rootWin.title('MultiColumnListbox Demo')
    rootWin.configure(width = 500, height = 300)
    rootWin.update()

    def dbl():
        print listbox.getcurselection()

    listbox = MultiColumnListbox(rootWin,
                                           #usehullsize = 1,
                                           labellist = ('Column 0',
                                                        'Column 1',
                                                        'Column 2',
                                                        'Column 3',
                                                        'Column 4',
                                                        #'Column 5',
                                                        #'Column 6',
                                                        #'Column 7',
                                                        #'Column 8',
                                                        #'Column 9',
                                                        ),
                                           horizflex = 'expand',
                                           #vertflex = 'elastic',
                                           dblclickcommand = dbl,
                                           )


    #print 'start adding item'
    for i in range(20):
        r = {}
        for j in range(5):
            r[('Column %d'%j)] = 'Really long item name %d'%i
        listbox.addrow(r)
    #print 'items added'

    listbox.pack(expand = 1, fill = 'both', padx = 10, pady = 10)


    exitButton = Tkinter.Button(rootWin, text="Quit", command=rootWin.quit)
    exitButton.pack(side = 'left', padx = 10, pady = 10)

    rootWin.mainloop()
