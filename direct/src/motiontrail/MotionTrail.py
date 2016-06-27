

from panda3d.core import *
from panda3d.direct import *
from direct.task import Task
from direct.showbase.DirectObject import DirectObject
from direct.directnotify.DirectNotifyGlobal import directNotify


def remove_task ( ):
    if (MotionTrail.task_added):
        total_motion_trails = len (MotionTrail.motion_trail_list)

        if total_motion_trails > 0:
            print("warning: %d motion trails still exist when motion trail task is removed" % (total_motion_trails))

        MotionTrail.motion_trail_list = [ ]

        taskMgr.remove (MotionTrail.motion_trail_task_name)

        print("MotionTrail task removed")

        MotionTrail.task_added = False
    return

class MotionTrailVertex:
    def __init__(self, vertex_id, vertex_function, context):
        self.vertex_id = vertex_id
        self.vertex_function = vertex_function
        self.context = context
        self.vertex = Vec4 (0.0, 0.0, 0.0, 1.0)

        # default
        self.start_color = Vec4 (1.0, 1.0, 1.0, 1.0)
        self.end_color = Vec4 (0.0, 0.0, 0.0, 1.0)
        self.v = 0.0

class MotionTrailFrame:
    def __init__ (self, current_time, transform):
        self.time = current_time
        self.transform = transform

class MotionTrail(NodePath, DirectObject):

    notify = directNotify.newCategory ("MotionTrail")

    task_added = False
    motion_trail_list = [ ]
    motion_trail_task_name = "motion_trail_task"

    global_enable = True

    @classmethod
    def setGlobalEnable (self, enable):
        MotionTrail.global_enable = enable

    def __init__ (self,name,parent_node_path):

        DirectObject.__init__(self)
        NodePath.__init__ (self,name)

        # required initialization
        self.active = True
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

        self.parent_node_path = parent_node_path

        self.previous_matrix = None
        self.calculate_relative_matrix = False

        self.playing = False;

        # default options
        self.continuous_motion_trail = True
        self.color_scale = 1.0
        self.time_window = 1.0
        self.sampling_time = 0.0
        self.square_t = True

#        self.task_transform = False
        self.root_node_path = None

        # node path states
        self.reparentTo (parent_node_path)
        self.geom_node = GeomNode ("motion_trail")
        self.geom_node_path = self.attachNewNode(self.geom_node)
        node_path = self.geom_node_path

        ### set render states

        node_path.setTwoSided (True)

        # set additive blend effects
        node_path.setTransparency (True)
        node_path.setDepthWrite (False)
        node_path.node ( ).setAttrib (ColorBlendAttrib.make (ColorBlendAttrib.MAdd))

        # do not light
        node_path.setLightOff ( )

        # disable writes to destination alpha, write out rgb colors only
        node_path.setAttrib (ColorWriteAttrib.make (ColorWriteAttrib.CRed | ColorWriteAttrib.CGreen | ColorWriteAttrib.CBlue));

        if (MotionTrail.task_added == False):
#            taskMgr.add (self.motion_trail_task, "motion_trail_task", priority = 50)
            taskMgr.add (self.motion_trail_task, MotionTrail.motion_trail_task_name)

            self.acceptOnce ("clientLogout", remove_task)

            MotionTrail.task_added = True


        self.relative_to_render = False

        self.use_nurbs = False
        self.resolution_distance = 0.5

        self.cmotion_trail = CMotionTrail ( )
        self.cmotion_trail.setGeomNode (self.geom_node)

        self.modified_vertices = True
        if base.config.GetBool('want-python-motion-trails', 0):
            self.use_python_version = True
        else:
            self.use_python_version = False

        return

    def delete(self):
        self.reset_motion_trail()
        self.reset_motion_trail_geometry()
        self.cmotion_trail.resetVertexList ( )
        self.removeNode()
        return

    def print_matrix (self, matrix):
        separator = ' '
        print(matrix.getCell (0, 0), separator, matrix.getCell (0, 1), separator, matrix.getCell (0, 2), separator, matrix.getCell (0, 3))
        print(matrix.getCell (1, 0), separator, matrix.getCell (1, 1), separator, matrix.getCell (1, 2), separator, matrix.getCell (1, 3))
        print(matrix.getCell (2, 0), separator, matrix.getCell (2, 1), separator, matrix.getCell (2, 2), separator, matrix.getCell (2, 3))
        print(matrix.getCell (3, 0), separator, matrix.getCell (3, 1), separator, matrix.getCell (3, 2), separator, matrix.getCell (3, 3))

    def motion_trail_task (self, task):

        current_time = task.time
        total_motion_trails = len (MotionTrail.motion_trail_list)

        index = 0
        while (index < total_motion_trails):
            motion_trail = MotionTrail.motion_trail_list [index]

            if (MotionTrail.global_enable):
                if (motion_trail.use_python_version):
                    # Python version
                    if (motion_trail.active and motion_trail.check_for_update (current_time)):
                        transform = None
                        if (motion_trail.root_node_path != None) and (motion_trail.root_node_path != render):
                            motion_trail.root_node_path.update ( )

                        if (motion_trail.root_node_path and (motion_trail.relative_to_render == False)):
                            transform = motion_trail.getMat(motion_trail.root_node_path)
                        else:
                            transform = Mat4 (motion_trail.getNetTransform ( ).getMat ( ))

                        if (transform != None):
                            motion_trail.update_motion_trail (current_time, transform)
                else:
                    # C++ version
                    if (motion_trail.active and motion_trail.cmotion_trail.checkForUpdate (current_time)):
                        transform = None
                        if (motion_trail.root_node_path != None) and (motion_trail.root_node_path != render):
                            motion_trail.root_node_path.update ( )

                        if (motion_trail.root_node_path and (motion_trail.relative_to_render == False)):
                            transform = motion_trail.getMat(motion_trail.root_node_path)
                        else:
                            transform = Mat4 (motion_trail.getNetTransform ( ).getMat ( ))

                        if (transform != None):
                            motion_trail.transferVertices ( )
                            motion_trail.cmotion_trail.updateMotionTrail (current_time, transform)

            else:
                motion_trail.reset_motion_trail()
                motion_trail.reset_motion_trail_geometry()

            index += 1

        return Task.cont

    def add_vertex (self, vertex_id, vertex_function, context):

        motion_trail_vertex = MotionTrailVertex (vertex_id, vertex_function, context)
        total_vertices = len (self.vertex_list)

        self.vertex_list [total_vertices : total_vertices] = [motion_trail_vertex]

        self.total_vertices = len (self.vertex_list)

        self.modified_vertices = True

        return motion_trail_vertex

    def set_vertex_color (self, vertex_id, start_color, end_color):
        if (vertex_id >= 0 and vertex_id < self.total_vertices):
            motion_trail_vertex = self.vertex_list [vertex_id]
            motion_trail_vertex.start_color = start_color
            motion_trail_vertex.end_color = end_color

        self.modified_vertices = True
        return

    def set_texture (self, texture):

        self.texture = texture
        if (texture):
            self.geom_node_path.setTexture (texture)
#            texture.setWrapU(Texture.WMClamp)
#            texture.setWrapV(Texture.WMClamp)
        else:
            self.geom_node_path.clearTexture ( )

        self.modified_vertices = True
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
            # this is based on the number of vertices only and not on the relative positions of the vertices
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

        self.modified_vertices = True
        return

    def transferVertices (self):

        # transfer only on modification
        if (self.modified_vertices):
            self.cmotion_trail.setParameters (self.sampling_time, self.time_window, self.texture != None, self.calculate_relative_matrix, self.use_nurbs, self.resolution_distance)

            self.cmotion_trail.resetVertexList ( )

            vertex_index = 0
            total_vertices = len (self.vertex_list)
            while (vertex_index < total_vertices):
                motion_trail_vertex = self.vertex_list [vertex_index]
                self.cmotion_trail.addVertex (motion_trail_vertex.vertex, motion_trail_vertex.start_color, motion_trail_vertex.end_color, motion_trail_vertex.v)
                vertex_index += 1

            self.modified_vertices = False

        return

    def register_motion_trail (self):
        MotionTrail.motion_trail_list = MotionTrail.motion_trail_list + [self]
        return

    def unregister_motion_trail (self):
        if (self in MotionTrail.motion_trail_list):
            MotionTrail.motion_trail_list.remove (self)
        return

    def begin_geometry (self):

        self.vertex_index = 0;

        if (self.texture != None):
            self.format = GeomVertexFormat.getV3c4t2 ( )
        else:
            self.format = GeomVertexFormat.getV3c4 ( )

        self.vertex_data = GeomVertexData ("vertices", self.format, Geom.UHStatic)

        self.vertex_writer = GeomVertexWriter (self.vertex_data, "vertex")
        self.color_writer = GeomVertexWriter (self.vertex_data, "color")
        if (self.texture != None):
            self.texture_writer = GeomVertexWriter (self.vertex_data, "texcoord")

        self.triangles = GeomTriangles (Geom.UHStatic)

    def add_geometry_quad (self, v0, v1, v2, v3, c0, c1, c2, c3, t0, t1, t2, t3):

        self.vertex_writer.addData3f (v0 [0], v0 [1], v0 [2])
        self.vertex_writer.addData3f (v1 [0], v1 [1], v1 [2])
        self.vertex_writer.addData3f (v2 [0], v2 [1], v2 [2])
        self.vertex_writer.addData3f (v3 [0], v3 [1], v3 [2])

        self.color_writer.addData4f (c0)
        self.color_writer.addData4f (c1)
        self.color_writer.addData4f (c2)
        self.color_writer.addData4f (c3)

        if (self.texture != None):
            self.texture_writer.addData2f (t0)
            self.texture_writer.addData2f (t1)
            self.texture_writer.addData2f (t2)
            self.texture_writer.addData2f (t3)

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

    def check_for_update (self, current_time):

        state = False
        if ((current_time - self.last_update_time) >= self.sampling_time):
            state = True

        if (self.pause):
            state = False

        update = state and self.enable

        return state

    def update_motion_trail (self, current_time, transform):

        if (len (self.frame_list) >= 1):
            if (transform == self.frame_list [0].transform):
                # ignore duplicate transform updates
                return

        if (self.check_for_update (current_time)):

            color_scale = self.color_scale;

            if (self.fade):
                elapsed_time = current_time - self.fade_start_time

                if (elapsed_time < 0.0):
                    print("elapsed_time < 0: %f" % (elapsed_time))
                    elapsed_time = 0.0

                if (elapsed_time < self.fade_time):
                    color_scale = (1.0 - (elapsed_time / self.fade_time)) * color_scale
                else:
                    color_scale = 0.0
                    self.fade_end = True

            self.last_update_time = current_time

            # remove expired frames
            minimum_time = current_time - self.time_window

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

                if (self.calculate_relative_matrix):
                    inverse_matrix = Mat4 (transform)
                    inverse_matrix.invertInPlace ( )

                if (self.use_nurbs and (total_frames >= 5)):

                    total_distance = 0.0
                    vector = Vec3 ( )

                    nurbs_curve_evaluator_list = [ ]

                    total_vertex_segments = self.total_vertices - 1

                    # create a NurbsCurveEvaluator for each vertex (the starting point for the trail)
                    index = 0
                    while (index < self.total_vertices):
                        nurbs_curve_evaluator = NurbsCurveEvaluator ( )
                        nurbs_curve_evaluator.reset (total_segments)
                        nurbs_curve_evaluator_list = nurbs_curve_evaluator_list + [nurbs_curve_evaluator]
                        index += 1

                    # add vertices to each NurbsCurveEvaluator
                    segment_index = 0
                    while (segment_index < total_segments):
                        motion_trail_frame_start = self.frame_list [segment_index]
                        motion_trail_frame_end = self.frame_list [segment_index + 1]

                        vertex_segement_index = 0

                        if (self.calculate_relative_matrix):
                            start_transform = Mat4 ( )
                            end_transform = Mat4 ( )

                            start_transform.multiply (motion_trail_frame_start.transform, inverse_matrix)
                            end_transform.multiply (motion_trail_frame_end.transform, inverse_matrix)

                        else:
                            start_transform = motion_trail_frame_start.transform
                            end_transform = motion_trail_frame_end.transform

                        motion_trail_vertex_start = self.vertex_list [0]

                        v0 = start_transform.xform (motion_trail_vertex_start.vertex)
                        v2 = end_transform.xform (motion_trail_vertex_start.vertex)

                        nurbs_curve_evaluator = nurbs_curve_evaluator_list [vertex_segement_index]

                        nurbs_curve_evaluator.setVertex (segment_index, v0)

                        while (vertex_segement_index < total_vertex_segments):

                            motion_trail_vertex_start = self.vertex_list [vertex_segement_index]
                            motion_trail_vertex_end = self.vertex_list [vertex_segement_index + 1]

                            v1 = start_transform.xform (motion_trail_vertex_end.vertex)
                            v3 = end_transform.xform (motion_trail_vertex_end.vertex)

                            nurbs_curve_evaluator = nurbs_curve_evaluator_list [vertex_segement_index + 1]

                            nurbs_curve_evaluator.setVertex (segment_index, v1)

                            if (vertex_segement_index == (total_vertex_segments - 1)):
                                v = v1 - v3
                                vector.set (v[0], v[1], v[2])
                                distance = vector.length()
                                total_distance += distance

                            vertex_segement_index += 1

                        segment_index += 1

                    # evaluate NurbsCurveEvaluator for each vertex
                    index = 0
                    nurbs_curve_result_list = [ ]
                    while (index < self.total_vertices):
                        nurbs_curve_evaluator = nurbs_curve_evaluator_list [index]
                        nurbs_curve_result = nurbs_curve_evaluator.evaluate ( )
                        nurbs_curve_result_list = nurbs_curve_result_list + [nurbs_curve_result]

                        nurbs_start_t = nurbs_curve_result.getStartT()
                        nurbs_end_t = nurbs_curve_result.getEndT()

                        index += 1

                    # create quads from NurbsCurveResult
                    total_curve_segments = total_distance / self.resolution_distance
                    if (total_curve_segments < total_segments):
                        total_curve_segments = total_segments;

                    v0 = Vec3 ( )
                    v1 = Vec3 ( )
                    v2 = Vec3 ( )
                    v3 = Vec3 ( )

                    def one_minus_x (x):
                        x = 1.0 - x
                        if (x < 0.0):
                            x = 0.0
                        return x

                    curve_segment_index = 0.0
                    while (curve_segment_index < total_curve_segments):

                        vertex_segement_index = 0

                        if (True):
                            st = curve_segment_index / total_curve_segments
                            et = (curve_segment_index + 1.0) / total_curve_segments
                        else:
                            st = curve_segment_index / total_segments
                            et = (curve_segment_index + 1.0) / total_segments

                        start_t = st
                        end_t = et

                        if (self.square_t):
                            start_t *= start_t
                            end_t *= end_t

                        motion_trail_vertex_start = self.vertex_list [0]

                        vertex_start_color = motion_trail_vertex_start.end_color + (motion_trail_vertex_start.start_color - motion_trail_vertex_start.end_color)
                        color_start_t = color_scale * start_t
                        color_end_t = color_scale * end_t
                        c0 = vertex_start_color * one_minus_x (color_start_t)
                        c2 = vertex_start_color * one_minus_x (color_end_t)

                        t0 = Vec2 (one_minus_x (st), motion_trail_vertex_start.v)
                        t2 = Vec2 (one_minus_x (et), motion_trail_vertex_start.v)

                        while (vertex_segement_index < total_vertex_segments):

                            motion_trail_vertex_start = self.vertex_list [vertex_segement_index]
                            motion_trail_vertex_end = self.vertex_list [vertex_segement_index + 1]

                            start_nurbs_curve_result = nurbs_curve_result_list [vertex_segement_index]
                            end_nurbs_curve_result = nurbs_curve_result_list [vertex_segement_index + 1]

                            start_nurbs_start_t = start_nurbs_curve_result.getStartT()
                            start_nurbs_end_t = start_nurbs_curve_result.getEndT()
                            end_nurbs_start_t = end_nurbs_curve_result.getStartT()
                            end_nurbs_end_t = end_nurbs_curve_result.getEndT()

                            start_delta_t = (start_nurbs_end_t - start_nurbs_start_t)
                            end_delta_t = (end_nurbs_end_t - end_nurbs_start_t)

                            start_nurbs_curve_result.evalPoint (start_nurbs_start_t + (start_delta_t * st), v0);
                            end_nurbs_curve_result.evalPoint (end_nurbs_start_t + (end_delta_t * st), v1);

                            start_nurbs_curve_result.evalPoint (start_nurbs_start_t + (start_delta_t * et), v2);
                            end_nurbs_curve_result.evalPoint (end_nurbs_start_t + (end_delta_t * et), v3);

                            # color
                            vertex_end_color = motion_trail_vertex_end.end_color + (motion_trail_vertex_end.start_color - motion_trail_vertex_end.end_color)

                            c1 = vertex_end_color * one_minus_x (color_start_t)
                            c3 = vertex_end_color * one_minus_x (color_end_t)

                            # uv
                            t1 = Vec2 (one_minus_x (st), motion_trail_vertex_end.v)
                            t3 = Vec2 (one_minus_x (et), motion_trail_vertex_end.v)

                            self.add_geometry_quad (v0, v1, v2, v3, c0, c1, c2, c3, t0, t1, t2, t3)

                            # reuse calculations
                            c0 = c1
                            c2 = c3

                            t0 = t1
                            t2 = t3

                            vertex_segement_index += 1

                        curve_segment_index += 1.0


                else:

                    segment_index = 0
                    while (segment_index < total_segments):
                        motion_trail_frame_start = self.frame_list [segment_index]
                        motion_trail_frame_end = self.frame_list [segment_index + 1]

                        start_t = (motion_trail_frame_start.time - minimum_time) / delta_time
                        end_t = (motion_trail_frame_end.time - minimum_time) / delta_time

                        st = start_t
                        et = end_t

                        if (self.square_t):
                            start_t *= start_t
                            end_t *= end_t

                        vertex_segement_index = 0
                        total_vertex_segments = self.total_vertices - 1

                        if (self.calculate_relative_matrix):
                            start_transform = Mat4 ( )
                            end_transform = Mat4 ( )
                            start_transform.multiply (motion_trail_frame_start.transform, inverse_matrix)
                            end_transform.multiply (motion_trail_frame_end.transform, inverse_matrix)
                        else:
                            start_transform = motion_trail_frame_start.transform
                            end_transform = motion_trail_frame_end.transform

                        motion_trail_vertex_start = self.vertex_list [0]

                        v0 = start_transform.xform (motion_trail_vertex_start.vertex)
                        v2 = end_transform.xform (motion_trail_vertex_start.vertex)

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

                            v1 = start_transform.xform (motion_trail_vertex_end.vertex)
                            v3 = end_transform.xform (motion_trail_vertex_end.vertex)

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
        self.cmotion_trail.reset ( );
        return

    def reset_motion_trail_geometry(self):
        if (self.geom_node != None):
            self.geom_node.removeAllGeoms ( )
        return

    def attach_motion_trail (self):
        self.reset_motion_trail ( )
        return

    def begin_motion_trail (self):
        if (self.continuous_motion_trail == False):
            self.reset_motion_trail ( )
            self.active = True;
            self.playing = True;
        return

    def end_motion_trail (self):
        if (self.continuous_motion_trail == False):
            self.active = False
            self.reset_motion_trail ( )
            self.reset_motion_trail_geometry ( )
            self.playing = False;
        return

    # the following functions are not currently supported in the C++ version

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

