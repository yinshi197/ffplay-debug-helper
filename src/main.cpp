#include <cmath>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "datactl.h"

#undef main

extern int ffplay_main(int argc, char **argv);

namespace {

std::string format_error(int errnum) {
    char buf[AV_ERROR_MAX_STRING_SIZE]{};
    av_strerror(errnum, buf, sizeof(buf));
    return std::string(buf);
}

std::string format_version(unsigned version) {
    std::ostringstream oss;
    unsigned major = version >> 16;
    unsigned minor = (version >> 8) & 0xFF;
    unsigned patch = version & 0xFF;
    oss << major << '.' << minor << '.' << patch;
    return oss.str();
}

std::string format_duration(int64_t duration) {
    if (duration < 0 || duration == AV_NOPTS_VALUE)
        return "unknown";
    const double total_seconds = duration / static_cast<double>(AV_TIME_BASE);
    int hours = static_cast<int>(total_seconds / 3600);
    int minutes = static_cast<int>((total_seconds - hours * 3600) / 60);
    const double seconds_component = total_seconds - hours * 3600 - minutes * 60;
    int whole_seconds = static_cast<int>(seconds_component);
    int milliseconds = static_cast<int>(std::round((seconds_component - whole_seconds) * 1000));

    if (milliseconds == 1000) {
        milliseconds = 0;
        ++whole_seconds;
        if (whole_seconds == 60) {
            whole_seconds = 0;
            ++minutes;
            if (minutes == 60) {
                minutes = 0;
                ++hours;
            }
        }
    }

    std::ostringstream oss;
    oss << std::setfill('0') << std::setw(2) << hours << ':'
        << std::setw(2) << minutes << ':'
        << std::setw(2) << whole_seconds << '.'
        << std::setw(3) << milliseconds;
    return oss.str();
}

void print_usage(const char *program_name) {
    std::cout << "Usage: " << program_name << " [--check-deps] [--probe <media>] <ffplay options>\n"
              << "\n"
              << "Additional helper options:\n"
              << "  --check-deps        Verify FFmpeg/SDL versions and initialization\n"
              << "  --probe <media>     Print container/stream metadata without starting playback\n"
              << "  -h, --help          Show this help message\n"
              << "\n"
              << "All unrecognized arguments are forwarded to the original ffplay entry point.\n";
}

bool run_dependency_check() {
    std::cout << "== Dependency check ==\n";
    std::cout << "FFmpeg configuration: " << avcodec_configuration() << "\n";
    std::cout << "Versions:\n";
    std::cout << "  libavformat: " << format_version(avformat_version()) << "\n";
    std::cout << "  libavcodec : " << format_version(avcodec_version()) << "\n";
    std::cout << "  libavutil  : " << format_version(avutil_version()) << "\n";
    std::cout << "  libswresample: " << format_version(swresample_version()) << "\n";
    std::cout << "  libswscale : " << format_version(swscale_version()) << "\n";
    std::cout << "  libavfilter: " << format_version(avfilter_version()) << "\n";

    SDL_version compiled{};
    SDL_version linked{};
    SDL_VERSION(&compiled);
    SDL_GetVersion(&linked);
    std::cout << "SDL versions:\n";
    std::cout << "  Compiled against: " << static_cast<int>(compiled.major) << '.'
              << static_cast<int>(compiled.minor) << '.'
              << static_cast<int>(compiled.patch) << "\n";
    std::cout << "  Linked at runtime: " << static_cast<int>(linked.major) << '.'
              << static_cast<int>(linked.minor) << '.'
              << static_cast<int>(linked.patch) << "\n";

    if (SDL_Init(0) != 0) {
        std::cerr << "SDL initialization failed: " << SDL_GetError() << "\n";
        return false;
    }
    SDL_Quit();

    const int net_init = avformat_network_init();
    if (net_init < 0) {
        std::cerr << "Warning: avformat_network_init failed: " << format_error(net_init) << "\n";
    } else {
        std::cout << "Network components available." << "\n";
        avformat_network_deinit();
    }

    std::cout << "Dependency check complete." << "\n";
    return true;
}

void print_metadata(const AVDictionary *metadata, const std::string &indent) {
    AVDictionaryEntry *entry = nullptr;
    while ((entry = av_dict_get(metadata, "", entry, AV_DICT_IGNORE_SUFFIX))) {
        std::cout << indent << entry->key << ": " << entry->value << "\n";
    }
}

bool probe_media(const std::string &path) {
    std::cout << "== Probe: " << path << " ==\n";
    AVFormatContext *fmt = nullptr;
    int ret = avformat_open_input(&fmt, path.c_str(), nullptr, nullptr);
    if (ret < 0) {
        std::cerr << "Unable to open input: " << format_error(ret) << "\n";
        return false;
    }

    ret = avformat_find_stream_info(fmt, nullptr);
    if (ret < 0) {
        std::cerr << "Unable to retrieve stream information: " << format_error(ret) << "\n";
        avformat_close_input(&fmt);
        return false;
    }

    if (fmt->iformat) {
        std::cout << "Format: " << fmt->iformat->name;
        if (fmt->iformat->long_name)
            std::cout << " (" << fmt->iformat->long_name << ")";
        std::cout << "\n";
    }

    if (fmt->duration != AV_NOPTS_VALUE)
        std::cout << "Duration: " << format_duration(fmt->duration) << "\n";
    if (fmt->bit_rate > 0)
        std::cout << "Bitrate: " << fmt->bit_rate / 1000 << " kb/s\n";
    if (fmt->start_time != AV_NOPTS_VALUE)
        std::cout << "Start time: " << fmt->start_time / static_cast<double>(AV_TIME_BASE) << " s\n";

    if (fmt->metadata) {
        std::cout << "Container metadata:\n";
        print_metadata(fmt->metadata, "  ");
    }

    for (unsigned i = 0; i < fmt->nb_streams; ++i) {
        const AVStream *stream = fmt->streams[i];
        const AVCodecParameters *par = stream->codecpar;
        const char *type = av_get_media_type_string(par->codec_type);
        if (!type)
            type = "unknown";

        const AVCodec *codec = avcodec_find_decoder(par->codec_id);
        const char *codec_name = codec && codec->long_name ? codec->long_name : avcodec_get_name(par->codec_id);

        std::cout << "\nStream #" << i << " (" << type << ")\n";
        std::cout << "  Codec: " << codec_name;
        if (codec_name && codec_name[0])
            std::cout << " (" << avcodec_get_name(par->codec_id) << ")";
        std::cout << "\n";

        if (par->codec_type == AVMEDIA_TYPE_VIDEO) {
            std::cout << "  Resolution: " << par->width << "x" << par->height << "\n";
            if (stream->avg_frame_rate.num && stream->avg_frame_rate.den) {
                std::ostringstream fps_stream;
                fps_stream << std::fixed << std::setprecision(3) << av_q2d(stream->avg_frame_rate);
                std::cout << "  Avg FPS: " << fps_stream.str() << "\n";
            }
            if (par->format != AV_PIX_FMT_NONE)
                std::cout << "  Pixel format: " << av_get_pix_fmt_name(static_cast<AVPixelFormat>(par->format)) << "\n";
        } else if (par->codec_type == AVMEDIA_TYPE_AUDIO) {
            std::cout << "  Sample rate: " << par->sample_rate << " Hz\n";
            std::cout << "  Channels: " << par->ch_layout.nb_channels << "\n";
            char layout[256];
            if (par->ch_layout.nb_channels > 0 &&
                av_channel_layout_describe(&par->ch_layout, layout, sizeof(layout)) >= 0) {
                std::cout << "  Layout: " << layout << "\n";
            }
            if (par->format != AV_SAMPLE_FMT_NONE)
                std::cout << "  Sample format: " << av_get_sample_fmt_name(static_cast<AVSampleFormat>(par->format)) << "\n";
        } else if (par->codec_type == AVMEDIA_TYPE_SUBTITLE) {
            std::cout << "  Subtitle codec ID: " << par->codec_id << "\n";
        }

        if (stream->time_base.den)
            std::cout << "  Time base: " << stream->time_base.num << '/' << stream->time_base.den << "\n";

        if (stream->metadata) {
            std::cout << "  Metadata:\n";
            print_metadata(stream->metadata, "    ");
        }
    }

    avformat_close_input(&fmt);
    std::cout << std::endl;
    return true;
}

} // namespace

int main(int argc, char **argv) {
    std::vector<char *> forwarded_args;
    forwarded_args.reserve(argc + 1);
    forwarded_args.push_back(argv[0]);

    bool requested_help = false;
    bool performed_action = false;
    bool dependency_check = false;
    std::vector<std::string> probe_paths;

    for (int i = 1; i < argc; ++i) {
        const char *arg = argv[i];
        if (!std::strcmp(arg, "--")) {
            for (++i; i < argc; ++i)
                forwarded_args.push_back(argv[i]);
            break;
        }
        if (!std::strcmp(arg, "--help") || !std::strcmp(arg, "-h")) {
            requested_help = true;
            continue;
        }
        if (!std::strcmp(arg, "--check-deps")) {
            dependency_check = true;
            continue;
        }
        if (!std::strcmp(arg, "--probe")) {
            if (i + 1 >= argc) {
                std::cerr << "--probe requires a media path" << std::endl;
                return 1;
            }
            probe_paths.emplace_back(argv[++i]);
            performed_action = true;
            continue;
        }
        forwarded_args.push_back(argv[i]);
    }

    if (requested_help) {
        print_usage(argv[0]);
        performed_action = true;
    }

    if (dependency_check) {
        if (!run_dependency_check())
            return 1;
        performed_action = true;
    }

    for (const auto &path : probe_paths) {
        if (!probe_media(path))
            return 1;
    }

    if (forwarded_args.size() > 1) {
        forwarded_args.push_back(nullptr);
        return ffplay_main(static_cast<int>(forwarded_args.size()) - 1, forwarded_args.data());
    }

    if (!performed_action) {
        print_usage(argv[0]);
    }

    return 0;
}

