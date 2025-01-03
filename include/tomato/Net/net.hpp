#if !defined(NET_HPP)
#define NET_HPP

#include <enet/enet.h>

#include "utils.hpp"


namespace tmt::net
{

    struct NetClient
    {
        NetClient();
        ~NetClient();

        void process();
        void send(u8 channel, const void* data, size_t size, bool reliable);
        bool connect(const std::string& host, u16 port);

    private:
        ENetHost* client;
        ENetPeer* serverPeer;
    };

    struct NetClientRef
    {
        u16 id;
        NetClientRef(ENetPeer* peer);

    private:
        friend struct NetServer;
        ENetPeer* peer;
    };

    struct NetServer
    {
        NetServer(u16 port, u32 maxClients);
        ~NetServer();

        void process();

        void broadcast(u8 channel, const void* data, size_t size, bool reliable);
        void sendToClient(u16 clientId, u8 channel, const void* data, size_t size, bool reliable);
        void disconnectClient(u16 clientId);

    private:
        std::vector<NetClientRef*> clients;

        ENetHost* server;
    };

    void init();

    void shutdown();
    void update();

} // namespace tmt::net


#endif // NET_HPP
