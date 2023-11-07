#ifndef MPL_UTILS_CORE_EVENT_FACTORY_H
#define MPL_UTILS_CORE_EVENT_FACTORY_H

#include "core/i_event_factory.h"

namespace mpl
{

class core_event_factory : public i_event_factory
{

public:
    core_event_factory& get_instance();

    core_event_factory() = default;

    // i_event_factory interface
public:
    i_message_event::u_ptr_t create_message(const event_t &event) override;
};

}

#endif // MPL_UTILS_CORE_EVENT_FACTORY_H
