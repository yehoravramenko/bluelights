#pragma comment(lib, "WS2_32.lib")

#include <print>
#include <WinSock2.h>

int main([[maybe_unused]] int argc, [[maybe_unused]] char **argv)
{
    WSADATA wsaData = {};
    int retval      = 0;

    // struct addrinfo *result = nullptr, *ptr = nullptr, hints = {};

    std::println("bluelights!");

    retval = WSAStartup(MAKEWORD(2, 2), &wsaData);

    if (retval != 0)
    {
        std::println("WSAStartup failed: {}", retval);
        return 1;
    }

    return 0;
}