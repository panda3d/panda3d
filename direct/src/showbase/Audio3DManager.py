"""Contains the Audio3DManager class."""

__all__ = ['Audio3DManager']

from panda3d.core import Vec3, VBase3, WeakNodePath, ClockObject
from direct.task.TaskManagerGlobal import Task, taskMgr
#
class Audio3DManager:

    def __init__(self, audio_manager, listener_target = None, root = None,
                 taskPriority = 51):
        self.audio_manager = audio_manager
        self.listener_target = listener_target

        if (root==None):
            self.root = render
        else:
            self.root = root

        self.sound_dict = {}
        self.vel_dict = {}
        self.listener_vel = VBase3(0, 0, 0)

        taskMgr.add(self.update, "Audio3DManager-updateTask", taskPriority)

    def loadSfx(self, name):
        """
        Use Audio3DManager.loadSfx to load a sound with 3D positioning enabled
        """
        sound = None
        if (name):
            sound=self.audio_manager.getSound(name, 1)
        return sound

    def setDistanceFactor(self, factor):
        """
        Control the scale that sets the distance units for 3D spacialized audio.
        Default is 1.0 which is adjust in panda to be meters.
        When you change this, don't forget that this effects the scale of setSoundMinDistance
        """
        self.audio_manager.audio3dSetDistanceFactor(factor)

    def getDistanceFactor(self):
        """
        Control the scale that sets the distance units for 3D spacialized audio.
        Default is 1.0 which is adjust in panda to be meters.
        """
        return self.audio_manager.audio3dGetDistanceFactor()

    def setDopplerFactor(self, factor):
        """
        Control the presence of the Doppler effect. Default is 1.0
        Exaggerated Doppler, use >1.0
        Diminshed Doppler, use <1.0
        """
        self.audio_manager.audio3dSetDopplerFactor(factor)

    def getDopplerFactor(self):
        """
        Control the presence of the Doppler effect. Default is 1.0
        Exaggerated Doppler, use >1.0
        Diminshed Doppler, use <1.0
        """
        return self.audio_manager.audio3dGetDopplerFactor()

    def setDropOffFactor(self, factor):
        """
        Exaggerate or diminish the effect of distance on sound. Default is 1.0
        Valid range is 0 to 10
        Faster drop off, use >1.0
        Slower drop off, use <1.0
        """
        self.audio_manager.audio3dSetDropOffFactor(factor)

    def getDropOffFactor(self):
        """
        Exaggerate or diminish the effect of distance on sound. Default is 1.0
        Valid range is 0 to 10
        Faster drop off, use >1.0
        Slower drop off, use <1.0
        """
        return self.audio_manager.audio3dGetDropOffFactor()

    def setSoundMinDistance(self, sound, dist):
        """
        Controls the distance (in units) that this sound begins to fall off.
        Also affects the rate it falls off.
        Default is 3.28 (in feet, this is 1 meter)
        Don't forget to change this when you change the DistanceFactor
        """
        sound.set3dMinDistance(dist)

    def getSoundMinDistance(self, sound):
        """
        Controls the distance (in units) that this sound begins to fall off.
        Also affects the rate it falls off.
        Default is 3.28 (in feet, this is 1 meter)
        """
        return sound.get3dMinDistance()

    def setSoundMaxDistance(self, sound, dist):
        """
        Controls the maximum distance (in units) that this sound stops falling off.
        The sound does not stop at that point, it just doesn't get any quieter.
        You should rarely need to adjust this.
        Default is 1000000000.0
        """
        sound.set3dMaxDistance(dist)

    def getSoundMaxDistance(self, sound):
        """
        Controls the maximum distance (in units) that this sound stops falling off.
        The sound does not stop at that point, it just doesn't get any quieter.
        You should rarely need to adjust this.
        Default is 1000000000.0
        """
        return sound.get3dMaxDistance()

    def setSoundVelocity(self, sound, velocity):
        """
        Set the velocity vector (in units/sec) of the sound, for calculating doppler shift.
        This is relative to the sound root (probably render).
        Default: VBase3(0, 0, 0)
        """
        if isinstance(velocity, tuple) and len(velocity) == 3:
            velocity = VBase3(*velocity)
        if not isinstance(velocity, VBase3):
            raise TypeError("Invalid argument 1, expected <VBase3>")
        self.vel_dict[sound] = velocity

    def setSoundVelocityAuto(self, sound):
        """
        If velocity is set to auto, the velocity will be determined by the
        previous position of the object the sound is attached to and the frame dt.
        Make sure if you use this method that you remember to clear the previous
        transformation between frames.
        """
        self.vel_dict[sound]=None

    def getSoundVelocity(self, sound):
        """
        Get the velocity of the sound.
        """
        if sound in self.vel_dict:
            vel = self.vel_dict[sound]
            if vel is not None:
                return vel

            for known_object in list(self.sound_dict.keys()):
                if self.sound_dict[known_object].count(sound):
                    node_path = known_object.getNodePath()
                    if not node_path:
                        # The node has been deleted.
                        del self.sound_dict[known_object]
                        continue

                    clock = ClockObject.getGlobalClock()
                    return node_path.getPosDelta(self.root) / clock.getDt()

        return VBase3(0, 0, 0)

    def setListenerVelocity(self, velocity):
        """
        Set the velocity vector (in units/sec) of the listener, for calculating doppler shift.
        This is relative to the sound root (probably render).
        Default: VBase3(0, 0, 0)
        """
        if isinstance(velocity, tuple) and len(velocity) == 3:
            velocity = VBase3(*velocity)
        if not isinstance(velocity, VBase3):
            raise TypeError("Invalid argument 0, expected <VBase3>")
        self.listener_vel = velocity

    def setListenerVelocityAuto(self):
        """
        If velocity is set to auto, the velocity will be determined by the
        previous position of the object the listener is attached to and the frame dt.
        Make sure if you use this method that you remember to clear the previous
        transformation between frames.
        """
        self.listener_vel = None

    def getListenerVelocity(self):
        """
        Get the velocity of the listener.
        """
        if self.listener_vel is not None:
            return self.listener_vel
        elif self.listener_target is not None:
            clock = ClockObject.getGlobalClock()
            return self.listener_target.getPosDelta(self.root) / clock.getDt()
        else:
            return VBase3(0, 0, 0)

    def attachSoundToObject(self, sound, object):
        """
        Sound will come from the location of the object it is attached to.
        If the object is deleted, the sound will automatically be removed.
        """
        # sound is an AudioSound
        # object is any Panda object with coordinates
        for known_object in list(self.sound_dict.keys()):
            if self.sound_dict[known_object].count(sound):
                # This sound is already attached to something
                #return 0
                # detach sound
                self.sound_dict[known_object].remove(sound)
                if len(self.sound_dict[known_object]) == 0:
                    # if there are no other sounds, don't track
                    # the object any more
                    del self.sound_dict[known_object]

        if object not in self.sound_dict:
            self.sound_dict[WeakNodePath(object)] = []

        self.sound_dict[object].append(sound)
        return 1


    def detachSound(self, sound):
        """
        sound will no longer have it's 3D position updated
        """
        for known_object in list(self.sound_dict.keys()):
            if self.sound_dict[known_object].count(sound):
                self.sound_dict[known_object].remove(sound)
                if len(self.sound_dict[known_object]) == 0:
                    # if there are no other sounds, don't track
                    # the object any more
                    del self.sound_dict[known_object]
                return 1
        return 0


    def getSoundsOnObject(self, object):
        """
        returns a list of sounds attached to an object
        """
        if object not in self.sound_dict:
            return []
        sound_list = []
        sound_list.extend(self.sound_dict[object])
        return sound_list


    def attachListener(self, object):
        """
        Sounds will be heard relative to this object. Should probably be the camera.
        """
        self.listener_target = object
        return 1


    def detachListener(self):
        """
        Sounds will be heard relative to the root, probably render.
        """
        self.listener_target = None
        return 1


    def update(self, task=None):
        """
        Updates position of sounds in the 3D audio system. Will be called automatically
        in a task.
        """
        # Update the positions of all sounds based on the objects
        # to which they are attached

        # The audio manager is not active so do nothing
        if hasattr(self.audio_manager, "getActive"):
            if self.audio_manager.getActive()==0:
                return Task.cont

        for known_object, sounds in list(self.sound_dict.items()):
            node_path = known_object.getNodePath()
            if not node_path:
                # The node has been deleted.
                del self.sound_dict[known_object]
                continue

            pos = node_path.getPos(self.root)

            for sound in sounds:
                vel = self.getSoundVelocity(sound)
                sound.set3dAttributes(pos[0], pos[1], pos[2], vel[0], vel[1], vel[2])

        # Update the position of the listener based on the object
        # to which it is attached
        if self.listener_target:
            pos = self.listener_target.getPos(self.root)
            forward = self.root.getRelativeVector(self.listener_target, Vec3.forward())
            up = self.root.getRelativeVector(self.listener_target, Vec3.up())
            vel = self.getListenerVelocity()
            self.audio_manager.audio3dSetListenerAttributes(pos[0], pos[1], pos[2], vel[0], vel[1], vel[2], forward[0], forward[1], forward[2], up[0], up[1], up[2])
        else:
            self.audio_manager.audio3dSetListenerAttributes(0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1)
        return Task.cont

    def disable(self):
        """
        Detaches any existing sounds and removes the update task
        """
        taskMgr.remove("Audio3DManager-updateTask")
        self.detachListener()
        for object in list(self.sound_dict.keys()):
            for sound in self.sound_dict[object]:
                self.detachSound(sound)

    #snake_case alias:
    get_doppler_factor = getDopplerFactor
    set_listener_velocity_auto = setListenerVelocityAuto
    attach_listener = attachListener
    set_distance_factor = setDistanceFactor
    attach_sound_to_object = attachSoundToObject
    get_drop_off_factor = getDropOffFactor
    set_doppler_factor = setDopplerFactor
    get_sounds_on_object = getSoundsOnObject
    set_sound_velocity_auto = setSoundVelocityAuto
    get_sound_max_distance = getSoundMaxDistance
    load_sfx = loadSfx
    get_distance_factor = getDistanceFactor
    set_listener_velocity = setListenerVelocity
    set_sound_max_distance = setSoundMaxDistance
    get_sound_velocity = getSoundVelocity
    get_listener_velocity = getListenerVelocity
    set_sound_velocity = setSoundVelocity
    set_sound_min_distance = setSoundMinDistance
    get_sound_min_distance = getSoundMinDistance
    detach_listener = detachListener
    set_drop_off_factor = setDropOffFactor
    detach_sound = detachSound

