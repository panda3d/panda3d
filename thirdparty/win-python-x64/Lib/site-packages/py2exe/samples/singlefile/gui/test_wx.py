import sys
import wx

# By default the executables created by py2exe write output to stderr
# into a logfile and display a messagebox at program end when there
# has been any output.  This logger silently overrides the default
# behaviour by silently swallowing everything.

if hasattr(sys, "frozen"):
    class logger:
        def write(self, text):
            pass # silently ignore everything

    sys.stdout = sys.stderr = logger()

class MyFrame(wx.Frame):
    def __init__(self, parent, ID, title, pos=wx.DefaultPosition,
                 size=(200, 200), style=wx.DEFAULT_FRAME_STYLE):
        wx.Frame.__init__(self, parent, ID, title, pos, size, style)
        panel = wx.Panel(self, -1)

        button = wx.Button(panel, 1003, "Close Me")
        button.SetPosition(wx.Point(15, 15))
        wx.EVT_BUTTON(self, 1003, self.OnCloseMe)
        wx.EVT_CLOSE(self, self.OnCloseWindow)

        button = wx.Button(panel, 1004, "Press Me")
        button.SetPosition(wx.Point(15, 45))
        wx.EVT_BUTTON(self, 1004, self.OnPressMe)

    def OnCloseMe(self, event):
        self.Close(True)

    def OnPressMe(self, event):
        # This raises an exception
        x = 1 / 0

    def OnCloseWindow(self, event):
        self.Destroy()

class MyApp(wx.App):
    def OnInit(self):
        frame = MyFrame(None, -1, "Hello from wxPython")
        frame.Show(True)
        self.SetTopWindow(frame)
        return True

if __name__ == "__main__":
    app = MyApp(0)
    app.MainLoop()
