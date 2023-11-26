#ifndef I_MEDIA_CONVERTER_COLLECTION_H
#define I_MEDIA_CONVERTER_COLLECTION_H

#include <memory>

namespace mpl::media
{

class i_media_converter_factory;

class i_media_converter_collection
{
public:
    enum class media_converter_type_t
    {
        undefined = 0,
        converter,
        encoder,
        decoder,
        smart
    };

    using u_ptr_t = std::unique_ptr<i_media_converter_factory>;
    using s_ptr_t = std::shared_ptr<i_media_converter_factory>;

    virtual ~i_media_converter_collection() = default;
    virtual i_media_converter_factory* get_factory(media_converter_type_t type) = 0;
};

}

#endif // I_MEDIA_CONVERTER_COLLECTION_H
