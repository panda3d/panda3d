#
#  FILE: DirBrowser.py
#
#  DESCRIPTION:
#    This file provides a generic Directory browser selection widget.
#
#  AUTHOR:  MontaVista Software, Inc. <source@mvista.com>
#
#  Copyright 2001 MontaVista Software Inc.
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


import os
import tkinter
import Pmw


class DirBrowserDialog(Pmw.MegaToplevel):
    def __init__(self, parent = None, **kw):
        cwd = os.getcwd()
        # Define the megawidget options.
        INITOPT = Pmw.INITOPT
        optiondefs = (
            ('path',               cwd,             None),
            ('hidedotfiles',       1,               INITOPT),
            ('label',              None,            INITOPT),
            #('labelmargin',        0,               INITOPT),
            #('labelpos',           None,            INITOPT),
            ('borderx',    20,  INITOPT),
            ('bordery',    20,  INITOPT),
            )

        self.defineoptions(kw, optiondefs)

        # Initialise the base class (after defining the options).
        Pmw.MegaToplevel.__init__(self, parent)

        interior = self.interior()

        self.childframe = self.createcomponent('childframe', (), None,
                                               tkinter.Frame,
                                               (interior,),
                                               borderwidth = 1,
                                               relief = 'raised',
                                              )
        self.childframe.pack(expand = 1,
                             fill = 'both',
                             )

        self.labelframe = self.createcomponent('labelframe', (), None,
                                              tkinter.Frame,
                                              (self.childframe,),
                                              borderwidth = 2,
                                              relief = 'groove',
                                              )
        self.labelframe.pack(padx = 10, pady = 10, expand = 1, fill = 'both')

        if self['label']:
            self.label = self.createcomponent('label', (), None,
                                              tkinter.Label,
                                              (self.childframe,),
                                              text = self['label'],
                                              )
            self.label.place(x = (10 + self['borderx']), y = 10, anchor = 'w')


        self.workframe = self.createcomponent('workframe', (), None,
                                              tkinter.Frame,
                                              (self.labelframe,),
                                              #borderwidth = 2,
                                              #relief = 'groove',
                                              )
        self.workframe.pack(padx = self['borderx'],
                            pady = self['bordery'],
                            expand = 1,
                            fill = 'both',
                            )

        self.buttonframe = self.createcomponent('buttonframe', (), None,
                                                tkinter.Frame,
                                                (interior,),
                                                borderwidth = 1,
                                                relief = 'raised',
                                                )
        self.buttonframe.pack(expand = 0,
                              fill = 'x',
                              )

        self.optbox = self.createcomponent('optbox', (), None,
                                           Pmw.OptionMenu,
                                           (self.workframe,),
                                           command = self.setpath,
                                           )
        self.optbox.bind('<Configure>', self._setMinimumSize)

        self.listbox = self.createcomponent('listbox', (), None,
                                            Pmw.ScrolledListBox,
                                            (self.workframe,),
                                            dblclickcommand = self._select,
                                            )

        path = self['path']
        self.entry = self.createcomponent('entryfield', (), None,
                                          Pmw.EntryField,
                                          (self.workframe,),
                                          value = path,
                                          command = self.enteredpath,
                                          labelpos = 'nw',
                                          label_text = 'Current Path:',
                                          )

        #self.createlabel(self.workframe, childCols = 1, childRows = 3)

        self.buttonbox = self.createcomponent('buttonbox', (), None,
                                              Pmw.ButtonBox,
                                              (self.buttonframe,),
                                              )
        self.buttonbox.add('OK', text = 'OK',
                           command = self.okbutton)
        self.buttonbox.add('Cancel', text = 'Cancel',
                           command =  self.cancelbutton)
        self.buttonbox.add('New Directory', text = 'New Directory',
                           command =  self.newdirbutton)

        self.buttonbox.alignbuttons()
        self.buttonbox.pack(expand = 1, fill = 'x')

        self.optbox.grid(row = 2, column = 2, sticky = 'ew')
        self.listbox.grid(row = 3, column = 2, sticky = 'news')
        self.entry.grid(row = 5, column = 2, sticky = 'ew')
        self.workframe.grid_rowconfigure(3, weight = 1)
        self.workframe.grid_rowconfigure(4, minsize = 20)
        self.workframe.grid_columnconfigure(2, weight = 1)


        self.setpath(self['path'])

        # Check keywords and initialise options.
        self.initialiseoptions()

    def setpath(self, path):
        path = os.path.abspath(os.path.expanduser(path))

        if os.path.isfile(path):
            path = os.path.dirname(path)

        dirlist = []
        hidedotfiles = self['hidedotfiles']
        try:
            posix = (os.name == 'posix')
            for entry in os.listdir(path):
                entryPath = path + '/' + entry
                if hidedotfiles and entry[0] == '.':
                    # skip dot files if desired
                    continue
                if not os.path.isdir(entryPath):
                    # skip files
                    continue
                if not os.access(entryPath, os.R_OK | os.X_OK):
                    # skip directories we can't enter any way
                    continue
                dirlist.append(entry)

        except:
            self.entry.setentry(self['path'])
            return

        self.entry.setentry(path)

        self['path'] = path

        dirlist.sort()
        if path != '/':
            dirlist.insert(0, '..')

        self.listbox.setlist(dirlist)
        pathlist = []
        while path != '/':
            pathlist.append(path)
            path = os.path.dirname(path)
        pathlist.append('/')
        self.optbox.setitems(pathlist, 0)

    def _setMinimumSize(self, event):
        # If the optionmenu changes width, make sure it does not
        # shrink later.
        owidth = self.optbox.winfo_width()
        self.workframe.grid_columnconfigure(2, minsize = owidth)

    def _select(self):
        sel = self.listbox.getcurselection()
        if self['path'] == '/':
            self['path'] = ''
        if len(sel) > 0:
            if sel[0] == '..':
                self.setpath(os.path.dirname(self['path']))
            else:
                self.setpath(self['path'] + '/' + sel[0])


    def getcurpath(self):
        return self['path']

    def enteredpath(self):
        self.setpath(self.entry.get())

    def okbutton(self):
        self.deactivate(self['path'])

    def cancelbutton(self):
        self.deactivate(None)

    def newdirbutton(self):
        CreateDirectoryPopup(self.interior(), self['path'])
        self.setpath(self['path'])



class CreateDirectoryPopup:
    def __init__(self, parent, path):
        self.path = path
        self.parent = parent
        self.newdirpopup = Pmw.PromptDialog(parent,
                                            buttons = ('OK', 'Cancel'),
                                            defaultbutton = 'OK',
                                            title = 'New Directory',
                                            entryfield_labelpos = 'nw',
                                            label_text = 'Enter new directory name for:\n%s'%self.path,
                                            command = self._buttonpress
                                            )

        self.newdirpopup.activate()

    def _buttonpress(self, button):
        if button == 'OK':
            newdirname = self.newdirpopup.get()
            dirlist = os.listdir(self.path)
            if newdirname in dirlist:
                ErrorPopup(self.parent,
                           'Error: "%s", already exists as a file or directory.'%newdirname)
            else:
                try:
                    os.mkdir(self.path + '/' + newdirname)
                except:
                    ErrorPopup(self.parent,
                               'Error: Could not create directory: "%s"'%newdirname)
                else:
                    self.newdirpopup.deactivate()
        else:
            self.newdirpopup.deactivate()


def ErrorPopup(parent, message):
    error = Pmw.MessageDialog(parent, title = 'Error',
                              message_text = message,
                              defaultbutton = 0,
                              )
    error.activate()

if __name__ == '__main__':

    rootWin = tkinter.Tk()

    Pmw.initialise()

    rootWin.title('Directory Browser Dialog Demo')

    def buildBrowser():
        # Create the hierarchical directory browser widget
        dirBrowserDialog = DirBrowserDialog(rootWin,
                                            #labelpos = 'nw',
                                            label = 'Select a directory',
                                            title = 'Directory Selector',
                                            #path = '~',
                                            #hidedotfiles = 0,
                                            )
        dir = dirBrowserDialog.activate()
        print('Selected Directory:', dir)

    dirButton = tkinter.Button(rootWin, text="Browser", command=buildBrowser)
    dirButton.pack(side = 'left', padx = 10, pady = 10)

    exitButton = tkinter.Button(rootWin, text="Quit", command=rootWin.quit)
    exitButton.pack(side = 'left', padx = 10, pady = 10)

    rootWin.mainloop()
