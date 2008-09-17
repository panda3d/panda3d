"""LargeBlobSenderConsts module"""

USE_DISK  = 0x01

ChunkSize = 100

FilePattern = 'largeBlob.%s'

def getLargeBlobPath():
    # this folder needs to be accessible by everyone that is going to level edit
    # an area as a group
    return config.GetString('large-blob-path', 'i:\\toontown_in_game_editor_temp')
