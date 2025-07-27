#ifndef HANDLERS_HPP
#define HANDLERS_HPP

#include <string>
#include <iostream>
#include <system_error>

const std::string &getCachedResponse(int keepAliveTimeout)
{
    static const std::string response = [&]()
    {
        std::string body =
            "<html><body><h1>Merhaba Dunya!</h1>"
            "<p>Bu son derece optimize C++ web sunucusudur.</p></body></html>";
        std::string headers =
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/html; charset=UTF-8\r\n"
            "Content-Length: " +
            std::to_string(body.size()) + "\r\n"
                                          "Connection: keep-alive\r\n"
                                          "Keep-Alive: timeout=" +
            std::to_string(keepAliveTimeout) + "\r\n"
                                               "\r\n";
        return headers + body;
    }();
    return response;
}

#endif
