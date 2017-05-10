#include <string>
#include <cstring>
#include <map>

#include "server/Response.hpp"
#include "server/TcpConnection.hpp"
#include "http/HttpStatus.hpp"
#include "error/ResponseError.hpp"
#include "error/TodoError.hpp"
#include "Config.hpp"

Response::Response(Config const& config, TcpConnection& conn) :
    m_config(config),
    m_conn(conn),
    m_headers_sent(false)
{
    // We want every response to have this header
    // It tells browsers that we want separate connections per request
    m_headers["Connection"] = "close";
}

void Response::send(void const* buf, size_t bufsize, bool raw)
{
    std::string response_line;
    response_line+="HTTP/1.0 ";
    response_line+=m_status_text;
    response_line+="\r\n";
    
    //send http/1.0 200 ok
    m_conn.puts(response_line);
    //send hears
    send_headers();
    //leave blank line
    response_line = "\r\n";
    m_conn.puts(response_line);
    //send actual content
    m_conn.putbuf(buf, bufsize);   
    //throw TodoError("2", "You need to implement sending responses");
}

void Response::send_headers()
{
    std::map<std::string, std::string>::iterator it;

    for ( it = m_headers.begin(); it != m_headers.end(); it++ )
    {
	std::string buffer;
	buffer += it->first.c_str();
	buffer += ": ";
	buffer += it->second.c_str();
	buffer += "\r\n";
   	m_conn.puts(buffer); 
    }
    m_headers_sent = true;
    //throw TodoError("2", "You need to implement sending headers");
}

void Response::set_header(std::string const& key, std::string const& value)
{
    m_headers[key] = value;
    //throw TodoError("2", "You need to implement controllers setting headers");
}

void Response::set_status(HttpStatus const& status)
{
    m_status_text = status.to_string();
}
