#----------------------------------------------------------------------
# Name:        wx.lib.graphics
# Purpose:     A wx.GraphicsContext-like API implemented using wx.lib.wxcairo
#
# Author:      Robin Dunn
#
# Created:     15-Sept-2008
# RCS-ID:      $Id: graphics.py 63527 2010-02-19 22:40:57Z RD $
# Copyright:   (c) 2008 by Total Control Software
# Licence:     wxWindows license
#----------------------------------------------------------------------

"""
This module implements an API similar to wx.GraphicsContext and the
related classes.  In this case the implementation for all platforms is
done using Cairo, via the wx.lib.wxcairo glue module.

Why do this?  Why not just use wx.GraphicsContext everywhere?  Using
Cairo on every platform enables us to more easily be totally
consistent on all platforms.  Implementing it in Python means that it
is easy to fill in the gaps in functionality with features of Cairo
that GraphicsContext may not provide, like converting text to a path,
using compositing operators, or being able to provide an
implementation for things like context.Clear().

Why not just use Cairo directly?  There may be times when you do want
to use wx.GrpahicsContext, so being able to share code between that
and this implementation is nice.  Also, I like the class hierarchy and
API exposed by the wx.GraphicsContext classes a little better than
Cairo's.
"""

import cairo
import math

import wx
import wx.lib.wxcairo



# Other ideas:
# 1. TextToPath (or maybe make this part of the Path class
# 2. Be able to add additional color stops to gradients.  Or maybe a GraphicsGradient or GraphicsPattern class
# 3. Relative moves, lines, curves, etc.
# 5. maybe expose cairo_paint, cairo_paint_with_alpha, cairo_mask?

#---------------------------------------------------------------------------
# Image surface formats

FORMAT_ARGB32 = cairo.FORMAT_ARGB32
FORMAT_RGB24  = cairo.FORMAT_RGB24
FORMAT_A8     = cairo.FORMAT_A8
FORMAT_A1     = cairo.FORMAT_A1


#---------------------------------------------------------------------------
# Compositing operators.  See http://cairographics.org/operators

# clear destination layer (bounded)
OPERATOR_CLEAR = cairo.OPERATOR_CLEAR         

# replace destination layer (bounded)
OPERATOR_SOURCE = cairo.OPERATOR_SOURCE       

# draw source layer on top of destination layer (bounded)
OPERATOR_OVER = cairo.OPERATOR_OVER           

# draw source where there was destination content (unbounded)
OPERATOR_IN = cairo.OPERATOR_IN               

# draw source where there was no destination content (unbounded)
OPERATOR_OUT = cairo.OPERATOR_OUT             

# draw source on top of destination content and only there
OPERATOR_ATOP = cairo.OPERATOR_ATOP           

# ignore the source
OPERATOR_DEST = cairo.OPERATOR_DEST           

# draw destination on top of source
OPERATOR_DEST_OVER = cairo.OPERATOR_DEST_OVER 

# leave destination only where there was source content (unbounded)
OPERATOR_DEST_IN = cairo.OPERATOR_DEST_IN     

# leave destination only where there was no source content
OPERATOR_DEST_OUT = cairo.OPERATOR_DEST_OUT   

# leave destination on top of source content and only there (unbounded)
OPERATOR_DEST_ATOP = cairo.OPERATOR_DEST_ATOP 

# source and destination are shown where there is only one of them
OPERATOR_XOR = cairo.OPERATOR_XOR             

# source and destination layers are accumulated
OPERATOR_ADD = cairo.OPERATOR_ADD             

# like over, but assuming source and dest are disjoint geometries
OPERATOR_SATURATE = cairo.OPERATOR_SATURATE   



#---------------------------------------------------------------------------
# Anti-alias modes.  Note that according to the Cairo docs none of the
# current backends support the the SUBPIXEL mode.

# Use the default antialiasing for the subsystem and target device
ANTIALIAS_DEFAULT = cairo.ANTIALIAS_DEFAULT  

# Use a bilevel alpha mask
ANTIALIAS_NONE = cairo.ANTIALIAS_NONE  

# Perform single-color antialiasing (using shades of gray for black
# text on a white background, for example).
ANTIALIAS_GRAY = cairo.ANTIALIAS_GRAY  

# Perform antialiasing by taking advantage of the order of subpixel
# elements on devices such as LCD panels
ANTIALIAS_SUBPIXEL = cairo.ANTIALIAS_SUBPIXEL  



#---------------------------------------------------------------------------
# A decorator that makes creating properties a little cleaner and simpler

def Property( function ):
    return property( **function() )

#---------------------------------------------------------------------------

NullGraphicsPen = None
NullGraphicsBrush = None
NullGraphicsFont = None
NullGraphicsMatrix = None
NullGraphicsPath = None


class GraphicsObject(object):
    # This probably isn't needed at all anymore since we'll just use
    # None insead of the Null objects, but we'll keep it anyway in
    # case it's needed to help write compatible code.
    def IsNull(self):
        return False


#---------------------------------------------------------------------------

class GraphicsPen(GraphicsObject):
    """
    A Pen is used to define the properties of how a stroke is drawn.
    """
    _capMap = { wx.CAP_BUTT       : cairo.LINE_CAP_BUTT,
                wx.CAP_ROUND      : cairo.LINE_CAP_ROUND,
                wx.CAP_PROJECTING : cairo.LINE_CAP_SQUARE }
    
    _joinMap = { wx.JOIN_BEVEL : cairo.LINE_JOIN_BEVEL,
                 wx.JOIN_MITER : cairo.LINE_JOIN_MITER,
                 wx.JOIN_ROUND : cairo.LINE_JOIN_ROUND }
        
    
    def __init__(self, colour=wx.BLACK, width=1, style=wx.SOLID):
        GraphicsObject.__init__(self)
        self._colour = _makeColour(colour)
        self._width = width
        self._style = style
        self._cap = wx.CAP_ROUND
        self._dashes = []
        self._join = wx.JOIN_ROUND
        self._stipple = None
        self._pattern = None
        

    @staticmethod
    def CreateFromPen(pen):
        """Convert a wx.Pen to a GraphicsPen"""
        assert isinstance(pen, wx.Pen)
        p = GraphicsPen(pen.Colour, pen.Width, pen.Style)
        p._cap = pen.Cap
        p._dashes = pen.Dashes
        p._join = pen.Join
        return p


    @staticmethod
    def CreateFromPattern(pattern, width=1):
        """
        Create a Pen directly from a Cairo Pattern object.  This is
        similar to using a stipple bitmap, but saves a step, and
        patterns can include gradients, etc.
        """
        p = GraphicsPen(wx.BLACK, width, wx.STIPPLE)
        p._pattern = pattern
        return p
    
        
    @Property
    def Colour():
        def fget(self):
            return self._colour
        def fset(self, value):
            self._colour = value
        return locals()

    @Property
    def Width():
        def fget(self):
            return self._width
        def fset(self, value):
            self._width = value
        return locals()

    @Property
    def Style():
        def fget(self):
            return self._style
        def fset(self, value):
            self._style = value
        return locals()

    @Property
    def Cap():
        def fget(self):
            return self._cap
        def fset(self, value):
            self._cap = value
        return locals()

    @Property
    def Dashes():
        def fget(self):
            return self._dashes
        def fset(self, value):
            self._dashes = value
        return locals()

    @Property
    def Join():
        def fget(self):
            return self._join
        def fset(self, value):
            self._join = value
        return locals()

    @Property
    def Stipple():
        def fget(self):
            return self._stipple
        def fset(self, value):
            self._stipple = value
            self._pattern = None
        return locals()

    @Property
    def Pattern():
        def fget(self):
            return self._pattern
        def fset(self, value):
            self._pattern = value
        return locals()


    
    def Apply(self, ctx):
        # set up the context with this pen's parameters
        ctx = ctx.GetNativeContext()
        ctx.set_line_width(self._width)
        ctx.set_line_cap(self._capMap[self._cap])
        ctx.set_line_join(self._joinMap[self._join])
        ctx.set_dash([])
        
        if self._style == wx.SOLID:
            ctx.set_source_rgba( *_colourToValues(self._colour) )
            
        elif self._style == wx.STIPPLE:
            if not self._pattern and self._stipple:
                # make a pattern from the stipple bitmap
                img = wx.lib.wxcairo.ImageSurfaceFromBitmap(self._stipple)
                self._pattern = cairo.SurfacePattern(img)
                self._pattern.set_extend(cairo.EXTEND_REPEAT)
            ctx.set_source(self._pattern)
        
        elif self._style == wx.USER_DASH:
            ctx.set_source_rgba( *_colourToValues(self._colour) )
            ctx.set_dash(self._dashes)
            
        elif self._style in [wx.DOT, wx.DOT_DASH, wx.LONG_DASH, wx.SHORT_DASH]:
            ctx.set_source_rgba( *_colourToValues(self._colour) )
            ctx.set_dash( _stdDashes(self._style, self._width) )
        
        elif self._style in [wx.BDIAGONAL_HATCH, wx.CROSSDIAG_HATCH, wx.FDIAGONAL_HATCH,
                             wx.CROSS_HATCH, wx.HORIZONTAL_HATCH, wx.VERTICAL_HATCH]:
            pass  # TODO  make a stock pattern...
        
    
#---------------------------------------------------------------------------

class GraphicsBrush(GraphicsObject):
    """
    A Brush is used to define how fills are painted.  They can have
    either a solid fill (colors with or without alpha), a stipple
    created from a wx.Bitmap, or a cairo Pattern object.
    """
    
    def __init__(self, colour=wx.BLACK, style=wx.SOLID):
        self._colour = _makeColour(colour)
        self._style = style
        self._stipple = None
        self._pattern = None
        

    @staticmethod
    def CreateFromBrush(brush):
        """Converts a wx.Brush to a GraphicsBrush"""
        assert isinstance(brush, wx.Brush)
        b = GraphicsBrush(brush.Colour, brush.Style)
        if brush.Style == wx.STIPPLE:
            b._stipple = brush.Stipple
        else:
            b._stipple = None
        return b


    @staticmethod
    def CreateFromPattern(pattern):
        """
        Create a Brush directly from a Cairo Pattern object.  This is
        similar to using a stipple bitmap, but saves a step, and
        patterns can include gradients, etc.
        """
        b = GraphicsBrush(style=wx.STIPPLE)
        b._pattern = pattern
        return b


    @Property
    def Colour():
        def fget(self):
            return self._colour
        def fset(self, value):
            self._colour = value
        return locals()

    @Property
    def Style():
        def fget(self):
            return self._style
        def fset(self, value):
            self._style = value
        return locals()

    @Property
    def Stipple():
        def fget(self):
            return self._stipple
        def fset(self, value):
            self._stipple = value
            self._pattern = None
        return locals()


    @Property
    def Pattern():
        def fget(self):
            return self._pattern
        def fset(self, value):
            self._pattern = value
        return locals()


    def Apply(self, ctx):
        ctx = ctx.GetNativeContext()
        
        if self._style == wx.SOLID:
            ctx.set_source_rgba( *_colourToValues(self._colour) )

        elif self._style == wx.STIPPLE:
            if not self._pattern and self._stipple:
                # make a pattern from the stipple bitmap
                img = wx.lib.wxcairo.ImageSurfaceFromBitmap(self._stipple)
                self._pattern = cairo.SurfacePattern(img)
                self._pattern.set_extend(cairo.EXTEND_REPEAT)
            ctx.set_source(self._pattern)
            
#---------------------------------------------------------------------------

class GraphicsFont(GraphicsObject):
    """
    """
    def __init__(self):
        # TODO: Should we be able to create a GrpahicsFont from other
        # properties, or will it always be via a wx.Font?  What about
        # creating from a cairo.FontFace or cairo.ScaledFont?
        self._font = None
        self._colour = None
        self._pointSize = None
        self._fontface = None
        # To remain consistent with the GC API a color is associated
        # with the font, and nothing else.  Since this is Cairo and
        # it's easy to do, we'll also allow a brush to be used...
        self._brush = None


    def IsNull(self):
        return self._font is None
    

    @staticmethod
    def CreateFromFont(font, colour=None):
        f = GraphicsFont()
        f._font = font
        f._colour = _makeColour(colour)
        f._pointSize = font.GetPointSize()
        f._fontface = wx.lib.wxcairo.FontFaceFromFont(font)
        return f


    @Property
    def Colour():
        def fget(self):
            return self._colour
        def fset(self, value):
            self._colour = value
        return locals()

    @Property
    def PointSize():
        def fget(self):
            return self._pointSize
        def fset(self, value):
            self._pointSize = value
        return locals()

    @Property
    def Brush():
        def fget(self):
            return self._brush
        def fset(self, value):
            self._brush = value
        return locals()


    def Apply(self, ctx, colour):
        nctx = ctx.GetNativeContext()
        if self._brush is not None:
            self._brush.Apply(ctx)
        else:
            if colour is None: colour = wx.BLACK
            nctx.set_source_rgba( *_colourToValues(colour) )
        nctx.set_font_face(self._fontface)
        nctx.set_font_size(self._pointSize)

        
#---------------------------------------------------------------------------

class GraphicsBitmap(GraphicsObject):
    """
    A GraphicsBitmap is a wrapper around a cairo ImageSurface.  It can
    be used as a source for drawing images, or as a target of drawing
    operations.
    """
    def __init__(self, width=-1, height=-1, format=FORMAT_ARGB32):
        """Create either a NULL GraphicsBitmap or an empty one if a size is given"""
        self._surface = None
        if width > 0 and height > 0:
            self._surface = cairo.ImageSurface(format, width, height)


    def IsNull(self):
        return self._surface is None

    
    @staticmethod
    def CreateFromBitmap(bitmap):
        """Create a GraphicsBitmap from a wx.Bitmap"""
        b = GraphicsBitmap()
        b._surface = wx.lib.wxcairo.ImageSurfaceFromBitmap(bitmap)
        return b


    @staticmethod
    def CreateFromPNG(filename):
        """Create a GraphicsBitmap from a PNG file"""
        b = GraphicsBitmap()
        b._surface = cairo.ImageSurface.create_from_png(filename)
        return b


    @staticmethod
    def CreateFromSurface(surface):
        """Use an existing cairo ImageSurface as a GraphicsBitmap"""
        b = GraphicsBitmap()
        b._surface = surface
        return b


    @staticmethod
    def CreateFromBuffer(buffer, width, height,
                         format=FORMAT_ARGB32, stride=-1):
        """
        Creates a GraphicsBitmap that uses the given buffer object as
        the pixel storage.  This means that the current contents of
        the buffer will be the initial state of the bitmap, and
        anything drawn to this surface will be stored in the given
        buffer.
        """
        b = GraphicsBitmap()
        if stride == -1:
            try:
                stride = cairo.ImageSurface.format_stride_for_width(format, width)
            except AttributeError:
                stride = width * 4
        b._surface = cairo.ImageSurface.create_for_data(
            buffer, format, width, height, stride)

        # save a reference to the buffer to ensure that it lives as
        # long as this object does
        b._buffer = buffer
        return b
        

    @Property
    def Width():
        def fget(self):
            return self._surface.get_width()
        return locals()

    @Property
    def Height():
        def fget(self):
            return self._surface.get_height()
        return locals()

    @Property
    def Size():
        def fget(self):
            return (self.Width, self.Height)
        return locals()


    @Property
    def Format():
        def fget(self):
            return self._surface.get_format()
        return locals()

    @Property
    def Stride():
        def fget(self):
            return self._surface.get_stride()
        return locals()

    @Property
    def Surface():
        def fget(self):
            return self._surface
        return locals()

    
#---------------------------------------------------------------------------

class GraphicsMatrix(GraphicsObject):
    """
    A matrix holds an affine transformations, such as a scale,
    rotation, shear, or a combination of these, and is used to convert
    between different coordinante spaces.
    """
    def __init__(self):
        self._matrix = cairo.Matrix()


    def Set(self, a=1.0, b=0.0, c=0.0, d=1.0, tx=0.0, ty=0.0):
        """Set the componenets of the matrix by value, default values
        are the identity matrix."""
        self._matrix = cairo.Matrix(a, b, c, d, tx, ty)


    def Get(self):
        """Return the component values of the matrix as a tuple."""
        return tuple(self._matrix)


    def GetNativeMatrix(self):
        return self._matrix


    def Concat(self, matrix):
        """Concatenates the matrix passed with the current matrix."""
        self._matrix = self._matrix * matrix._matrix
        return self


    def Invert(self):
        """Inverts the matrix."""
        self._matrix.invert()
        return self
    

    def IsEqual(self, matrix):
        """Returns True if the elements of the transformation matricies are equal."""
        return self._matrix == matrix._matrix
    

    def IsIdentity():
        """Returns True if this is the identity matrix."""
        return self._matrix == cairo.Matrix()


    def Rotate(self, angle):
        """Rotates the matrix in radians"""
        self._matrix.rotate(angle)
        return self


    def Scale(self, xScale, yScale):
        """Scale the matrix"""
        self._matrix.scale(xScale, yScale)
        return self


    def Translate(self, dx, dy):
        """Translate the metrix.  This shifts the origin."""
        self._matrix.translate(dx, dy)
        return self


    def TransformPoint(self, x, y):
        """Applies this matrix to a point and returns the result"""
        return self._matrix.transform_point(x, y)


    def TransformDistance(self, dx, dy):
        """
        Applies this matrix to a distance (ie. performs all transforms
        except translations.)
        """
        return self._matrix.transform_distance(dx, dy)


    def Clone(self):
        m = GraphicsMatrix()
        m.Set(*self.Get())
        return m
    
#---------------------------------------------------------------------------

class GraphicsPath(GraphicsObject):
    """
    A GraphicsPath is a representaion of a geometric path, essentially
    a collection of lines and curves.  Paths can be used to define
    areas to be stroked and filled on a GraphicsContext.
    """
    def __init__(self):
        # A path is essentially just a context that we use just for
        # collecting path moves, lines, and curves in order to apply
        # them to the real context.  So we'll use a 1x1 image surface
        # for the backend, since we won't ever actually use it for
        # rendering in this context.
        surface = cairo.ImageSurface(FORMAT_ARGB32, 1, 1)
        self._pathContext = cairo.Context(surface)


    def AddArc(self, x, y, radius, startAngle, endAngle, clockwise=True):
        """
        Adds an arc of a circle centering at (x,y) with radius, from
        startAngle to endAngle.
        """
        # clockwise means positive in our system (y pointing downwards) 
        if clockwise or endAngle-startAngle >= 2*math.pi:
            self._pathContext.arc(x, y, radius, startAngle, endAngle)
        else:
            self._pathContext.arc_negative(x, y, radius, startAngle, endAngle)
        return self


    def AddArcToPoint(self, x1, y1 , x2, y2, radius ):
        """
        Adds a an arc to two tangents connecting (current) to (x1,y1)
        and (x1,y1) to (x2,y2), also a straight line from (current) to
        (x1,y1)
        """
        current = wx.Point2D(*self.GetCurrentPoint())
        p1 = wx.Point2D(x1, y1)
        p2 = wx.Point2D(x2, y2)

        v1 = current - p1
        v1.Normalize()
        v2 = p2 - p1
        v2.Normalize()

        alpha = v1.GetVectorAngle() - v2.GetVectorAngle()
        if alpha < 0:
            alpha = 360 + alpha
        alpha = math.radians(alpha)

        dist = radius / math.sin(alpha/2) * math.cos(alpha/2)
        
        # calculate tangential points
        t1 = (v1 * dist) + p1
        t2 = (v2 * dist) + p1

        nv1 = wx.Point2D(*v1.Get())
        nv1.SetVectorAngle(v1.GetVectorAngle() - 90)
        c = t1 + nv1 * radius

        a1 = v1.GetVectorAngle() + 90
        a2 = v2.GetVectorAngle() - 90

        self.AddLineToPoint(t1.x, t1.y)
        self.AddArc(c.x, c.y, radius, math.radians(a1), math.radians(a2), True)
        self.AddLineToPoint(p2.x, p2.y)
        return self
        

    def AddCircle(self, x, y, radius):
        """
        Appends a new closed sub-path as a circle around (x,y).
        """
        self.MoveToPoint(x + radius, y)
        self.AddArc( x, y, radius, 0, 2*math.pi, False)
        self.CloseSubpath()
        return self


    def AddCurveToPoint(self, cx1, cy1, cx2, cy2, x, y):
        """
        Adds a cubic Bezier curve from the current point, using two
        control points and an end point.
        """
        self._pathContext.curve_to(cx1, cy1, cx2, cy2, x, y)
        return self


    def AddEllipse(self, x, y, w, h):
        """
        Appends an elipse fitting into the given rectangle as a closed sub-path.
        """
        rw = w / 2.0
        rh = h / 2.0
        xc = x + rw
        yc = y + rh
        m = GraphicsMatrix()
        m.Translate(xc, yc)
        m.Scale(rw / rh, 1.0)
        p = GraphicsPath()
        p.AddCircle(0,0, rh)
        p.Transform(m)
        self.AddPath(p)
        return self
        

    def AddLineToPoint(self, x, y):
        """
        Adds a straight line from the current point to (x,y)
        """
        self._pathContext.line_to(x, y)
        return self


    def AddPath(self, path):
        """
        Appends the given path to this path.
        """
        self._pathContext.append_path(path.GetNativePath())
        return self


    def AddQuadCurveToPoint(self, cx, cy, x, y):
        """
        Adds a quadratic Bexier curve from the current point, using a
        control point and an end point.
        """
        # calculate using degree elevation to a cubic bezier
        start = wx.Point2D()
        start.x, start.y = self.GetCurrentPoint()
        end = wx.Point2D(x, y)
        c = wx.Point2D(cx, cy)
        c1 = start * (1/3.0) + c * (2/3.0)
        c2 = c * (2/3.0) + end * (1/3.0)
        self.AddCurveToPoint(c1.x, c1.y, c2.x, c2.y, x, y);
        return self
        

    def AddRectangle(self, x, y, w, h):
        """
        Adds a new rectanlge as a closed sub-path.
        """
        self._pathContext.rectangle(x, y, w, h)
        return self


    def AddRoundedRectangle(self, x, y, w, h, radius):
        """
        Adds a new rounded rectanlge as a closed sub-path.
        """
        if radius == 0:
            self.AddRectangle(x,y,w,h)
        else:
            self.MoveToPoint( x + w, y + h / 2.0)
            self.AddArcToPoint(x + w, y + h, x + w / 2.0, y + h, radius)
            self.AddArcToPoint(x, y + h, x, y + h / 2.0, radius)
            self.AddArcToPoint(x, y , x + w / 2.0, y, radius)
            self.AddArcToPoint(x + w, y, x + w, y + h / 2.0, radius)
            self.CloseSubpath()
        return self
            

    def CloseSubpath(self):
        """
        Adds a line segment to the path from the current point to the
        beginning of the current sub-path, and closes this sub-path.
        """
        self._pathContext.close_path()
        return self


    def Contains(self, x, y, fillStyle=wx.ODDEVEN_RULE):
        """
        Returns True if the point lies within the path.
        """
        d = { wx.WINDING_RULE : cairo.FILL_RULE_WINDING,
              wx.ODDEVEN_RULE : cairo.FILL_RULE_EVEN_ODD }
        rule = d[fillStyle]
        self._pathContext.set_fill_rule(rule)
        return self._pathContext.in_stroke(x,y) or self._pathContext.in_fill(x,y)
        

    def GetCurrentPoint(self):
        """
        Gets the current point of the path, which is conceptually the
        final point reached by the last path operation.
        """
        return self._pathContext.get_current_point()


    def GetNativePath(self):
        """
        Returns the path as a cairo.Path object.
        """
        return self._pathContext.copy_path()


    def MoveToPoint(self, x, y):
        """
        Begins a new sub-path at (x,y) by moving the "current point" there.
        """
        self._pathContext.move_to(x, y)
        return self


    def Transform(self, matrix):
        """
        Transforms each point in this path by the matirx
        """
        # as we don't have a true path object, we have to apply the
        # inverse matrix to the context
        # TODO: should we clone the matrix before inverting it?
        m = matrix.GetNativeMatrix()
        m.invert()
        self._pathContext.transform(m)
        return self


    def Clone(self):
        """
        Return a new path initialized with the current contents of this path.
        """
        p = GraphicsPath()
        p.AddPath(self)
        return p


    def GetBox(self):
        """
        Return the bounding box enclosing all points on this path.
        """
        x1,y1,x2,y2 = self._pathContext.stroke_extents()
        if x2 < x1:
            x = x2
            w = x1 - x2
        else:
            x = x1
            w = x2 - x1

        if y2 < y1:
            y = y2
            h = y1 - y2
        else:
            y = y1
            h = y2 - y1
        return (x, y, w, h)

    
#---------------------------------------------------------------------------

class GraphicsContext(GraphicsObject):
    """
    The GraphicsContext is the object which facilitates drawing to a surface.
    """
    def __init__(self, context=None):
        self._context = context
        self._pen = None
        self._brush = None
        self._font = None
        self._fontColour = None
        

    def IsNull(self):
        return self._context is None

    
    @staticmethod
    def Create(dc):
        # TODO:  Support creating directly from a wx.Window too.
        assert isinstance(dc, wx.DC)
        ctx = wx.lib.wxcairo.ContextFromDC(dc)
        return GraphicsContext(ctx)

    @staticmethod
    def CreateFromNative(cairoContext):
        return GraphicsContext(cairoContext)

    @staticmethod
    def CreateMeasuringContext():
        """
        If you need a temporary context just to quickly measure some
        text extents, or etc. then using this function will be a
        little less expensive than creating a real DC for it.
        """
        surface = cairo.ImageSurface(FORMAT_ARGB32, 1, 1)
        ctx = cairo.Context(surface)
        return GraphicsContext(ctx)

    @staticmethod
    def CreateFromSurface(surface):
        """
        Wrap a context around the given cairo Surface.  Note that a
        GraphicsBitmap contains a cairo ImageSurface which is
        accessible via the Surface property.        
        """
        return GraphicsContext(cairo.Context(surface))


    @Property
    def Context():
        def fget(self):
            return self._context
        return locals()


    # Our implementation is able to create these things direclty, but
    # we'll keep them here too for compatibility with wx.GraphicsContext.
    
    def CreateBrush(self, brush):
        """
        Create a brush from a wx.Brush.
        """
        return GraphicsBrush.CreateFromBrush(brush)

    def CreateFont(self, font, colour=None):
        """
        Create a font from a wx.Font
        """
        return GraphicsFont.CreateFromFont(font, colour)


    def CreateLinearGradientBrush(self, x1, y1, x2, y2, c1, c2):
        """
        Create a gradient brush that morphs from colour c1 at (x1,y1)
        to colour c2 at (x2,y2).
        """
        c1 = _makeColour(c1)
        c2 = _makeColour(c2)
        pattern = cairo.LinearGradient(x1, y1, x2, y2)
        pattern.add_color_stop_rgba(0.0, *_colourToValues(c1))
        pattern.add_color_stop_rgba(1.0, *_colourToValues(c2))
        return GraphicsBrush.CreateFromPattern(pattern)

    def CreateRadialGradientBrush(self, xo, yo, xc, yc, radius, oColour, cColour):
        """
        Creates a brush with a radial gradient originating at (xo,yo)
        with colour oColour and ends on a circle around (xc,yc) with
        radius r and colour cColour.
        """
        oColour = _makeColour(oColour)
        cColour = _makeColour(cColour)
        pattern = cairo.RadialGradient(xo, yo, 0.0, xc, yc, radius)
        pattern.add_color_stop_rgba(0.0, *_colourToValues(oColour))
        pattern.add_color_stop_rgba(1.0, *_colourToValues(cColour))
        return GraphicsBrush.CreateFromPattern(pattern)

      
    def CreateMatrix(self, a=1.0, b=0, c=0, d=1.0, tx=0, ty=0):
        """
        Create a new matrix object.
        """
        m = GraphicsMatrix()
        m.Set(a, b, c, d, tx, ty)
        return m
    
    def CreatePath(self):
        """
        Create a new path obejct.
        """
        return GraphicsPath()

    def CreatePen(self, pen):
        """
        Create a new pen from a wx.Pen.
        """
        return GraphicsPen.CreateFromPen(pen)




    def PushState(self):
        """
        Makes a copy of the current state of the context and saves it
        on an internal stack of saved states.  The saved state will be
        restored when PopState is called.
        """
        self._context.save()

    def PopState(self):
        """
        Restore the most recently saved state which was saved with
        PushState.
        """
        self._context.restore()
            

    def Clip(self, x, y, w, h):
        """
        Adds the rectangle to the current clipping region.  The
        clipping region causes drawing operations to be limited to the
        clipped areas of the context.
        """
        p = GraphicsPath()
        p.AddRectangle(x, y, w, h)
        self._context.append_path(p.GetNativePath())
        self._context.clip()
        

    def ClipRegion(self, region):
        """
        Adds the wx.Region to the current clipping region.
        """
        p = GraphicsPath()
        ri = wx.RegionIterator(region)
        while ri:
            rect = ri.GetRect()
            p.AddRectangle( *rect )
            ri.Next()
        self._context.append_path(p.GetNativePath())
        self._context.clip()
        

    def ResetClip(self):
        """
        Resets the clipping region to the original shape of the context.
        """
        self._context.reset_clip()
        

    def GetNativeContext(self):
        return self._context


    # Since DC logical functions are conceptually different than
    # compositing operators don't pretend they are the same thing, or
    # try ot implement them using the compositing operators.
    def GetLogicalFunction(self):
        raise NotImplementedError("See GetCompositingOperator")
    def SetLogicalFunction(self, function):
        raise NotImplementedError("See SetCompositingOperator")


    def Translate(self, dx, dy):
        """
        Modifies the current transformation matrix by translating the
        user-space origin by (dx, dy).
        """
        self._context.translate(dx, dy)
        

    def Scale(self, xScale, yScale):
        """
        Modifies the current transformation matrix by translating the
        user-space axes by xScale and yScale.
        """
        self._context.scale(xScale, yScale)
        

    def Rotate(self, angle):
        """
        Modifies the current transformation matrix by rotating the
        user-space axes by angle radians.
        """
        self._context.rotate(angle)


    def ConcatTransform(self, matrix):
        """
        Modifies the current transformation matrix by applying matrix
        as an additional transformation.
        """
        self._context.transform(matrix.GetNativeMatrix())
        

    def SetTransform(self, matrix):
        """
        Set the context's current transformation matrix to matrix.
        """
        self._context.set_matrix(matrix.GetNativeMatrix())
        

    def GetTransform(self):
        """
        Returns the context's current transformation matrix.
        """
        gm = GraphicsMatrix()
        gm.Set( *tuple(self._context.get_matrix()) )
        return gm


    def SetPen(self, pen):
        """
        Set the pen to be used for stroking lines in future drawing
        operations.  Either a wx.Pen or a GraphicsPen object may be
        used.
        """
        if isinstance(pen, wx.Pen):
            if not pen.Ok() or pen.Style == wx.TRANSPARENT:
                pen = None
            else:
                pen = GraphicsPen.CreateFromPen(pen)
        self._pen = pen
        

    def SetBrush(self, brush):
        """
        Set the brush to be used for filling shapes in future drawing
        operations.  Either a wx.Brush or a GraphicsBrush object may
        be used.
        """
        if isinstance(brush, wx.Brush):
            if not brush.Ok() or brush.Style == wx.TRANSPARENT:
                brush = None
            else:
                brush = GraphicsBrush.CreateFromBrush(brush)
        self._brush = brush


    def SetFont(self, font, colour=None):
        """
        Sets the font to be used for drawing text.  Either a wx.Font
        or a GrpahicsFont may be used.
        """
        if isinstance(font, wx.Font):
            font = GraphicsFont.CreateFromFont(font, colour)
        self._font = font
        if colour is not None:
            self._fontColour = _makeColour(colour)
        else:
            self._fontColour = font._colour
        

    def StrokePath(self, path):
        """
        Strokes the path (draws the lines) using the current pen.
        """
        if self._pen:
            offset = _OffsetHelper(self)
            self._context.append_path(path.GetNativePath())
            self._pen.Apply(self)
            self._context.stroke()
            
                                      
    def FillPath(self, path, fillStyle=wx.ODDEVEN_RULE):
        """
        Fills the path using the current brush.
        """
        if self._brush:
            offset = _OffsetHelper(self)
            self._context.append_path(path.GetNativePath())
            self._brush.Apply(self)
            d = { wx.WINDING_RULE : cairo.FILL_RULE_WINDING,
                  wx.ODDEVEN_RULE : cairo.FILL_RULE_EVEN_ODD }
            rule = d[fillStyle]
            self._context.set_fill_rule(rule)
            self._context.fill()
            

    def DrawPath(self, path, fillStyle=wx.ODDEVEN_RULE):
        """
        Draws the path by first filling it and then stroking it.
        """
        # TODO: this could be optimized by moving the stroke and fill
        # code here and only loading the path once.
        self.FillPath(path, fillStyle)
        self.StrokePath(path)
        

    def DrawText(self, text, x, y, backgroundBrush=None):
        """
        Draw the text at (x,y) using the current font.  If
        backgroundBrush is set then it is used to fill the rectangle
        behind the text.
        """
        if backgroundBrush:
            formerBrush = self._brush
            formerPen = self._pen
            self.SetBrush(backgroundBrush)
            self.SetPen(None)
            width, height = self.GetTextExtent(text)
            path = GraphicsPath()
            path.AddRectangle(x, y, width, height)
            self.FillPath(path)
            self._DrawText(text, x, y)
            self.SetBrush(formerBrush)
            self.SetPen(formerPen)

        else:
            self._DrawText(text, x, y)
            
            
    def _DrawText(self, text, x, y, angle=None):
        # helper used by DrawText and DrawRotatedText
        if angle is not None:
            self.PushState()
            self.Translate(x, y)
            self.Rotate(-angle)
            x = y = 0
            
        self._font.Apply(self, self._fontColour)
        # Cairo's x,y for drawing text is at the baseline, so we need to adjust
        # the position we move to by the ascent.
        fe = self._context.font_extents()
        ascent = fe[0]
        self._context.move_to( x, y + ascent )
        self._context.show_text(text)

        if angle is not None:
            self.PopState()
            

    def DrawRotatedText(self, text, x, y, angle, backgroundBrush=None):
        """
        Draw the text at (x,y) using the current font and rotated
        angle radians.  If backgroundBrush is set then it is used to
        fill the rectangle behind the text.
        """
        if backgroundBrush:
            formerBrush = self._brush
            formerPen = self._pen
            self.SetBrush(backgroundBrush)
            self.SetPen(None)
            width, height = self.GetTextExtent(text)
            path = GraphicsPath()
            path.AddRectangle(0, 0, width, height)
            self.PushState()
            self.Translate(x, y)
            self.Rotate(-angle)
            self.FillPath(path)
            self.PopState()
            self._DrawText(text, x, y, angle)
            self.SetBrush(formerBrush)
            self.SetPen(formerPen)

        else:
            self._DrawText(text, x, y, angle)
            

    def GetFullTextExtent(self, text):
        """
        Returns the (width, height, descent, externalLeading) of the
        text using the current font.
        """
        if not text:
            return (0,0,0,0)

        self._font.Apply(self, self._fontColour)

        te = self._context.text_extents(text)
        width = te[2]

        fe = self._context.font_extents()
        height = fe[2]
        descent = fe[1]
        ascent = fe[0]
        externalLeading = max(0, height - (ascent + descent))

        return (width, height, descent, externalLeading)
        

    def GetTextExtent(self, text):
        """
        Returns the (width, height) of the text using the current
        font.
        """
        (width, height, descent, externalLeading) = self.GetFullTextExtent(text)
        return (width, height)

    
    def GetPartialTextExtents(self, text):
        raise NotImplementedError("TODO")
    

    def DrawBitmap(self, bmp, x, y, w=-1, h=-1):
        """
        Draw the bitmap at (x,y).  If the width and height parameters
        are passed then the bitmap is scaled to fit that size.  Either
        a wx.Bitmap or a GraphicsBitmap may be used.
        """
        if isinstance(bmp, wx.Bitmap):
            bmp = GraphicsBitmap.CreateFromBitmap(bmp)

        # In case we're scaling the image by using a width and height
        # different than the bitmap's size, create a pattern
        # transformation on the surface and draw the transformed
        # pattern.
        self.PushState()
        pattern = cairo.SurfacePattern(bmp.Surface)

        bw, bh = bmp.Size
        if w == -1: w = bw
        if h == -1: h = bh
        scaleX = w / float(bw)
        scaleY = h / float(bh)

        self._context.scale(scaleX, scaleY)
        self._context.translate(x, y)
        self._context.set_source(pattern)

        # use the original size here since the context is scaled already...
        self._context.rectangle(0, 0, bw, bh)
        # fill the rectangle with the pattern
        self._context.fill()
        
        self.PopState()

        
    def DrawIcon(self, icon, x, y, w=-1, h=-1):
        raise NotImplementedError("TODO")


    def StrokeLine(self, x1, y1, x2, y2):
        """
        Strokes a single line using the current pen.
        """
        path = GraphicsPath()
        path.MoveToPoint(x1, y1)
        path.AddLineToPoint(x2, y2)
        self.StrokePath(path)
        

    def StrokeLines(self, points):
        """
        Stroke a series of conencted lines using the current pen.
        Points is a sequence of points or 2-tuples, and lines are
        drawn from point to point through the end of the sequence.
        """
        path = GraphicsPath()
        x, y = points[0]
        path.MoveToPoint(x, y)
        for point in points[1:]:
            x, y = point
            path.AddLineToPoint(x, y)
        self.StrokePath(path)
        

    def StrokeLineSegments(self, beginPoints, endPoints):
        """
        Stroke a series of lines using the current pen.  For each line
        the begin point is taken from the beginPoints sequence and the
        ending point is taken from the endPoints sequence.
        """
        path = GraphicsPath()
        for begin, end in zip(beginPoints, endPoints):
            path.MoveToPoint(begin[0], begin[1])
            path.AddLineToPoint(end[0], end[1])
        self.StrokePath(path)
        

    def DrawLines(self, points, fillStyle=wx.ODDEVEN_RULE):
        """
        Stroke and fill a series of connected lines using the current
        pen and current brush.
        """
        path = GraphicsPath()
        x, y = points[0]
        path.MoveToPoint(x, y)
        for point in points[1:]:
            x, y = point
            path.AddLineToPoint(x, y)
        self.DrawPath(path, fillStyle)


    def DrawRectangle(self, x, y, w, h):
        """
        Stroke and fill a rectangle using the current pen and current
        brush.
        """
        path = GraphicsPath()
        path.AddRectangle(x, y, w, h)
        self.DrawPath(path)


    def DrawEllipse(self, x, y, w, h):
        """
        Stroke and fill an elipse that fits in the given rectangle,
        using the current pen and current brush.
        """
        path = GraphicsPath()
        path.AddEllipse(x, y, w, h)
        self.DrawPath(path)

    
    def DrawRoundedRectangle(self, x, y, w, h, radius):
        """
        Stroke and fill a rounded rectangle using the current pen and
        current brush.
        """
        path = GraphicsPath()
        path.AddRoundedRectangle(x, y, w, h, radius)
        self.DrawPath(path)
        


    # Things not in wx.GraphicsContext

    def DrawCircle(self, x, y, radius):
        """
        Stroke and fill a circle centered at (x,y) with the given
        radius, using the current pen and brush.
        """
        path = GraphicsPath()
        path.AddCircle(x, y, radius)
        self.DrawPath(path)
        

    def ClipPath(self, path):
        """
        Set the clip region to the path.
        """
        self._context.append_path(path.GetNativePath())
        self._context.clip()

    
    def Clear(self, colour=None):
        """
        Clear the context using the given color or the currently set brush.
        """
        if colour is not None:
            brush = GraphicsBrush(colour)
        elif self._brush is None:
            brush = GraphicsBrush(wx.WHITE)
        else:
            brush = self._brush

        self.PushState()
        op = self._context.get_operator()
        self._context.set_operator(cairo.OPERATOR_SOURCE)
        self._context.reset_clip()

        brush.Apply(self)
        self._context.paint()
        
        self._context.set_operator(op)
        self.PopState()


    def GetCompositingOperator(self):
        """
        Returns the current compositing operator for the context.
        """
        return self._context.get_operator()

    def SetCompositingOperator(self, op):
        """
        Sets the compositin operator to be used for all drawing
        operations.  The default operator is OPERATOR_OVER.
        """
        return self._context.set_operator(op)


    def GetAntialiasMode(self):
        """
        Returns the current antialias mode.
        """
        return self._context.get_antialias()

    def SetAntialiasMode(self, mode=ANTIALIAS_DEFAULT):
        """
        Set the antialiasing mode of the rasterizer used for drawing
        shapes. This value is a hint, and a particular backend may or
        may not support a particular value.
        """
        self._context.set_antialias(mode)

        
#---------------------------------------------------------------------------
# Utility functions

def _makeColour(colour):
    # make a wx.Color from any of the allowed typemaps (string, tuple,
    # etc.)
    if type(colour) == tuple:
        return wx.Colour(*colour)
    elif isinstance(colour, basestring):
        return wx.NamedColour(colour)
    else:
        return colour

    
def _colourToValues(c):    
    # Convert wx.Colour components to a set of values between 0 and 1
    return tuple( [x/255.0 for x in c.Get(True)] )
    


class _OffsetHelper(object):
    def __init__(self, ctx):
        self.ctx = ctx
        self.offset = 0
        if ctx._pen:
            penwidth = ctx._pen.Width
            if penwidth == 0:
                penwidth = 1
            self.offset = (penwidth % 2) == 1;
        if self.offset:
            ctx.Translate(0.5, 0.5)

    def __del__(self):
        if self.offset:
            self.ctx.Translate(-0.5, -0.5)


            
def _stdDashes(style, width):
    if width < 1.0:
        width = 1.0
        
    if style == wx.DOT:
        dashes = [ width, width + 2.0]
    elif style == wx.DOT_DASH:
        dashes = [ 9.0, 6.0, 3.0, 3.0 ]
    elif style ==  wx.LONG_DASH:
        dashes = [ 19.0, 9.0 ]
    elif style == wx.SHORT_DASH:
        dashes = [ 9.0, 6.0 ]

    return dashes

        
#---------------------------------------------------------------------------


