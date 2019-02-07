#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string>
#include <fstream>
#include <string>
#include <stdexcept>
#include "NetStream.h"

using std::ifstream;
using std::runtime_error;
using std::string;
namespace TcpUtils
{
class tcpclnt_error : public runtime_error
{
    using runtime_error::runtime_error;
};

class TcpClnt
{
  public:
    TcpClnt() = delete;
    TcpClnt(sockaddr_in const &addr);
    TcpClnt(string ip, int port);
    int GetSock();
    NetStream& GetNetStream();
    void Connect();
  private:
    int serverfd;
    sockaddr_in srv_addr;
    NetStream ns;
};
} // namespace TcpUtils
