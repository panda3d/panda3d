title = 'Pmw.ScrolledText demonstration'

# Import Pmw from this directory tree.
import sys
sys.path[:0] = ['../../..']

import os
import math
import string
import tkinter
import Pmw

class Demo:
    def __init__(self, parent):

        # Create the ScrolledText with headers.
        fixedFont = Pmw.logicalfont('Fixed')
        self.st = Pmw.ScrolledText(parent,
                # borderframe = 1,
                labelpos = 'n',
                label_text='ScrolledText with headers',
                columnheader = 1,
                rowheader = 1,
                rowcolumnheader = 1,
                usehullsize = 1,
                hull_width = 400,
                hull_height = 300,
                text_wrap='none',
                text_font = fixedFont,
                Header_font = fixedFont,
                Header_foreground = 'blue',
                rowheader_width = 3,
                rowcolumnheader_width = 3,
                text_padx = 4,
                text_pady = 4,
                Header_padx = 4,
                rowheader_pady = 4,
        )

        self.st.pack(padx = 5, pady = 5, fill = 'both', expand = 1)

        funcs = 'atan cos cosh exp log log10 sin sinh sqrt tan tanh'
        funcs = funcs.split()

        # Create the header for the row headers
        self.st.component('rowcolumnheader').insert('end', 'x')

        # Create the column headers
        headerLine = ''
        for column in range(len(funcs)):
            headerLine = headerLine + ('%-7s   ' % (funcs[column],))
        headerLine = headerLine[:-3]
        self.st.component('columnheader').insert('0.0', headerLine)

        self.st.tag_configure('yellow', background = 'yellow')

        # Create the data rows and the row headers
        numRows = 50
        tagList = []
        for row in range(1, numRows):
            dataLine = ''
            x = row / 5.0
            for column in range(len(funcs)):
                value = eval('math.' + funcs[column] + '(' + str(x) + ')')
                data = str(value)[:7]
                if value < 0:
                    tag1 = '%d.%d' % (row, len(dataLine))
                    tag2 = '%d.%d' % (row, len(dataLine) + len(data))
                    tagList.append(tag1)
                    tagList.append(tag2)
                data = '%-7s' % (data,)
                dataLine = dataLine + data + '   '
            dataLine = dataLine[:-3]
            header = '%.1f' % (x,)
            if row < numRows - 1:
                dataLine = dataLine + '\n'
                header = header + '\n'
            self.st.insert('end', dataLine)
            self.st.component('rowheader').insert('end', header)
        self.st.tag_add(*('yellow',) + tuple(tagList))

        # Prevent users' modifying text and headers
        self.st.configure(
            text_state = 'disabled',
            Header_state = 'disabled',
        )

######################################################################

# Create demo in root window for testing.
if __name__ == '__main__':
    root = tkinter.Tk()
    Pmw.initialise(root)
    root.title(title)

    exitButton = tkinter.Button(root, text = 'Exit', command = root.destroy)
    exitButton.pack(side = 'bottom')
    widget = Demo(root)
    root.mainloop()
