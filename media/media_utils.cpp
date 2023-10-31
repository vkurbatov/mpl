#include "media_utils.h"
#include "media_types.h"

#include "utils/property_writer.h"
#include "utils/option_helper.h"
#include "media_option_types.h"

#include "core/common_types.h"


using namespace mpl::media;

namespace mpl::utils
{

namespace detail
{

class option_loader
{
    option_reader       m_options;
    property_writer     m_property;
public:
    option_loader(const i_option& options
                  , i_property& property)
        : m_options(options)
        , m_property(property)
    {

    }

    template<typename T>
    bool load(option_id_t id, const std::string& key)
    {
        if (auto o = m_options.get<T*>(id))
        {
            return m_property.set(key, *o);
        }

        return false;
    }

};

class option_saver
{
    option_writer       m_options;
    property_reader     m_property;
public:
    option_saver(i_option& options
                 , const i_property& property)
        : m_options(options)
        , m_property(property)
    {

    }

    template<typename T>
    bool save(option_id_t id, const std::string& key)
    {
        if (auto p = m_property.get<T>(key))
        {
            return m_options.set(id, *p);
        }

        return false;
    }
};

}

timestamp_t get_video_frame_time(double frame_rate)
{
    if (frame_rate != 0.0)
    {
        return video_sample_rate / frame_rate;
    }

    return timestamp_null;
}

bool convert_format_options(const i_option &options, i_property &property)
{
    bool result = false;
    detail::option_loader loader(options
                                 , property);

    for (const auto& id : options.ids())
    {
        switch(id)
        {
            case opt_frm_track_id:
                result |= loader.load<std::int32_t>(id, "track_id");
            break;
            case opt_frm_stream_id:
                result |= loader.load<std::int32_t>(id, "device_id");
            break;
            case opt_frm_layer_id:
                result |= loader.load<std::int32_t>(id, "layer_id");
            break;
            case opt_codec_extra_data:
            {
                if (auto e = option_reader(options).get<std::shared_ptr<octet_string_t>>(id))
                {
                    result |= property_writer(property).set("extra_data", *e->get());
                }
            }
            break;
            case opt_codec_params:
                result |= loader.load<std::string>(id, "codec_params");
            break;
        }
    }


    return result;
}

bool convert_format_options(const i_property &property, i_option &options)
{
    bool result = false;
    detail::option_saver saver(options
                               , property);

    result |= saver.save<std::int32_t>(opt_frm_track_id, "track_id");
    result |= saver.save<std::int32_t>(opt_frm_stream_id, "device_id");
    result |= saver.save<std::int32_t>(opt_frm_layer_id, "layer_id");

    if (auto e = property_reader(property).get<octet_string_t>("extra_data"))
    {
        result |= option_writer(options).set(opt_codec_extra_data
                                             , std::make_shared<octet_string_t>(std::move(*e)));
    }

    result |= saver.save<std::string>(opt_codec_params, "codec_params");

    return result;
}


}
