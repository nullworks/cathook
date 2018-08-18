#include "common.hpp"
#include "external/PathFinder/src/PathFinder.h"
#include "external/PathFinder/src/AStar.h"

namespace hacks::shared::pathfinder
{
Vector loc;
// std::vector<Vector> findPath(Vector loc, Vector dest);
// bool initiatenavfile();

// CatCommand navset("nav_set", "Debug nav set",
//                  [](const CCommand &args) { loc = g_pLocalPlayer->v_Origin;
//                  });

// CatCommand navfind("nav_find", "Debug nav find", [](const CCommand &args) {
//    std::vector<Vector> path = findPath(g_pLocalPlayer->v_Origin, loc);
//    if (path.empty())
//    {
//        logging::Info("Pathing: No path found");
//    }
//    std::string output = "Pathing: Path found! Path: ";
//    for (int i = 0; i < path.size(); i++)
//    {
//        output.append(format(path.at(i).x, ",", format(path.at(i).y)));
//    }
//    logging::Info(output.c_str());
//});

// CatCommand navinit("nav_init", "Debug nav init",
//                   [](const CCommand &args) { initiatenavfile(); });

class navNode : public AStarNode
{
    //    navNode()
    //    {
    //    }

    //    ~navNode()
    //    {
    //    }
public:
    Vector vecLoc;
};

std::vector<navNode *> nodes;
// settings::Bool enabled{ "pathing.enabled", 0 };

bool initiatenavfile()
{
    // if (!enabled)
    //    return false;
    logging::Info("Pathing: Initiating path...");

    // This will not work, please fix
    std::string dir =
        "/home/elite/.steam/steam/steamapps/common/Team Fortress 2/tf/maps/";
    std::string levelName = g_IEngine->GetLevelName();
    int dotpos            = levelName.find('.');
    levelName             = levelName.substr(0, dotpos);
    levelName.append(".nav");
    dir.append(levelName);

    CNavFile navData(dir.c_str());
    if (!navData.m_isOK)
    {
        logging::Info("Pathing: Failed to parse nav file!");
        return false;
    }
    std::vector<CNavArea> *areas = &navData.m_areas;
    int nodeCount                = areas->size();

    nodes.clear();
    nodes.reserve(nodeCount);

    // register nodes
    for (int i = 0; i < nodeCount; i++)
    {
        navNode *node{};
        // node->setPosition(areas->at(i).m_center.x, areas->at(i).m_center.y);
        node->vecLoc = areas->at(i).m_center;
        nodes.push_back(node);
    }

    for (int i = 0; i < nodeCount; ++i)
    {
        std::vector<NavConnect> *connections = &areas->at(i).m_connections;
        int childCount                       = connections->size();
        navNode *currNode                    = nodes.at(i);
        for (int j = 0; j < childCount; j++)
        {
            currNode->addChild(nodes.at(connections->at(j).id), 1.0f);
        }
    }
    logging::Info("Path init successful");
    return true;
}

int findClosestNavSquare(Vector vec)
{
    float bestDist = 999999.0f;
    int bestSquare = -1;
    for (int i = 0; i < nodes.size(); i++)
    {
        float dist = nodes.at(i)->vecLoc.DistTo(vec);
        if (dist < bestDist)
        {
            bestDist   = dist;
            bestSquare = i;
        }
    }
    return bestSquare;
}

std::vector<Vector> findPath(Vector loc, Vector dest)
{
    if (nodes.empty())
        return std::vector<Vector>(0);

    int id_loc  = findClosestNavSquare(loc);
    int id_dest = findClosestNavSquare(dest);

    navNode &node_loc  = *nodes.at(id_loc);
    navNode &node_dest = *nodes.at(id_dest);

    PathFinder<navNode> p;
    std::vector<navNode *> pathNodes;

    p.setStart(node_loc);
    p.setGoal(node_dest);

    p.findPath<navNode>(pathNodes);

    std::vector<Vector> path;
    for (int i = 0; i < pathNodes.size(); i++)
    {
        path.push_back(pathNodes.at(i)->vecLoc);
    }
    return path;
}
} // namespace hacks::shared::pathfinder
