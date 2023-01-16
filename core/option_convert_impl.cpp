#include "convert_utils.h"
#include "property_writer.h"
#include "option_helper.h"
#include "option_types.h"

#include "common_types.h"

//#include "media_types.h"

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

template<>
bool convert(const i_option& options, i_property& property)
{
    bool result = false;
    detail::option_loader loader(options
                                 , property);

    for (const auto& id : options.ids())
    {
        switch(id)
        {
            case opt_fmt_stream_id:
                result |= loader.load<std::int32_t>(id, "stream_id");
            break;
            case opt_fmt_device_id:
                result |= loader.load<std::int32_t>(id, "device_id");
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

template<>
bool convert(const i_property& property, i_option& options)
{
    bool result = false;
    detail::option_saver saver(options
                               , property);

    property_reader reader(property);

    result |= saver.save<std::int32_t>(opt_fmt_stream_id, "stream_id");
    result |= saver.save<std::int32_t>(opt_fmt_device_id, "device_id");

    if (auto e = property_reader(property).get<octet_string_t>("extra_data"))
    {
        result |= option_writer(options).set(opt_codec_extra_data
                                             , std::make_shared<octet_string_t>(std::move(*e)));
    }

    result |= saver.save<std::string>(opt_codec_params, "codec_params");

    return result;

}

}
