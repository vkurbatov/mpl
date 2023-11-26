#ifndef MPL_MEDIA_I_MESSAGE_MEDIA_DATA_H
#define MPL_MEDIA_I_MESSAGE_MEDIA_DATA_H

#include "core/i_message_data.h"
#include "media_types.h"

namespace mpl::media
{

class i_message_media_data : public i_message_data
{
public:
    using u_ptr_t = std::unique_ptr<i_message_media_data>;
    using s_ptr_t = std::shared_ptr<i_message_media_data>;
    virtual media_data_type_t data_type() const = 0;
};

}

#endif // MPL_MEDIA_I_MESSAGE_MEDIA_DATA_H
