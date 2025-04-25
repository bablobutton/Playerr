#include "AudioDecoder.hpp"
#include <QDebug>
#include <QFile>

AudioDecoder::AudioDecoder(const QString& inputPath, const QString& outputPath)
    : m_inputPath(inputPath), m_outputPath(outputPath) {}

AudioDecoder::~AudioDecoder() {
    cleanup();
}

bool AudioDecoder::init() {
    m_formatContext = avformat_alloc_context();
    if (avformat_open_input(&m_formatContext, m_inputPath.toUtf8().constData(), nullptr, nullptr) != 0) {
        qWarning() << "ERROR: Can't open input file";
        return false;
    }

    if (avformat_find_stream_info(m_formatContext, nullptr) < 0) {
        qWarning() << "ERROR: Can't find stream info";
        return false;
    }

    m_streamIndex = av_find_best_stream(m_formatContext, AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0);
    if (m_streamIndex < 0) {
        qWarning() << "ERROR: No audio stream found";
        return false;
    }

    AVStream* stream = m_formatContext->streams[m_streamIndex];
    const AVCodec* codec = avcodec_find_decoder(stream->codecpar->codec_id);
    if (!codec) {
        qWarning() << "ERROR: No decoder found";
        return false;
    }

    m_codecContext = avcodec_alloc_context3(codec);
    if (avcodec_parameters_to_context(m_codecContext, stream->codecpar) < 0) {
        qWarning() << "ERROR: Failed to copy codec parameters to context";
        return false;
    }

    if (avcodec_open2(m_codecContext, codec, nullptr) < 0) {
        qWarning() << "ERROR: Failed to open codec";
        return false;
    }

    m_outputSampleFormat = AV_SAMPLE_FMT_S16;
    m_outputSampleRate = m_codecContext->sample_rate;

    if (m_codecContext->ch_layout.order == AV_CHANNEL_ORDER_UNSPEC) {
        av_channel_layout_default(&m_codecContext->ch_layout, m_codecContext->ch_layout.nb_channels);
    }

    av_channel_layout_default(&m_outputChannelLayout, m_codecContext->ch_layout.nb_channels);

    if (swr_alloc_set_opts2(&m_swrContext,
                            &m_outputChannelLayout,
                            m_outputSampleFormat,
                            m_outputSampleRate,
                            &m_codecContext->ch_layout,
                            m_codecContext->sample_fmt,
                            m_codecContext->sample_rate,
                            0, nullptr) < 0) {
        qWarning() << "ERROR: Failed to allocate resampler";
        return false;
    }

    if (swr_init(m_swrContext) < 0) {
        qWarning() << "ERROR: Failed to initialize resampler";
        return false;
    }

    m_packet = av_packet_alloc();
    m_frame = av_frame_alloc();

    return true;
}

bool AudioDecoder::decode() {
    if (!init()) return false;

    QFile file(m_outputPath);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "ERROR: Can't open output file";
        return false;
    }

    while (av_read_frame(m_formatContext, m_packet) == 0) {
        if (m_packet->stream_index != m_streamIndex) {
            av_packet_unref(m_packet);
            continue;
        }

        if (avcodec_send_packet(m_codecContext, m_packet) < 0) {
            av_packet_unref(m_packet);
            continue;
        }

        while (avcodec_receive_frame(m_codecContext, m_frame) == 0) {
            AVFrame* resampledFrame = av_frame_alloc();
            av_channel_layout_copy(&resampledFrame->ch_layout, &m_outputChannelLayout);
            resampledFrame->format = m_outputSampleFormat;
            resampledFrame->sample_rate = m_outputSampleRate;
            resampledFrame->nb_samples = m_frame->nb_samples;

            if (av_frame_get_buffer(resampledFrame, 0) < 0) {
                qWarning() << "ERROR: Failed to allocate buffer for resampled frame";
                av_frame_free(&resampledFrame);
                continue;
            }

            if (swr_convert_frame(m_swrContext, resampledFrame, m_frame) == 0) {
                int dataSize = av_samples_get_buffer_size(nullptr,
                                                          resampledFrame->ch_layout.nb_channels,
                                                          resampledFrame->nb_samples,
                                                          (AVSampleFormat)resampledFrame->format,
                                                          1);
                file.write(reinterpret_cast<char*>(resampledFrame->data[0]), dataSize);
            }

            av_frame_free(&resampledFrame);
            av_frame_unref(m_frame);
        }

        av_packet_unref(m_packet);
    }

    file.close();
    return true;
}

void AudioDecoder::cleanup() {
    if (m_frame) av_frame_free(&m_frame);
    if (m_packet) av_packet_free(&m_packet);
    if (m_codecContext) avcodec_free_context(&m_codecContext);
    if (m_formatContext) avformat_close_input(&m_formatContext);
    if (m_swrContext) swr_free(&m_swrContext);
    av_channel_layout_uninit(&m_outputChannelLayout);
}
