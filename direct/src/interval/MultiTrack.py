"""MultiTrack module: contains the MultiTrack class"""

import Interval


class MultiTrack(Interval.Interval):
    # Name counter
    multiTrackNum = 1
    # Class methods
    def __init__(self, trackList, name=None):
        """__init__(trackList, name)
        """
        # Record track list
        self.tlist = trackList
        # Generate name if necessary
        if (name == None):
            name = 'MultiTrack-%d' % MultiTrack.multiTrackNum
            MultiTrack.multiTrackNum = MultiTrack.multiTrackNum + 1
        # Duration is max of all track durations
        duration = self.__computeDuration()
        # Initialize superclass
        Interval.Interval.__init__(self, name, duration)
        # Update stopEventList after initialization
        # It is the union of the stopEventLists of all tracks in the MultiTrack
        for t in self.tlist:
            self.stopEventList = self.stopEventList + t.stopEventList

    # Access track at given index
    def __getitem__(self, item):
        return self.tlist[item]
    
    def __computeDuration(self):
        """ __computeDuration()
            Returns the duration of the longest Track 
        """
        duration = 0.0
        for t in self.tlist:
            dur = t.getDuration()
            if (dur > duration):
                duration = dur
        return duration

    def updateFunc(self, t, event = Interval.IVAL_NONE):
        """ updateFunc(t, event)
            Go to time t
        """
        
        for track in self.tlist:
            # We used to try to be smart about calling this only in
            # certain cases, but in fact we just called it in every
            # case anyway, so might as well eliminate all of the
            # comparisons.
            track.setT(t, event)

    # Print out representation of MultiTrack
    def __repr__(self, indent=0):
        """ __repr__(indent)
        """
        str = Interval.Interval.__repr__(self, indent) + '\n'
        for t in self.tlist:
            str = str + t.__repr__(indent+1)
        return str









class Parallel(MultiTrack):
    def __init__(self, *tracks, **kw):
        MultiTrack.__init__(self, tracks, **kw)
