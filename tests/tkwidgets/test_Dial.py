import tkinter as tk
from direct.tkwidgets.Dial import Dial


def test_Dial():
    tl = tk.Toplevel()
    d = Dial(tl)
    d2 = Dial(tl, dial_numSegments = 12, max = 360,
              dial_fRollover = 0, value = 180)
    d3 = Dial(tl, dial_numSegments = 12, max = 90, min = -90,
              dial_fRollover = 0)
    d4 = Dial(tl, dial_numSegments = 16, max = 256,
              dial_fRollover = 0)
    d.pack(expand = 1, fill = tk.X)
    d2.pack(expand = 1, fill = tk.X)
    d3.pack(expand = 1, fill = tk.X)
    d4.pack(expand = 1, fill = tk.X)
