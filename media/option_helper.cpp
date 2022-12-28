#include "option_helper.h"

namespace mpl
{

option_reader::option_reader(const i_option &options)
    : m_options(options)
{

}

option_writer::option_writer(i_option &options)
    : option_reader(options)
    , m_options(options)
{

}

bool option_writer::remove(const i_option::option_key_t &key)
{
    return m_options.set(key, {});
}


}
