import pandac.AudioSound
import pandac.AudioManager

class Audio3DManager:

    def __init__(self, audio_manager, listener_target = None):
        self.audio_manager = audio_manager
        self.listener_target = listener_target

        self.sound_dict = {}        


    def attachSoundToObject(self, sound, object):
        # sound is an AudioSound
        # object is any Panda object with coordinates
        for known_object in self.sound_dict.keys():
            if sound_dict[known_object].count(sound):
                # This sound is already attached to something
                return 0
                
        if not self.sound_dict.has_key(object):
            self.sound_dict[object] = []
            
        self.sound_dict[object].append(sound)
        return 1


    def detachSound(self, sound):
        for known_object in self.sound_dict.keys():
            if sound_dict[known_object].count(sound):
                self.sound_dict[known_object].remove()
                if len(sound_dict[known_object]) == 0:
                    # if there are no other sounds, don't track
                    # the object any more
                    del self.sound_dict[known_object]
                return 1
        return 0


    def getSoundsOnObject(self, object):
        if not self.sound_dict.has_key(object):
            return []
        sound_list = []
        sound_list.extend(self.sound_dict[object])
        return sound_list
    

    def attachListener(self, object):
        self.listener_target = object
        return 1


    def detachListener(self):
        self.listener_target = None
        return 1


    def update(self):
        # Update the positions of all sounds based on the objects
        # to which they are attached
        for known_object in self.sound_dict.keys():
            tracked_sound = 0
            while tracked_sound < len(self.sound_dict[known_object]):
                sound = self.sound_dict[known_object][tracked_sound]
                if self.listener_target:
                    pos = known_object.getPos(self.listener_target)
                else:
                    pos = known_object.getPos()
                sound.set3dAttributes(pos[0], pos[1], pos[2], 0,0,0)
                tracked_sound += 1

        # Update the position of the listener based on the object
        # to which it is attached
        if self.listener_target:
            pos = self.listener_target.getPos()
            self.audio_manager.audio3dSetListenerAttributes(pos[0], pos[1], pos[2], 0,0,0, 0,1,0, 0,0,1)
        else:
            self.audio_manager.audio3dSetListenerAttributes(0,0,0, 0,0,0, 0,1,0, 0,0,1)
        self.audio_manager.audio3dUpdate()
        return 1
