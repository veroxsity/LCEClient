#ifdef _LINUX64
#include "stdafx.h"
#include "Linux64/Network/WinsockNetLayer.h"
#include "../../Minecraft.World/DisconnectPacket.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>

// Static member definitions
pthread_t          WinsockNetLayer::s_joinThread       = 0;
volatile WinsockNetLayer::eJoinState WinsockNetLayer::s_joinState = WinsockNetLayer::eJoinState_Idle;
volatile int       WinsockNetLayer::s_joinAttempt      = 0;
volatile bool      WinsockNetLayer::s_joinCancel       = false;
char               WinsockNetLayer::s_joinIP[256]      = {};
int                WinsockNetLayer::s_joinPort         = 0;
uint8_t            WinsockNetLayer::s_joinAssignedSmallId = 0xFF;
DisconnectPacket::eDisconnectReason WinsockNetLayer::s_joinRejectReason =
    DisconnectPacket::eDisconnectReason_Unknown;
LinuxSocket        WinsockNetLayer::s_listenSocket         = INVALID_SOCKET_FD;
LinuxSocket        WinsockNetLayer::s_hostConnectionSocket = INVALID_SOCKET_FD;
pthread_t          WinsockNetLayer::s_acceptThread         = 0;
pthread_t          WinsockNetLayer::s_clientRecvThread     = 0;
bool WinsockNetLayer::s_isHost = false, WinsockNetLayer::s_connected = false;
bool WinsockNetLayer::s_active = false, WinsockNetLayer::s_initialized = false;
uint8_t  WinsockNetLayer::s_localSmallId = 0, WinsockNetLayer::s_hostSmallId = 0;
uint32_t WinsockNetLayer::s_nextSmallId  = 1;
int      WinsockNetLayer::s_hostGamePort = WIN64_NET_DEFAULT_PORT;
pthread_mutex_t WinsockNetLayer::s_sendLock        = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t WinsockNetLayer::s_connectionsLock = PTHREAD_MUTEX_INITIALIZER;
std::vector<Win64RemoteConnection> WinsockNetLayer::s_connections;
LinuxSocket     WinsockNetLayer::s_advertiseSock   = INVALID_SOCKET_FD;
pthread_t       WinsockNetLayer::s_advertiseThread = 0;
volatile bool   WinsockNetLayer::s_advertising     = false;
Win64LANBroadcast WinsockNetLayer::s_advertiseData = {};
pthread_mutex_t WinsockNetLayer::s_advertiseLock   = PTHREAD_MUTEX_INITIALIZER;
LinuxSocket     WinsockNetLayer::s_discoverySock   = INVALID_SOCKET_FD;
pthread_t       WinsockNetLayer::s_discoveryThread = 0;
volatile bool   WinsockNetLayer::s_discovering     = false;
pthread_mutex_t WinsockNetLayer::s_discoveryLock   = PTHREAD_MUTEX_INITIALIZER;
std::vector<Win64LANSession> WinsockNetLayer::s_discoveredSessions;
pthread_mutex_t WinsockNetLayer::s_disconnectLock  = PTHREAD_MUTEX_INITIALIZER;
std::vector<uint8_t> WinsockNetLayer::s_disconnectedSmallIds;
pthread_mutex_t WinsockNetLayer::s_freeSmallIdLock = PTHREAD_MUTEX_INITIALIZER;
std::vector<uint8_t> WinsockNetLayer::s_freeSmallIds;
LinuxSocket     WinsockNetLayer::s_smallIdToSocket[256];
pthread_mutex_t WinsockNetLayer::s_smallIdToSocketLock = PTHREAD_MUTEX_INITIALIZER;
LinuxSocket     WinsockNetLayer::s_splitScreenSocket[XUSER_MAX_COUNT];
uint8_t         WinsockNetLayer::s_splitScreenSmallId[XUSER_MAX_COUNT];
pthread_t       WinsockNetLayer::s_splitScreenRecvThread[XUSER_MAX_COUNT];

bool g_Win64MultiplayerHost = false, g_Win64MultiplayerJoin = false;
int  g_Win64MultiplayerPort = WIN64_NET_DEFAULT_PORT;
char g_Win64MultiplayerIP[256] = {};
bool g_Win64DedicatedServer = false;
int  g_Win64DedicatedServerPort = WIN64_NET_DEFAULT_PORT;
char g_Win64DedicatedServerBindIP[256] = "0.0.0.0";
bool g_Win64DedicatedServerLanAdvertise = false;

// Internal helpers
static void SetSocketNoDelay(LinuxSocket fd) {
    int yes = 1; setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &yes, sizeof(yes));
}
static void SetSocketReuseAddr(LinuxSocket fd) {
    int yes = 1; setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
}

// Length-prefixed send: 4-byte LE header + payload
bool WinsockNetLayer::SendOnSocket(LinuxSocket sock, const void* data, int dataSize) {
    if (sock == INVALID_SOCKET_FD || dataSize <= 0) return false;
    uint32_t len = (uint32_t)dataSize;
    const char* p = (const char*)&len;
    int sent = 0;
    while (sent < 4) { int r = (int)send(sock, p+sent, 4-sent, MSG_NOSIGNAL); if (r<=0) return false; sent+=r; }
    p = (const char*)data; sent = 0;
    while (sent < dataSize) { int r=(int)send(sock,p+sent,dataSize-sent,MSG_NOSIGNAL); if(r<=0) return false; sent+=r; }
    return true;
}

static bool RecvFull(LinuxSocket sock, void* buf, int size) {
    char* p = (char*)buf; int received = 0;
    while (received < size) { int r=(int)recv(sock,p+received,size-received,0); if(r<=0) return false; received+=r; }
    return true;
}

// Init / Shutdown
bool WinsockNetLayer::Initialize() {
    if (s_initialized) return true;
    for (int i=0;i<256;i++) s_smallIdToSocket[i]=INVALID_SOCKET_FD;
    for (int i=0;i<XUSER_MAX_COUNT;i++) {
        s_splitScreenSocket[i]=INVALID_SOCKET_FD;
        s_splitScreenSmallId[i]=0xFF;
        s_splitScreenRecvThread[i]=0;
    }
    s_initialized = s_active = true;
    printf("[Net] Initialized (POSIX)\n");
    return true;
}

void WinsockNetLayer::Shutdown() {
    StopAdvertising(); StopDiscovery();
    s_active = s_initialized = false;
    if (s_listenSocket!=INVALID_SOCKET_FD) { close(s_listenSocket); s_listenSocket=INVALID_SOCKET_FD; }
    if (s_hostConnectionSocket!=INVALID_SOCKET_FD) { close(s_hostConnectionSocket); s_hostConnectionSocket=INVALID_SOCKET_FD; }
}

// Hosting
bool WinsockNetLayer::HostGame(int port, const char* bindIp) {
    if (s_listenSocket!=INVALID_SOCKET_FD) return false;
    s_listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (s_listenSocket==INVALID_SOCKET_FD) return false;
    SetSocketReuseAddr(s_listenSocket); SetSocketNoDelay(s_listenSocket);
    struct sockaddr_in addr={}; addr.sin_family=AF_INET; addr.sin_port=htons((uint16_t)port);
    addr.sin_addr.s_addr = bindIp ? inet_addr(bindIp) : INADDR_ANY;
    if (bind(s_listenSocket,(struct sockaddr*)&addr,sizeof(addr))!=0 || listen(s_listenSocket,8)!=0) {
        close(s_listenSocket); s_listenSocket=INVALID_SOCKET_FD; return false;
    }
    s_isHost=s_connected=true; s_localSmallId=s_hostSmallId=0; s_hostGamePort=port;
    pthread_create(&s_acceptThread,nullptr,AcceptThreadProc,nullptr);
    pthread_detach(s_acceptThread);
    printf("[Net] Hosting on port %d\n",port); return true;
}

// Joining
bool WinsockNetLayer::JoinGame(const char* ip, int port) {
    LinuxSocket sock=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
    if (sock==INVALID_SOCKET_FD) return false;
    SetSocketNoDelay(sock);
    struct sockaddr_in addr={}; addr.sin_family=AF_INET; addr.sin_port=htons((uint16_t)port);
    inet_pton(AF_INET,ip,&addr.sin_addr);
    if (connect(sock,(struct sockaddr*)&addr,sizeof(addr))!=0) { close(sock); return false; }
    s_hostConnectionSocket=sock; s_connected=true; s_isHost=false;
    pthread_create(&s_clientRecvThread,nullptr,ClientRecvThreadProc,nullptr);
    pthread_detach(s_clientRecvThread);
    printf("[Net] Joined %s:%d\n",ip,port); return true;
}

bool WinsockNetLayer::BeginJoinGame(const char* ip, int port) {
    if (s_joinState!=eJoinState_Idle) return false;
    strncpy(s_joinIP,ip,sizeof(s_joinIP)-1); s_joinPort=port;
    s_joinCancel=false; s_joinState=eJoinState_Connecting; s_joinAttempt=0;
    pthread_create(&s_joinThread,nullptr,JoinThreadProc,nullptr); pthread_detach(s_joinThread);
    return true;
}
void WinsockNetLayer::CancelJoinGame()  { s_joinCancel=true; }
WinsockNetLayer::eJoinState WinsockNetLayer::GetJoinState() { return s_joinState; }
int  WinsockNetLayer::GetJoinAttempt()     { return s_joinAttempt; }
int  WinsockNetLayer::GetJoinMaxAttempts() { return JOIN_MAX_ATTEMPTS; }
DisconnectPacket::eDisconnectReason WinsockNetLayer::GetJoinRejectReason() { return s_joinRejectReason; }
bool WinsockNetLayer::FinalizeJoin() {
    if (s_joinState!=eJoinState_Success) return false;
    s_localSmallId=s_joinAssignedSmallId; s_joinState=eJoinState_Idle; return true;
}

// Send / socket bookkeeping
bool WinsockNetLayer::SendToSmallId(uint8_t targetSmallId, const void* data, int dataSize) {
    pthread_mutex_lock(&s_smallIdToSocketLock);
    LinuxSocket sock=s_smallIdToSocket[targetSmallId];
    pthread_mutex_unlock(&s_smallIdToSocketLock);
    if (sock==INVALID_SOCKET_FD) return false;
    pthread_mutex_lock(&s_sendLock);
    bool ok=SendOnSocket(sock,data,dataSize);
    pthread_mutex_unlock(&s_sendLock);
    return ok;
}
LinuxSocket WinsockNetLayer::GetSocketForSmallId(uint8_t id) {
    pthread_mutex_lock(&s_smallIdToSocketLock);
    LinuxSocket s=s_smallIdToSocket[id];
    pthread_mutex_unlock(&s_smallIdToSocketLock);
    return s;
}
void WinsockNetLayer::ClearSocketForSmallId(uint8_t id) {
    pthread_mutex_lock(&s_smallIdToSocketLock);
    s_smallIdToSocket[id]=INVALID_SOCKET_FD;
    pthread_mutex_unlock(&s_smallIdToSocketLock);
}
void WinsockNetLayer::CloseConnectionBySmallId(uint8_t id) {
    LinuxSocket sock=GetSocketForSmallId(id);
    if (sock!=INVALID_SOCKET_FD) { close(sock); ClearSocketForSmallId(id); }
    pthread_mutex_lock(&s_disconnectLock); s_disconnectedSmallIds.push_back(id); pthread_mutex_unlock(&s_disconnectLock);
    PushFreeSmallId(id);
}
bool WinsockNetLayer::PopDisconnectedSmallId(uint8_t* out) {
    pthread_mutex_lock(&s_disconnectLock);
    if (s_disconnectedSmallIds.empty()) { pthread_mutex_unlock(&s_disconnectLock); return false; }
    *out=s_disconnectedSmallIds.front(); s_disconnectedSmallIds.erase(s_disconnectedSmallIds.begin());
    pthread_mutex_unlock(&s_disconnectLock); return true;
}
void WinsockNetLayer::PushFreeSmallId(uint8_t id) {
    pthread_mutex_lock(&s_freeSmallIdLock); s_freeSmallIds.push_back(id); pthread_mutex_unlock(&s_freeSmallIdLock);
}

// Split-screen stubs
bool WinsockNetLayer::JoinSplitScreen(int,uint8_t*) { return false; }
void WinsockNetLayer::CloseSplitScreenConnection(int) {}
LinuxSocket WinsockNetLayer::GetLocalSocket(uint8_t) { return INVALID_SOCKET_FD; }
uint8_t WinsockNetLayer::GetSplitScreenSmallId(int) { return 0xFF; }

void WinsockNetLayer::HandleDataReceived(uint8_t from, uint8_t to, unsigned char* data, unsigned int size) {
    extern void IQNet_DispatchPacket(uint8_t,uint8_t,unsigned char*,unsigned int);
    IQNet_DispatchPacket(from,to,data,size);
}

// Thread procs
struct RecvThreadArgs { LinuxSocket sock; uint8_t smallId; };

void* WinsockNetLayer::RecvThreadProc(void* param) {
    RecvThreadArgs* args=(RecvThreadArgs*)param;
    LinuxSocket sock=args->sock; uint8_t smallId=args->smallId; delete args;
    uint8_t lenBuf[4];
    while (true) {
        if (!RecvFull(sock,lenBuf,4)) break;
        uint32_t dataLen=lenBuf[0]|(lenBuf[1]<<8)|(lenBuf[2]<<16)|(lenBuf[3]<<24);
        if (!dataLen||dataLen>(uint32_t)WIN64_NET_MAX_PACKET_SIZE) break;
        unsigned char* buf=new unsigned char[dataLen];
        if (!RecvFull(sock,buf,(int)dataLen)) { delete[] buf; break; }
        HandleDataReceived(smallId,0xFF,buf,dataLen); delete[] buf;
    }
    CloseConnectionBySmallId(smallId); return nullptr;
}

void* WinsockNetLayer::AcceptThreadProc(void*) {
    while (s_isHost&&s_listenSocket!=INVALID_SOCKET_FD) {
        struct sockaddr_in ca={}; socklen_t al=sizeof(ca);
        LinuxSocket cs=accept(s_listenSocket,(struct sockaddr*)&ca,&al);
        if (cs==INVALID_SOCKET_FD) { if (!s_isHost) break; usleep(10000); continue; }
        SetSocketNoDelay(cs);
        pthread_mutex_lock(&s_freeSmallIdLock);
        uint8_t newId; if (!s_freeSmallIds.empty()) { newId=s_freeSmallIds.back(); s_freeSmallIds.pop_back(); }
        else newId=(uint8_t)(s_nextSmallId++);
        pthread_mutex_unlock(&s_freeSmallIdLock);
        if (newId==WIN64_SMALLID_REJECT) { close(cs); continue; }
        pthread_mutex_lock(&s_smallIdToSocketLock); s_smallIdToSocket[newId]=cs; pthread_mutex_unlock(&s_smallIdToSocketLock);
        SendOnSocket(cs,&newId,1);
        RecvThreadArgs* args=new RecvThreadArgs{cs,newId};
        pthread_t t; pthread_create(&t,nullptr,RecvThreadProc,args); pthread_detach(t);
        printf("[Net] Client connected, smallId=%d\n",(int)newId);
    }
    return nullptr;
}

void* WinsockNetLayer::ClientRecvThreadProc(void*) {
    LinuxSocket sock=s_hostConnectionSocket;
    uint8_t assignedId=0xFF;
    if (!RecvFull(sock,&assignedId,1)) { s_connected=false; return nullptr; }
    s_localSmallId=assignedId;
    pthread_mutex_lock(&s_smallIdToSocketLock); s_smallIdToSocket[s_hostSmallId]=sock; pthread_mutex_unlock(&s_smallIdToSocketLock);
    uint8_t lenBuf[4];
    while (s_connected) {
        if (!RecvFull(sock,lenBuf,4)) break;
        uint32_t dataLen=lenBuf[0]|(lenBuf[1]<<8)|(lenBuf[2]<<16)|(lenBuf[3]<<24);
        if (!dataLen||dataLen>(uint32_t)WIN64_NET_MAX_PACKET_SIZE) break;
        unsigned char* buf=new unsigned char[dataLen];
        if (!RecvFull(sock,buf,(int)dataLen)) { delete[] buf; break; }
        HandleDataReceived(s_hostSmallId,s_localSmallId,buf,dataLen); delete[] buf;
    }
    s_connected=false; close(sock); s_hostConnectionSocket=INVALID_SOCKET_FD; return nullptr;
}

void* WinsockNetLayer::JoinThreadProc(void*) {
    for (int attempt=0;attempt<JOIN_MAX_ATTEMPTS&&!s_joinCancel;attempt++) {
        s_joinAttempt=attempt+1;
        LinuxSocket sock=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
        if (sock==INVALID_SOCKET_FD) continue;
        SetSocketNoDelay(sock);
        struct sockaddr_in addr={}; addr.sin_family=AF_INET; addr.sin_port=htons((uint16_t)s_joinPort);
        inet_pton(AF_INET,s_joinIP,&addr.sin_addr);
        if (connect(sock,(struct sockaddr*)&addr,sizeof(addr))==0) {
            s_hostConnectionSocket=sock; s_connected=true; s_isHost=false;
            s_joinState=eJoinState_Success;
            pthread_create(&s_clientRecvThread,nullptr,ClientRecvThreadProc,nullptr);
            pthread_detach(s_clientRecvThread); return nullptr;
        }
        close(sock); if (!s_joinCancel) usleep(500000);
    }
    s_joinState=s_joinCancel?eJoinState_Cancelled:eJoinState_Failed; return nullptr;
}

// LAN Advertising
bool WinsockNetLayer::StartAdvertising(int port,const wchar_t* name,uint32_t gs,uint32_t tp,uint8_t st,uint16_t nv) {
    if (s_advertising) return false;
    s_advertiseSock=socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
    if (s_advertiseSock==INVALID_SOCKET_FD) return false;
    int yes=1; setsockopt(s_advertiseSock,SOL_SOCKET,SO_BROADCAST,&yes,sizeof(yes));
    int reuse=1; setsockopt(s_advertiseSock,SOL_SOCKET,SO_REUSEADDR,&reuse,sizeof(reuse));
    s_advertiseData={WIN64_LAN_BROADCAST_MAGIC,nv,(uint16_t)port,{},0,0,gs,tp,st,1};
    if (name) wcsncpy(s_advertiseData.hostName,name,31);
    s_advertising=true;
    pthread_create(&s_advertiseThread,nullptr,AdvertiseThreadProc,nullptr); pthread_detach(s_advertiseThread);
    return true;
}
void WinsockNetLayer::StopAdvertising() {
    s_advertising=false;
    if (s_advertiseSock!=INVALID_SOCKET_FD) { close(s_advertiseSock); s_advertiseSock=INVALID_SOCKET_FD; }
}
void WinsockNetLayer::UpdateAdvertisePlayerCount(uint8_t c) { pthread_mutex_lock(&s_advertiseLock); s_advertiseData.playerCount=c; pthread_mutex_unlock(&s_advertiseLock); }
void WinsockNetLayer::UpdateAdvertiseMaxPlayers(uint8_t m)  { pthread_mutex_lock(&s_advertiseLock); s_advertiseData.maxPlayers=m; pthread_mutex_unlock(&s_advertiseLock); }
void WinsockNetLayer::UpdateAdvertiseJoinable(bool j)       { pthread_mutex_lock(&s_advertiseLock); s_advertiseData.isJoinable=j?1:0; pthread_mutex_unlock(&s_advertiseLock); }

void* WinsockNetLayer::AdvertiseThreadProc(void*) {
    struct sockaddr_in bc={}; bc.sin_family=AF_INET; bc.sin_port=htons(WIN64_LAN_DISCOVERY_PORT); bc.sin_addr.s_addr=INADDR_BROADCAST;
    while (s_advertising) {
        pthread_mutex_lock(&s_advertiseLock); Win64LANBroadcast pkt=s_advertiseData; pthread_mutex_unlock(&s_advertiseLock);
        sendto(s_advertiseSock,&pkt,sizeof(pkt),0,(struct sockaddr*)&bc,sizeof(bc));
        usleep(1000000);
    }
    return nullptr;
}

// LAN Discovery
bool WinsockNetLayer::StartDiscovery() {
    if (s_discovering) return false;
    s_discoverySock=socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
    if (s_discoverySock==INVALID_SOCKET_FD) return false;
    int reuse=1; setsockopt(s_discoverySock,SOL_SOCKET,SO_REUSEADDR,&reuse,sizeof(reuse));
    struct sockaddr_in addr={}; addr.sin_family=AF_INET; addr.sin_port=htons(WIN64_LAN_DISCOVERY_PORT); addr.sin_addr.s_addr=INADDR_ANY;
    if (bind(s_discoverySock,(struct sockaddr*)&addr,sizeof(addr))!=0) { close(s_discoverySock); s_discoverySock=INVALID_SOCKET_FD; return false; }
    struct timeval tv={1,0}; setsockopt(s_discoverySock,SOL_SOCKET,SO_RCVTIMEO,&tv,sizeof(tv));
    s_discovering=true;
    pthread_create(&s_discoveryThread,nullptr,DiscoveryThreadProc,nullptr); pthread_detach(s_discoveryThread);
    return true;
}
void WinsockNetLayer::StopDiscovery() {
    s_discovering=false;
    if (s_discoverySock!=INVALID_SOCKET_FD) { close(s_discoverySock); s_discoverySock=INVALID_SOCKET_FD; }
}
std::vector<Win64LANSession> WinsockNetLayer::GetDiscoveredSessions() {
    pthread_mutex_lock(&s_discoveryLock); auto copy=s_discoveredSessions; pthread_mutex_unlock(&s_discoveryLock); return copy;
}
void* WinsockNetLayer::DiscoveryThreadProc(void*) {
    while (s_discovering) {
        Win64LANBroadcast pkt; struct sockaddr_in sa={}; socklen_t al=sizeof(sa);
        ssize_t r=recvfrom(s_discoverySock,&pkt,sizeof(pkt),0,(struct sockaddr*)&sa,&al);
        if (r!=(ssize_t)sizeof(Win64LANBroadcast)||pkt.magic!=WIN64_LAN_BROADCAST_MAGIC) continue;
        Win64LANSession s={}; inet_ntop(AF_INET,&sa.sin_addr,s.hostIP,sizeof(s.hostIP));
        s.hostPort=pkt.gamePort; s.netVersion=pkt.netVersion; s.playerCount=pkt.playerCount;
        s.maxPlayers=pkt.maxPlayers; s.gameHostSettings=pkt.gameHostSettings;
        s.texturePackParentId=pkt.texturePackParentId; s.subTexturePackId=pkt.subTexturePackId;
        s.isJoinable=pkt.isJoinable!=0; wcsncpy(s.hostName,pkt.hostName,31);
        pthread_mutex_lock(&s_discoveryLock);
        bool found=false;
        for (auto& ex:s_discoveredSessions) if (strcmp(ex.hostIP,s.hostIP)==0&&ex.hostPort==s.hostPort) { ex=s; found=true; break; }
        if (!found) s_discoveredSessions.push_back(s);
        pthread_mutex_unlock(&s_discoveryLock);
    }
    return nullptr;
}

#endif // _LINUX64
