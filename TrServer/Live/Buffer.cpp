#include "Buffer.h"
#include "../Scheduler/SocketsOps.h"


int Buffer::read(int fd)                        //¶ÁÊý¾Ý»º´æ
{
    const int n = ::recv(fd, RX_buf, sizeof(RX_buf), 0);
    if (n <= 0)
    {
        return -1;
    }
    else
    {
        string temp_message(RX_buf, n);
        message = temp_message;
        return n;
    }
}

