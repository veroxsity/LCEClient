#pragma once
#ifdef _LINUX64

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <vector>
#include <stdint.h>
#include "../Common/Network/NetworkPlayerInterface.h"
#include "../../Minecraft.World/DisconnectPacket.h"

#define WIN64_NET_DEFAULT_PORT      25565
#define WIN64_NET_MAX_CLIENTS       255
#define WIN64_SMALLID_REJECT        0xFF
#define WIN64_NET_RECV_BUFFER_SIZE  65536
#define WIN64_NET_MAX_PACKET_SIZE   (4 * 1024 * 1024)
#define WIN64_LAN_DISCOVERY_PORT    25566
#define WIN64_LAN_BROADCAST_MAGIC   0x4D434C4E

typedef int LinuxSocket;
#define INVALID_SOCKET_FD (-1)

#pragma pack(push, 1)
struct Win64LANBroadcast {
    uint32_t magic; uint16_t netVersion; uint16_t gamePort;
    wchar_t  hostName[32];
    uint8_t  playerCount; uint8_t maxPlayers;
    uint32_t gameHostSettings; uint32_t texturePackParentId;
    uint8_t  subTexturePackId; uint8_t isJoinable;
};
#pragma pack(pop)

struct Win64LANSession {
    char     hostIP[64]; int hostPort; wchar_t hostName[32];
    uint16_t netVersion; uint8_t playerCount; uint8_t maxPlayers;
    uint32_t gameHostSettings; uint32_t texturePackParentId;
    uint8_t  subTexturePackId; bool isJoinable; uint32_t lastSeenTick;
};

struct Win64RemoteConnection {
    LinuxSocket tcpSocket; uint8_t smallId; pthread_t recvThread; volatile bool active;
};

class WinsockNetLayer {
public:
    static bool Initialize();
    static void Shutdown();
    static bool HostGame(int port, const char* bindIp = nullptr);
    static bool JoinGame(const char* ip, int port);

    enum eJoinState { eJoinState_Idle, eJoinState_Connecting, eJoinState_Success,
                      eJoinState_Failed, eJoinState_Rejected, eJoinState_Cancelled };
    static bool       BeginJoinGame(const char* ip, int port);
    static void       CancelJoinGame();
    static eJoinState GetJoinState();
    static int        GetJoinAttempt();
    static int        GetJoinMaxAttempts();
    static DisconnectPacket::eDisconnectReason GetJoinRejectReason();
    static bool       FinalizeJoin();

    static bool SendToSmallId(uint8_t targetSmallId, const void* data, int dataSize);
    static bool SendOnSocket(LinuxSocket sock, const void* data, int dataSize);

    static bool        JoinSplitScreen(int padIndex, uint8_t* outSmallId);
    static void        CloseSplitScreenConnection(int padIndex);
    static LinuxSocket GetLocalSocket(uint8_t senderSmallId);
    static uint8_t     GetSplitScreenSmallId(int padIndex);

    static bool    IsHosting()   { return s_isHost; }
    static bool    IsConnected() { return s_connected; }
    static bool    IsActive()    { return s_active; }
    static uint8_t GetLocalSmallId() { return s_localSmallId; }
    static uint8_t GetHostSmallId()  { return s_hostSmallId; }
    static int     GetHostPort()     { return s_hostGamePort; }

    static LinuxSocket GetSocketForSmallId(uint8_t smallId);
    static void HandleDataReceived(uint8_t from, uint8_t to, unsigned char* data, unsigned int size);
    static bool PopDisconnectedSmallId(uint8_t* outSmallId);
    static void PushFreeSmallId(uint8_t smallId);
    static void CloseConnectionBySmallId(uint8_t smallId);
    static void ClearSocketForSmallId(uint8_t smallId);

    static bool StartAdvertising(int gamePort, const wchar_t* hostName, uint32_t gameSettings,
                                 uint32_t texPackId, uint8_t subTexId, uint16_t netVer);
    static void StopAdvertising();
    static void UpdateAdvertisePlayerCount(uint8_t count);
    static void UpdateAdvertiseMaxPlayers(uint8_t maxPlayers);
    static void UpdateAdvertiseJoinable(bool joinable);
    static bool StartDiscovery();
    static void StopDiscovery();
    static std::vector<Win64LANSession> GetDiscoveredSessions();

private:
    static void* AcceptThreadProc(void* param);
    static void* RecvThreadProc(void* param);
    static void* ClientRecvThreadProc(void* param);
    static void* AdvertiseThreadProc(void* param);
    static void* DiscoveryThreadProc(void* param);
    static void* JoinThreadProc(void* param);

    static pthread_t           s_joinThread;
    static volatile eJoinState s_joinState;
    static volatile int        s_joinAttempt;
    static volatile bool       s_joinCancel;
    static char                s_joinIP[256];
    static int                 s_joinPort;
    static uint8_t             s_joinAssignedSmallId;
    static DisconnectPacket::eDisconnectReason s_joinRejectReason;
    static const int           JOIN_MAX_ATTEMPTS = 4;

    static LinuxSocket  s_listenSocket, s_hostConnectionSocket;
    static pthread_t    s_acceptThread, s_clientRecvThread;
    static bool         s_isHost, s_connected, s_active, s_initialized;
    static uint8_t      s_localSmallId, s_hostSmallId;
    static uint32_t     s_nextSmallId;
    static int          s_hostGamePort;

    static pthread_mutex_t s_sendLock, s_connectionsLock;
    static std::vector<Win64RemoteConnection> s_connections;

    static LinuxSocket     s_advertiseSock;
    static pthread_t       s_advertiseThread;
    static volatile bool   s_advertising;
    static Win64LANBroadcast s_advertiseData;
    static pthread_mutex_t s_advertiseLock;

    static LinuxSocket     s_discoverySock;
    static pthread_t       s_discoveryThread;
    static volatile bool   s_discovering;
    static pthread_mutex_t s_discoveryLock;
    static std::vector<Win64LANSession> s_discoveredSessions;

    static pthread_mutex_t s_disconnectLock;
    static std::vector<uint8_t> s_disconnectedSmallIds;
    static pthread_mutex_t s_freeSmallIdLock;
    static std::vector<uint8_t> s_freeSmallIds;
    static LinuxSocket     s_smallIdToSocket[256];
    static pthread_mutex_t s_smallIdToSocketLock;
    static LinuxSocket     s_splitScreenSocket[XUSER_MAX_COUNT];
    static uint8_t         s_splitScreenSmallId[XUSER_MAX_COUNT];
    static pthread_t       s_splitScreenRecvThread[XUSER_MAX_COUNT];
};

extern bool g_Win64MultiplayerHost, g_Win64MultiplayerJoin;
extern int  g_Win64MultiplayerPort;
extern char g_Win64MultiplayerIP[256];
extern bool g_Win64DedicatedServer;
extern int  g_Win64DedicatedServerPort;
extern char g_Win64DedicatedServerBindIP[256];
extern bool g_Win64DedicatedServerLanAdvertise;

#endif // _LINUX64
