#ifndef LIBAV_UTILS_H
#define LIBAV_UTILS_H

#include "libav_base.h"

struct AVCodec;
struct AVCodecContext;
struct AVCodecParameters;
struct AVDictionary;
struct AVStream;

namespace ffmpeg
{

namespace utils
{

libav_option_list_t parse_option_list(const std::string& options);
libav_option_map_t parse_option_map(const std::string& options);

bool is_global_header_format(const std::string& format_name);

extra_data_t extract_global_header(const stream_info_t& stream_info);

device_type_t fetch_device_type(const std::string& uri);

url_format_t fetch_url_format(const std::string& url);

void merge_codec_params(AVCodecContext& av_context
                            , codec_params_t& codec_params);

void set_options(AVDictionary** av_options
                 , const std::string& options);

void set_options(AVDictionary** av_options
                 , const libav_option_map_t& params);

std::string error_string(std::int32_t av_errno);


}


AVCodecContext& operator << (AVCodecContext& av_context
                             , const media_info_t& media_info);
AVCodecParameters& operator << (AVCodecParameters& av_codecpar
                                , const media_info_t& media_info);

AVCodecContext& operator >> (const media_info_t& media_info
                             , AVCodecContext& av_context);
AVCodecParameters& operator >> (const media_info_t& media_info
                                , AVCodecParameters& av_codecpar);

media_info_t& operator << (media_info_t& media_info
                             , const AVCodecContext& av_context);
media_info_t& operator << (media_info_t& media_info
                                , const AVCodecParameters& av_codecpar);

media_info_t& operator >> (const AVCodecContext& av_context
                           , media_info_t& media_info);
media_info_t& operator >> (const AVCodecParameters& av_codecpar
                           , media_info_t& media_info);

AVStream& operator << (AVStream& av_stream
                       , const stream_info_t& stream_info);

stream_info_t& operator << (stream_info_t& stream_info
                            , const AVStream& av_stream);

}

#endif // LIBAV_UTILS_H
