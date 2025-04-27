#pragma once

#include <QString>

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/samplefmt.h>
#include <libavutil/channel_layout.h>
#include <libswresample/swresample.h>
}

class AudioDecoder {
public:
    AudioDecoder();
    ~AudioDecoder();

    bool open(const QString& inputPath);
    int decode(uint8_t* outputBuffer, int outputBufferSize);
    void close();

    int sampleRate() const { return m_outputSampleRate; }
    int channels()   const { return m_outputChannels; }
    int bytesPerSample() const { return av_get_bytes_per_sample(m_outputSampleFormat); }

    int bitrate() const;
    qint64 durationUs() const;
    QString sampleFormatName() const;

private:
    bool initResampler();

    QString m_inputPath;
    AVFormatContext* m_formatContext = nullptr;
    AVCodecContext*  m_codecContext  = nullptr;
    SwrContext*      m_swrContext    = nullptr;
    AVPacket*        m_packet        = nullptr;
    AVFrame*         m_frame         = nullptr;

    int m_streamIndex = -1;
    AVChannelLayout m_inputChannelLayout;
    AVChannelLayout m_outputChannelLayout;
    AVSampleFormat  m_outputSampleFormat = AV_SAMPLE_FMT_S16;
    int m_outputSampleRate = 0;
    int m_outputChannels   = 0;
    bool m_endOfFile       = false;
};