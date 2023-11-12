#ifndef MPL_MEDIA_FORMAT_FACTORY_IMPL_H
#define MPL_MEDIA_FORMAT_FACTORY_IMPL_H

#include "i_media_format_factory.h"

namespace mpl::media
{

class media_format_factory_impl : public i_media_format_factory
{
    media_type_t        m_media_type;

public:
    using u_ptr_t = std::unique_ptr<media_format_factory_impl>;
    using s_ptr_t = std::shared_ptr<media_format_factory_impl>;

    static u_ptr_t create(media_type_t media_type = media_type_t::undefined);

    media_format_factory_impl(media_type_t media_type = media_type_t::undefined);

    void set_media_type(media_type_t media_type);
    media_type_t media_type() const;

    // i_media_format_factory interface
public:
    i_media_format::u_ptr_t create_format(const i_property &format_params) override;
};

}

#endif // MPL_MEDIA_FORMAT_FACTORY_IMPL_H
