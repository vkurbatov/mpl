#ifndef MPL_I_MEDIA_CONVERTER_H
#define MPL_I_MEDIA_CONVERTER_H

#include "core/i_message_sink.h"
#include "i_media_format.h"

namespace mpl::media
{

class i_media_converter : public i_message_sink
{
public:
    using u_ptr_t = std::unique_ptr<i_media_converter>;
    using s_ptr_t = std::shared_ptr<i_media_converter>;

    virtual ~i_media_converter() = default;
    virtual const i_media_format& input_format() const = 0;
    virtual const i_media_format& output_format() const = 0;
    virtual void set_sink(i_message_sink* output_sink) = 0;
};

}

#endif // MPL_I_MEDIA_CONVERTER_H
