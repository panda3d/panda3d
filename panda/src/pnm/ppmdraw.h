/* ppmdraw.h - header file for simple drawing routines in libppm
**
** Simple, yes, and also fairly slow if the truth be told; but also very
** flexible and powerful.
**
** The two basic concepts are the drawproc and clientdata.  All the drawing
** routines take a drawproc that does the actual drawing.  A drawproc draws
** a single point, and it looks like this:
*/
void ppmd_point_drawproc ARGS(( pixel** pixels, int cols, int rows, pixval maxval, int x, int y, char* clientdata ));
/*
** So, you call a drawing routine, e.g. ppmd_line(), and pass it a drawproc;
** it calls the drawproc for each point it wants to draw.  Why so complicated?
** Because you can define your own drawprocs to do more interesting things than
** simply draw the point.  For example, you could make one that calls back into
** another drawing routine, say ppmd_circle() to draw a circle at each point
** of a line.
**
** Slow?  Well sure, we're talking results here, not realtime.  You can do
** tricks with this arrangement that you couldn't even think of before.
** Still, to speed things up for the 90% case you can use this:
*/
#if __STDC__
#define PPMD_NULLDRAWPROC (void (*)(pixel**, int, int, pixval, int, int, char*)) 0
#else /*__STDC__*/
#define PPMD_NULLDRAWPROC (void (*)()) 0
#endif /*__STDC__*/
/*
** Just like ppmd_point_drawproc() it simply draws the point, but it's done
** inline, and clipping is assumed to be handled at a higher level.
**
** Now, what about clientdata.  Well, it's an arbitrary pointer, and can
** mean something different to each different drawproc.  For the above two
** drawprocs, clientdata should be a pointer to a pixel holding the color
** to be drawn.  Other drawprocs can use it to point to something else,
** e.g. some structure to be modified, or they can ignore it.
*/


/* Outline drawing routines.  Lines, splines, circles, etc. */

int ppmd_setlinetype ARGS(( int type ));
#define PPMD_LINETYPE_NORMAL 0
#define PPMD_LINETYPE_NODIAGS 1
/* If you set NODIAGS, all pixels drawn by ppmd_line() will be 4-connected
** instead of 8-connected; in other words, no diagonals.  This is useful
** for some applications, for example when you draw many parallel lines
** and you want them to fit together without gaps.
*/

int ppmd_setlineclip ARGS(( int clip ));
#define ppmd_setlineclipping(x)     ppmd_setlineclip(x)
/* Normally, ppmd_line() clips to the edges of the pixmap.  You can use this
** routine to disable the clipping, for example if you are using a drawproc
** that wants to do its own clipping.
*/

void ppmd_line ARGS(( pixel** pixels, int cols, int rows, pixval maxval, int x0, int y0, int x1, int y1, void (*drawprocP)(pixel**, int, int, pixval, int, int, char*), char* clientdata ));
/* Draws a line from (x0, y0) to (x1, y1).
*/

void ppmd_spline3 ARGS(( pixel** pixels, int cols, int rows, pixval maxval, int x0, int y0, int x1, int y1, int x2, int y2, void (*drawprocP)(pixel**, int, int, pixval, int, int, char*), char* clientdata ));
/* Draws a three-point spline from (x0, y0) to (x2, y2), with (x1, y1) as
** the control point.  All drawing is done via ppmd_line(), so the routines
** that control it control ppmd_spline3() as well.
*/

void ppmd_polyspline ARGS(( pixel** pixels, int cols, int rows, pixval maxval, int x0, int y0, int nc, int* xc, int* yc, int x1, int y1, void (*drawprocP)(pixel**, int, int, pixval, int, int, char*), char* clientdata ));
/* Draws a bunch of splines end to end.  (x0, y0) and (x1, y1) are the initial
** and final points, and the xc and yc are the intermediate control points.
** nc is the number of these control points.
*/

void ppmd_circle ARGS(( pixel** pixels, int cols, int rows, pixval maxval, int cx, int cy, int radius, void (*drawprocP)(pixel**, int, int, pixval, int, int, char*), char* clientdata ));
/* Draws a circle centered at (cx, cy) with the specified radius.
*/


/* Simple filling routines.  Ok, so there's only one. */

void ppmd_filledrectangle ARGS(( pixel** pixels, int cols, int rows, pixval maxval, int x, int y, int width, int height, void (*drawprocP)(pixel**, int, int, pixval, int, int, char*), char* clientdata ));
/* Fills in the rectangle [x, y, width, height].
*/


/* Arbitrary filling routines.  With these you can fill any outline that
** you can draw with the outline routines.
*/

char* ppmd_fill_init ARGS(( void ));
/* Returns a blank fillhandle.
*/

void ppmd_fill_drawproc ARGS(( pixel** pixels, int cols, int rows, pixval maxval, int x, int y, char* clientdata ));
/* Use this drawproc to trace the outline you want filled.  Be sure to use
** the fillhandle as the clientdata.
*/

void ppmd_fill ARGS(( pixel** pixels, int cols, int rows, pixval maxval, char* fillhandle, void (*drawprocP)(pixel**, int, int, pixval, int, int, char*), char* clientdata ));
/* Once you've traced the outline, give the fillhandle to this routine to
** do the actual drawing.  As usual, it takes a drawproc and clientdata;
** you could define drawprocs to do stipple fills and such.
*/
