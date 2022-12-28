#ifndef I_OPTION_H
#define I_OPTION_H

#include "tools/base/any_base.h"
#include <memory>
#include <cstdint>
#include <functional>
#include <vector>


namespace mpl
{

class i_option
{
public:

    using option_key_t = std::int64_t;
    using option_value_t = base::any;
    using foreach_handler_t = std::function<bool(const option_key_t& key
                                                 , const option_value_t& value)>;
    using option_key_list_t = std::vector<option_key_t>;

    using u_ptr_t = std::unique_ptr<i_option>;
    using s_ptr_t = std::shared_ptr<i_option>;
    virtual ~i_option() = default;
    virtual bool has_option(const option_key_t& key) const = 0;
    virtual bool set(const option_key_t& key, option_value_t&& value) = 0;
    virtual bool set(const option_key_t& key, const option_value_t& value) = 0;
    virtual option_key_list_t keys() const = 0;
    virtual option_value_t get(const option_key_t& key) const = 0;
    virtual void foreach(const foreach_handler_t& handler) const = 0;
    virtual void clear() = 0;
    virtual std::size_t merge(const i_option& option) = 0;
    virtual bool is_equal(const i_option& option) const = 0;
    virtual std::size_t count() const = 0;
    virtual u_ptr_t clone() const = 0;

};

}

#endif // I_OPTION_H
