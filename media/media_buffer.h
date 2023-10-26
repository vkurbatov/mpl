#ifndef MPL_MEDIA_BUFFER_H
#define MPL_MEDIA_BUFFER_H

#include "core/i_message_sink.h"
#include <map>

namespace mpl::media
{

class media_buffer : public i_message_sink
{
public:
    struct config_t
    {
        std::size_t     buffer_size;
        bool            timestamp_sync;

        config_t(std::size_t buffer_size = 0
                , bool timestamp_sync = false);

        bool is_transit() const;
    };
private:

public:
    media_buffer();
    void set_sink(i_message_sink* output_sink);
};

}

#endif // MPL_MEDIA_BUFFER_H
