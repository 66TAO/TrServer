#include "Buffer.h"
#include "../Scheduler/SocketsOps.h"
#include "../Base/Log.h"


int Buffer::read(int fd)                        //读数据缓存
{
    const int n = ::recv(fd, RX_buf, sizeof(RX_buf), 0);
    /*if (n <= 0)
    {
        return -1;
    }*/
    if (n < 0)
    {
        // 发生错误
        LOGE( "Error receiving data: %s", strerror(errno));
        return -1;
    }
    else if (n == 0)
    {
        // 对端正常关闭连接
        LOGI("Connection closed by the client.");
        return -1;
    }
    else
    {
        string temp_message(RX_buf, n);
        message = temp_message;
        return n;
    }
}

