"""Undocumented Module"""

__all__ = ['ExcelHandler']

"""
A simple XML parser for Excel XML data. Built on top of xml.sax

Example use:
e=ExcelHandler()
parse('myData.xml', e)
print e.tables

"""

from xml.sax import saxutils
from xml.sax import parse

class ExcelHandler(saxutils.DefaultHandler):
    def __init__(self):
        self.chars=[]
        self.isNumber = 0
        self.cells=[]
        self.rows=[]
        self.tables=[]
        
    def characters(self, content):
        self.chars.append(content)
        
    def startElement(self, name, attrs):
        if name=="Data":
            if attrs.get('ss:Type') == "Number":
                self.isNumber = 1
            else:
                self.isNumber = 0
        elif name=="Cell":
            self.chars=[]
        elif name=="Row":
            self.cells=[]
        elif name=="Table":
            self.rows=[]
            
    def endElement(self, name):
        if name=="Data":
            pass
        elif name=="Cell":
            s = ''.join(self.chars)
            if s:
                if self.isNumber:
                    # Determine if it is an int or float and use
                    # return the best fit
                    floatVersion = float(s)
                    intVersion = int(floatVersion)
                    if floatVersion == intVersion:
                        # If the float is equal to the int, it must be an int
                        s = intVersion
                    else:
                        # Keep the precision and return a float
                        s = floatVersion
                # Convert the string "None" to the python object None
                elif s == "None":
                    s = None
                self.cells.append(s)
        elif name=="Row":
            self.rows.append(self.cells)
        elif name=="Table":
            self.tables.append(self.rows)

