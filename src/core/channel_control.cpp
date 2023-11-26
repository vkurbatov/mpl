#include "channel_control.h"
#include "i_property.h"

namespace mpl
{

namespace detail
{

bool compare_property(const i_property* lhs
                      , const i_property* rhs)
{
    return lhs == rhs
            || (lhs != nullptr && rhs != nullptr && lhs->is_equal(*rhs));
}

}

const channel_control_t &channel_control_t::undefined()
{
    static channel_control_t undefined_contol(channel_control_id_t::undefined);
    return undefined_contol;
}

const channel_control_t &channel_control_t::open()
{
    static channel_control_t open_contol(channel_control_id_t::open
                                         , "open");
    return open_contol;
}

const channel_control_t &channel_control_t::close()
{
    static channel_control_t close_contol(channel_control_id_t::close
                                          , "close");
    return close_contol;
}

const channel_control_t &channel_control_t::connect()
{
    static channel_control_t connect_contol(channel_control_id_t::connect
                                            , "connect");
    return connect_contol;
}

const channel_control_t &channel_control_t::shutdown()
{
    static channel_control_t close_shutdown(channel_control_id_t::shutdown
                                            , "shutdown");
    return close_shutdown;
}

const channel_control_t &channel_control_t::start()
{
    static channel_control_t close_shutdown(channel_control_id_t::start
                                            , "start");
    return close_shutdown;
}

const channel_control_t &channel_control_t::stop()
{
    static channel_control_t close_shutdown(channel_control_id_t::stop
                                            , "stop");
    return close_shutdown;
}

channel_control_t channel_control_t::command(const i_property *input_params
                                             , i_property *output_params)
{
    return
    {
        channel_control_id_t::command
        , "command"
        , input_params
        , output_params
    };
}

channel_control_t::channel_control_t(channel_control_id_t control_id
                                     , const std::string &name
                                     , const i_property *input_params
                                     , i_property *output_params)
    : control_id(control_id)
    , name(name)
    , input_params(input_params)
    , output_params(output_params)
{

}

channel_control_t::channel_control_t(channel_control_id_t control_id
                                     , const i_property *input_params
                                     , i_property *output_params)
    : channel_control_t(control_id
                        , {}
                        , input_params
                        , output_params)
{

}

bool channel_control_t::operator ==(const channel_control_t &other) const
{
    return control_id == other.control_id
            && name == other.name
            && detail::compare_property(input_params, other.input_params)
            && detail::compare_property(output_params, other.output_params);
}

bool channel_control_t::operator !=(const channel_control_t &other) const
{
    return !operator == (other);
}

bool channel_control_t::has_params() const
{
    return input_params != nullptr
            && output_params != nullptr;
}

}
