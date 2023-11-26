#ifndef MPL_I_ENGINE_H
#define MPL_I_ENGINE_H

#include <memory>

namespace mpl
{

class i_engine
{
public:
    using u_ptr_t = std::unique_ptr<i_engine>;
    using s_ptr_t = std::shared_ptr<i_engine>;

    virtual ~i_engine() = default;

    virtual bool start() = 0;
    virtual bool stop() = 0;
    virtual bool is_started() const = 0;

};

}

#endif // MPL_I_ENGINE_H
