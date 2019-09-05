/*
 * IsPlayingTimeDemo.cpp
 *
 *  Created on: May 13, 2018
 *      Author: bencat07
 */
#include "HookedMethods.hpp"
#include "MiscTemporary.hpp"

static settings::Boolean enable_debug_servercmd{ "debug.servercmdkeyvalues", "false" };
namespace hooked_methods
{
std::vector<KeyValues *> Iterate(KeyValues *event, int depth);
DEFINE_HOOKED_METHOD(ServerCmdKeyValues, void, IVEngineClient013 *_this, KeyValues *kv)
{
    if (!enable_debug_servercmd)
        return original::ServerCmdKeyValues(_this, kv);
    logging::Info("START SERVERCMD KEYVALUES");
    auto peer_list = hooked_methods::Iterate(kv, 10);
    // loop through all our peers
    for (KeyValues *dat : peer_list)
    {
        auto data_type = dat->m_iDataType;
        auto name      = dat->GetName();
        logging::Info("%s", name, data_type);
        switch (dat->m_iDataType)
        {
        case KeyValues::types_t::TYPE_NONE:
        {
            logging::Info("%s is typeless", name);
            break;
        }
        case KeyValues::types_t::TYPE_STRING:
        {
            if (dat->m_sValue && *(dat->m_sValue))
            {
                logging::Info("%s is String: %s", name, dat->m_sValue);
            }
            else
            {
                logging::Info("%s is String: %s", name, "");
            }
            break;
        }
        case KeyValues::types_t::TYPE_WSTRING:
        {
            break;
        }

        case KeyValues::types_t::TYPE_INT:
        {
            logging::Info("%s is int: %d", name, dat->m_iValue);
            break;
        }

        case KeyValues::types_t::TYPE_UINT64:
        {
            logging::Info("%s is double: %f", name, *(double *) dat->m_sValue);
            break;
        }

        case KeyValues::types_t::TYPE_FLOAT:
        {
            logging::Info("%s is float: %f", name, dat->m_flValue);
            break;
        }
        case KeyValues::types_t::TYPE_COLOR:
        {
            logging::Info("%s is Color: { %u %u %u %u}", name, dat->m_Color[0], dat->m_Color[1], dat->m_Color[2], dat->m_Color[3]);
            break;
        }
        case KeyValues::types_t::TYPE_PTR:
        {
            logging::Info("%s is Pointer: %x", name, dat->m_pValue);
            break;
        }

        default:
            break;
        }
    }
    logging::Info("END SERVERCMD KEYVALUES");
    original::ServerCmdKeyValues(_this, kv);
}
} // namespace hooked_methods
