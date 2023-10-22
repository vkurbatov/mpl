#ifndef WBS_BIO_HANDLER_H
#define WBS_BIO_HANDLER_H

#include "ssl_pointers.h"

#include <functional>
#include <memory>
#include <cstdint>

namespace pt::ssl
{

class bio_handler
{
public:
    using read_handler_t = std::function<std::int32_t(void* data, std::size_t size)>;
    using write_handler_t = std::function<std::int32_t(const void* data, std::size_t size)>;
    using ctrl_handler_t = std::function<std::int32_t(std::int32_t ctrl_id, void* data, std::int64_t length)>;
private:
    read_handler_t      m_read_handler;
    write_handler_t     m_write_handler;
    ctrl_handler_t      m_ctrl_handler;

public:
    using unique_s_ptr_t = std::unique_ptr<bio_handler>;
    using s_ptr_t = std::shared_ptr<bio_handler>;
    bio_handler();


};

}

#endif // WBS_BIO_HANDLER_H
