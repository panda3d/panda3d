from direct.directnotify import DirectNotifyGlobal

# TODO: add callback mechanism when values change.
# Should we announce every change through the messenger?
# Should you be able to hang a hook on a particular name?

class BulletinBoard:
    """This class implements a global location for key/value pairs to be
    stored. Intended to prevent coders from putting global variables directly
    on showbase, so that potential name collisions can be more easily
    detected."""
    notify = DirectNotifyGlobal.directNotify.newCategory('BulletinBoard')

    def __init__(self):
        self._dict = {}

    def get(self, name):
        return self._dict.get(name)

    def has(self, name):
        return name in self._dict

    def post(self, name, value=None):
        if name in self._dict:
            BulletinBoard.notify.warning('changing %s from %s to %s' % (
                name, self._dict[name], value))
        self.update(name, value)

    def update(self, name, value):
        """can use this to set value the first time"""
        if name in self._dict:
            BulletinBoard.notify.info('update: posting %s' % (name))
        self._dict[name] = value
        
    def remove(self, name):
        if name in self._dict:
            del self._dict[name]
        
    def __repr__(self):
        return str(self._dict)
