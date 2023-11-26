#ifndef I_SSL_CHANNEL_H
#define I_SSL_CHANNEL_H

namespace pt::ssl
{

class i_ssl_channel
{
public:
    virtual ~i_ssl_channel() = default;
    virtual bool open();
    virtual bool close();
    virtual bool is_open() const;
};

}

#endif // I_SSL_CHANNEL_H
