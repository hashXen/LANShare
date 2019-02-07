#ifndef TCPSRV_H
#define TCPSRV_H

#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <vector>
#include <string>
#include <cstring>
#include <map>
#include <exception>
#include <sys/select.h>

using std::map;
using std::runtime_error;
using std::string;
using std::vector;

namespace TcpUtils
{
class tcpsrv_error : public runtime_error
{
    using runtime_error::runtime_error;
};

class TcpSrv
{
  public:
    TcpSrv();
    TcpSrv(int port, int backlog, int capacity);
    TcpSrv(int port, int backlog) : TcpSrv(port, backlog, 20) {}
    TcpSrv(int port) : TcpSrv(port, 5, 20) {}
    void Setup(int port, int backlog, int capacity);
    bool IsReady() {return ready;}
    ~TcpSrv();
    int Send(int clientfd, void *buf, size_t len, int num_tries = 5);
    int Send(int clientfd, string msg, int num_tries = 5);
    int Recv(int clientfd, void *buf, size_t len, int num_tries = 5);
    void Broadcast(void *buf, size_t len);
    void Broadcast(string mesg);
    void RemoveClient(int clientfd);
    int Accept();
    map<int, sockaddr_in> GetClients() {return clients;}
    vector<int> GetUnprocessedClients(); // C++11's preferred way to return
    unsigned int GetCapacity();
    unsigned int GetNumClients();
    // forbid copying
    TcpSrv(TcpSrv const &) = delete;
    TcpSrv& operator=(TcpSrv const &) = delete;

  private:
    void UpdateClientList();
    map<int, sockaddr_in> clients;
    int serverfd;
    unsigned int capacity;
    fd_set all_fds;
    int max_fd;
    bool ready{false};
};
} // namespace TcpUtils
#endif
