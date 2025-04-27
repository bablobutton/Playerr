#include "AudioStreamBuffer.hpp"
#include <cstring> // memcpy

AudioStreamBuffer::AudioStreamBuffer(AudioDecoder& decoder, double bufferDurationSec)
    : m_decoder(decoder)
{
    m_frameSize = m_decoder.channels() * m_decoder.bytesPerSample();
    m_bufferSize = static_cast<size_t>(m_decoder.sampleRate() * bufferDurationSec * m_frameSize);
    m_buffer.resize(m_bufferSize, 0);
}

bool AudioStreamBuffer::fill() {
    if (m_eof) return false;

    uint8_t temp[8192]; // временный маленький буфер для одного чтения

    while (availableSamples() < m_bufferSize / 2) {
        int bytesRead = m_decoder.decode(temp, sizeof(temp));
        if (bytesRead > 0) {
            writeToBuffer(temp, static_cast<size_t>(bytesRead));
        } else {
            m_eof = true;
            break;
        }
    }
    return true;
}

bool AudioStreamBuffer::getSamples(uint8_t* outBuffer, size_t bytesToRead) {
    if (isEmpty()) return false;

    size_t bytesAvailable = availableSamples();
    size_t bytesToCopy = (bytesToRead < bytesAvailable) ? bytesToRead : bytesAvailable;

    if (m_readPos + bytesToCopy <= m_bufferSize) {
        std::memcpy(outBuffer, &m_buffer[m_readPos], bytesToCopy);
        m_readPos += bytesToCopy;
    } else {
        size_t firstPart = m_bufferSize - m_readPos;
        size_t secondPart = bytesToCopy - firstPart;
        std::memcpy(outBuffer, &m_buffer[m_readPos], firstPart);
        std::memcpy(outBuffer + firstPart, &m_buffer[0], secondPart);
        m_readPos = secondPart;
    }

    if (m_readPos >= m_bufferSize) m_readPos = 0;
    return true;
}

bool AudioStreamBuffer::isEmpty() const {
    return m_readPos == m_writePos;
}

bool AudioStreamBuffer::isEof() const {
    return m_eof;
}

int AudioStreamBuffer::sampleRate() const {
    return m_decoder.sampleRate();
}

int AudioStreamBuffer::channels() const {
    return m_decoder.channels();
}

int AudioStreamBuffer::bytesPerSample() const {
    return m_decoder.bytesPerSample();
}

void AudioStreamBuffer::writeToBuffer(const uint8_t* data, size_t size) {
    if (size > m_bufferSize) {
        // слишком большой кусок, обрезаем
        data += size - m_bufferSize;
        size = m_bufferSize;
    }

    if (m_writePos + size <= m_bufferSize) {
        std::memcpy(&m_buffer[m_writePos], data, size);
        m_writePos += size;
    } else {
        size_t firstPart = m_bufferSize - m_writePos;
        size_t secondPart = size - firstPart;
        std::memcpy(&m_buffer[m_writePos], data, firstPart);
        std::memcpy(&m_buffer[0], data + firstPart, secondPart);
        m_writePos = secondPart;
    }

    if (m_writePos >= m_bufferSize) m_writePos = 0;
}

size_t AudioStreamBuffer::availableSamples() const {
    if (m_writePos >= m_readPos)
        return m_writePos - m_readPos;
    else
        return m_bufferSize - (m_readPos - m_writePos);
}
