/*
 * https_request.cpp
 *
 *  Created on: Jul 23, 2017
 *      Author: nullifiedcat
 */

#include <core/logging.hpp>
#include "https_request.hpp"

#include <string.h>
#include <netdb.h>
#include <unistd.h>

#include <openssl/err.h>

#include <stdexcept>
#include <memory>

#include "xorstring.h"

namespace https
{

SSL_CTX *ssl_context;

bool initialized = false;

RAII_HTTPS_Socket::RAII_HTTPS_Socket(const std::string &host) : hostname_(host)
{
    if (!initialized)
    {
        logging::Info(XORSTR("Initializing SSL"));
        initialize();
    }
    sock_ = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_ < 0)
    {
        throw std::runtime_error(XORSTR("Socket creation error"));
    }
    host_ = gethostbyname(hostname_.c_str());
    if (not host_)
    {
        throw std::runtime_error(XORSTR("Could not resolve hostname: ") + host);
    }
    memset(&addr_, 0, sizeof(addr_));
    addr_.sin_family = AF_INET;
    addr_.sin_port   = htons(443);
    memcpy(&addr_.sin_addr.s_addr, host_->h_addr, host_->h_length);
    if (connect(sock_, (sockaddr *) &addr_, sizeof(addr_)) < 0)
    {
        close(sock_);
        sock_ = -1;
        throw std::runtime_error(XORSTR("Couldn't connect to host"));
    }
    ssl_connect();
}

RAII_HTTPS_Socket::~RAII_HTTPS_Socket()
{
    ssl_die();
    if (sock_ >= 0)
        close(sock_);
}

bool RAII_HTTPS_Socket::ssl_connect()
{
    connection_ = SSL_new(ssl_context);
    SSL_set_fd(connection_, sock_);
    int ret = SSL_connect(connection_);
    if (ret != 1)
    {
        logging::Info(XORSTR("SSL connection error: %d, %d, %x\n"), ret,
                      SSL_get_error(connection_, ret), ERR_get_error());
        return false;
    }
    return true;
}

void RAII_HTTPS_Socket::ssl_die()
{
    if (connection_)
    {
        SSL_free(connection_);
        connection_ = nullptr;
    }
}

std::string RAII_HTTPS_Socket::get(const std::string &path)
{
    constexpr size_t rq_size = 1024 * 8;
    constexpr size_t rs_size = 1024 * 1024;

    std::unique_ptr<char> buffer_rq(new char[rq_size]),
        buffer_rs(new char[rs_size]);

    memset(buffer_rq.get(), 0, rq_size);
    int rq_length = snprintf(buffer_rq.get(), rq_size,
                             XORSTR("GET %s HTTP/1.0\r\nHost: %s\r\n\r\n"),
                             path.c_str(), hostname_.c_str());
    int sent  = 0;
    int chunk = 0;
    do
    {
        chunk =
            SSL_write(connection_, buffer_rq.get() + sent, rq_length - sent);
        if (chunk < 0)
        {
            throw std::runtime_error(XORSTR("Error writing to Secure Socket: ") +
                                     std::to_string(ERR_get_error()));
        }
        else if (chunk == 0)
        {
            break;
        }
        sent += chunk;
    } while (sent < rq_length);

    memset(buffer_rs.get(), 0, rs_size);
    int total    = rs_size - 1;
    int received = 0;
    do
    {
        chunk =
            SSL_read(connection_, buffer_rs.get() + received, total - received);
        if (chunk < 0)
            throw std::runtime_error(XORSTR("Error reading from socket"));
        if (chunk == 0)
            break;
        received += chunk;
    } while (received < total);

    if (received == total)
        throw std::runtime_error(XORSTR("Response too large"));

    return std::string(buffer_rs.get());
}

void initialize()
{
    SSL_load_error_strings();
    SSL_library_init();
    ssl_context = SSL_CTX_new(SSLv23_client_method());
    initialized = true;
}
}
