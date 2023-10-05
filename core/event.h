#ifndef MPL_EVENT_H
#define MPL_EVENT_H

#include <string>

namespace mpl
{

struct event_t
{
    using event_id_t = std::uint32_t;

    const event_id_t  event_id;
    const std::string name;

    static event_id_t register_event(const std::string_view& event_name);

protected:
    event_t(const event_id_t event_id
            , const std::string_view& name = {});

    event_t(const event_t& other) = default;
    event_t(event_t&& other) = default;
    // заглушки нужны, чтобы производные структуры декларировали
    // "пятерку" по умолчанию, так как есть константные методы
    event_t& operator=(const event_t& other);
    event_t& operator=(event_t&& other) = default;
};

}

#endif // MPL_EVENT_H
