#ifndef MPL_I_PARAMETRIZABLE_H
#define MPL_I_PARAMETRIZABLE_H

namespace mpl
{

class i_property;

class i_parametrizable
{
public:
    virtual ~i_parametrizable() = default;
    virtual bool set_params(const i_property& params) = 0;
    virtual bool get_params(i_property& params) const = 0;
};

}

#endif // MPL_I_PARAMETRIZABLE_H
