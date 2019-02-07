#include "TcpSrv.h"
#include <iostream>
#include <sstream>
#include <thread>

using std::cin;
using std::cout;
using std::endl;
using std::exception;
using std::map;
using std::memset;
using std::ostringstream;

using namespace TcpUtils;

TcpSrv::TcpSrv() {}

TcpSrv::TcpSrv(int port, int backlog, int capacity)
{
    Setup(port, backlog, capacity);
}

TcpSrv::Setup(int port, int backlog, int capacity)
{
    // Set max to FD_SETSIZE for select
    this->capacity = capacity > FD_SETSIZE ? FD_SETSIZE : capacity;

    // Get a socket
    serverfd = socket(PF_INET, SOCK_STREAM, 0);
    if (serverfd < 0)
    {
        throw tcpsrv_error("Failed to get a server socket");
    }

    // Zero the set
    FD_ZERO(&all_fds);

    // Make address reusable
    int on = 1;
    int status = setsockopt(serverfd, SOL_SOCKET, SO_REUSEADDR,
                            (const char *)&on, sizeof(on));
    if (status < 0)
    {
        throw tcpsrv_error("Failed to make the server socket address reusable");
    }

    // Set up address
    struct sockaddr_in *addr = new sockaddr_in();
    addr->sin_family = PF_INET;
    addr->sin_port = htons(port);
    memset(&(addr->sin_zero), 0, 8);
    addr->sin_addr.s_addr = INADDR_ANY;

    // Bind the socket to "addr:port"
    if (bind(serverfd, (struct sockaddr *)addr, sizeof(*addr)) < 0)
    {
        // bind failed; could be because port is in use.
        ostringstream errorStringStream;
        errorStringStream << "Failed to bind the socket. Is the port ";
        errorStringStream << port;
        errorStringStream << " in use?";
        throw tcpsrv_error(errorStringStream.str().c_str());
    }

    // Listen on serverfd.
    if (listen(serverfd, backlog) < 0)
    {
        // listen failed
        throw tcpsrv_error("Failed to listen on the server socket");
    }
    ready = true;
}

/* Accepts a new client and adds the client to the clients map. If the server is 
at capacity, return -1. If accept() runs into an error, throw a tcpsrv_error.
   On success, Accept() returns the newly accepted client's socket.
*/
int TcpSrv::Accept()
{
    sockaddr_in addr;
    int clientfd;
    socklen_t addrlen = sizeof(sockaddr_in);
    if (clients.size() < capacity)
    {
        if ((clientfd = accept(serverfd, (sockaddr *)&addr, &addrlen)) == -1)
        {
            throw tcpsrv_error("Failed to accept a new client");
        }
        else
        {
            clients[clientfd] = addr;
            max_fd = clientfd > max_fd ? clientfd : max_fd;
            FD_SET(clientfd, &all_fds);
            return clientfd;
        }
    }
    return -1;
}

TcpSrv::~TcpSrv()
{
    for (map<int, sockaddr_in>::iterator it = clients.begin(); it != clients.end();
         ++it)
    {
        close(it->first);
    }
    close(serverfd);
}

int TcpSrv::Send(int clientfd, void *buf, size_t len, int num_tries)
{
    if (clients.count(clientfd))
    {
        while (num_tries > 0)
        {
            if (write(clientfd, buf, len) < 0)
            {
                --num_tries;
                std::this_thread::sleep_for(std::chrono::seconds(2)); // wait 2 seconds
            }
            else
            {
                return 0;
            }
        }                        // Out of tries
        clients.erase(clientfd); // delete the disconnected client from the list
        return -1;
    }
    return -2;
}

int TcpSrv::Send(int clientfd, string msg, int num_tries)
{
    return TcpSrv::Send(clientfd, (void *)msg.c_str(), msg.length() + 1, num_tries);
}

int TcpSrv::Recv(int clientfd, void *buf, size_t len, int num_tries)
{
    if (clients.count(clientfd))
    {
        while (num_tries > 0)
        {
            int num_read;
            if ((num_read = read(clientfd, buf, len)) < 0)
            {
                --num_tries;
                std::this_thread::sleep_for(std::chrono::seconds(2)); // wait 2 seconds
            } else if (num_read == 0){
                RemoveClient(clientfd);
                return 0;
            }
            else
            {
                return num_read;
            }
        }                        // out of tries
        RemoveClient(clientfd); // delete the disconnected client from the list
        return -1;
    }
    return -2;
}

void TcpSrv::Broadcast(void *buf, size_t len)
{
    for (map<int, sockaddr_in>::iterator it = clients.begin(); it != clients.end();
         ++it)
    {
        Send(it->first, buf, len);
    }
}

void TcpSrv::Broadcast(string mesg)
{
    Broadcast((void *)mesg.c_str(), mesg.length() + 1);
}

void TcpSrv::RemoveClient(int clientfd)
{
    FD_CLR(clientfd, &all_fds);
    clients.erase(clientfd);
}

unsigned int TcpSrv::GetCapacity()
{
    return capacity;
}

unsigned int TcpSrv::GetNumClients()
{
    return clients.size();
}

vector<int> TcpSrv::GetUnprocessedClients()
{
    fd_set listen_fds = all_fds;
    vector<int> clients_ready;
    if (clients.size() == 0) {
        return clients_ready;
    }
    cout << "Before Select" << endl;
    int nready = select(max_fd + 1, &listen_fds, nullptr, nullptr, nullptr);
    cout << "Past Select" << endl;
    if (nready == -1)
    {
        throw tcpsrv_error("Failed to select unprocessed clients");
    }

    int count = 0;
    for (map<int, sockaddr_in>::iterator it = clients.begin();
         count < nready && it != clients.end();
         ++it)
    {
        int clientfd = it->first;
        if (FD_ISSET(clientfd, &listen_fds))
        {
            clients_ready.push_back(clientfd);
            ++count;
        }
    }
    return clients_ready;
}
