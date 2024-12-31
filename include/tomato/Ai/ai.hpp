#if !defined(AI_HPP)
#define AI_HPP

#include <DetourNavMeshQuery.h>
#include "utils.hpp"
#include "Obj/obj.hpp"

class rcContext;
class dtQueryFilter;
class dtNavMeshQuery;
class dtNavMesh;

namespace tmt::ai
{
    struct AgentInfo;
    struct NavMeshOBB;
    struct NavMesh;


    struct NavigationMgr
    {
        std::vector<AgentInfo> agentInfos;
        std::vector<NavMeshOBB> obbs;
        NavMesh* navMesh;
        rcContext* ctx;

        void CalculateAllMeshes();

        static NavigationMgr* pInstance;

        NavigationMgr();

        void Update();

        void Calculate(NavMesh* navMesh);
    };

    struct AgentInfo
    {
        float size;
        float height;
        float walkableSlopeAngle;
        float maxClimb;
        int id = -1;


        AgentInfo();

        AgentInfo(float sz, float hg, float s, float maxClimb)
        {
            size = sz;
            height = hg;
            walkableSlopeAngle = s;
            this->maxClimb = maxClimb;
            id = NavigationMgr::pInstance->agentInfos.size();
        }
    };

    struct NavMeshObject
    {
        render::Mesh* mesh;
        glm::mat4 modelMatrix;
    };

    struct NavMeshOBB
    {
        glm::vec3 center;
        glm::vec3 extent;
        glm::mat4 rotation;
    };

    struct NavMesh
    {
        NavMesh();
        void Init(std::vector<NavMeshObject> meshes);
        void Init(std::vector<NavMeshOBB> meshes);

        dtNavMesh* Calculate(AgentInfo info);

        std::vector<dtNavMesh*> navMeshes;

    private:
        std::vector<float> vertices;
        std::vector<int> indices;
    };


    struct PathfindingAgent : obj::Object
    {

        PathfindingAgent(AgentInfo info);

        std::vector<glm::vec3> FindPath(const glm::vec3& startPos, const glm::vec3& endPos);

        void Update() override;

        AgentInfo info;

    private:
        dtNavMesh* m_navMesh;
        dtNavMeshQuery* m_navQuery;
        dtQueryFilter m_filter;
    };

    struct NavMeshSurface : obj::Object
    {
        NavMeshSurface();

        void Update() override;
    };


} // namespace tmt::ai


#endif // AI_HPP
