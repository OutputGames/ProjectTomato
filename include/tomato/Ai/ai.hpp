#if !defined(AI_HPP)
#define AI_HPP

#include "DetourCommon.h"
#include "DetourNavMesh.h"
#include "DetourNavMeshQuery.h"
#include "utils.hpp"
#include "Obj/obj.hpp"

#include "Recast.h"

namespace tmt::ai
{
    struct NavMesh;


    struct AgentInfo
    {
        float size;
        float height;
        float walkableSlopeAngle;
    };

    struct NavigationMgr
    {
        std::vector<AgentInfo> agentInfos;
        std::vector<NavMesh*> navMeshes;

        static NavigationMgr* pInstance;

        NavigationMgr();

        void Calculate(NavMesh* navMesh);
    };

    struct NavMesh
    {
        NavMesh();
        void Init(std::vector<glm::vec3> vertices, std::vector<int> indices);

        dtNavMesh* Calculate(AgentInfo info);

        std::vector<dtNavMesh*> navMeshes;

    private:
        std::vector<float> vertices;
        std::vector<int> indices;
    };


    struct PathfindingAgent : obj::Object
    {

        PathfindingAgent();

        std::vector<glm::vec3> FindPath(const glm::vec3& startPos, const glm::vec3& endPos);

    private:
        dtNavMesh* m_navMesh;
        dtNavMeshQuery* m_navQuery;
        dtQueryFilter m_filter;
    };


} // namespace tmt::ai


#endif // AI_HPP
