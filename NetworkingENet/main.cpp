#include <enet/enet.h>

#include <iostream>
#include <string>

ENetAddress address;
ENetHost* server = nullptr;
ENetHost* client = nullptr;

bool CreateServer()
{
    address.host = ENET_HOST_ANY;
    address.port = 1234;
    server = enet_host_create(&address /* the address to bind the server host to */,
        32      /* allow up to 32 clients and/or outgoing connections */,
        2      /* allow up to 2 channels to be used, 0 and 1 */,
        0      /* assume any amount of incoming bandwidth */,
        0      /* assume any amount of outgoing bandwidth */);

    return server != nullptr;
}

bool CreateClient()
{
    client = enet_host_create(NULL /* create a client host */,
        1 /* only allow 1 outgoing connection */,
        2 /* allow up 2 channels to be used, 0 and 1 */,
        0 /* assume any amount of incoming bandwidth */,
        0 /* assume any amount of outgoing bandwidth */);

    return client != nullptr;
}

int main(int argc, char** argv)
{
    if (enet_initialize() != 0)
    {
        fprintf(stderr, "An error occurred while initializing ENet.\n");
        std::cout << "An error occurred while initializing ENet." << std::endl;
        return EXIT_FAILURE;
    }
    atexit(enet_deinitialize);

    std::cout << "1) Create Server " << std::endl;
    std::cout << "2) Create Client " << std::endl;
    int UserInput;
    std::cin >> UserInput;
    if (UserInput == 1)
    {
        if (!CreateServer())
        {
            fprintf(stderr,
                "An error occurred while trying to create an ENet server host.\n");
            exit(EXIT_FAILURE);
        }

        while (1)
        {
            ENetEvent event;
            while (enet_host_service(server, &event, 1000) > 0)
            {
                switch (event.type)
                {
                case ENET_EVENT_TYPE_CONNECT:

                    std::cout << "A new client connected from "
                        << event.peer->address.host
                        << ":" << event.peer->address.port
                        << std::endl;
                    event.peer->data = (void*)("Client information");

                    {
                        ENetPacket* packet = enet_packet_create("hello",
                            strlen("hello") + 1,
                            ENET_PACKET_FLAG_RELIABLE);

                        enet_host_broadcast(server, 0, packet);
                        //enet_peer_send(event.peer, 0, packet);
                        enet_host_flush(server);
                    }
                    break;

                case ENET_EVENT_TYPE_RECEIVE:

                    std::cout << "A packet of length " << event.packet->dataLength << std::endl
                              << " containing " << event.packet->data
                              << std::endl;
                        //<< " was received from " << event.peer->data
                        //<< " on channel " << event.channelID << std::endl;
                    enet_packet_destroy(event.packet);



                    break;

                case ENET_EVENT_TYPE_DISCONNECT:
                    std::cout << (char*)event.peer->data << "disconnnected." << std::endl;
                    event.peer->data = NULL;
                }
            }
        }
    }
    else if (UserInput == 2)
    {
        if (!CreateClient())
        {
            fprintf(stderr,
                "An error occurred while trying to create an ENet client host.\n");
            exit(EXIT_FAILURE);
        }

        ENetAddress address;
        ENetEvent event;
        ENetPeer* peer;

        enet_address_set_host(&address, "127.0.0.1");
        address.port = 1234;

        peer = enet_host_connect(client, &address, 2, 0);
        if (peer == NULL)
        {
            fprintf(stderr,
                "No available peers for initiating an ENet connection.\n");
            exit(EXIT_FAILURE);
        }

        if (enet_host_service(client, &event, 5000) > 0 &&
            event.type == ENET_EVENT_TYPE_CONNECT)
        {
            std::cout << "Connection to 127.0.0.1:1234 succeeded." << std::endl;
        }
        else
        {
            enet_peer_reset(peer);
            std::cout << "Connection to some.server.net:1234 failed." << std::endl;
        }

        while (1)
        {
            ENetEvent event;
            while (enet_host_service(client, &event, 1000) > 0)
            {


                switch (event.type)
                {
                case ENET_EVENT_TYPE_RECEIVE:
                    std::cout << "A packet of length "
                              << event.packet->dataLength << std::endl
                              << " containing " << (char*)event.packet->data << std::endl;
                    enet_packet_destroy(event.packet);

                    std::cout << "User Message: ";
                    std::string msg;
                    std::cin >> msg;
                        //getline(std::cin, msg);

                    {

                        ENetPacket* packet = enet_packet_create(msg.c_str(),
                            msg.length() + 1,
                            ENET_PACKET_FLAG_RELIABLE);

                        enet_host_broadcast(client, 0, packet);
                        //enet_peer_send(event.peer, 0, packet);
                        enet_host_flush(client);
                    }

                }

            }
        }

    }
    else 
    {
        std::cout << "Invalid Input" << std::endl;
    }
    if (server != nullptr)
    {
        enet_host_destroy(server);
    }
    if (client != nullptr)
    {
        enet_host_destroy(client);
    }
    return EXIT_SUCCESS;
}