"""LargeBlobSenderConsts module"""

USE_DISK  = 0x01

ChunkSize = 100

FilePattern = 'largeBlob.%s'

def getLargeBlobPath():
    return config.GetString('large-blob-path', 'i:\beta\largeblob')
