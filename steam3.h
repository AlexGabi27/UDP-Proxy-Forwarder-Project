#include <iostream>
#include <fstream>
#include <vector>

#include "sdk/public/steam/steam_gameserver.h"
#include "sdk/public/steam/isteamgameserver.h"

using namespace std;

bool start_net_port = false;

int current_ip = 0;
bool ticket_mode = false;

int connects = 0;

void concat(char *dst, char *src)
{
    for(int i = 0; i < strlen(src); i++)
    {
        dst[strlen(dst)] = src[i];
    }
}

int random(int min, int max, int port) //range : [min, max]
{
   srand( time(NULL) + port);
   return min + rand() % (( max + 1 ) - min);
}

bool CheckIfIsValidPacket(char *buffer)
{
    if(strstr(buffer, "\xff\xff\xff\xff"))
    {
        return 1;
    }

    return 0;
}

bool CheckIfTSourceEngineQuery(char *buffer)
{
    if(strstr(buffer, "TSource Engine Query"))
    {
        return 1;
    }

    return 0;
}

bool CheckIfGetChallenge(char *buffer)
{
    if(strstr(buffer, "getchallenge steam"))
    {
        return 1;
    }
    return 0;
}

bool CheckIfConnectPacket(char *buffer)
{
    if(strstr(buffer, "connect 48"))
    {
        return 1;
    }
    return 0;
}

void Log(string buffer_to_log)
{
    std::cout << buffer_to_log << std::endl;
}

// int SetTSourceBuffer(char *buffer)
// {
//     bzero(&buffer, sizeof(buffer));
//     int len = 0;

//     char static_buff[] = "\x00\x63\x73\x74\x72\x69\x6B\x65\x00\x43\x6F\x75\x6E\x74\x65\x72\x2D\x53\x74\x72\x69\x6B\x65\x00\x0A\x00\x1F\x20\x00\x64\x01\x00\x01\x31\x2E\x31\x2E\x32\x2E\x37\x2F\x53\x74\x64\x69\x6F\x00";
//     char static_buff2[] = "\x00\x63\x73\x74\x72\x69\x6B\x65\x00\x43\x6F\x75\x6E\x74\x65\x72\x2D\x53\x74\x72\x69\x6B\x65\x00\x0A\x00\x01\x20\x00\x64\x01\x00\x01\x31\x2E\x31\x2E\x32\x2E\x37\x2F\x53\x74\x64\x69\x6F\x00";

//     int static_buff_len = 47;
//     char start_buff[] = "\xff\xff\xff\xffI0"; // {0xff, 0xff, 0xff, 0xff, 0xff, 'I', 'O'};
//     char sep[] = {"\x00"};

//     if (hostname_curr >= hostname_len)
//         hostname_curr = 0;
//     if (maps_curr >= maps_len)
//         maps_curr = 0;

//     for(int i = 0; i < strlen(start_buff); i++)
//     {
//         buffer[len++] = start_buff[i];
//     }

//     for(int i = 0; i < strlen(hostnames[hostname_curr]); i++)
//     {
//         buffer[len++] = hostnames[hostname_curr][i];
//     }

//     buffer[len++] = sep[0];

//     for(int i = 0; i < strlen(maps[maps_curr]); i++)
//     {
//         buffer[len++] = maps[maps_curr][i];
//     }

//     for (int i = 0; i < static_buff_len; i++)
//     {
//         buffer[len++] = static_buff[i];
//     }

//     return len-1;
// }

// int LoadHostnames()
// {
//     fstream host("config//hostnames.txt");

//     if(host.fail())
//     {
//         return 0;
//     }

//     string line;
//     while(getline(host, line))
//     {
//         strcpy(hostnames[hostname_len++], line.c_str());
//     }

//     hostname_len--;
//     host.close();

//     return 1;
// }

// int LoadMaps()
// {
//     fstream map("config//maps.txt");

//     if(map.fail())
//     {
//         return 0;
//     }

//     string line;
//     while(getline(map, line))
//     {
//         strcpy(maps[maps_len++], line.c_str());
//     }

//     maps_len--;
//     map.close();

//     return 1;
// }

void SetServerParams()
{
    // useless data in my eyes, but who knows what Steamworks does in the back
    SteamGameServer()->SetProduct("cstrike");
    SteamGameServer()->SetModDir("cstrike");
    SteamGameServer()->SetDedicatedServer(true);
    SteamGameServer()->LogOnAnonymous();
    // SteamGameServer()->SetAdvertiseServerActive(true);
    SteamGameServer()->EnableHeartbeats(true);
    SteamGameServer()->SetHeartbeatInterval(400);
    SteamGameServer()->ForceHeartbeat();

    SteamGameServer()->SetKeyValue("protocol", "48");

    SteamGameServer()->SetMaxPlayerCount(32);
    SteamGameServer()->SetBotPlayerCount(0);
    SteamGameServer()->SetServerName(("This is a dummy CS 1.6 server!"));
    SteamGameServer()->SetMapName("de_alejandro");
}

class CSteam3Server
{
protected:
        bool m_bHasActivePlayers;
        bool m_bWantToBeSecure;
        bool m_bLanOnly;
        CSteamID m_SteamIDGS;

public:
    CSteam3Server() :
        m_CallbackGSClientApprove(this, &CSteam3Server::OnGSClientApprove),
        m_CallbackGSClientDeny(this, &CSteam3Server::OnGSClientDeny),
        m_CallbackGSClientKick(this, &CSteam3Server::OnGSClientKick),
        m_CallbackGSPolicyResponse(this, &CSteam3Server::OnGSPolicyResponse),
        m_CallbackLogonSuccess(this, &CSteam3Server::OnLogonSuccess),
        m_CallbackLogonFailure(this, &CSteam3Server::OnLogonFailure),
        m_SteamIDGS(1, 0, k_EUniverseInvalid, k_EAccountTypeInvalid)
    {}

    STEAM_GAMESERVER_CALLBACK(CSteam3Server, OnGSClientApprove, GSClientApprove_t, m_CallbackGSClientApprove)
    {
        auto steamid = pParam->m_SteamID;
        cout << "[Steam] OnGSClientApprove steamid = " << steamid.ConvertToUint64() << '\n';
        connects++;
    }

    STEAM_GAMESERVER_CALLBACK(CSteam3Server, OnGSClientDeny, GSClientDeny_t, m_CallbackGSClientDeny)
    {
        auto steamid = pParam->m_SteamID;
        cout << "[Steam] OnGSClientDeny steamid = " << steamid.ConvertToUint64() << '\n';
    }

    STEAM_GAMESERVER_CALLBACK(CSteam3Server, OnGSClientKick, GSClientKick_t, m_CallbackGSClientKick)
    {
        auto steamid = pParam->m_SteamID;
        auto reason = pParam->m_eDenyReason;
    }

    STEAM_GAMESERVER_CALLBACK(CSteam3Server, OnGSPolicyResponse, GSPolicyResponse_t, m_CallbackGSPolicyResponse)
    {
        if (pParam->m_bSecure)
            cout << "[Steam] VAC secure mode is activated\n";
        else
            cout << "[Steam] VAC secure mode disabled\n";

        start_net_port = true;
    }

    STEAM_GAMESERVER_CALLBACK(CSteam3Server, OnLogonSuccess, SteamServersConnected_t, m_CallbackLogonSuccess)
    {
        cout << "[Steam] Connection to Steam servers successful.\n";

        m_SteamIDGS = SteamGameServer()->GetSteamID();
    }

    STEAM_GAMESERVER_CALLBACK(CSteam3Server, OnLogonFailure, SteamServerConnectFailure_t, m_CallbackLogonFailure)
    {
        cout << "[Steam] Could not establish connection to Steam servers.\n";
    }
};