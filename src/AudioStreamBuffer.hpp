#pragma once

#include "AudioDecoder.hpp"
#include <vector>
#include <cstdint>

class AudioStreamBuffer {
public:
    explicit AudioStreamBuffer(AudioDecoder& decoder, double bufferDurationSec = 2.0);

    bool fill(); // наполняет буфер новыми данными
    bool getSamples(uint8_t* outBuffer, size_t bytesToRead); // копирует samples в outBuffer

    bool isEmpty() const;
    bool isEof() const;

    int sampleRate() const;
    int channels() const;
    int bytesPerSample() const;

private:
    AudioDecoder& m_decoder;

    std::vector<uint8_t> m_buffer;
    size_t m_bufferSize = 0;
    size_t m_readPos = 0;
    size_t m_writePos = 0;
    bool m_eof = false;

    size_t m_frameSize = 0; // bytes per frame (channels × bytes_per_sample)

    void writeToBuffer(const uint8_t* data, size_t size);
    size_t availableSamples() const;
};
