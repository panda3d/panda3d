/**
 * flac test for panda3d
 * 
 * @file flac_test.cpp
 * @author Stavros P.
 * @date 2025-03-05
 */
#define DR_FLAC_IMPLEMENTATION
#include "dr_flac.h"
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <memory>
#include <chrono>

constexpr const char* test_std_flac = "test_stereo_44100.flac";
constexpr const char* test_ogg_flac = "test_stereo_44100.oga";
constexpr const char* test_24bit_flac = "test_24bit_48000.flac";
constexpr const char* test_corrupted = "corrupted.flac";

#define test_check(condition, message) \
    if (!(condition)) { \
        std::cerr << "test failed: " << (message) << " (" << __FILE__ << ":" << __LINE__ << ")\n"; \
        return false; \
    }

static size_t total_allocated = 0;
static size_t allocation_count = 0;

void* test_malloc(size_t sz, void*) {
    total_allocated += sz;
    allocation_count++;
    return malloc(sz);
}

void* test_realloc(void* p, size_t sz, void*) {
    return realloc(p, sz);
}

void test_free(void* p, void*) {
    free(p);
}

// i dont remember
drflac_bool32 test_metadata_handler(void* puserdata, drflac_metadata* pmetadata) {
    auto* count = static_cast<int*>(puserdata);
    (*count)++;
    return DRFLAC_TRUE;
}

bool test_basic_decoding(const char* filename, unsigned expected_channels, unsigned expected_samplerate) {
    drflac* pflac = drflac_open_file(filename, nullptr);
    test_check(pflac != nullptr, "failed to open file");
    
    test_check(pflac->channels == expected_channels, "channel count mismatch");
    test_check(pflac->sampleRate == expected_samplerate, "sample rate mismatch");

    std::vector<int16_t> buffer(pflac->totalPCMFrameCount * pflac->channels);
    const uint64_t frames_read = drflac_read_pcm_frames_s16(pflac, pflac->totalPCMFrameCount, buffer.data());
    test_check(frames_read == pflac->totalPCMFrameCount, "incomplete read");

    drflac_close(pflac);
    return true;
}

bool test_ogg_flac_support() {
    if (!test_basic_decoding(test_ogg_flac, 2, 44100)) return false;

    auto decode_entire_file = [](const char* filename) -> std::vector<int16_t> {
        drflac* pflac = drflac_open_file(filename, nullptr);
        if (!pflac) return {};
        
        // for clarity
        std::cout << filename << " frames: " << pflac->totalPCMFrameCount << "\n";
        
        std::vector<int16_t> result(pflac->totalPCMFrameCount * pflac->channels);
        drflac_read_pcm_frames_s16(pflac, pflac->totalPCMFrameCount, result.data());
        drflac_close(pflac);
        return result;
    };

    const auto std_data = decode_entire_file(test_std_flac);
    const auto ogg_data = decode_entire_file(test_ogg_flac);

    constexpr uint64_t expected_frames = 44100 * 5;  
    constexpr uint64_t expected_samples = expected_frames * 2;
    
    test_check(std_data.size() == expected_samples, "standard flac size mismatch");
    test_check(ogg_data.size() == expected_samples, "ogg-flac size mismatch");
    
    // verify actual sample data matches
    test_check(memcmp(std_data.data(), ogg_data.data(), expected_samples * sizeof(int16_t)) == 0,
              "audio data mismatch");

    return true;
}

// main test runner
int main() {
    using namespace std::chrono;
    
    bool all_passed = true;
    auto test_start = high_resolution_clock::now();

    all_passed &= test_basic_decoding(test_std_flac, 2, 44100);
    all_passed &= test_ogg_flac_support();

    auto duration = duration_cast<milliseconds>(high_resolution_clock::now() - test_start);
    
    if (all_passed) {
        std::cout << "all flac tests passed in " << duration.count() << "ms\n";
        return EXIT_SUCCESS;
    } else {
        std::cout << "some tests failed\n";
        return EXIT_FAILURE;
    }
}