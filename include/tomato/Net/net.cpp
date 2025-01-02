#include "net.hpp"

#include <cstdio>
#include <enet/enet.h>

ENetAddress address;
ENetHost* server;

void tmt::net::init()
{
    if (enet_initialize() != 0)
    {
        fprintf(stderr, "An error occurred while initializing ENet.\n");
        return;
    }

    atexit(enet_deinitialize);

    address.host = ENET_HOST_ANY;
    address.port = 1234;

    server = enet_host_create(&address, 32, 2, 0, 0);

    if (server == nullptr)
    {
        fprintf(stderr, "An error occurred while trying to create an ENet server host.\n");
        exit(EXIT_FAILURE);
    }

    atexit([]()
    {
        enet_host_destroy(server);
    });

}
