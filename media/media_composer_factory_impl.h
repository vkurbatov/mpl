#ifndef MPL_MEDIA_COMPOSER_FACTORY_IMPL_H
#define MPL_MEDIA_COMPOSER_FACTORY_IMPL_H

#include "i_media_composer_factory.h"

namespace mpl::media
{

class i_media_converter_factory;
class i_layout_manager;

class media_composer_factory_impl : public i_media_composer_factory
{
    i_media_converter_factory&  m_media_converter_factory;
    i_layout_manager&           m_layout_manager;
public:

    using u_ptr_t = std::unique_ptr<media_composer_factory_impl>;
    using s_ptr_t = std::shared_ptr<media_composer_factory_impl>;

    static u_ptr_t create(i_media_converter_factory& media_converter_factory
                          , i_layout_manager& layout_manager);

    media_composer_factory_impl(i_media_converter_factory& media_converter_factory
                                , i_layout_manager& layout_manager);

    // i_media_composer_factory interface
public:
    i_media_composer::u_ptr_t create_composer(const i_property &params) override;
};

}

#endif // MPL_MEDIA_COMPOSER_FACTORY_IMPL_H
