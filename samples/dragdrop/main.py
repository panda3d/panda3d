'''
Drag & Drop Demo
Use the mouse to drag and drop colored squares from moving frames or the background.
Use left mouse click to grab a square, release it to ...well, release it.
'''
from panda3d.core import *
from direct.gui.DirectGui import *
from direct.showbase import ShowBase
from direct.interval.LerpInterval import LerpPosInterval
from direct.interval.MetaInterval import Sequence
import random

from vector import vector

class DragItem:
    def __init__(self,app_drag,app_drop,pos,app_parent):
        a=1
        
        x,y=pos
        
        frame=DirectFrame(frameColor=(random.uniform(0, 1),random.uniform(0, 1),random.uniform(0, 1),1.0),
                           frameSize=_rec2d(30, 30),
                           state=DGG.NORMAL,
                           parent=app_parent)#black_frame)
        #bind the events
        frame.bind(DGG.B1PRESS, app_drag, [self])
        frame.bind(DGG.B1RELEASE, app_drop)
        frame.set_pos(_pos2d(x*32,y*32))
        self.frame=frame
        

class SnapFrame:
    
    def bind(self,*args,**kwargs):
        self.frame.bind(*args,**kwargs)
        
    def __init__(self,pos,grid_pos=(0,0),frame_kwargs={},color=(1,1,1,1.0),drag_drop_type=None):
        
        if frame_kwargs=={}:
            
            kwargs={"frameColor":color,
                    "frameSize":_rec2d(50,50),
                    "state":DGG.NORMAL,
                    "parent":pixel2d}
                    
        #if drag_drop_type!=None:
        kwargs["frameColor"]=color
        #if color!=
   
        
        self.frame=DirectFrame(**kwargs)
        
        #pos=(pos[0]+offset[0]*grid_pos[0],pos[1]+offset[1]*grid_pos[1])
        
        self.frame.set_pos(_pos2d(*pos))
        
        ##make it move
        #Sequence(LerpPosInterval(self.white_frame, 5.0,_pos2d(32,256)),
        #         LerpPosInterval(self.white_frame, 5.0,_pos2d(32,32)) ).loop()
        
        self.frame.drag_drop_type=drag_drop_type
    


class target_colored_grid:
    def __init__(self,pos,offset,colors,rows_collums=(5,1)):
        
        col_l=["white","black","light_red","light_blue","light_green"]
        random.shuffle(col_l)
        
        x,y=rows_collums
        self.d={}
        
        x_i=0
        while x_i < x:
            col_key=col_l[x_i]
            col_i=colors[col_key]
            y_i=0
            self.d[x_i]={}
            while y_i < y:
                
                dd_type=None
                
                #if x_i==2:
                #    dd_type="red"
                    
                pos_i=(pos[0]+offset[0]*x_i,pos[1]+offset[1]*y_i)
                self.d[x_i][y_i]=SnapFrame(pos_i,grid_pos=(x_i,y_i),color=col_i,drag_drop_type=col_key)
                
                y_i+=1
            x_i+=1


class grid:
    def __init__(self,pos,offset,rows_collums=(3,3)):
        x,y=rows_collums
        self.d={}
        
        x_i=0
        while x_i < x:
            
            y_i=0
            self.d[x_i]={}
            while y_i < y:
                
                dd_type=None
                
                if x_i==2:
                    dd_type="red"
                    
                pos_i=(pos[0]+offset[0]*x_i,pos[1]+offset[1]*y_i)
                self.d[x_i][y_i]=SnapFrame(pos_i,grid_pos=(x_i,y_i),drag_drop_type=dd_type)
                
                y_i+=1
            x_i+=1

#Helper functions
def _pos2d(x,y):
    return Point3(x,0,-y)

def _rec2d(width, height):
    return (width, 0, 0, -height)

def get_local_center(ob):

    h=ob.getHeight()
    w=abs(ob.getWidth())
    
    w = w/2
    h = h/2
    
    return h,w

def get_mid_point(ob):
    h,w=get_local_center(ob)
    
    r=ob.getPos()
        
    r=r-LPoint3f(h,w)
    
    return r
    left,right,bottom,top=ob.frameSize()
    
    half_r=(right-left)/2
    half_b=(bottom-top)/2
    
    mid_x = left + half_r
    mid_y = top + half_b
    
    #w = ob.width
    #h = ob.height
    
    
    #pos = ob.getPos()
    
    #r = pos-Point3(h,w,0)
    
    return mid_x,mid_y

def get_offset(ob):
    a=1

def set_center(ob,pos):
    ob.setPos(pos)
    a=1
    

class App:
    def __init__(self):
        
        #for demonstration purposes:
        #get a few colors as "types"
        
        #red green blue white black
        # only provide red green blue cubes
        
        colors={"red":(1,0.2,0.2,1.0),"light_red":(1,0.5,0.5,1.0),
                "green":(0.2,1,0.2,1.0),"light_green":(0.5,1,0.5,1.0),
                "blue":(0.2,0.2,1,1.0),"light_blue":(0.5,0.5,1,1.0),
                "white":(1,1,1,1.0),"black":(0,0,0,1.0)}
                
                
        
        
        
        #add a title/instructions
        wp = WindowProperties.getDefault()
        wp.set_title("Drag & Drop the colored squares")
        WindowProperties.setDefault(wp)
        #init showvase
        base = ShowBase.ShowBase()
        #make a frame
        
        #make a grid on one sides
        #self.grid=grid((32,32),(64,64))#3,3)
        
        self.grid=target_colored_grid((32,32),(64,64),colors)
        
        self.bind_events(self.grid)
        
        #make a grid on the other side.
        #self.grid2=grid((264,32),(64,64))
        
        #self.bind_events(self.grid2)
        
        #add some items to be dragged and dropped.
        self.current_last_anchor=None
        self.current_drag_drop_type=None
        self.last_hover_drag_drop_type=None
        
        color_l=["red","green","blue"]
        
        self.default_grid=grid((64,300),(64,64),(3,1))
        self.bind_events(self.default_grid)
        self.drag_items={}
        #make some squares to be drag/dropped     
        x=0   
        for col in color_l:
            self.drag_items[x]={}
            #for y in range(8):
            
            #frame=DragItem(self.drag,self.drop,(x,y),self.black_frame)
            
            rel_col=colors[col]
            
            frame=DirectFrame(frameColor=rel_col,
                               frameSize=_rec2d(30, 30),
                               state=DGG.NORMAL,
                               parent=pixel2d)
                               #parent=self.black_frame)#black_frame)
                               
            #bind the events
            frame.bind(DGG.B1PRESS, self.drag, [frame])
            frame.bind(DGG.B1RELEASE, self.drop)
            #frame.set_pos(_pos2d(x*32,y*32))
            frame.set_pos(_pos2d(32+x*128,264))
            #frame.set_pos(_pos2d(64+x*64,32))
            
            frame.drag_drop_type=col
            
            #anchot here
            frame.last_drag_drop_anchor=self.default_grid.d[x][0]
            frame.setPos(get_mid_point(self.default_grid.d[x][0].frame))
            #self.current_drag_drop_anchor_pos=self.grid.d[0][0].frame.getPos()
            #frame.
            
            self.drag_items[x]=frame #took out y
            x+=1

        self.current_dragged=None
        self.last_hover_in=None
        
        
        #run a task tracking the mouse cursor
        taskMgr.add(self.update, "update", sort=-50)
        
    def bind_events(self,grid):
        
        #bind the events
        for key1 in grid.d:
            for key2 in grid.d[key1]:
                ob=grid.d[key1][key2]
                ob.bind(DGG.WITHIN, self.hover_in, [ob])
                
                ob.bind(DGG.WITHOUT, self.hover_out)
        
    
    def hover_in(self, widget, mouse_pos=None):
        '''Set the widget to be the target to drop objects onto'''
                    
        self.last_hover_in=widget
        self.last_hover_drag_drop_type=widget.frame.drag_drop_type
        
    def hover_out(self, mouse_pos=None):
        '''Clear the target to drop objects onto'''
        self.last_hover_in=None
        self.last_hover_drag_drop_type=None
    
    def update(self, task=None):
        '''Track the mouse pos and move self.current_dragged to where the cursor is '''
        
        if self.current_dragged:
            
            if base.mouseWatcherNode.has_mouse():
                mpos = base.mouseWatcherNode.get_mouse()
                #h,w=self.current_dragged.h,self.current_dragged.w
                #pos=Point3(mpos.get_x()+w/2 ,0, mpos.get_y()+h/2)
                pos=Point3(mpos.get_x(),0, mpos.get_y())
                
                self.current_dragged.set_pos(pixel2d.get_relative_point(render2d, pos))
        if task:
            return task.again

    def drag(self, widget, mouse_pos=None):
        '''Set the widget to be the currently dragged object'''
        
        self.current_last_drag_drop_anchor=widget.last_drag_drop_anchor#self.grid.d[0][0]
        
        h,w=get_local_center(widget)
        
        widget.reparent_to(pixel2d)
        self.current_dragged=widget
        self.current_dragged.h=h
        self.current_dragged.w=w
        self.update()

    def drop(self, mouse_pos=None):
        '''Drop the currently dragged object on the last object the cursor hovered over'''
        if self.current_dragged:
            
            if self.last_hover_in:
                if self.current_dragged.drag_drop_type not in self.last_hover_drag_drop_type:
                #if self.last_hover_drag_drop_type!=None:
                    #in this case the frame the user hovers over is invalid.
                    #the last one gets picked as default target and is snapped to.
                    snap_target=self.current_last_drag_drop_anchor
                    lock_pos=get_mid_point(snap_target.frame)
                    
                else:
                    #everything in order, snap to the one that's currently being
                    #hovered over.
                    snap_target=self.last_hover_in
                    lock_pos=get_mid_point(snap_target.frame)
                    self.current_dragged.last_drag_drop_anchor=snap_target
                    
                
                h,w=get_local_center(self.current_dragged)
                lock_pos=lock_pos+LPoint3f(h,w)
                lock_pos=LVecBase3f(*lock_pos)
                self.current_dragged.setPos(lock_pos)
                
                self.current_dragged.wrt_reparent_to(snap_target.frame)
                
            self.current_dragged=None


app=App()
base.run()
