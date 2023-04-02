#ifndef MPL_CHANNEL_TYPES_H
#define MPL_CHANNEL_TYPES_H

namespace mpl
{

enum class channel_state_t
{
    undefined = -1,
    ready,
    create,
    opening,
    open,
    starting,
    started,
    connecting,
    connected,
    stopping,
    stopped,
    disconnecting,
    disconnected,
    closing,
    closed,
    failed,
    remove
};

enum class channel_control_id_t
{
    undefined = -1,
    open,
    close,
    connect,
    shutdown,
    start,
    stop,
    configure,
    command,
    custom
};

}

#endif // MPL_CHANNEL_TYPES_H
