#include <string>
#include <cstring>
#include <cstdlib>
#include <stdexcept>
#include <limits.h>

#include "Config.hpp"
#include "controller/Controller.hpp"
#include "server/Request.hpp"
#include "server/Response.hpp"
#include "server/TcpConnection.hpp"
#include "http/HttpStatus.hpp"
#include "error/TodoError.hpp"
#include "Utils.hpp"

Controller::Controller(Config const& config) : m_config(config)
{
    
}

Controller::~Controller()
{
    
}

void Controller::send_error_response(Response& res, HttpStatus const& status, std::string response_text)
{
    try
    {
        res.set_status(status);
        res.set_header("Content-Type", "text/plain");
        res.set_header("Content-Length", std::to_string(response_text.size()));
        res.send(response_text.c_str(), response_text.size());
    }
    catch (std::runtime_error const& e)
    {
        d_warn("Could not write error response");
        d_warnf("Error: %s", e.what());
    }
}

void Controller::send_error_response(Config const& config, TcpConnection* conn, HttpStatus const& status, std::string response_text)
{
    try
    {
        Response res(config, *conn);
        res.set_status(status);
        res.set_header("Content-Type", "text/plain");
        res.set_header("Content-Length", std::to_string(response_text.size()));
        res.send(response_text.c_str(), response_text.size());
    }
    catch (std::runtime_error const& e)
    {
        d_warn("Could not write error response");
        d_warnf("Error: %s", e.what());
    }
}

bool Controller::resolve_requested_path(std::string const& requested, std::string const& basedir, std::string& resolved) const noexcept
{
    char* resolved_path = (char*)malloc(1024*sizeof(char));
    std::string temp = basedir+requested;
    char* ptr = realpath(temp.c_str(), resolved_path);

    if (ptr == NULL) {
    	free(resolved_path);
    	//std::string error = path+"could not be found\r\n";
	//Controller::send_error_response(res, HttpStatus::NotFound, error);
	return false;
    }
    
    std::string str1(resolved_path, strlen(resolved_path));
    
    char* resolvedStaticDir = (char*)malloc(1024*sizeof(char));
    resolvedStaticDir = realpath(basedir.c_str(), resolvedStaticDir);
    std::string str2(resolvedStaticDir, strlen(resolvedStaticDir));

    if (str1.substr(0, str2.length()) !=  str2){
	free(resolved_path);
	free(resolvedStaticDir);
	return false;
    }
    resolved = str1;
    free(resolved_path);
    free(resolvedStaticDir);
    return true;
    //throw TodoError("4", "You need to implement resolving request paths");
    
}
