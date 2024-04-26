/*
 * @Author: jiejie
 * @Github: https://github.com/jiejieTop
 * @Date: 2020-01-10 23:45:59
 * @LastEditTime: 2020-04-25 17:50:58
 * @Description: the code belongs to jiejie, please keep the author information and source code according to the license.
 */
#include "mqtt_log.h"
#include "platform_net_socket.h"

int platform_net_socket_connect(const char *host, const char *port, int proto)
{
	return 0;
}

int platform_net_socket_recv(int fd, void *buf, size_t len, int flags)
{
    return 0;
}

int platform_net_socket_recv_timeout(int fd, unsigned char *buf, int len, int timeout)
{
	return 0;
}

int platform_net_socket_write(int fd, void *buf, size_t len)
{
	return 0;
}


int platform_net_socket_write_timeout(int fd, unsigned char *buf, int len, int timeout)
{
	return 0;
}

int platform_net_socket_close(int fd)
{
	return 0;
}

int platform_net_socket_set_block(int fd)
{
	return 0;
}

int platform_net_socket_set_nonblock(int fd)
{
	return 0;
}

int platform_net_socket_setsockopt(int fd, int level, int optname, const void *optval, socklen_t optlen)
{
	return 0;
}

