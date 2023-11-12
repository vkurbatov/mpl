#ifndef IO_ENDPOINT_H
#define IO_ENDPOINT_H

#include <string>

namespace pt::io
{

struct endpoint_t
{
    enum class type_t
    {
        undefined = 0,
        serial,
        ip,
        ux,
        pipe
    };

    static const endpoint_t& undefined();

    const type_t type;

    endpoint_t();
    endpoint_t(const endpoint_t& other) = default;
    endpoint_t(endpoint_t&& other) = default;
    endpoint_t& operator = (const endpoint_t& other);
    endpoint_t& operator = (endpoint_t&& other) = default;

    virtual ~endpoint_t() = default;
    virtual bool operator == (const endpoint_t& other) const;
    virtual bool operator != (const endpoint_t& other) const;
    virtual bool is_valid() const;
    virtual std::string to_string() const;

protected:
    endpoint_t(type_t type);
};

}

#endif // IO_ENDPOINT_H
