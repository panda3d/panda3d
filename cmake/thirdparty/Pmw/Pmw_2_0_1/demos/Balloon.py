title = 'Pmw.Balloon demonstration'

# Import Pmw from this directory tree.
import sys
sys.path[:0] = ['../../..']

import tkinter
import Pmw

class Demo:
    def __init__(self, parent):
        # Create the Balloon.
        self.balloon = Pmw.Balloon(parent)

        # Create some widgets and megawidgets with balloon help.
        frame = tkinter.Frame(parent)
        frame.pack(padx = 10, pady = 5)
        field = Pmw.EntryField(frame,
                labelpos = 'nw',
                label_text = 'Command:')
        field.setentry('mycommand -name foo')
        field.pack(side = 'left', padx = 10)
        self.balloon.bind(field, 'Command to\nstart/stop',
                'Enter the shell command to control')

        start = tkinter.Button(frame, text='Start')
        start.pack(side='left', padx = 10)
        self.balloon.bind(start, 'Start the command')

        stop = tkinter.Button(frame, text='Stop')
        stop.pack(side='left', padx = 10)
        self.balloon.bind(stop, 'Stop the command')

        self.suicide = tkinter.Button(frame, text='Kill me soon!',
            command = self.killButton)
        self.suicide.pack(side='left', padx = 10)
        self.balloon.bind(self.suicide, 'Watch this button disappear!')

        scrolledCanvas = Pmw.ScrolledCanvas(parent,
                canvas_width = 300,
                canvas_height = 115,
        )
        scrolledCanvas.pack()
        canvas = scrolledCanvas.component('canvas')
        self.canvas = canvas

        # Create some canvas items and individual help.
        item = canvas.create_arc(5, 5, 35, 35, fill = 'red', extent = 315)
        self.balloon.tagbind(canvas, item, 'This is help for\nan arc item')
        item = canvas.create_bitmap(20, 150, bitmap = 'question')
        self.balloon.tagbind(canvas, item, 'This is help for\na bitmap')
        item = canvas.create_line(50, 60, 70, 80, 85, 20, width = 5)
        self.balloon.tagbind(canvas, item, 'This is help for\na line item')
        item = canvas.create_text(10, 90, text = 'Canvas items with balloons',
                anchor = 'nw', font = field.cget('entry_font'))
        self.balloon.tagbind(canvas, item, 'This is help for\na text item')

        # Create two canvas items which have the same tag and which use
        # the same help.
        canvas.create_rectangle(100, 10, 170, 50, fill = 'aliceblue',
                tags = 'TAG1')
        self.bluecircle = canvas.create_oval(110, 30, 160, 80, fill = 'blue',
                tags = 'TAG1')
        self.balloon.tagbind(canvas, 'TAG1',
                'This is help for the two blue items' + '\n' * 10 +
                    'It is very, very big.',
                'This is help for the two blue items')
        item = canvas.create_text(180, 10, text = 'Delete',
                anchor = 'nw', font = field.cget('entry_font'))
        self.balloon.tagbind(canvas, item,
                'After 2 seconds,\ndelete the blue circle')
        canvas.tag_bind(item, '<ButtonPress>', self._canvasButtonpress)
        scrolledCanvas.resizescrollregion()

        scrolledText = Pmw.ScrolledText(parent,
                text_width = 32,
                text_height = 4,
                text_wrap = 'none',
        )
        scrolledText.pack(pady = 5)
        text = scrolledText.component('text')
        self.text = text

        text.insert('end',
                'This is a text widget with ', '',
                ' balloon', 'TAG1',
                '\nhelp. Find the ', '',
                ' text ', 'TAG1',
                ' tagged with', '',
                ' help.', 'TAG2',
                '\n', '',
                'Remove tag 1.', 'TAG3',
                '\nAnother line.\nAnd another', '',
        )
        text.tag_configure('TAG1', borderwidth = 2, relief = 'sunken')
        text.tag_configure('TAG3', borderwidth = 2, relief = 'raised')

        self.balloon.tagbind(text, 'TAG1',
                'There is one secret\nballoon help.\nCan you find it?')
        self.balloon.tagbind(text, 'TAG2',
                'Well done!\nYou found it!')
        self.balloon.tagbind(text, 'TAG3',
                'After 2 seconds\ndelete the tag')
        text.tag_bind('TAG3', '<ButtonPress>', self._textButtonpress)

        frame = tkinter.Frame(parent)
        frame.pack(padx = 10)
        self.toggleBalloonVar = tkinter.IntVar()
        self.toggleBalloonVar.set(1)
        toggle = tkinter.Checkbutton(frame,
                variable = self.toggleBalloonVar,
                text = 'Balloon help', command = self.toggle)
        toggle.pack(side = 'left', padx = 10)
        self.balloon.bind(toggle, 'Toggle balloon help\non and off')

        self.toggleStatusVar = tkinter.IntVar()
        self.toggleStatusVar.set(1)
        toggle = tkinter.Checkbutton(frame,
                variable = self.toggleStatusVar,
                text = 'Status help', command = self.toggle)
        toggle.pack(side = 'left', padx = 10)
        self.balloon.bind(toggle,
                'Toggle status help on and off, on and off' + '\n' * 10 +
                    'It is very, very big, too.',
                'Toggle status help on and off')

        # Create and pack the MessageBar.
        messageBar = Pmw.MessageBar(parent,
                entry_width = 40,
                entry_relief='groove',
                labelpos = 'w',
                label_text = 'Status:')
        messageBar.pack(fill = 'x', expand = 1, padx = 10, pady = 5)

        # Configure the balloon to display its status messages in the
        # message bar.
        self.balloon.configure(statuscommand = messageBar.helpmessage)

    def toggle(self):
        if self.toggleBalloonVar.get():
            if self.toggleStatusVar.get():
                self.balloon.configure(state = 'both')
            else:
                self.balloon.configure(state = 'balloon')
        else:
            if self.toggleStatusVar.get():
                self.balloon.configure(state = 'status')
            else:
                self.balloon.configure(state = 'none')

    def killButton(self):
        # Test for old bug when destroying widgets 1) while the
        # balloon was up and 2) during the initwait period.
        print('Destroying button in 2 seconds')
        self.suicide.after(2000, self.suicide.destroy)

    def _canvasButtonpress(self, event):
        print('Destroying blue circle in 2 seconds')
        self.canvas.after(2000, self.deleteBlueCircle)

    def deleteBlueCircle(self):
        self.balloon.tagunbind(self.canvas, self.bluecircle)
        self.canvas.delete(self.bluecircle)

    def _textButtonpress(self, event):
        print('Deleting the text tag in 2 seconds')
        self.text.after(2000, self.deleteTextTag)

    def deleteTextTag(self):
        self.balloon.tagunbind(self.text, 'TAG1')
        self.text.tag_delete('TAG1')


######################################################################

# Create demo in root window for testing.
if __name__ == '__main__':
    root = tkinter.Tk()
    Pmw.initialise(root, 12, fontScheme = 'default')
    root.title(title)

    exitButton = tkinter.Button(root, text = 'Exit', command = root.destroy)
    exitButton.pack(side = 'bottom')
    widget = Demo(root)
    root.mainloop()
