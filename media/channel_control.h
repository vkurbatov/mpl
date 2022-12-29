#ifndef MPL_CHANNEL_CONTROL_H
#define MPL_CHANNEL_CONTROL_H

#include "channel_types.h"
#include <string>

namespace mpl
{

class i_property;

struct channel_control_t
{
    channel_control_id_t    control_id;
    std::string             name;
    const i_property*       input_params;
    i_property*             output_params;

    static const channel_control_t& undefined();
    static const channel_control_t& open();
    static const channel_control_t& close();
    static const channel_control_t& connect();
    static const channel_control_t& shutdown();
    static const channel_control_t& start();
    static const channel_control_t& stop();

    static channel_control_t configure(const i_property* input_params
                                       , i_property* output_params);

    static channel_control_t set_config(const i_property* input_params);
    static channel_control_t get_config(i_property* output_params);

    channel_control_t(channel_control_id_t control_id
                      , const std::string& name
                      , const i_property* input_params = nullptr
                      , i_property* output_params = nullptr);

    channel_control_t(channel_control_id_t control_id = channel_control_id_t::undefined
                      , const i_property* input_params = nullptr
                      , i_property* output_params = nullptr);

    bool operator == (const channel_control_t& other) const;
    bool operator != (const channel_control_t& other) const;

    bool has_params() const;

};

}

#endif // MPL_CHANNEL_CONTROL_H
