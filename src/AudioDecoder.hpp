#ifndef AUDIODECODER_HPP
#define AUDIODECODER_HPP

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswresample/swresample.h>
#include <libavutil/channel_layout.h>
#include <libavutil/opt.h>
#include <libavutil/samplefmt.h>
}

#include <QString>

class AudioDecoder {
public:
    AudioDecoder(const QString& inputPath, const QString& outputPath);
    ~AudioDecoder();

    bool decode();

private:
    bool init();
    void cleanup();

    QString m_inputPath;
    QString m_outputPath;

    AVFormatContext* m_formatContext = nullptr;
    AVCodecContext* m_codecContext = nullptr;
    AVPacket* m_packet = nullptr;
    AVFrame* m_frame = nullptr;

    SwrContext* m_swrContext = nullptr;

    AVChannelLayout m_outputChannelLayout = {};
    AVSampleFormat m_outputSampleFormat = AV_SAMPLE_FMT_S16;
    int m_outputSampleRate = 0;

    int m_streamIndex = -1;
};

#endif // AUDIODECODER_HPP
