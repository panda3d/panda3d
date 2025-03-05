#define DR_FLAC_IMPLEMENTATION
#include "dr_flac.h"
#include <iostream>
#include <cstdint>

int main() {
    const char* filename = "./test.flac";
    drflac* pFlac = drflac_open_file(filename, NULL);
    
    if (!pFlac) {
        std::cerr << "Failed to open FLAC file\n";
        return 1;
    }
    
    std::cout << "Channels: " << pFlac->channels << "\n";
    std::cout << "Sample rate: " << pFlac->sampleRate << "\n";
    std::cout << "Total frames: " << pFlac->totalPCMFrameCount << "\n";
    
    // Increase buffer size for larger reads
    int16_t buffer[8192];
    
    // Read initial frames
    uint64_t framesRead = drflac_read_pcm_frames_s16(pFlac, 512, buffer);
    std::cout << "Read " << framesRead << " frames\n";
    
    // Seek to a specific frame and read again
    drflac_seek_to_pcm_frame(pFlac, 1000);
    framesRead = drflac_read_pcm_frames_s16(pFlac, 512, buffer);
    std::cout << "After seek, read " << framesRead << " frames\n";
    
    // Read more frames to test handling of larger buffers
    framesRead = drflac_read_pcm_frames_s16(pFlac, 2048, buffer);
    std::cout << "Read " << framesRead << " frames with larger buffer\n";
    
    // Seek to a different frame and test
    drflac_seek_to_pcm_frame(pFlac, 10000);
    framesRead = drflac_read_pcm_frames_s16(pFlac, 512, buffer);
    std::cout << "After seeking to frame 10000, read " << framesRead << " frames\n";
    
    // Read until EOF
    uint64_t totalFramesRead = 0;
    while (true) {
        framesRead = drflac_read_pcm_frames_s16(pFlac, 512, buffer);
        if (framesRead == 0) {
            break; // EOF reached
        }
        totalFramesRead += framesRead;
    }
    std::cout << "Total frames read: " << totalFramesRead << "\n";
    
    // Read a single sample from the buffer to test data
    std::cout << "First sample: " << buffer[0] << "\n";
    
    drflac_close(pFlac);
    return 0;
}
