#include <string>
#include <iostream>
#include <unistd.h>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include "Utils.hpp"
#include "Config.hpp"
#include "server/TcpConnection.hpp"
#include "error/ConnectionError.hpp"
#include "error/SocketError.hpp"
#include "error/TodoError.hpp"

TcpConnection::TcpConnection(Config const& config, int master_fd) :
    m_config(config),
    m_master(master_fd),
    m_shutdown(false)
{
    //set up child sockaddr_in
    struct sockaddr_in client_addr;
    
    socklen_t size = sizeof(client_addr);
    // accept()
    m_conn = accept(m_master,(struct sockaddr *)&client_addr,&size);

    //throw TodoError("2", "You need to implement construction of TcpConnections");
}

TcpConnection::~TcpConnection() noexcept
{
    d_printf("Closing connection on %d", m_conn);

    if (close(m_conn) == -1) d_errorf("Could not close connection %d", m_conn);
}

void TcpConnection::shutdown()
{
    d_printf("Shutting down connection on %d", m_conn);
    
    if (::shutdown(m_conn, SHUT_RDWR) == -1) d_errorf("Could not shut down connection %d", m_conn);

    m_shutdown = true;
}

bool TcpConnection::getc(unsigned char* c)
{
    // read byte from m_conn and store it in c
    int count = recv(m_conn, c, 1, 0);   
    // no more byte return false
    if (count == 0) return false;
    return true;

    //throw TodoError("2", "You have to implement reading from connections");
}

void TcpConnection::putc(unsigned char c)
{
    char buff[1];
    *buff = c;
    // write c to m_conn
    write(m_conn, buff, 1);
    //throw TodoError("2", "You need to implement writing characters to connections");
}

void TcpConnection::puts(std::string const& str)
{
    // write string to m_conn
    //std::cout << "im using puts to send" << str <<std::endl;
    //char buff[str.length()];
    //str.copy(buff, str.length(), 0);
    write(m_conn, str.c_str(), str.length());
    //throw TodoError("2", "You need to implement writing strings to connections");
}

void TcpConnection::putbuf(void const* buf, size_t bufsize)
{
    write(m_conn, buf, bufsize);
    //throw TodoError("2", "You need to implement writing buffers to connections");
}
