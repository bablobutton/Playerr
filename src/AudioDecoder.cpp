#include "AudioDecoder.hpp"
#include <QDebug>

AudioDecoder::AudioDecoder() = default;

AudioDecoder::~AudioDecoder() {
    close();
}

bool AudioDecoder::open(const QString& inputPath) {
    m_inputPath = inputPath;

    if (avformat_open_input(&m_formatContext, m_inputPath.toUtf8().constData(), nullptr, nullptr) < 0) {
        qWarning() << "ERROR: Failed to open input file";
        return false;
    }
    if (avformat_find_stream_info(m_formatContext, nullptr) < 0) {
        qWarning() << "ERROR: Failed to find stream info";
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
        qWarning() << "ERROR: Decoder not found";
        return false;
    }

    m_codecContext = avcodec_alloc_context3(codec);
    if (!m_codecContext || avcodec_parameters_to_context(m_codecContext, stream->codecpar) < 0) {
        qWarning() << "ERROR: Failed to copy codec parameters";
        return false;
    }
    if (avcodec_open2(m_codecContext, codec, nullptr) < 0) {
        qWarning() << "ERROR: Failed to open codec";
        return false;
    }

    // ensure input layout is set
    if (m_codecContext->ch_layout.order == AV_CHANNEL_ORDER_UNSPEC) {
        av_channel_layout_default(&m_codecContext->ch_layout, m_codecContext->ch_layout.nb_channels);
    }
    av_channel_layout_copy(&m_inputChannelLayout, &m_codecContext->ch_layout);

    // save output params and init default layout
    m_outputSampleRate   = m_codecContext->sample_rate;
    m_outputChannels     = m_codecContext->ch_layout.nb_channels;
    av_channel_layout_default(&m_outputChannelLayout, m_outputChannels);

    if (!initResampler())
        return false;

    m_packet = av_packet_alloc();
    m_frame  = av_frame_alloc();
    m_endOfFile = false;

    return true;
}

bool AudioDecoder::initResampler() {
    if (!m_codecContext)
        return false;

    if (m_swrContext)
        swr_free(&m_swrContext);

    if (swr_alloc_set_opts2(&m_swrContext,
                            &m_outputChannelLayout,
                            m_outputSampleFormat,
                            m_outputSampleRate,
                            &m_inputChannelLayout,
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
    return true;
}

int AudioDecoder::decode(uint8_t* outputBuffer, int outputBufferSize) {
    if (!m_formatContext || !m_codecContext || !m_swrContext)
        return -1;

    while (true) {
        if (!m_endOfFile) {
            if (av_read_frame(m_formatContext, m_packet) < 0) {
                m_endOfFile = true;
                avcodec_send_packet(m_codecContext, nullptr);
            } else if (m_packet->stream_index == m_streamIndex) {
                avcodec_send_packet(m_codecContext, m_packet);
                av_packet_unref(m_packet);
            } else {
                av_packet_unref(m_packet);
                continue;
            }
        }

        int ret = avcodec_receive_frame(m_codecContext, m_frame);
        if (ret == AVERROR(EAGAIN))
            continue;
        if (ret == AVERROR_EOF)
            return 0;
        if (ret < 0) {
            qWarning() << "ERROR: Failed to receive frame";
            return -1;
        }

        // resample into outputBuffer
        int maxSamples = outputBufferSize / av_get_bytes_per_sample(m_outputSampleFormat);
        int samplesConverted = swr_convert(m_swrContext,
                                           &outputBuffer,
                                           maxSamples,
                                           (const uint8_t**)m_frame->data,
                                           m_frame->nb_samples);
        av_frame_unref(m_frame);
        if (samplesConverted < 0) {
            qWarning() << "ERROR: Failed to resample";
            return -1;
        }
        return samplesConverted * m_outputChannels * av_get_bytes_per_sample(m_outputSampleFormat);
    }
}

void AudioDecoder::close() {
    if (m_frame)         av_frame_free(&m_frame);
    if (m_packet)        av_packet_free(&m_packet);
    if (m_swrContext)    swr_free(&m_swrContext);
    if (m_codecContext)  avcodec_free_context(&m_codecContext);
    if (m_formatContext) avformat_close_input(&m_formatContext);

    av_channel_layout_uninit(&m_inputChannelLayout);
    av_channel_layout_uninit(&m_outputChannelLayout);
}

int AudioDecoder::bitrate() const {
    return m_formatContext ? m_formatContext->bit_rate : 0;
}

qint64 AudioDecoder::durationUs() const {
    return (m_formatContext && m_formatContext->duration != AV_NOPTS_VALUE)
           ? m_formatContext->duration
           : 0;
}

QString AudioDecoder::sampleFormatName() const {
    return QString::fromUtf8(av_get_sample_fmt_name(m_outputSampleFormat));
}