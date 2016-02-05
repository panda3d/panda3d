
class CachedDOData:
    # base class for objects that are used to store data in the CRDataCache
    #
    # stores a minimal set of cached data for DistributedObjects between instantiations

    def __init__(self):
        # override and store cached data
        # this object now owns the data
        # ownership will either pass back to another instantion of the object,
        # or the data will be flushed
        pass

    def destroy(self):
        # override and handle this object being destroyed
        # this is destruction of this object, not the cached data (see flush)
        pass

    def flush(self):
        # override and destroy the cached data
        # cached data is typically created by the DistributedObject and destroyed here
        pass
