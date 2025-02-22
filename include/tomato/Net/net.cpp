#include "net.hpp"

#include <cstdio>

#include "utils.hpp"


tmt::net::NetClient::NetClient()
{

}

tmt::net::NetClient::~NetClient()
{
    enet_peer_reset(serverPeer);
    enet_host_destroy(client);
}

void tmt::net::NetClient::process()
{
    ENetEvent event;
    while (enet_host_service(client, &event, 0) > 0)
    {
        switch (event.type)
        {
            case ENET_EVENT_TYPE_CONNECT:
                printf("Connected to server.\n");
                break;
            case ENET_EVENT_TYPE_RECEIVE:
                printf("A packet of length %lu containing %s was received from %s on channel %u.\n",
                       event.packet->dataLength, event.packet->data, event.peer->data, event.channelID);
                enet_packet_destroy(event.packet);
                break;
            case ENET_EVENT_TYPE_DISCONNECT:
                printf("Disconnected.\n");
                event.peer->data = nullptr;
        }
    }
}

void tmt::net::NetClient::send(u8 channel, const void* data, size_t size, bool reliable)
{

    if (!serverPeer)
        return;

    ENetPacket* packet = enet_packet_create(
        data, size, reliable ? ENET_PACKET_FLAG_RELIABLE : ENET_PACKET_FLAG_UNRELIABLE_FRAGMENT);
    packet->userData = &serverPeer->connectID;

    enet_peer_send(serverPeer, channel, packet);
}

bool tmt::net::NetClient::connect(const std::string& host, u16 port)
{
    ENetAddress address;

    enet_address_set_host(&address, host.c_str());
    address.port = port;

    client = enet_host_create(nullptr, 1, 2, 0, 0);
    if (client == nullptr)
    {
        fprintf(stderr, "An error occurred while trying to create the client.\n");
        exit(EXIT_FAILURE);
    }

    serverPeer = enet_host_connect(client, &address, 2, 0);
    if (client == nullptr)
    {
        fprintf(stderr, "No available peers for initiating an ENet connection.\n");
        exit(EXIT_FAILURE);
    }

    return true;
}

tmt::net::NetServer::NetServer(u16 port, u32 maxClients)
{
    ENetAddress address;
    address.host = ENET_HOST_ANY;
    address.port = port;

    var server = enet_host_create(&address, maxClients, 2, 0, 0);

    if (server == nullptr)
    {
        fprintf(stderr, "An error occurred while trying to create an ENet server host.\n");
        exit(EXIT_FAILURE);
    }

    this->server = server;
}

tmt::net::NetServer::~NetServer()
{
    enet_host_destroy(server);
}

void tmt::net::NetServer::process()
{

    ENetEvent event;
    while (enet_host_service(server, &event, 0) > 0)
    {
        switch (event.type)
        {
            case ENET_EVENT_TYPE_CONNECT:
            {
                printf("A new client connected from %x:%u.\n", event.peer->address.host, event.peer->address.port);

                var ref = new NetClientRef(event.peer);

                ref->id = clients.size();

                clients.push_back(ref);
            }
            break;
            case ENET_EVENT_TYPE_RECEIVE:
                printf("A packet of length %lu containing %s was received from %s on channel %u.\n",
                       event.packet->dataLength, event.packet->data, event.peer->data, event.channelID);
                enet_packet_destroy(event.packet);
                break;
            case ENET_EVENT_TYPE_DISCONNECT:
            {
                disconnectClient(event.peer->connectID);
                printf("%s disconnected.\n", event.peer->data);
            }
            break;

        }
    }
}

void tmt::net::NetServer::broadcast(u8 channel, const void* data, size_t size, bool reliable)
{
    ENetPacket* packet =
        enet_packet_create(data, size, reliable ? ENET_PACKET_FLAG_RELIABLE : ENET_PACKET_FLAG_UNRELIABLE_FRAGMENT);
    enet_host_broadcast(server, channel, packet);
}

void tmt::net::NetServer::sendToClient(u16 clientId, u8 channel, const void* data, size_t size, bool reliable)
{
    if (clientId >= clients.size())
        return;
    ENetPacket* packet =
        enet_packet_create(data, size, reliable ? ENET_PACKET_FLAG_RELIABLE : ENET_PACKET_FLAG_UNRELIABLE_FRAGMENT);
    enet_peer_send(clients[clientId]->peer, channel, packet);
}

void tmt::net::NetServer::disconnectClient(u16 clientId)
{
    if (clientId >= clients.size())
        return;
    enet_peer_disconnect(clients[clientId]->peer, 0);
    clients.erase(clients.begin() + clientId);
}

void tmt::net::init()
{

    if (enet_initialize() != 0)
    {
        fprintf(stderr, "An error occurred while initializing ENet.\n");
        return;
    }

    atexit(enet_deinitialize);
}
