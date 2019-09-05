/*
 * Created on 29.07.18.
 */

#include <common.hpp>
#include <hacks/AntiAntiAim.hpp>

namespace hacks::shared::anti_anti_aim
{
static settings::Boolean enable{ "anti-anti-aim.enable", "false" };

void createMove()
{
    if (!enable)
        return;
    if (CE_BAD(LOCAL_E))
        return;

    IClientEntity *entity{ nullptr };
    for (int i = 0; i <= g_IEngine->GetMaxClients(); i++)
    {
        resolveEnt(i, entity);
    }
}

void resolveEnt(int IDX, IClientEntity *entity)
{
    if (IDX == g_IEngine->GetLocalPlayer())
        return;
    entity = g_IEntityList->GetClientEntity(IDX);
    if (entity && !entity->IsDormant() && !NET_BYTE(entity, netvar.iLifeState))
    {
        float quotat = 0;
        float quotaf = 0;
        if (!g_Settings.brute.choke[IDX].empty())
            for (auto it : g_Settings.brute.choke[IDX])
            {
                if (it)
                    quotat++;
                else
                    quotaf++;
            }
        float quota            = quotat / quotaf;
        Vector &netangles      = NET_VECTOR(entity, netvar.m_angEyeAngles);
        Vector angles          = QAngleToVector(entity->GetAbsAngles());
        static bool brutepitch = false;
        if (g_Settings.brute.brutenum[IDX] > 5)
        {
            g_Settings.brute.brutenum[IDX] = 0;
            brutepitch                     = !brutepitch;
        }
        if (quota > 0.8f)
            brutepitch = true;
        angles.y = fmod(angles.y + 180.0f, 360.0f);
        if (angles.y < 0)
            angles.y += 360.0f;
        angles.y -= 180.0f;
        if (angles.x >= 90)
            angles.x = -89;
        if (angles.x <= -90)
            angles.x = 89;
        if (quota < 0.8f)
            switch (g_Settings.brute.brutenum[IDX])
            {
            case 0:
                break;
            case 1:
                angles.y += 180.0f;
                break;
            case 2:
                angles.y -= 90.0f;
                break;
            case 3:
                angles.y += 90.0f;
                break;
            case 4:
                angles.y -= 180.0f;
                break;
            case 5:
                angles.y = 0.0f;
                break;
            }
        if (brutepitch)
            switch (g_Settings.brute.brutenum[IDX] % 4)
            {
            case 0:
                break;
            case 1:
                angles.x = -89.0f;
                break;
            case 2:
                angles.x = 89.0f;
                break;
            case 3:
                angles.x = 0.0f;
                break;
            }
        const_cast<QAngle &>(entity->GetAbsAngles()) = VectorToQAngle(angles);
        netangles                                    = angles;
    }
}

void ResetPlayer(int idx)
{
    g_Settings.brute.choke[idx]       = {};
    g_Settings.brute.brutenum[idx]    = 0;
    g_Settings.brute.last_angles[idx] = {};
    g_Settings.brute.lastsimtime      = 0.0f;
}
class ResolverListener : public IGameEventListener
{
public:
    virtual void FireGameEvent(KeyValues *event)
    {
        if (!enable)
            return;
        std::string name(event->GetName());
        if (name == "player_activate")
        {
            int uid    = event->GetInt("userid");
            int entity = g_IEngine->GetPlayerForUserID(uid);
            ResetPlayer(entity);
        }
        else if (name == "player_disconnect")
        {
            int uid    = event->GetInt("userid");
            int entity = g_IEngine->GetPlayerForUserID(uid);
            ResetPlayer(entity);
        }
    }
};

static ResolverListener listener;
static InitRoutine init([]() { g_IGameEventManager->AddListener(&listener, false); });
} // namespace hacks::shared::anti_anti_aim
