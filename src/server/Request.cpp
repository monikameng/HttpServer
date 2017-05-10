#include <unordered_map>
#include <vector>
#include <string>
#include <cstring>
#include <iostream>
#include <cassert>
#include <cstdlib>
#include <stdexcept>

#include "server/Request.hpp"
#include "http/HttpStatus.hpp"
#include "server/TcpConnection.hpp"
#include "Config.hpp"
#include "Utils.hpp"
#include "error/RequestError.hpp"
#include "error/ConnectionError.hpp"
#include "error/TodoError.hpp"


Request::Request(Config const& config, TcpConnection& conn) :
    m_config(config),
    m_conn(conn)
{
    std::string request_line = parse_raw_line();
    
    parse_method(request_line);
    parse_route(request_line);
    parse_version(request_line);

    // the previous three parse_* calls should consume the entire line
    if (!request_line.empty())
    {
        throw RequestError(HttpStatus::BadRequest, "Malformed request-line\n");
    }

    parse_headers();
    parse_body();
}

void Request::parse_method(std::string& raw_line)
{
    std::string temp = raw_line.substr(0,3);
    if (temp.compare("GET")==0) {
	m_method = "GET";
	raw_line = raw_line.substr(4,raw_line.length()-4);
	std::cout<<"the method is GET" << std::endl;
	return;
    }
    temp = raw_line.substr(0,4);
    if (temp.compare("POST")==0) {
    	m_method = "POST";
	raw_line = raw_line.substr(5,raw_line.length()-5);
	std::cout<<"the method is POST" << std::endl;	
	return;
    }
    throw RequestError(HttpStatus::MethodNotAllowed, "Method Not Allowed");
    //throw TodoError("2", "You have to implement parsing methods");
}

void Request::parse_route(std::string& raw_line)
{
    size_t firstSlash = raw_line.find('/');
    if (firstSlash == std::string::npos || firstSlash != 0) {
	throw RequestError(HttpStatus::BadRequest, "Bad Request");
    }
    size_t firstSpace = raw_line.find(' ');

    m_path = raw_line.substr(firstSlash, (firstSpace-firstSlash));
    raw_line = raw_line.substr(firstSpace+1, raw_line.length()-firstSlash);
    //FOR EXTRA CREDIT: call parse_querystring() with the
    //start of the parsed path after the first "?" to fill in the m_query map

    //throw TodoError("2", "You have to implement parsing routes");
}

void Request::parse_querystring(std::string query, std::unordered_map<std::string, std::string>& parsed)
{
    throw TodoError("6", "You have to implement parsing querystrings");
}

void Request::parse_version(std::string& raw_line)
{
    std::size_t http1 = raw_line.find("HTTP/1.0");
    std::size_t http2 = raw_line.find("HTTP/1.1");

    if (http1 != std::string::npos){
	m_version = "HTTP/1.0";
    } else if (http2 != std::string::npos){
	m_version = "HTTP/1.1";
    } else 
    	throw RequestError(HttpStatus::HttpVersionNotSupported, "HTTP Version Not Supported");
   
    raw_line = raw_line.substr(http1+8, raw_line.length()-8);
    if (raw_line.empty()) return;
    while (!raw_line.empty()) {
	if (raw_line.at(0)==' '){
		raw_line=raw_line.substr(1,raw_line.length()-1);
	} else return;
    }
    //throw TodoError("2", "You have to implement parsing HTTP version");
}

void Request::parse_headers()
{
    std::string line;
    std::string key;
    std::string value;
    line = parse_raw_line();
    while (line.length() != 0) {
	size_t colon = line.find(':');
	key = line.substr(0, colon);
	value = line.substr(colon+2, line.length()-(colon+2));
	m_headers.insert(std::pair<std::string, std::string>(key,value));
    
    	line = parse_raw_line();
    }
    //throw TodoError("2", "You have to implement parsing headers");
}

void Request::parse_body()
{
    if (m_method == "GET") return;

    throw TodoError("6", "You have to implement parsing request bodies");
}

std::string Request::parse_raw_line()
{
   unsigned char temp;
   std::string buffer;
   while (m_conn.getc(&temp) != false && buffer.length() != m_max_buf) {
	if (temp == '\r') {
		if (m_conn.getc(&temp)!=false){
			if (temp == '\n') break;
			else {
				buffer += '\r';
				buffer += temp;
				continue;
			}
		} else break;
	} else {
		buffer += temp;
	}
   }
   return buffer;
   //throw TodoError("2", "You need to implement line fetching");
}

void Request::print() const noexcept
{
    std::cout << m_method << ' ' << m_path << ' ' << m_version << std::endl;
#ifdef DEBUG    
    for (auto const& el : m_headers)
    {
        std::cout << el.first << ": " << el.second << std::endl;
    }

    for (auto const& el : m_query)
    {
        std::cerr << el.first << ": " << el.second << std::endl;
    }

    for (auto const& el : m_body_data)
    {
        std::cerr << el.first << ": " << el.second << std::endl;
    }
#endif	
}

bool Request::try_header(std::string const& key, std::string& value) const noexcept
{
    if (m_headers.find(key) == m_headers.end())
    {
        return false;
    }
    else
    {
        value = m_headers.at(key);
        return true;
    }
}

std::string const& Request::get_path() const noexcept
{
    return m_path;
}

std::string const& Request::get_method() const noexcept
{
    return m_method;
}

std::string const& Request::get_version() const noexcept
{
    return m_version;
}

std::unordered_map<std::string, std::string> const& Request::get_headers() const noexcept
{
    return m_headers;
}

std::unordered_map<std::string, std::string> const& Request::get_query() const noexcept
{
    return m_query;
}

std::unordered_map<std::string, std::string> const& Request::get_body() const noexcept
{
    return m_body_data;
}
