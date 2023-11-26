#include "smart_transcoder_factory.h"

#include "utils/property_writer.h"
#include "utils/option_helper.h"

#include "audio_frame_impl.h"
#include "video_frame_impl.h"

#include "utils/task_manager_impl.h"

#include "media_option_types.h"

#include <queue>
#include "shared_mutex"

#include "tools/utils/sync_base.h"

#include "log/log_tools.h"

#include <cstring>
#include <iostream>

namespace mpl::media
{

namespace detail
{

template<media_type_t MediaType>
struct format_types_t;

template<>
struct format_types_t<media_type_t::audio>
{
    using i_format_t = i_audio_format;
    using i_frame_t = i_audio_frame;
    using format_impl_t = audio_format_impl;
    using frame_impl_t = audio_frame_impl;
};

template<>
struct format_types_t<media_type_t::video>
{
    using i_format_t = i_video_format;
    using i_frame_t = i_video_frame;
    using format_impl_t = video_format_impl;
    using frame_impl_t = video_frame_impl;
};

template<typename Format>
bool is_compatible_format(const Format& input_format
                          , const Format& output_format)
{
    return input_format.is_compatible(output_format);
}

void merge_format(const i_audio_format& input_format
                  , audio_format_impl& output_format)
{
    if (!output_format.is_valid())
    {
        output_format.options().merge(input_format.options());
        if (output_format.format_id() == audio_format_id_t::undefined)
        {
            output_format.set_format_id(input_format.format_id());
        }

        if (output_format.sample_rate() == 0)
        {
            output_format.set_sample_rate(input_format.sample_rate());
        }

        if (output_format.channels() == 0)
        {
            output_format.set_channels(input_format.channels());
        }

    }
}

void merge_format(const i_video_format& input_format
                  , video_format_impl& output_format)
{
    if (!output_format.is_valid())
    {

        output_format.options().merge(input_format.options());
        if (output_format.format_id() == video_format_id_t::undefined)
        {
            output_format.set_format_id(input_format.format_id());
        }

        if (output_format.width() == 0)
        {
            output_format.set_width(input_format.width());
        }

        if (output_format.height() == 0)
        {
            output_format.set_height(input_format.height());
        }

        if (output_format.frame_rate() == 0.0f)
        {
            output_format.set_frame_rate(input_format.frame_rate());
        }

    }
}


template<media_type_t MediaType>
class converter_manager
{
    using format_impl_t = typename detail::format_types_t<MediaType>::format_impl_t;

    i_media_converter_factory&  m_media_decoders;
    i_media_converter_factory&  m_media_encoders;
    i_media_converter_factory&  m_media_converters;

public:
    converter_manager(i_media_converter_factory& media_decoders
                      , i_media_converter_factory& media_encoders
                      , i_media_converter_factory& media_converters)
        : m_media_decoders(media_decoders)
        , m_media_encoders(media_encoders)
        , m_media_converters(media_converters)
    {

    }

    i_media_converter::u_ptr_t create_decoder(const format_impl_t& encode_format
                                              , i_message_sink* sink)
    {
        if (auto decoder_params = encode_format.get_params("format"))
        {
            if (auto decoder = m_media_decoders.create_converter(*decoder_params))
            {
                decoder->set_sink(sink);
                return decoder;
            }
        }

        return nullptr;
    }
    i_media_converter::u_ptr_t create_encoder(const format_impl_t& decode_format
                                              , i_message_sink* sink)
    {
        if (auto encoder_params = decode_format.get_params("format"))
        {
            if (auto encoder = m_media_encoders.create_converter(*encoder_params))
            {
                encoder->set_sink(sink);
                return encoder;
            }
        }

        return nullptr;

    }
    i_media_converter::u_ptr_t create_converter(const format_impl_t& convert_format
                                                , i_message_sink* sink)
    {
        if (auto converter_params = convert_format.get_params("format"))
        {
            if (auto converter = m_media_converters.create_converter(*converter_params))
            {
                converter->set_sink(sink);
                return converter;
            }
        }

        return nullptr;

    }
};

template<media_type_t MediaType>
class frame_sink : public i_message_sink
{
    using i_frame_t = typename format_types_t<MediaType>::i_frame_t;
    using frame_handler_t = std::function<bool(const i_frame_t& frame)>;

    frame_handler_t m_frame_handler;

public:
    frame_sink(frame_handler_t&& frame_handler)
        : m_frame_handler(frame_handler)
    {

    }

    // i_message_sink interface
public:
    bool send_message(const i_message &message) override
    {
        if (message.category() == message_category_t::data)
        {
            const auto& media_frame = static_cast<const i_media_frame&>(message);
            if (media_frame.media_type() == MediaType)
            {

                return m_frame_handler(static_cast<const i_frame_t&>(media_frame));
            }
        }

        return false;
    }
};

}

template<media_type_t MediaType>
class smart_transcoder : public i_media_converter
{

    using mutex_t = pt::utils::spin_lock;
    using i_format_t = typename detail::format_types_t<MediaType>::i_format_t;
    using i_frame_t = typename detail::format_types_t<MediaType>::i_frame_t;
    using format_impl_t = typename detail::format_types_t<MediaType>::format_impl_t;
    using frame_impl_t = typename detail::format_types_t<MediaType>::frame_impl_t;
    using frame_queue_t = std::queue<i_message::u_ptr_t>;

    enum class transcoder_state_t
    {
        input,
        decode,
        convert,
        encode,
        output
    };

    struct params_t
    {
        bool            transcode_always;
        bool            transcode_async;
        std::size_t     max_overrun;
        params_t(bool transcode_always = false
                , bool transcode_async = false)
            : transcode_always(transcode_always)
            , transcode_async(transcode_async)
            , max_overrun(10)
        {

        }

        params_t(const i_property& params)
            : params_t()
        {
            load(params);
        }

        bool load(const i_property& params)
        {
            property_reader reader(params);
            return reader.get("transcode_always", transcode_always)
                    | reader.get("transcode_async", transcode_async)
                    | reader.get("max_overrun", max_overrun);
        }

        bool store(i_property& params) const
        {
            property_writer writer(params);
            return writer.set("transcode_always", transcode_always, false)
                    && writer.set("transcode_async", transcode_async, false)
                    && writer.set("max_overrun", max_overrun);
        }
    };

    class async_frame_manager
    {
        mutable mutex_t                 m_safe_mutex;
        mutable mutex_t                 m_exec_mutex;

        i_task_manager&                 m_task_manager;
        smart_transcoder&               m_owner;

        frame_queue_t                   m_frame_queue;
        i_task::s_ptr_t                 m_task;
        i_task_manager::task_handler_t  m_task_handler;

        std::atomic_bool                m_processed;

public:

        async_frame_manager(smart_transcoder& owner
                            , i_task_manager& task_manager)
            : m_task_manager(task_manager)
            , m_owner(owner)
            , m_task(nullptr)
            , m_task_handler([&] { on_task_execute(); })
            , m_processed(false)
        {
            mpl_log_debug("async_frame_manager #", this, ": init: owner: ", &m_owner);
        }

        ~async_frame_manager()
        {
            mpl_log_debug("async_frame_manager #", this, ": destruction, processed: ", is_processed());

            m_processed.store(false, std::memory_order_release);
            std::lock_guard lock(m_exec_mutex);
        }

        inline std::size_t max_overrun() const
        {
            return m_owner.m_params.max_overrun;
        }

        bool push_frame(const i_frame_t& frame)
        {
            if (auto clone_frame = frame.clone())
            {
                std::lock_guard lock(m_safe_mutex);

                m_frame_queue.emplace(std::move(clone_frame));

                while (max_overrun() > 0
                       && m_frame_queue.size() > max_overrun())
                {
                    mpl_log_debug("async_frame_manager #", this, ": drop frame, overrun");
                    m_frame_queue.pop();
                }

                bool flag = false;
                if (m_processed.compare_exchange_strong(flag, true))
                {
                    auto task_handler = m_task_handler;
                    mpl_log_trace("async_frame_manager #", this, ": add async transcoder task");
                    m_task = m_task_manager.add_task(std::move(task_handler));
                }

                return true;
            }

            return false;
        }

        inline bool is_processed() const
        {
            return m_processed.load(std::memory_order_acquire);
        }

        i_message::u_ptr_t fetch_frame()
        {
            std::lock_guard lock(m_safe_mutex);
            if (is_processed()
                    && !m_frame_queue.empty())
            {
                auto frame = std::move(m_frame_queue.front());
                m_frame_queue.pop();
                return frame;
            }

            return nullptr;
        }

        void on_task_execute()
        {
            std::lock_guard lock(m_exec_mutex);
            while (auto frame = fetch_frame())
            {
                mpl_log_trace("async_frame_manager #", this, ": async transcoder frame");
                m_owner.on_transcode_frame(static_cast<const i_frame_t&>(*frame));
            }

            mpl_log_trace("async_frame_manager #", this, ": async task completed");
            m_processed.store(false, std::memory_order_release);
        }
    };


    detail::converter_manager<MediaType>    m_converter_manager;

    params_t                                m_params;

    format_impl_t                           m_input_format;
    format_impl_t                           m_output_format;
    format_impl_t                           m_real_output_format;

    i_media_converter::u_ptr_t              m_decoder;
    i_media_converter::u_ptr_t              m_encoder;
    i_media_converter::u_ptr_t              m_converter;

    detail::frame_sink<MediaType>           m_decoder_sink;
    detail::frame_sink<MediaType>           m_encoder_sink;
    detail::frame_sink<MediaType>           m_converter_sink;

    i_message_sink*                         m_output_sink;
    async_frame_manager                     m_async_manager;

    bool                                    m_is_init;
    bool                                    m_is_transit;

public:
    using u_ptr_t = std::unique_ptr<smart_transcoder>;

    static u_ptr_t create(const i_property &params
                          , i_task_manager& task_manager
                          , i_media_converter_factory &media_decoders
                          , i_media_converter_factory &media_encoders
                          , i_media_converter_factory &media_converters)
    {
        property_reader reader(params);
        format_impl_t media_format;
        if (reader.get("format", media_format))
        {
            params_t internal_params(params);

            return std::make_unique<smart_transcoder>(std::move(media_format)
                                                      , std::move(internal_params)
                                                      , task_manager
                                                      , media_decoders
                                                      , media_encoders
                                                      , media_converters);
        }

        return nullptr;
    }


    smart_transcoder(format_impl_t&& output_format
                     , params_t&& params
                     , i_task_manager& task_manager
                     , i_media_converter_factory &media_decoders
                     , i_media_converter_factory &media_encoders
                     , i_media_converter_factory &media_converters)
        : m_converter_manager(media_decoders
                              , media_encoders
                              , media_converters)
        , m_params(std::move(params))
        , m_output_format(std::move(output_format))
        , m_real_output_format(m_output_format)
        , m_decoder_sink([&](const auto& frame)
                        { return on_decoder_frame(frame); } )
        , m_encoder_sink([&](const auto& frame)
                        { return on_encoder_frame(frame); } )
        , m_converter_sink([&](const auto& frame)
                        { return on_converter_frame(frame); } )
        , m_output_sink(nullptr)
        , m_async_manager(*this
                          , task_manager)
        , m_is_init(false)
        , m_is_transit(true)
    {
        mpl_log_info("smart transcoder #", this, ": init: ", m_output_format.info().to_string());
    }

    ~smart_transcoder()
    {
        mpl_log_info("smart transcoder #", this, ": destruction");
    }

    bool check_and_init_converters(const i_format_t& input_format
                                   , const i_format_t& output_format)
    {
        if (!detail::is_compatible_format(input_format
                                          , output_format)
                || m_params.transcode_always)
        {

            format_impl_t input_convert_format(input_format);
            format_impl_t output_convert_format(output_format);

            mpl_log_info("smart transcoder #", this, ": init converters: ", input_convert_format.info().to_string()
                         , "->", output_convert_format.info().to_string());

            // encoder
            if (output_format.is_encoded())
            {
                if (m_encoder == nullptr
                        || !m_encoder->output_format().is_compatible(output_format))
                {
                    m_encoder.reset();
                    m_encoder = m_converter_manager.create_encoder(output_format
                                                                   , &m_encoder_sink);

                    if (!m_encoder)
                    {
                        mpl_log_error("smart transcoder #", this, ": init converters: create encoder for ", input_convert_format.info().to_string() ," failed");
                        return false;
                    }

                    output_convert_format.assign(static_cast<const i_format_t&>(m_encoder->input_format()));
                    mpl_log_info("smart transcoder #", this, ": init converters: create encoder #", m_encoder.get(), "for ", output_convert_format.info().to_string() ," success");
                }
            }
            else
            {
                mpl_log_debug("smart transcoder #", this, ": init converters: no encoder required");
                m_encoder.reset();
            }

            // decoder
            if (input_format.is_encoded())
            {
                if (m_decoder == nullptr
                        || !m_decoder->input_format().is_compatible(input_format))
                {
                    m_decoder.reset();
                    m_decoder = m_converter_manager.create_decoder(input_format
                                                                   , &m_decoder_sink);

                    if (!m_decoder)
                    {
                        mpl_log_error("smart transcoder #", this, ": create decoder for ", output_convert_format.info().to_string(), " failed");
                        return false;
                    }

                    input_convert_format.assign(static_cast<const i_format_t&>(m_decoder->output_format()));
                    mpl_log_info("smart transcoder #", this, ": init converters: create encoder #", m_encoder.get(), "for ", input_convert_format.info().to_string() ," success");
                }
            }
            else
            {
                mpl_log_debug("smart transcoder #", this, ": init converters: no decoder required");
                m_decoder.reset();
            }

            // converter
            if (input_convert_format.is_compatible(output_convert_format))
            {
                mpl_log_debug("smart transcoder #", this, ": init converters: no converter required");
                m_converter.reset();
            }
            else
            {
                m_converter.reset();
                m_converter = m_converter_manager.create_converter(output_convert_format
                                                                   , &m_converter_sink);

                if (!m_converter)
                {
                    mpl_log_error("smart transcoder #", this, ": create converter for ", output_convert_format.info().to_string(), " failed");
                    return false;
                }

                mpl_log_info("smart transcoder #", this, ": init converters: create converter #", m_converter.get(), "for ", output_convert_format.info().to_string() ," success");
            }
        }
        else
        {
            reset_converters();
        }

        return true;
    }

    template<transcoder_state_t State = transcoder_state_t::input>
    bool convert_and_write_frame(const i_frame_t& frame)
    {
        switch(State)
        {
            case transcoder_state_t::input:
                mpl_log_trace("smart transcoder #", this, ": input");
                return m_is_transit
                        ? convert_and_write_frame<transcoder_state_t::output>(frame)
                        : convert_and_write_frame<transcoder_state_t::decode>(frame);
            break;
            case transcoder_state_t::decode:
                mpl_log_trace("smart transcoder #", this, ": decode");
                return m_decoder != nullptr
                        ? m_decoder->send_message(frame)
                        : convert_and_write_frame<transcoder_state_t::convert>(frame);
            break;
            case transcoder_state_t::convert:
                mpl_log_trace("smart transcoder #", this, ": convert");
                return m_converter != nullptr
                        ? m_converter->send_message(frame)
                        : convert_and_write_frame<transcoder_state_t::encode>(frame);
            break;
            case transcoder_state_t::encode:
                mpl_log_trace("smart transcoder #", this, ": encode");
                return m_encoder != nullptr
                        ? m_encoder->send_message(frame)
                        : convert_and_write_frame<transcoder_state_t::output>(frame);
            break;
            case transcoder_state_t::output:
                if (m_output_sink)
                {
                    mpl_log_trace("smart transcoder #", this, ": output");
                    return m_output_sink->send_message(frame);
                }
            break;
            default:;
        }

        return false;
    }

    bool on_media_frame(const i_media_frame& media_frame)
    {
        if (media_frame.media_type() == MediaType)
        {
            if (m_params.transcode_async == false)
            {
                return on_transcode_frame(static_cast<const i_frame_t&>(media_frame));
            }
            else
            {
                return m_async_manager.push_frame(static_cast<const i_frame_t&>(media_frame));
            }
        }

        return false;
    }

    bool on_transcode_frame(const i_frame_t& media_frame)
    {
        if (!m_input_format.is_compatible(media_frame.format())
                || !m_is_init)
        {
            m_input_format.assign(media_frame.format());

            m_real_output_format = m_output_format;
            detail::merge_format(m_input_format
                                 , m_real_output_format);

            m_is_init = check_and_init_converters(m_input_format
                                                  , m_real_output_format);

            if (!m_is_init)
            {
                reset_converters();
            }

            m_is_transit = is_transit();
        }

        if (m_is_init)
        {
            return convert_and_write_frame(media_frame);
        }

        mpl_log_warning("smart transcoder #", this, ": frame transcode error");

        return false;
    }

    bool on_decoder_frame(const i_frame_t& frame)
    {
        return convert_and_write_frame<transcoder_state_t::convert>(frame);
    }

    bool on_converter_frame(const i_frame_t& frame)
    {
        return convert_and_write_frame<transcoder_state_t::encode>(frame);
    }

    bool on_encoder_frame(const i_frame_t& frame)
    {
        return convert_and_write_frame<transcoder_state_t::output>(frame);
    }


    void reset_converters()
    {
        mpl_log_debug("smart transcoder #", this, ": reset converters");

        m_decoder.reset();
        m_encoder.reset();
        m_converter.reset();
    }

    void reset()
    {
        mpl_log_debug("smart transcoder #", this, ": reset");

        reset_converters();
        m_is_init = false;
        m_is_transit = true;
    }

    bool is_transit() const
    {
        return m_decoder == nullptr
                && m_converter == nullptr
                && m_encoder == nullptr;
    }


    // i_message_sink interface
public:
    bool send_message(const i_message &message) override
    {
        if (message.category() == message_category_t::data)
        {
            return on_media_frame(static_cast<const i_media_frame&>(message));
        }

        return false;
    }

    // i_media_converter interface
public:
    const i_media_format &input_format() const override
    {
        return m_input_format;
    }
    const i_media_format &output_format() const override
    {
        return m_real_output_format;
    }
    void set_sink(i_message_sink *output_sink) override
    {
        m_output_sink = output_sink;
    }
};

smart_transcoder_factory::smart_transcoder_factory(i_task_manager& task_manager
                                                   , i_media_converter_factory &media_decoders
                                                   , i_media_converter_factory &media_encoders
                                                   , i_media_converter_factory &media_converters)
    : m_task_manager(task_manager)
    , m_media_decoders(media_decoders)
    , m_media_encoders(media_encoders)
    , m_media_converters(media_converters)
{

}

i_media_converter::u_ptr_t smart_transcoder_factory::create_converter(const i_property &params)
{
    property_reader reader(params);
    switch(reader.get<media_type_t>("format.media_type"
                                    , media_type_t::undefined))
    {
        case media_type_t::audio:
            return smart_transcoder<media_type_t::audio>::create(params
                                                                 , m_task_manager
                                                                 , m_media_decoders
                                                                 , m_media_encoders
                                                                 , m_media_converters);
        break;
        case media_type_t::video:
            return smart_transcoder<media_type_t::video>::create(params
                                                                 , m_task_manager
                                                                 , m_media_decoders
                                                                 , m_media_encoders
                                                                 , m_media_converters);
        break;
        default:;
    }

    return nullptr;
}



}
