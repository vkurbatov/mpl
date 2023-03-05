#ifndef MPL_AUDIO_FORMAT_IMPL_H
#define MPL_AUDIO_FORMAT_IMPL_H

#include "i_audio_format.h"
#include "core/i_parametrizable.h"
#include "core/option_impl.h"
#include <string>

namespace mpl::media
{

class audio_format_impl : public i_audio_format
        , public i_parametrizable
{
    audio_format_id_t       m_format_id;
    std::int32_t            m_sample_rate;
    std::int32_t            m_channels;
    option_impl             m_options;

public:
    using u_ptr_t = std::unique_ptr<audio_format_impl>;
    using s_ptr_t = std::shared_ptr<audio_format_impl>;

    static u_ptr_t create(const audio_format_id_t& format_id = audio_format_id_t::undefined
                          , std::int32_t sample_rate = 0
                          , std::int32_t channels = 0);

    static u_ptr_t create(const i_audio_format& other);
    static u_ptr_t create(const i_property& params);

    audio_format_impl(const audio_format_id_t& format_id = audio_format_id_t::undefined
                      , std::int32_t sample_rate = 0
                      , std::int32_t channels = 0);

    audio_format_impl(const i_audio_format& other);
    audio_format_impl(const i_property& params);

    audio_format_impl& set_format_id(const audio_format_id_t& format_id);
    audio_format_impl& set_sample_rate(std::int32_t sample_rate);
    audio_format_impl& set_channels(std::int32_t channels);
    audio_format_impl& set_options(const i_option& options);
    audio_format_impl& set_options(option_impl&& options);

    audio_format_impl& assign(const i_audio_format& other);

    bool set_params(const i_property& params) override;
    bool get_params(i_property& params) const override;
    i_property::u_ptr_t get_params(const std::string& path = {}) const;


    i_option& options();

    // i_media_format interface
public:
    media_type_t media_type() const override;
    bool is_encoded() const override;
    bool is_convertable() const override;
    i_media_format::u_ptr_t clone() const override;
    bool is_equal(const i_media_format &other) const override;
    bool is_compatible(const i_media_format &other) const override;
    const i_option& options() const override;
    bool is_valid() const override;

    // i_audio_format interface
public:
    audio_format_id_t format_id() const override;
    int32_t sample_rate() const override;
    int32_t channels() const override;
};

}

#endif // MPL_AUDIO_FORMAT_IMPL_H
