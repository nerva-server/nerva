#ifndef SERVER_CONFIG_HPP
#define SERVER_CONFIG_HPP

#include <string>

class ServerConfig
{
public:
    const int PORT = 8080;
    const int BUFFER_SIZE = 4096;
    const int THREAD_POOL_SIZE = 100;
    const int KEEP_ALIVE_TIMEOUT = 5;
    const int MAX_CONNECTIONS = 500000;
    const int ACCEPT_QUEUE_SIZE = 65535;
    const int ACCEPT_RETRY_DELAY_MS = 10;
    const int MAX_EVENTS = 8192;
    
    ServerConfig() = default;
};

#endif
