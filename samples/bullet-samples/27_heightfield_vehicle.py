#!/usr/bin/env python

import sys

from direct.showbase.ShowBase import ShowBase
from direct.showbase.InputStateGlobal import inputState
from direct.gui.OnscreenText import OnscreenText

from panda3d.core import NodePath, load_prc_file_data, look_at, PStatClient
from panda3d.core import AmbientLight, DirectionalLight, TransformState
from panda3d.core import LPoint3, LVector3, LVector4, LTexCoord
from panda3d.core import DynamicHeightfield, PfmFile, PNMImage
from panda3d.core import ShaderTerrainMesh, Shader, Texture, SamplerState
from panda3d.core import PfmVizzer, CardMaker, TextNode

from panda3d.bullet import BulletWorld, BulletDebugNode, BulletRigidBodyNode
from panda3d.bullet import BulletHeightfieldShape, BulletBoxShape
from panda3d.bullet import BulletVehicle


class HeightfieldVehicle(ShowBase):
    def __init__(self, heightfield_fn="heightfield.png"):
        # Store the heightfield's filename.
        self.heightfield_fn = heightfield_fn

        """
        Load some configuration variables, it's important for this to happen
        before ShowBase is initialized
        """
        load_prc_file_data("", """
            sync-video #t
            textures-power-2 none
            ###gl-coordinate-system default
            notify-level-gobj warning
            notify-level-grutil debug
            notify-level-shader_terrain debug
            notify-level-bullet debug
            ### model paths
            model-path /usr/share/panda3d
            model-path /home/juzzuj/code/prereq/bullet-samples/bullet-samples
            """)
        ShowBase.__init__(self)
        base.set_background_color(0.1, 0.1, 0.8, 1)
        base.set_frame_rate_meter(True)

        # Increase camera Field Of View and set near and far planes
        base.camLens.set_fov(90)
        base.camLens.set_near_far(0.1, 50000)

        # Lights
        alight = AmbientLight('ambientLight')
        alight.set_color(LVector4(0.5, 0.5, 0.5, 1))
        alightNP = render.attach_new_node(alight)

        dlight = DirectionalLight('directionalLight')
        dlight.set_direction(LVector3(1, 1, -1))
        dlight.set_color(LVector4(0.7, 0.7, 0.7, 1))
        dlightNP = render.attach_new_node(dlight)

        render.clear_light()
        render.set_light(alightNP)
        render.set_light(dlightNP)

        # Basic game controls
        self.accept('escape', self.do_exit)
        self.accept('f1', base.toggle_wireframe)
        self.accept('f2', base.toggle_texture)
        self.accept('f3', self.toggle_debug)
        self.accept('f5', self.do_screenshot)
        self.accept('r', self.do_reset)

        # Vehicle controls
        inputState.watchWithModifiers('forward', 'w')
        inputState.watchWithModifiers('turnLeft', 'a')
        inputState.watchWithModifiers('reverse', 's')
        inputState.watchWithModifiers('turnRight', 'd')
        inputState.watchWithModifiers('forward', 'arrow_up')
        inputState.watchWithModifiers('turnLeft', 'arrow_left')
        inputState.watchWithModifiers('reverse', 'arrow_down')
        inputState.watchWithModifiers('turnRight', 'arrow_right')

        self.accept('space', self.reset_vehicle)

        # Controls to do with the terrain
        #self.accept('t', self.rise_in_front)
        self.accept('t', self.deform_terrain, ["raise"])
        self.accept('g', self.deform_terrain, ["depress"])
        self.accept('b', self.drop_boxes)

        # Some debugging and miscellaneous controls
        self.accept('e', self.query_elevation)
        self.accept('c', self.convert_coordinates)
        self.accept('p', self.save)
        self.accept('h', self.hide_terrain)

        # Task
        taskMgr.add(self.update, 'updateWorld')

        self.setup()

    """
    Macro-like function used to reduce the amount to code needed to create the
    on screen instructions
    """
    def genLabelText(self, i, text, parent="a2dTopLeft"):
        return OnscreenText(text=text, parent=getattr(base, parent), scale=.05,
                            pos=(0.06, -.065 * i), fg=(1, 1, 1, 1),
                            align=TextNode.ALeft)

    def cleanup(self):
        self.world = None
        self.worldNP.remove_node()
        self.terrain.remove_node()
        self.skybox.remove_node()
        del self.terrain_node

    def do_exit(self):
        self.cleanup()
        sys.exit(1)

    def do_reset(self):
        self.cleanup()
        self.setup()

    def toggle_debug(self):
        if self.debugNP.is_hidden():
          self.debugNP.show()
        else:
          self.debugNP.hide()

    def do_screenshot(self):
        base.screenshot('HeightfieldVehicle')

    def update(self, task):
        # Get the time passed since the last frame
        dt = globalClock.get_dt()
        # Pass dt as parameter (we need it for sensible steering calculations)
        self.process_input(dt)

        """
        Basically, we want to put our camera where the camera floater is. We
        need the floater's world (=render-relative) position (it's parented to
        the vehicle)
        """
        floater_pos = render.get_relative_point(self.camera_floater,
                                                LVector3(0))
        """
        If the camera floater is under the terrain surface, adjust it, so that
        it stays above the terrain.
        """
        elevation_at_floater_pos = self.query_elevation(floater_pos)
        if elevation_at_floater_pos.z >= floater_pos.z:
            floater_pos.z = elevation_at_floater_pos.z + 1.
        # Now we set our camera's position and make it look at the vehicle.
        base.cam.set_pos(floater_pos)
        base.cam.look_at(self.vehicleNP)

        # Let the Bullet library do physics calculations.
        self.world.do_physics(dt, 10, 0.008)
        return task.cont

    def process_input(self, dt):
        # Relax steering towards straight
        self.steering *= 1 - self.steering_relax_factor * dt

        engine_force = 0.0
        brake_force = 0.0

        if inputState.isSet('forward'):
            engine_force = 1000.0
            brake_force = 0.0

        if inputState.isSet('reverse'):
            engine_force = 0.0
            brake_force = 100.0

        if inputState.isSet('turnLeft'):
            self.steering += dt * self.steering_increment
            self.steering = min(self.steering, self.steering_clamp)

        if inputState.isSet('turnRight'):
            self.steering -= dt * self.steering_increment
            self.steering = max(self.steering, -self.steering_clamp)

        # Lower steering intensity for high speeds
        self.steering *= 1. - (self.vehicle.get_current_speed_km_hour() *
                               self.steering_speed_reduction_factor)

        # Apply steering to front wheels
        #self.vehicle.get_wheels()[0].set_steering(self.steering)
        #self.vehicle.get_wheels()[1].set_steering(self.steering)
        #self.vehicle.wheels[0].steering = self.steering
        #self.vehicle.wheels[1].steering = self.steering
        self.vehicle.set_steering_value(self.steering, 0)
        self.vehicle.set_steering_value(self.steering, 1)

        # Apply engine and brake to rear wheels
        #self.vehicle.wheels[2].engine_force = engine_force
        #self.vehicle.wheels[3].engine_force = engine_force
        #self.vehicle.wheels[2].brake = brake_force
        #self.vehicle.wheels[3].brake = brake_force
        self.vehicle.apply_engine_force(engine_force, 2)
        self.vehicle.apply_engine_force(engine_force, 3)
        self.vehicle.set_brake(brake_force, 2)
        self.vehicle.set_brake(brake_force, 3)

    def setup(self):
        # Bullet physics world
        self.worldNP = render.attach_new_node('World')
        self.debugNP = self.worldNP.attach_new_node(BulletDebugNode('Debug'))
        self.world = BulletWorld()
        self.world.set_gravity(LVector3(0, 0, -9.81))
        self.world.set_debug_node(self.debugNP.node())

        # Vehicle
        # Steering info
        self.steering = 0.0              # degrees
        self.steering_clamp = 45.0       # degrees
        self.steering_increment = 120.0  # degrees per second
        # How fast steering relaxes to straight ahead
        self.steering_relax_factor = 2.0
        # How much steering intensity decreases with higher speeds
        self.steering_speed_reduction_factor = 0.003

        # Chassis collision box (note: Bullet uses half-measures)
        shape = BulletBoxShape(LVector3(0.6, 1.4, 0.5))
        ts = TransformState.make_pos(LPoint3(0, 0, 0.5))
        self.vehicleNP = self.worldNP.attach_new_node(
            BulletRigidBodyNode('Vehicle'))
        self.vehicleNP.node().add_shape(shape, ts)
        # Set initial position
        self.vehicleNP.set_pos(0, 70, -10)
        self.vehicleNP.node().set_mass(800.0)
        self.vehicleNP.node().set_deactivation_enabled(False)
        self.world.attach(self.vehicleNP.node())
        # Camera floater
        self.attach_camera_floater()
        # BulletVehicle
        self.vehicle = BulletVehicle(self.world, self.vehicleNP.node())
        self.world.attach(self.vehicle)

        # Vehicle model
        self.yugoNP = loader.load_model('models/yugo/yugo.egg')
        self.yugoNP.reparent_to(self.vehicleNP)
        # Right front wheel
        np = loader.load_model('models/yugo/yugotireR.egg')
        np.reparent_to(self.worldNP)
        self.add_wheel(LPoint3( 0.70,  1.05, 0.3), True, np)
        # Left front wheel
        np = loader.load_model('models/yugo/yugotireL.egg')
        np.reparent_to(self.worldNP)
        self.add_wheel(LPoint3(-0.70,  1.05, 0.3), True, np)
        # Right rear wheel
        np = loader.load_model('models/yugo/yugotireR.egg')
        np.reparent_to(self.worldNP)
        self.add_wheel(LPoint3( 0.70, -1.05, 0.3), False, np)
        # Left rear wheel
        np = loader.load_model('models/yugo/yugotireL.egg')
        np.reparent_to(self.worldNP)
        self.add_wheel(LPoint3(-0.70, -1.05, 0.3), False, np)

        # Load a skybox
        self.skybox = self.loader.load_model(
            "samples/shader-terrain/models/skybox.bam")
        self.skybox.reparent_to(self.render)
        self.skybox.set_scale(20000)
        skybox_texture = self.loader.load_texture(
            "samples/shader-terrain/textures/skybox.jpg")
        skybox_texture.set_minfilter(SamplerState.FT_linear)
        skybox_texture.set_magfilter(SamplerState.FT_linear)
        skybox_texture.set_wrap_u(SamplerState.WM_repeat)
        skybox_texture.set_wrap_v(SamplerState.WM_mirror)
        skybox_texture.set_anisotropic_degree(16)
        self.skybox.set_texture(skybox_texture)
        skybox_shader = Shader.load(Shader.SL_GLSL,
            "samples/shader-terrain/skybox.vert.glsl",
            "samples/shader-terrain/skybox.frag.glsl")
        self.skybox.set_shader(skybox_shader)

        # Terrain
        self.setup_terrain()

    def add_wheel(self, pos, front, np):
        wheel = self.vehicle.create_wheel()

        wheel.set_node(np.node())
        wheel.set_chassis_connection_point_cs(pos)
        wheel.set_front_wheel(front)

        wheel.set_wheel_direction_cs(LVector3(0, 0, -1))
        wheel.set_wheel_axle_cs(LVector3(1, 0, 0))
        wheel.set_wheel_radius(0.25)
        wheel.set_max_suspension_travel_cm(40.0)

        wheel.set_suspension_stiffness(40.0)
        wheel.set_wheels_damping_relaxation(2.3)
        wheel.set_wheels_damping_compression(4.4)
        wheel.set_friction_slip(100.0)
        wheel.set_roll_influence(0.1)

    def attach_camera_floater(self):
        """
        Set up an auxiliary camera floater, which is parented to the vehicle.
        Every frame base.cam's position will be set to the camera floater's.
        """
        camera_behind = 8
        camera_above = 3
        self.camera_floater = NodePath("camera_floater")
        self.camera_floater.reparent_to(self.vehicleNP)
        self.camera_floater.set_y(-camera_behind)
        self.camera_floater.set_z(camera_above)

    def reset_vehicle(self):
        reset_pos = self.vehicleNP.get_pos()
        reset_pos.z += 3
        self.vehicleNP.node().clear_forces()
        self.vehicleNP.node().set_linear_velocity(LVector3(0))
        self.vehicleNP.node().set_angular_velocity(LVector3(0))
        self.vehicleNP.set_pos(reset_pos)
        self.vehicleNP.set_hpr(LVector3(0))

    def drop_boxes(self):
        """
        Puts a stack of boxes at a distance in front of the vehicle.
        The boxes will not necessarily spawn above the terrain and will not
        be cleaned up.
        """
        model = loader.load_model('models/box.egg')
        model.set_pos(-0.5, -0.5, -0.5)
        model.flatten_light()
        shape = BulletBoxShape(LVector3(0.5, 0.5, 0.5))
        ahead = self.vehicleNP.get_pos() + self.vehicle.get_forward_vector()*15

        for i in range(6):
            node = BulletRigidBodyNode('Box')
            node.set_mass(5.0)
            node.add_shape(shape)
            node.set_deactivation_enabled(False)
            np = render.attach_new_node(node)
            np.set_pos(ahead.x, ahead.y, ahead.z + i*2)
            self.world.attach(node)
            model.copy_to(np)

    def query_elevation(self, xy_pos=None):
        """
        Query elevation for xy_pos if present, else for vehicle position.
        No xy_pos means verbose mode.
        """
        query_pos = xy_pos or self.vehicleNP.get_pos()
        """
        This method is accurate and may be useful for placing
        objects on the terrain surface.
        """
        result = self.world.ray_test_closest(
            LPoint3(query_pos.x, query_pos.y, -10000),
            LPoint3(query_pos.x, query_pos.y, 10000))
        if result.has_hit():
            hit_pos = result.get_hit_pos()
            if not xy_pos:
                print("Bullet heightfield elevation at "
                    "X {:.2f} | Y {:.2f} is {:.3f}".format(
                    hit_pos.x, hit_pos.y, hit_pos.z))
        else:
            hit_pos = None
            if not xy_pos:
                print("Could not query elevation at {}".format(xy_pos))

        """
        This method is less accurate than the one above.
        Under heavy ray-testing stress (ray tests are performed for all vehicle
        wheels, the above elevation query etc.) Bullet sometimes seems to be a
        little unreliable.
        """
        texspace_pos = self.terrain.get_relative_point(render, query_pos)
        stm_pos = self.terrain_node.uv_to_world(
            LTexCoord(texspace_pos.x, texspace_pos.y))
        if not xy_pos:
            print("ShaderTerrainMesh elevation at "
                "X {:.2f} | Y {:.2f} is {:.3f}".format(
                stm_pos.x, stm_pos.y, stm_pos.z))

        return hit_pos or stm_pos

    def setup_terrain(self):
        """
        Terrain info
        Units are meters, which is preferable when working with Bullet.
        """
        self.terrain_scale = LVector3(512, 512, 100)
        self.terrain_pos = LVector3(-256, -256, -70)
        # sample values for a 4096 x 4096px heightmap.
        #self.terrain_scale = LVector3(4096, 4096, 1000)
        #self.terrain_pos = LVector3(-2048, -2048, -70)
        """
        Diamond_subdivision is an alternating triangulation scheme and may
        produce better results.
        """
        use_diamond_subdivision = True

        """
        Construct the terrain
        Without scaling, any ShaderTerrainMesh is 1x1x1 units.
        """
        self.terrain_node = ShaderTerrainMesh()
        """
        Set a heightfield, the heightfield should be a 16-bit png and
        have a quadratic size of a power of two.
        """
        heightfield = Texture()
        heightfield.read(self.heightfield_fn)
        heightfield.set_keep_ram_image(True)
        self.terrain_node.heightfield = heightfield

        # Display characteristic values of the heightfield texture
        #minpoint, maxpoint, avg = LPoint3(), LPoint3(), LPoint3()
        #heightfield.calc_min_max(minpoint, maxpoint)
        #heightfield.calc_average_point(avg, 0.5, 0.5, 0.5)
        #print("avg: {} min: {} max: {}".format(avg.x, minpoint.x, maxpoint.x))

        """
        Set the target triangle width. For a value of 10.0 for example,
        the ShaderTerrainMesh will attempt to make every triangle 10 pixels
        wide on screen.
        """
        self.terrain_node.target_triangle_width = 10.0
        if use_diamond_subdivision:
            """
            This has to be specified before calling .generate()
            The default is false.
            """
            load_prc_file_data("", "stm-use-hexagonal-layout true")

        self.terrain_node.generate()
        """
        Attach the terrain to the main scene and set its scale. With no scale
        set, the terrain ranges from (0, 0, 0) to (1, 1, 1)
        """
        self.terrain = self.render.attach_new_node(self.terrain_node)
        self.terrain.set_scale(self.terrain_scale)
        self.terrain.set_pos(self.terrain_pos)
        """
        Set a vertex and a fragment shader on the terrain. The
        ShaderTerrainMesh only works with an applied shader.
        """
        terrain_shader = Shader.load(Shader.SL_GLSL,
            "samples/shader-terrain/terrain.vert.glsl",
            "samples/shader-terrain/terrain.frag.glsl")
        self.terrain.set_shader(terrain_shader)
        self.terrain.set_shader_input("camera", base.camera)
        # Set some texture on the terrain
        grass_tex = self.loader.load_texture(
            "samples/shader-terrain/textures/grass.png")
        grass_tex.set_minfilter(SamplerState.FT_linear_mipmap_linear)
        grass_tex.set_anisotropic_degree(16)
        self.terrain.set_texture(grass_tex)

        """
        Set up the DynamicHeightfield (it's a type of PfmFile). We load the
        same heightfield image as with ShaderTerrainMesh.
        """
        self.DHF = DynamicHeightfield()
        self.DHF.read(self.heightfield_fn)
        """
        Set up empty PfmFiles to prepare stuff in that is going to
        dynamically modify our terrain.
        """
        self.StagingPFM = PfmFile()
        self.RotorPFM = PfmFile()

        """
        Set up the BulletHeightfieldShape (=collision terrain) and give it
        some sensible physical properties.
        """
        self.HFS = BulletHeightfieldShape(self.DHF, self.terrain_scale.z,
                                          STM=True)
        if use_diamond_subdivision:
            self.HFS.set_use_diamond_subdivision(True)
        HFS_rigidbody = BulletRigidBodyNode("BulletTerrain")
        HFS_rigidbody.set_static(True)
        friction = 2.0
        HFS_rigidbody.set_anisotropic_friction(
            LVector3(friction, friction, friction/1.3))
        HFS_rigidbody.set_restitution(0.3)
        HFS_rigidbody.add_shape(self.HFS)
        self.world.attach(HFS_rigidbody)

        HFS_NP = NodePath(HFS_rigidbody)
        HFS_NP.reparent_to(self.worldNP)
        """
        This aligns the Bullet terrain with the ShaderTerrainMesh rendered
        terrain. It will be exact as long as the terrain vertex shader from
        the STM sample is used and no additional tessellation shader.
        For Bullet (as for other physics engines) the origin of objects is at
        the center.
        """
        HFS_NP.set_pos(self.terrain_pos + self.terrain_scale/2)
        HFS_NP.set_sx(self.terrain_scale.x / heightfield.get_x_size())
        HFS_NP.set_sy(self.terrain_scale.y / heightfield.get_y_size())

        # Disables Bullet debug rendering for the terrain, because it is slow.
        #HFS_NP.node().set_debug_enabled(False)

        """
        Finally, link the ShaderTerrainMesh and the BulletHeightfieldShape to
        the DynamicHeightfield. From now on changes to the DynamicHeightfield
        will propagate to the (visible) ShaderTerrainMesh and the (collidable)
        BulletHeightfieldShape.
        """
        self.HFS.set_dynamic_heightfield(self.DHF)
        self.terrain_node.set_dynamic_heightfield(self.DHF)

    def deform_terrain(self, mode="raise", duration=1.0):
        self.deform_duration = duration
        """
        Calculate distance to the spot at which we want to raise the terrain.
        At its current speed the vehicle would reach it in 2.5 seconds.
        [km/h] / 3.6 = [m/s] * [s] = [m]
        """
        distance = self.vehicle.get_current_speed_km_hour() / 3.6 * 2.5
        spot_pos_world = (self.vehicleNP.get_pos() +
                          self.vehicle.get_forward_vector() * distance)

        spot_pos_heightfield = self.terrain_node.world_to_heightfield(
            spot_pos_world)
        xcenter = spot_pos_heightfield.x
        ycenter = spot_pos_heightfield.y
        """
        To create a smooth hill/depression we call PfmFile.pull_spot to create
        a sort of gradient. You can use self.cardmaker_debug to view it.

        From the Panda3D API documentation of PfmFile.pull_spot:
        Applies delta * t to the point values within radius (xr, yr)
        distance of (xc, yc). The t value is scaled from 1.0 at the center
        to 0.0 at radius (xr, yr), and this scale follows the specified
        exponent. Returns the number of points affected.
        """
        # Delta to apply to the point values in DynamicHeightfield array.
        delta = LVector4(0.001)
        # Make the raised spot elliptical.
        xradius = 10.0  # meters
        yradius = 6.0  # meters
        # Choose an exponent
        exponent = 0.6
        # Counter-clockwise angle between Y-axis
        angle = self.vehicle.get_forward_vector().signed_angle_deg(
            LVector3.forward(), LVector3.down())

        # Define all we need to repeatedly deform the terrain using a task.
        self.spot_params = [delta, xcenter, ycenter, xradius, yradius,
                            exponent, mode]

        # Clear staging area to a size that fits our raised region.
        self.StagingPFM.clear(int(xradius)*2, int(yradius)*2, num_channels=1)

        """
        There are two options:
        (1) Rotate our hill/depression to be perpendicular
            to the vehicle's trajectory. This is the default.
        (2) Just put it in the terrain unrotated.
        """

        # Option (1)
        self.StagingPFM.pull_spot(delta,
                                  xradius-0.5, yradius-0.5,
                                  xradius, yradius,
                                  exponent)

        # Rotate wider side so it's perpendicular to vehicle's trajectory.
        self.RotorPFM.rotate_from(self.StagingPFM, angle)

        self.raise_start_time = globalClock.get_real_time()
        taskMgr.do_method_later(0.03, self.deform_perpendicular,
                                "DeformPerpendicularSpot")

        # Option (2)
        #taskMgr.do_method_later(0.03, self.deform_regular, "RaiseASpot")

    def deform_perpendicular(self, task):
        if (globalClock.get_real_time() - self.raise_start_time <
            self.deform_duration):
            (delta, xcenter, ycenter,
             xradius, yradius, exponent, mode) = self.spot_params

            """
            Copy rotated hill to our DynamicHeightfield.
            The values from RotorPFM are added to the ones found in the
            DynamicHeightfield.
            """
            self.DHF.add_sub_image(self.RotorPFM,
                int(xcenter - self.RotorPFM.get_x_size()/2),
                int(ycenter - self.RotorPFM.get_y_size()/2),
                0, 0, self.RotorPFM.get_x_size(), self.RotorPFM.get_y_size(),
                1.0 if mode == "raise" else -1.0)

            # Output images of our PfmFiles.
            #self.RotorPFM.write("dbg_RotorPFM.png")
            #self.StagingPFM.write("dbg_StagingPFM.png")
            return task.again

        self.cardmaker_debug()
        return task.done

    def deform_regular(self, task):
        if (globalClock.get_real_time() - self.raise_start_time <
            self.deform_duration):
            (delta, xcenter, ycenter,
             xradius, yradius, exponent, mode) = self.spot_params
            self.DHF.pull_spot(delta * (1.0 if mode == "raise" else -1.0),
                               xcenter, ycenter, xradius, yradius, exponent)
            return task.again
        return task.done

    def convert_coordinates(self):
        vpos = self.vehicleNP.get_pos()
        print("VPOS world: ", vpos)
        W2H = self.terrain_node.world_to_heightfield(vpos)
        print("W2H: ", W2H)
        H2W = self.terrain_node.heightfield_to_world(LTexCoord(W2H.x, W2H.y))
        print("H2W: ", H2W)
        W2U = self.terrain_node.world_to_uv(vpos)
        print("W2U: ", W2U)
        U2W = self.terrain_node.uv_to_world(LTexCoord(W2U.x, W2U.y))
        print("U2W: ", U2W)

    def hide_terrain(self):
        if self.terrain.is_hidden():
            self.terrain.show()
        else:
            self.terrain.hide()

    def save(self):
        self.DHF.write("dbg_dynamicheightfield.png")

    def pfmvizzer_debug(self, pfm):
        vizzer = PfmVizzer(pfm)
        """
        To align the mesh we generate with PfmVizzer with our 
        ShaderTerrainMesh, BulletHeightfieldShape, and DynamicHeightfield, we
        need to set a negative scale on the Y axis. This reverses the winding
        order of the triangles in the mesh, which is why we choose
        PfmVizzer.MF_back
        Note that this does not accurately match up with our mesh because the
        interpolation differs. Still, PfmVizzer can be useful when
        inspecting, distorting, etc. PfmFiles.
        """
        self.vizNP = vizzer.generate_vis_mesh(PfmVizzer.MF_back)
        self.vizNP.set_texture(loader.load_texture("maps/grid.rgb"))
        self.vizNP.reparent_to(render)
        if pfm == self.DHF or pfm == self.terrain_node.heightfield:
            self.vizNP.set_pos(self.terrain_pos.x, -self.terrain_pos.y,
                               self.terrain_pos.z)
            self.vizNP.set_scale(self.terrain_scale.x, -self.terrain_scale.y,
                                 self.terrain_scale.z)

    def cardmaker_debug(self):
        for node in render2d.find_all_matches("pfm"):
            node.remove_node()
        for text in base.a2dBottomLeft.find_all_matches("*"):
            text.remove_node()
        width = 0.2  # render2d coordinates range: [-1..1]
        # Pseudo-normalize our PfmFile for better contrast.
        normalized_pfm = PfmFile(self.RotorPFM)
        max_p = LVector3()
        normalized_pfm.calc_min_max(LVector3(), max_p)
        normalized_pfm *= 1.0 / max_p.x
        # Put it in a texture
        tex = Texture()
        tex.load(normalized_pfm)
        # Apply the texture to a quad and put it in the lower left corner.
        cm = CardMaker("pfm")
        cm.set_frame(0, width, 0,
            width / normalized_pfm.get_x_size() * normalized_pfm.get_y_size())
        card = base.render2d.attach_new_node(cm.generate())
        card.set_pos(-1, 0, -1)
        card.set_texture(tex)
        # Display max value text
        self.genLabelText(-3,
                          "Max value: {:.3f} == {:.2f}m".format(max_p.x,
                            max_p.x * self.terrain_scale.z),
                          parent="a2dBottomLeft")


game = HeightfieldVehicle()
game.run()
