#ifndef NETSTREAM_H
#define NETSTREAM_H

#include <string>
#include <unistd.h>
#include <stdexcept>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/select.h>
#include <vector>
#include <iostream>

using std::runtime_error;
using std::string;
using std::vector;
using std::wstring;

namespace TcpUtils
{
enum ErrorType
{
    ReadError,
    WriteError,
    CloseError,
    NoSocketError,
    CastError
};

class netstream_error : public runtime_error
{
  public:
    using runtime_error::runtime_error;
    netstream_error(const char *what, enum ErrorType et) : runtime_error(what)
    {
        this->et = et;
    }
    ErrorType GetErrorType() { return et; }

  private:
    ErrorType et;
};

class NetStream
{
  public:
    NetStream(int sockfd);
    NetStream();
    ~NetStream();
    virtual NetStream &operator<<(int i);
    virtual NetStream &operator>>(int &i);
    virtual NetStream &operator<<(long l);
    virtual NetStream &operator>>(long &l);
    virtual NetStream &operator<<(short s);
    virtual NetStream &operator>>(short &s);
    virtual NetStream &operator<<(uint16_t us);
    virtual NetStream &operator>>(uint16_t &us);
    virtual NetStream &operator<<(uint32_t ui);
    virtual NetStream &operator>>(uint32_t &ui);
    virtual NetStream &operator<<(uint64_t ul);
    virtual NetStream &operator>>(uint64_t &ul);
    virtual NetStream &operator<<(char c);
    virtual NetStream &operator>>(char &c);
    virtual NetStream &operator<<(string str);
    virtual NetStream &operator>>(string &str);
    virtual NetStream &operator<<(wstring wstr);
    virtual NetStream &operator>>(wstring &wstr);
    virtual NetStream &operator<<(const char *c_str);
    virtual NetStream &operator<<(const wchar_t *wc_str);
    virtual void Write(void *sendBuf, size_t len);
    virtual void Read(void *recvBuf, size_t len);

    void SetSocket(int sockfd);
    bool SocketSet();
    void Close();

     // forbid copying
     NetStream(NetStream const&) = delete;
     NetStream& operator=(NetStream const&) = delete;

  protected:
    int sockfd;
    bool sockset{false};
    inline void checkSockSet();

  private:
    NetStream &castBySizeOut(void *d, size_t s);
    NetStream &castBySizeIn(void *d, size_t s);
    static vector<int> sockfds;
};
} // namespace TcpUtils
#endif
