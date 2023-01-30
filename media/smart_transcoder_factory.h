#ifndef MPL_SMART_TRANSCODER_FACTORY_H
#define MPL_SMART_TRANSCODER_FACTORY_H

#include "i_media_converter_factory.h"

namespace mpl::media
{

class smart_transcoder_factory : public i_media_converter_factory
{
    i_media_converter_factory&  m_media_decoders;
    i_media_converter_factory&  m_media_encoders;
    i_media_converter_factory&  m_media_converters;

public:
    using u_ptr_t = std::unique_ptr<i_media_converter_factory>;

    smart_transcoder_factory(i_media_converter_factory& media_decoders
                             , i_media_converter_factory& media_encoders
                             , i_media_converter_factory& media_converters);


    // i_media_converter_factory interface
public:
    i_media_converter::u_ptr_t create_converter(const i_media_format &convert_format) override;
};

}

#endif // MPLSMART_TRANSCODER_FACTORY_H
