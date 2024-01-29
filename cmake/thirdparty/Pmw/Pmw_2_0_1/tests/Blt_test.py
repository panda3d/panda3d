# Tests for Blt widgets.

import os
import tkinter
import Test
import Pmw

Test.initialise()
testData = ()

# Blt vector type

def _setVectorItem(index, value):
    w = Test.currentWidget()
    w[index] = value

def _getVectorItem(index):
    w = Test.currentWidget()
    return w[index]

def _getVectorSlice(index1, index2):
    w = Test.currentWidget()
    return w[index1:index2]

def _delVectorItem(index):
    w = Test.currentWidget()
    del w[index]

def _vectorExpr(instanceMethod):
    w = Test.currentWidget()
    name = '::' + str(w)
    if instanceMethod:
        w.expr(name + '+ 0.5')
    else:
        return Pmw.Blt.vector_expr(name + '* 2')

def _vectorNames():
    name = '::' + str(Test.currentWidget())
    names = Pmw.Blt.vector_names()
    if name not in names:
        return names
    name2 = Pmw.Blt.vector_names(name)
    if name2 != (name,):
        return name2
    return None

if Test.haveBlt():
    c = Pmw.Blt.Vector
    tests = (
      (c.set, [1, 2, 3, 4, 5, 6, 7, 8, 9, 10]),
      (c.__repr__, (), '[1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0]'),
      (c.set, ((1, 2, 3, 4, 5, 6, 7, 8, 9, 10),)),
      (c.__repr__, (), '[1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0]'),
      (c.__str__, (), 'PY_VEC4'),
      (_getVectorItem, 7, 8),
      (_getVectorSlice, (3, 6), [4.0, 5.0, 6.0]),
      (_delVectorItem, 9),
      (c.get, (), [1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0]),
      (c.append, 10),
      (c.get, (), [1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0]),
      (c.length, (), 10),
      (c.append, 5),
      (c.__len__, (), 11),
      (c.count, 5, 2),
      (c.count, 20, 0),
      (c.search, 5, (4, 10)),
      (c.search, 20),
      (c.index, 5, 4),
      # Commented tests do not work because of a bug in the blt vector command.
      # (c.clear, ()),
      (_getVectorItem, 4, 5),
      #(c.remove, 5),     # This causes a core in blt 2.4 under Solaris 2.5
      #(c.index, 5, 9),
      (c.min, (), 1.0),
      (c.max, (), 10.0),
      # (c.reverse, ()),
      # (c.reverse, ()),
      # (c.get, (), [1.0, 2.0, 3.0, 4.0, 6.0, 7.0, 8.0, 9.0, 10.0, 5.0]),
      # (c.insert, (3, 66)),
      # (c.search, 66, (3,)),
      (c.get, (), [1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 5.0]),
      (c.blt_sort, ()),
      (c.get, (), [1.0, 2.0, 3.0, 4.0, 5.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0]),
      (c.blt_sort_reverse, ()),
      (c.get, (), [10.0, 9.0, 8.0, 7.0, 6.0, 5.0, 5.0, 4.0, 3.0, 2.0, 1.0]),
      (_setVectorItem, (7, 77)),
      (c.search, 77, (7,)),
      (_setVectorItem, (2, 77)),
      (c.search, 77, (2, 7)),
      (_setVectorItem, (11, 77), 'TclError: can\'t set "PY_VEC4(11)": index "11" is out of range'),
      (c.get, (), [10.0, 9.0, 77.0, 7.0, 6.0, 5.0, 5.0, 77.0, 3.0, 2.0, 1.0]),
      (c.delete, (1, 3, 5)),
      (c.get, (), [10.0, 77.0, 6.0, 5.0, 77.0, 3.0, 2.0, 1.0]),
      (c.length, (), 8),
      (c.length, (9), 9),
      (c.get, (), [10.0, 77.0, 6.0, 5.0, 77.0, 3.0, 2.0, 1.0, 0.0]),
      (c.range, (1, 3), [77.0, 6.0, 5.0]),
      (_vectorExpr, 0, (20.0, 154.0, 12.0, 10.0, 154.0, 6.0, 4.0, 2.0, 0.0)),
      (_vectorExpr, 1),
      (c.get, (), [10.5, 77.5, 6.5, 5.5, 77.5, 3.5, 2.5, 1.5, 0.5]),
      (_vectorNames, ()),
    )
    testData = testData + ((c, ((tests, {}),)),)

    tests = (
      (c.get, (), [0.0, 0.0, 0.0, 0.0]),
      (c.length, (), 4),
    )
    testData = testData + ((c, ((tests, {'size' : 4}),)),)

    tests = (
      # (c.get, (), [0.0, 0.0, 0.0]),  Does not work.
      (c.length, (), 3),
      (_getVectorItem, 2, 0),
      (_getVectorItem, 4, 0),
      (_getVectorItem, 5, 'IndexError: 5'),
    )
    testData = testData + ((c, ((tests, {'size' : '2:4'}),)),)

#=============================================================================

# Blt graph widget

def _axisCommand(graph, value):
    return 'XX ' + value

def _createMarkerButton():
    w = Test.currentWidget()
    button = tkinter.Button(w, text = 'This is\na button')
    w.marker_create('window', coords=(10, 200), window=button)

def _axisNamesSorted(pattern = None):
    w = Test.currentWidget()
    if pattern is None:
        names = list(w.axis_names())
    else:
        names = list(w.axis_names(pattern))
    names.sort()
    return tuple(names)

def _penNamesSorted(pattern = None):
    w = Test.currentWidget()
    if pattern is None:
        names = list(w.pen_names())
    else:
        names = list(w.pen_names(pattern))
    names.sort()
    return tuple(names)

if Test.haveBlt():
    c = Pmw.Blt.Graph
    tests = (
      ('height', 700),
      ('width', 900),
      (c.pack, (), {'fill': 'both', 'expand': 1}),

      (Test.num_options, (), 43),
      (c.pen_create, 'pen1', {'fill': 'green', 'symbol': 'circle'}),
      (c.line_create, 'line1', {'xdata': (1, 2, 3, 4, 5, 6, 7, 8, 9, 10), 'ydata': (7, 2, 1, 4, 7, 3, 9, 3, 8, 5), 'pen': 'pen1',}),
      (c.bar_create, 'bar1', {'xdata': Test.vector_x, 'ydata': Test.vector_y[0], 'foreground': 'blue'}),
      (c.bar_create, 'bar2', {'xdata': Test.vector_x, 'ydata': Test.vector_y[1], 'foreground': 'magenta'}),
      (c.line_create, 'line2', {'xdata': Test.vector_x, 'ydata': Test.vector_y[2], 'color': 'red'}),

      (c.marker_create, 'text', {'coords': (25, 200), 'rotate': 45, 'text':
        'This is\na marker', 'name': 'myMarker1'}, 'myMarker1'),
      (c.marker_create, 'line', {'coords': (35, 120, 15, 280), 'linewidth': 4}, 'marker1'),
      (c.marker_create, 'polygon', {'coords': (35, 40, 45, 40, 45, 120, 35, 120), 'linewidth': 4}, 'marker2'),
      (c.marker_create, 'bitmap', {'coords': (25, 200), 'rotate': 45, 'bitmap': 'questhead'}, 'marker3'),
      (_createMarkerButton, ()),

      (c.marker_after, 'myMarker1'),
      (c.marker_before, ('myMarker1', 'marker3')),
      (c.marker_create, 'text', {'coords': (10, 10), 'text':
        'Bye', 'name': 'myMarker2'}, 'myMarker2'),
      (c.marker_names, 'my*', ('myMarker1', 'myMarker2')),
      (c.marker_exists, 'myMarker2', 1),
      (c.marker_delete, ('myMarker1', 'myMarker2', 'marker3')),
      (c.marker_exists, 'myMarker2', 0),
      (c.marker_names, (), ('marker1', 'marker2', 'marker4')),
      (c.marker_type, 'marker1', 'line'),

      (c.marker_cget, ('marker1', 'linewidth'), '4'),
      (c.marker_cget, ('marker2', 'linewidth'), '4'),
      (c.marker_configure, (('marker1', 'marker2'),), {'linewidth': 5}),
      (c.marker_cget, ('marker1', 'linewidth'), '5'),
      (c.marker_cget, ('marker2', 'linewidth'), '5'),

      ('background', '#ffdddd'),
      ('barmode', 'stacked'),
      ('barwidth', 0.5),
      ('borderwidth', 100),
      ('borderwidth', 10),
      ('barwidth', 0.9),
      ('bottommargin', 100),
      ('bufferelements', 1),
      ('cursor', 'watch'),
      ('font', Test.font['variable']),
      ('foreground', 'blue'),
      ('halo', 20),
      ('barmode', 'aligned'),
      ('invertxy', 1),
      ('justify', 'left'),
      ('leftmargin', 100),
      ('plotbackground', 'aquamarine'),
      ('plotborderwidth', 4),
      ('plotrelief', 'groove'),
      ('relief', 'ridge'),
      ('rightmargin', 100),
      ('takefocus', '0'),
      ('tile', Test.earthris),
      ('barmode', 'infront'),
      ('title', 'Hello there\nmy little lovely'),
      ('topmargin', 100),
      ('invertxy', 0),

      # Change colours so that axis and legend are visible against image tile.
      (c.xaxis_configure, (), {'color': 'green'}),
      (c.yaxis_configure, (), {'color': 'green'}),
      (c.legend_configure, (), {'background': '#ffffcc'}),

      (c.axis_cget, ('x', 'color'), 'green'),
      (c.axis_configure, ('x2'), {'color': 'red'}),
      (c.axis_cget, ('x2', 'color'), 'red'),
      (c.axis_create, 'myaxis', {'rotate': 45}),
      (c.axis_cget, ('myaxis', 'rotate'), '45.0'),
      (_axisNamesSorted, (), ('myaxis', 'x', 'x2', 'y', 'y2')),
      (_axisNamesSorted, ('*x*'), ('myaxis', 'x', 'x2')),
      # Blt 2.4u returns the empty string for the axis use command
      # (c.y2axis_use, 'myaxis', 'myaxis'),
      (c.axis_delete, 'myaxis'),

      (c.extents, 'leftmargin', 100),
      (c.inside, (1000, 1000), 0),
      (c.inside, (400, 400), 1),
      (c.snap, Test.emptyimage),

      (c.element_bind, ('line1', '<1>', Test.callback), Test.callback),
      (c.element_bind, 'line1', ('<Button-1>',)),
      (c.legend_bind, ('line1', '<1>', Test.callback), Test.callback),
      (c.legend_bind, 'line1', ('<Button-1>',)),
      (c.marker_bind, ('marker1', '<1>', Test.callback), Test.callback),
      (c.marker_bind, 'marker1', ('<Button-1>',)),

      (c.pen_create, 'mypen', {'type' : 'bar', 'foreground': 'red'}),
      (c.pen_cget, ('mypen', 'foreground'), 'red'),
      (c.pen_configure, ('mypen'), {'foreground': 'green'}),
      (c.pen_cget, ('mypen', 'foreground'), 'green'),
      (_penNamesSorted, (), ('activeBar', 'activeLine', 'mypen', 'pen1')),
      (_penNamesSorted, ('*pen*'), ('mypen', 'pen1')),
      (c.pen_delete, 'mypen'),

      # These tests are not portable
      # (c.invtransform, (0, 0), (-10.2518, 507.203)),
      # (c.transform, (-10.2518, 507.203), (0.0, 0.0)),

      # Reset margins to automatic
      ('bottommargin', 0),
      ('leftmargin', 0),
      ('rightmargin', 0),
      ('topmargin', 0),

      (c.crosshairs_configure, (), {'hide': 0}),
      (c.crosshairs_configure, (), {'position': '@300,300'}),
      (c.crosshairs_configure, (), {'color': 'seagreen4'}),
      (c.crosshairs_toggle, ()),
      (c.crosshairs_cget, 'hide', 1),
      (c.crosshairs_configure, (), {'dashes': (4, 8, 8, 8)}),
      (c.crosshairs_configure, (), {'linewidth': 4}),
      (c.crosshairs_toggle, ()),
      (c.crosshairs_cget, 'hide', 0),
      (c.crosshairs_off, ()),
      (c.crosshairs_cget, 'hide', 1),
      (c.crosshairs_on, ()),
      (c.crosshairs_cget, 'hide', 0),

      # Blt 2.4u gives an error with this (looks like activeBar
      # is same as activeLine):
      # (c.pen_configure, 'activeBar', {'foreground': '#ffffaa'}),

      (c.element_configure, 'bar2', {'foreground': '#ffffaa'}),
      # Blt 2.4u segmentation faults around here, remove tests:
      # (c.element_activate, 'bar1'),
      # (c.element_activate, 'bar2'),
      # (c.element_deactivate, ('bar1', 'bar2')),
      # (c.element_deactivate, ()),
      # (c.element_activate, ('bar2',) + tuple(range(Test.vectorSize / 2))),
      (c.element_configure, 'bar1', {'ydata': Test.vector_y[1]}),
      (c.element_configure, 'bar2', {'ydata': Test.vector_y[0]}),

      (c.element_cget, ('bar1', 'barwidth'), '0.0'),
      (c.element_cget, ('bar2', 'barwidth'), '0.0'),
      (c.element_configure, (('bar1', 'bar2'),), {'barwidth': 0.5}),
      (c.element_cget, ('bar1', 'barwidth'), '0.5'),
      (c.element_cget, ('bar2', 'barwidth'), '0.5'),

      # These tests are not portable
      # (c.element_closest, (330, 430), {}, {'x': 18.0, 'dist': 17.0, 'name': 'bar1', 'index': 18, 'y': 156.0}),
      # (c.element_closest, (0, 0)),
      # (c.element_closest, (0, 0), {'halo': 500}, {'x': 0.0, 'dist': 154.797, 'name': 'line2', 'index': 0, 'y': 359.0}),
      # (c.element_closest, (0, 0), {'halo': 500, 'interpolate': 1}, {'x': -0.0320109, 'dist': 154.797, 'name': 'line2', 'index': 0, 'y': 358.879}),

      (c.element_type, 'bar2', 'bar'),
      (c.element_type, 'line2', 'line'),

      (c.legend_activate, ('line1', 'bar2',)),
      (c.legend_activate, ()),
      (c.legend_deactivate, ('line1', 'bar2',)),
      (c.legend_deactivate, ()),
      (c.legend_configure, (), {'hide': 1}),
      (c.legend_cget, 'hide', 1),
      (c.legend_configure, (), {'hide': 0}),
      (c.legend_configure, (), {'position': 'left', 'anchor': 'nw', 'ipadx': 100, 'ipady': 100}),
      (c.legend_get, '@150,150', 'line1'),

      # This probably works, but I haven't installed the prologue file
      # (c.postscript_output, '/tmp/tmp.ps', {'landscape': 1}),
      # (os.unlink, '/tmp/tmp.ps'),

      (c.element_show, (), ('line2', 'bar2', 'bar1', 'line1')),
      (c.element_show, (('line1', 'bar1'),), ('line1', 'bar1')),
      (c.element_names, (), ('line1', 'line2', 'bar1', 'bar2')),
      (c.element_names, 'line*', ('line1', 'line2')),
      (c.element_show, (('line1', 'line2', 'bar1', 'bar2'),), ('line1', 'line2', 'bar1', 'bar2')),
      (c.element_exists, 'bar1', 1),
      (c.element_delete, ('bar1', 'bar2')),
      (c.element_names, (), ('line1', 'line2')),
      (c.element_exists, 'bar1', 0),
      (c.element_delete, ()),
      (c.element_names, (), ('line1', 'line2')),

      (c.grid_configure, (), {'hide': 0}),
      (c.grid_toggle, ()),
      (c.grid_cget, 'hide', 1),
      (c.grid_on, ()),
      (c.grid_cget, 'hide', 0),
      (c.grid_off, ()),

      # These tests are not portable
      # (c.xaxis_invtransform, 0, -37.1153),
      # (c.axis_limits, 'x', (-0.98, 49.98)),
      # (c.axis_transform, ('x', 0), 360),
      # (c.axis_invtransform, ('y', 0), 444.198),
      # (c.yaxis_limits, (), (-6.96, 406.96)),
      # (c.axis_transform, ('y', 0), 620),
      # (c.axis_invtransform, ('x2', 0), -25.1491),
      # (c.axis_limits, 'x2', (-10.4, 10.4)),
      # (c.x2axis_transform, 0, 598),
      # (c.y2axis_invtransform, 0, 12.2713),
      # (c.axis_limits, 'y2', (-10.4, 10.4)),
      # (c.axis_transform, ('y2', 0), 341),
    )

    testData = testData + ((c, ((tests, {}),)),)

if __name__ == '__main__':
    #Test.enableBlt()
    Test.runTests(testData)
