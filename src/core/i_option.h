#ifndef I_OPTION_H
#define I_OPTION_H

#include "option_types.h"
#include "i_option_value.h"

#include <memory>
#include <functional>
#include <vector>


namespace mpl
{


class i_option
{
public:
    using option_id_t = std::int64_t;
    using foreach_handler_t = std::function<bool(const option_id_t& id
                                                 , const i_option_value& value)>;
    using option_id_list_t = std::vector<option_id_t>;

    using u_ptr_t = std::unique_ptr<i_option>;
    using s_ptr_t = std::shared_ptr<i_option>;
    virtual ~i_option() = default;
    virtual bool has_option(const option_id_t& id) const = 0;
    virtual bool set(const option_id_t& id, i_option_value::u_ptr_t&& value) = 0;
    virtual bool set(const option_id_t& id, const i_option_value& value) = 0;
    virtual option_id_list_t ids() const = 0;
    virtual const i_option_value* get(const option_id_t& id) const = 0;
    virtual void foreach(const foreach_handler_t& handler) const = 0;
    virtual void clear() = 0;
    virtual std::size_t merge(const i_option& option) = 0;
    virtual bool is_equal(const i_option& option) const = 0;
    virtual std::size_t count() const = 0;
    virtual u_ptr_t clone() const = 0;
};

}

#endif // I_OPTION_H
