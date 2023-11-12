#include "tls_fingerprint.h"
#include "utils/enum_utils.h"
#include "utils/common_utils.h"

namespace mpl::net
{

tls_fingerprint_t::tls_fingerprint_t(tls_hash_method_t method
                                     , const hash_t &hash)
    : method(method)
    , hash(hash)
{

}

bool tls_fingerprint_t::operator ==(const tls_fingerprint_t &other) const
{
    return method == other.method
            && hash == other.hash;
}

bool tls_fingerprint_t::operator !=(const tls_fingerprint_t &other) const
{
    return !operator == (other);
}

bool tls_fingerprint_t::is_compatible(const tls_fingerprint_t &other) const
{
    return method == other.method
            && hash.size() == other.hash.size();
}

bool tls_fingerprint_t::is_defined() const
{
    return method != tls_hash_method_t::undefined;
}

bool tls_fingerprint_t::from_string(const std::string &params_line)
{
    auto args = utils::split_lines(params_line, ' ', 1);
    if (args.size() == 2)
    {

        if (auto m = utils::string_to_enum<tls_hash_method_t>(args[0]))
        {
            if (auto h = utils::create_raw_array(args[1], ":")
                    ; !h.empty())
            {
                method = *m;
                hash = std::move(h);
                return true;
            }
        }
    }

    return false;
}

std::string tls_fingerprint_t::to_string() const
{
    if (is_defined())
    {
        return utils::enum_to_string(method)
                .append(" ")
                .append(utils::hex_to_string(hash.data()
                                             , hash.size()
                                             , ":", true)
                        );
    }

    return {};
}



}
