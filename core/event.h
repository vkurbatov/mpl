#ifndef MPL_EVENT_H
#define MPL_EVENT_H

#include "event_types.h"
#include <string>

namespace mpl
{

struct event_t
{
    const event_id_t  event_id;
    const std::string name;

    virtual bool operator == (const event_t& other) const;
    virtual bool operator != (const event_t& other) const;

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
