/*
 * https_request.hpp
 *
 *  Created on: Jul 23, 2017
 *      Author: nullifiedcat
 */

#pragma once

#include <openssl/ssl.h>
#include <netinet/in.h>

#include <string>

namespace https
{

extern SSL_CTX *ssl_context;

class RAII_HTTPS_Socket
{
public:
    RAII_HTTPS_Socket(const std::string &host);
    ~RAII_HTTPS_Socket();

    bool ssl_connect();
    void ssl_die();

    std::string get(const std::string &path);

    const std::string hostname_;

    hostent *host_{ nullptr };
    sockaddr_in addr_{};
    int sock_{ -1 };
    SSL *connection_{ nullptr };
};

void initialize();
} // namespace https
