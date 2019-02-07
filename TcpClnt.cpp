#include <cstring>
#include "TcpClnt.h"
using namespace TcpUtils;

TcpClnt::TcpClnt(sockaddr_in const& addr) {
    serverfd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverfd == -1) {
        throw tcpclnt_error("Failed to get a TCP socket");
    }
    srv_addr = addr;
}

TcpClnt::TcpClnt(string ip, int port) {
    auto ipaddr = inet_addr(ip.c_str());
    if (ipaddr == INADDR_NONE) {
        throw tcpclnt_error("Invalid IP address provided.");
    }    
    srv_addr.sin_addr.s_addr = ipaddr;
    srv_addr.sin_port = htons(port);
    srv_addr.sin_family = AF_INET;
    memset(srv_addr.sin_zero, 0, 8);
}

void TcpClnt::Connect() {
    socklen_t len = sizeof(sockaddr);
    if (connect(serverfd, reinterpret_cast<sockaddr *>(&srv_addr), len) == -1) {
        throw tcpclnt_error("connect() failed on current socket");
    }
}

int TcpClnt::GetSock() {
    return serverfd;
}

NetStream& TcpClnt::GetNetStream() {
    ns.SetSocket(serverfd);
    return ns;
}
