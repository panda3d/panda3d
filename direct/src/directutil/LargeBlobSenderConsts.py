"""LargeBlobSenderConsts module"""

USE_DISK  = 0x01

ChunkSize = 100

FilePattern = 'largeBlob.%s'

def getLargeBlobPath():
    path = config.GetString('large-blob-path', '')
    if len(path) == 0:
        assert 0, (
            'you must config large-blob-path to beta/largeblob, i.e.\n'
            'large-blob-path i:\\beta\\largeblob')
    return path
