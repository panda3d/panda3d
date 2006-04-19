
from pandac.PandaModules import *

from otp.otpbase import OTPRender


class MotionTrailVertex:
    def __init__(self, vertex_id, vertex_function, context):
        self.vertex_id = vertex_id
        self.vertex_function = vertex_function
        self.context = context
        self.vertex = Vec4 (0.0, 0.0, 0.0, 1.0)

        self.start_color = Vec4 (1.0, 1.0, 1.0, 1.0)
        self.end_color = Vec4 (0.0, 0.0, 0.0, 1.0)
        self.v = 0.0
        
class MotionTrailFrame:
    def __init__ (self, current_time, transform):
        self.time = current_time
        self.transform = transform
        
class MotionTrail(NodePath):
    notify = directNotify.newCategory ("MotionTrail")

    def __init__ (self,name,parent_node_path):

        NodePath.__init__ (self,name)

        # required initialization
        self.enable = True

        self.pause = False
        self.pause_time = 0.0

        self.fade = False
        self.fade_end = False
        self.fade_start_time = 0.0
        self.fade_color_scale = 1.0
        
        self.total_vertices = 0
        self.last_update_time = 0.0
        self.texture = None
        self.vertex_list = [ ]
        self.frame_list = [ ]

        # default options
        self.color_scale = 1.0
        self.time_window = 1.0
        self.sampling_time = 1.0 / 30.0
        self.square_t = True

        # render
        self.reparentTo (parent_node_path)
        self.geom_node = GeomNode ("motion_trail") 
        self.geom_node_path = self.attachNewNode(self.geom_node) 
        node_path = self.geom_node_path

        # set render states
        node_path.node ( ).setAttrib (ColorBlendAttrib.make (ColorBlendAttrib.MAdd))
        node_path.setTransparency (True)
        node_path.setDepthWrite (False)
        node_path.setTwoSided (True)

        # do not light
        node_path.setLightOff ( )
        
        # do not display in reflections
        OTPRender.renderReflection (False, self, 'motion_trail', None)

        return

    def add_vertex (self, vertex_id, vertex_function, context):

        motion_trail_vertex = MotionTrailVertex (vertex_id, vertex_function, context)
        total_vertices = len (self.vertex_list)
        self.vertex_list [total_vertices : total_vertices] = [motion_trail_vertex]
        self.total_vertices = len (self.vertex_list)

        return motion_trail_vertex
    
    def set_texture (self, texture):

        self.texture = texture        
        self.geom_node_path.setTexture (texture)
 
#        texture.setWrapU(Texture.WMClamp)
#        texture.setWrapV(Texture.WMClamp)
        
        return

    def update_vertices (self):

        total_vertices = len (self.vertex_list)
        self.total_vertices = total_vertices
        if (total_vertices >= 2):        
            vertex_index = 0
            while (vertex_index < total_vertices):
                motion_trail_vertex = self.vertex_list [vertex_index]
                motion_trail_vertex.vertex = motion_trail_vertex.vertex_function (motion_trail_vertex, motion_trail_vertex.vertex_id, motion_trail_vertex.context)
                vertex_index += 1

            # calculate v coordinate
            vertex_index = 0
            float_vertex_index = 0.0
            float_total_vertices = 0.0
            float_total_vertices = total_vertices - 1.0
            while (vertex_index < total_vertices):
                motion_trail_vertex = self.vertex_list [vertex_index]
                motion_trail_vertex.v = float_vertex_index / float_total_vertices
                vertex_index += 1
                float_vertex_index += 1.0
                
#                print "motion_trail_vertex.v", motion_trail_vertex.v
                
        return

    def begin_geometry (self):

        self.vertex_index = 0;

        self.format = GeomVertexFormat.getV3c4t2 ( ) 
        self.vertex_data = GeomVertexData ("vertices", self.format, Geom.UHStatic) 

        self.vertex_writer = GeomVertexWriter (self.vertex_data, "vertex") 
        self.color_writer = GeomVertexWriter (self.vertex_data, "color")
        self.texture_writer = GeomVertexWriter (self.vertex_data, "texcoord")
        
        self.triangles = GeomTriangles (Geom.UHStatic) 
        
    def add_geometry_quad (self, v0, v1, v2, v3, c0, c1, c2, c3, t0, t1, t2, t3):
                    
        self.vertex_writer.addData3f (v0 [0], v0 [1], v0 [2])         
        self.vertex_writer.addData3f (v1 [0], v1 [1], v1 [2]) 
        self.vertex_writer.addData3f (v2 [0], v2 [1], v2 [2]) 
        self.vertex_writer.addData3f (v3 [0], v3 [1], v3 [2]) 

        self.color_writer.addData4f (c0 [0], c0 [1], c0 [2], c0 [3])
        self.color_writer.addData4f (c1 [0], c1 [1], c1 [2], c1 [3])
        self.color_writer.addData4f (c2 [0], c2 [1], c2 [2], c2 [3])
        self.color_writer.addData4f (c3 [0], c3 [1], c3 [2], c3 [3])

        self.texture_writer.addData2f (t0 [0], t0 [1])
        self.texture_writer.addData2f (t1 [0], t1 [1])
        self.texture_writer.addData2f (t2 [0], t2 [1])
        self.texture_writer.addData2f (t3 [0], t3 [1])

        vertex_index = self.vertex_index;
        
        self.triangles.addVertex (vertex_index + 0) 
        self.triangles.addVertex (vertex_index + 1) 
        self.triangles.addVertex (vertex_index + 2) 
        self.triangles.closePrimitive ( ) 

        self.triangles.addVertex (vertex_index + 1) 
        self.triangles.addVertex (vertex_index + 3) 
        self.triangles.addVertex (vertex_index + 2) 
        self.triangles.closePrimitive ( ) 

        self.vertex_index += 4

    def end_geometry (self):
        self.geometry = Geom (self.vertex_data) 
        self.geometry.addPrimitive (self.triangles) 

        self.geom_node.removeAllGeoms ( )         
        self.geom_node.addGeom (self.geometry)         

    def update_motion_trail (self, current_time, transform):

        update = False
        if ((current_time - self.last_update_time) >= self.sampling_time):
            update = True        
    
        if (self.pause):
            update = False
            
        if (update):
            color_scale = self.color_scale;

            if (self.fade):
                elapsed_time = current_time - self.fade_start_time                

                if (elapsed_time < 0.0):
                    elapsed_time = 0.0
                    print "elapsed_time < 0", elapsed_time
                    
                if (elapsed_time < self.fade_time):
                    color_scale = (1.0 - (elapsed_time / self.fade_time)) * color_scale
                else:
                    color_scale = 0.0
                    self.fade_end = True
                    
            self.last_update_time = current_time

            # remove expired frames
            minimum_time = current_time - self.time_window
 
            """
            print "*** minimum_time", minimum_time
            """
            
            index = 0
            last_frame_index = len (self.frame_list) - 1

            while (index <= last_frame_index):
                motion_trail_frame = self.frame_list [last_frame_index - index]
                if (motion_trail_frame.time >= minimum_time):
                    break
                index += 1    

            if (index > 0):
                self.frame_list [last_frame_index - index: last_frame_index + 1] = [ ]

            # add new frame to beginning of list
            motion_trail_frame = MotionTrailFrame (current_time, transform)
            self.frame_list = [motion_trail_frame] + self.frame_list
            
            # convert frames and vertices to geometry
            total_frames = len (self.frame_list)
            
            """
            print "total_frames", total_frames

            index = 0;            
            while (index < total_frames):
                motion_trail_frame = self.frame_list [index]
                print "frame time", index, motion_trail_frame.time 
                index += 1              
            """
            
            if ((total_frames >= 2) and (self.total_vertices >= 2)):

                self.begin_geometry ( )

                total_segments = total_frames - 1

                last_motion_trail_frame = self.frame_list [total_segments]

                minimum_time = last_motion_trail_frame.time
                delta_time = current_time - minimum_time

#                print "minimum_time", minimum_time
#                print "delta_time", delta_time 
                
                segment_index = 0
                while (segment_index < total_segments):                    
                    motion_trail_frame_start = self.frame_list [segment_index]
                    motion_trail_frame_end = self.frame_list [segment_index + 1]

                    start_t = (motion_trail_frame_start.time - minimum_time) / delta_time
                    end_t = (motion_trail_frame_end.time - minimum_time) / delta_time

                    st = start_t
                    et = end_t

#                    print "st", st
#                    print "et", et
                    
                    if (self.square_t):                    
                        start_t *= start_t
                        end_t *= end_t
                    
#                    print "start_t", start_t
#                    print "end_t", end_t
                    
                    vertex_segement_index = 0
                    total_vertex_segments = self.total_vertices - 1

                    motion_trail_vertex_start = self.vertex_list [0]
                    v0 = motion_trail_frame_start.transform.xform (motion_trail_vertex_start.vertex)
                    v2 = motion_trail_frame_end.transform.xform (motion_trail_vertex_start.vertex)

                    vertex_start_color = motion_trail_vertex_start.end_color + (motion_trail_vertex_start.start_color - motion_trail_vertex_start.end_color)
                    color_start_t = color_scale * start_t
                    color_end_t = color_scale * end_t
                    c0 = vertex_start_color * color_start_t
                    c2 = vertex_start_color * color_end_t

                    t0 = Vec2 (st, motion_trail_vertex_start.v)
                    t2 = Vec2 (et, motion_trail_vertex_start.v)

                    while (vertex_segement_index < total_vertex_segments):

                        motion_trail_vertex_start = self.vertex_list [vertex_segement_index]
                        motion_trail_vertex_end = self.vertex_list [vertex_segement_index + 1]

                        v1 = motion_trail_frame_start.transform.xform (motion_trail_vertex_end.vertex)
                        v3 = motion_trail_frame_end.transform.xform (motion_trail_vertex_end.vertex)

                        """
                        print v0
                        print v1
                        print v2
                        print v3
                        """

                        # color
                        vertex_end_color = motion_trail_vertex_end.end_color + (motion_trail_vertex_end.start_color - motion_trail_vertex_end.end_color)

                        c1 = vertex_end_color * color_start_t
                        c3 = vertex_end_color * color_end_t

                        # uv
                        t1 = Vec2 (st, motion_trail_vertex_end.v)
                        t3 = Vec2 (et, motion_trail_vertex_end.v)

                        self.add_geometry_quad (v0, v1, v2, v3, c0, c1, c2, c3, t0, t1, t2, t3)

                        # reuse calculations
                        v0 = v1
                        v2 = v3

                        c0 = c1
                        c2 = c3
                        
                        t0 = t1
                        t2 = t3
                        
                        vertex_segement_index += 1

                    segment_index += 1
                    
                self.end_geometry ( )
                
        return
  
    def enable_motion_trail(self, enable):
        self.enable = enable
        return

    def reset_motion_trail(self):
        self.frame_list = [ ]
        return

    def set_fade (self, time, current_time):
        if (self.pause == False):
            self.fade_color_scale = 1.0

            if (time == 0.0):
                self.fade = False
            else:
                self.fade_start_time = current_time
                self.fade_time = time
                self.fade = True            
        return 
        
    def pause_motion_trail(self, current_time):
        if (self.pause == False):
            self.pause_time = current_time
            self.pause = True
        return

    def resume_motion_trail(self, current_time):
        if (self.pause):
            delta_time = current_time - self.pause_time

            frame_index = 0
            total_frames = len (self.frame_list)
            while (frame_index < total_frames):
                motion_trail_frame = self.frame_list [frame_index]
                motion_trail_frame.time += delta_time
                frame_index += 1

            if (self.fade):      
                self.fade_start_time += delta_time

            self.pause = False
        return

    def toggle_pause_motion_trail (self, current_time):
        if (self.pause):
            self.resume_motion_trail (current_time)
        else:
            self.pause_motion_trail (current_time)
