#include "NetStream.h"
#include <algorithm>

using namespace TcpUtils;

vector<int> NetStream::sockfds;

NetStream::NetStream(int sockfd)
{
    if (std::find(sockfds.begin(), sockfds.end(), sockfd) != sockfds.end())
    {
        throw netstream_error("Failed to initialize: an instance of NetStream is on this socket");
    }
    SetSocket(sockfd);
    sockfds.push_back(sockfd);
}
NetStream::NetStream() {}
NetStream::~NetStream()
{
    auto it = std::find(sockfds.begin(), sockfds.end(), sockfd);
    sockfds.erase(it);
}
NetStream &NetStream::operator<<(int i)
{
    return castBySizeOut(&i, sizeof(i));
}
NetStream &NetStream::operator>>(int &i)
{
    return castBySizeIn(&i, sizeof(i));
}
NetStream &NetStream::operator<<(long l)
{
    return castBySizeOut(&l, sizeof(l));
}
NetStream &NetStream::operator>>(long &l)
{
    return castBySizeIn(&l, sizeof(l));
}
NetStream &NetStream::operator<<(short s)
{
    return castBySizeOut(&s, sizeof(s));
}
NetStream &NetStream::operator>>(short &s)
{
    return castBySizeIn(&s, sizeof(s));
}
NetStream &NetStream::operator<<(uint32_t ui)
{
    uint32_t nui = htonl(ui);
    Write(&nui, sizeof(uint32_t));
    return *this;
}
NetStream &NetStream::operator>>(uint32_t &i)
{
    checkSockSet();
    uint32_t net_i;
    Read(&net_i, sizeof(uint32_t));
    i = ntohl(net_i);
    return *this;
}
NetStream &NetStream::operator<<(uint16_t us)
{
    checkSockSet();
    uint16_t nus = htons(us);
    Write(&nus, sizeof(uint16_t));
    return *this;
}
NetStream &NetStream::operator>>(uint16_t &us)
{
    checkSockSet();
    uint16_t net_s;
    Read(&net_s, sizeof(uint16_t));
    us = ntohs(net_s);
    return *this;
}
NetStream &NetStream::operator<<(uint64_t ul)
{
    // divide the long into two ints with a union trick
    union {
        uint32_t low_high[2];
        uint64_t ulong_int;
    };
    ulong_int = ul;
    return *this << low_high[1] << low_high[0];
}
NetStream &NetStream::operator>>(uint64_t &ul)
{
    union {
        uint32_t low_high[2];
        uint64_t ulong_int;
    };
    *this >> low_high[1] >> low_high[0];
    ul = ulong_int;
    return *this;
}
NetStream &NetStream::operator<<(string str)
{
    Write(const_cast<char *>(str.c_str()), str.length() + 1);
    return *this;
}
NetStream &NetStream::operator>>(string &str)
{
    char c = '\n'; // anything, as long as it's not '\0'
    str = string("");
    while (c != '\0')
    {
        *this >> c;
        str.push_back(c);
    }
    return *this;
}
NetStream &NetStream::operator<<(char c)
{
    Write(&c, sizeof(char));
    return *this;
}

NetStream &NetStream::operator>>(char &c)
{
    Read(&c, sizeof(char));
    return *this;
}
NetStream &NetStream::operator<<(wstring wstr)
{
    for (auto it = wstr.begin(); it != wstr.end(); ++it)
    {
        wchar_t wc = *it;
        *this << wc;
    }
    return *this << L'\0';
}
NetStream &NetStream::operator>>(wstring &wstr)
{
    checkSockSet();
    wchar_t wc = L'a'; // as long as it's not L'\0'
    while (wc != L'\0')
    {
        castBySizeIn(&wc, sizeof(wchar_t));
        wstr.push_back(wc);
    }
    return *this;
}
NetStream &NetStream::operator<<(const char *c_str)
{
    return *this << string(c_str);
}
NetStream &NetStream::operator<<(const wchar_t *wc_str)
{
    for (int i = 0; wc_str[i] != L'\0'; ++i)
    {
        wchar_t wc = wc_str[i];
        *this << wc;
    }
    return *this << L'\0';
}
void NetStream::SetSocket(int sockfd)
{
    this->sockfd = sockfd;
    sockset = true;
}
bool NetStream::SocketSet()
{
    return sockset;
}
void NetStream::Close()
{
    checkSockSet();
    if (close(sockfd) == -1)
    {
        throw netstream_error("close() failed on current socket", CloseError);
    }
    sockset = false;
}
void NetStream::checkSockSet()
{
    if (!sockset)
    {
        throw netstream_error("No socket set.", NoSocketError);
    }
}
void NetStream::Write(void *sendBuf, size_t len)
{
    checkSockSet();
    size_t total_sent = 0;
    int sent;
    char *buf = reinterpret_cast<char *>(sendBuf);
    while (total_sent != len)
    {
        sent = write(sockfd, &buf[total_sent], len - total_sent);
        if (sent == -1)
        {
            throw netstream_error("write() failed on current socket", WriteError);
        }
        total_sent += sent;
    }
}
void NetStream::Read(void *recvBuf, size_t len)
{
    checkSockSet();
    size_t total_received = 0;
    int received;
    char *buf = reinterpret_cast<char *>(recvBuf);
    while (total_received != len)
    {
        received = read(sockfd, &buf[total_received], len - total_received);
        if (received == -1)
        {
            throw netstream_error("read() failed on current socket", ReadError);
        }
        total_received += received;
    }
}
NetStream &NetStream::castBySizeOut(void *d, size_t s)
{
    switch (s)
    {
    case 2:
    {
        uint16_t us = *reinterpret_cast<uint16_t *>(d);
        return *this << us;
    }
    case 4:
    {
        uint32_t ui = *reinterpret_cast<uint32_t *>(d);
        return *this << ui;
    }
    case 8:
    {
        uint64_t ul = *reinterpret_cast<uint64_t *>(d);
        return *this << ul;
    }
    default:
    {
        throw netstream_error("Invalid size provided for casting", CastError);
        break;
    }
    }
}
NetStream &NetStream::castBySizeIn(void *d, size_t s)
{
    switch (s)
    {
    case 2:
    {
        return *this >> *reinterpret_cast<uint16_t *>(d);
    }
    case 4:
    {
        return *this >> *reinterpret_cast<uint32_t *>(d);
    }
    case 8:
    {
        return *this >> *reinterpret_cast<uint64_t *>(d);
    }
    default:
    {
        throw netstream_error("Invalid size provided for casting", CastError);
        break;
    }
    }
}
