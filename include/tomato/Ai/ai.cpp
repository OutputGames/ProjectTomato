#include "ai.hpp"

#include "DetourCommon.h"
#include "DetourNavMesh.h"
#include "DetourNavMeshBuilder.h"
#include "DetourNavMeshQuery.h"
#include "DetourNavMeshQuery.h"
#include "Recast.h"
#include "Recast.h"
#include "Debug/debug.hpp"
#include "Math/math.hpp"
#include "Render/render.hpp"

using namespace tmt::ai;

NavigationMgr* NavigationMgr::pInstance = nullptr;

PathfindingAgent::PathfindingAgent(AgentInfo info):
    info(info),
    m_navMesh(nullptr)
{
    m_navQuery = dtAllocNavMeshQuery();
    m_navQuery->init(NavigationMgr::pInstance->navMesh->navMeshes[info.id], 2048);
    m_navMesh = NavigationMgr::pInstance->navMesh->navMeshes[info.id];
    m_filter = dtQueryFilter();
    m_filter.setIncludeFlags(0xffff);
    m_filter.setExcludeFlags(0);
}

std::vector<glm::vec3> PathfindingAgent::FindPath(const glm::vec3& startPos, const glm::vec3& endPos)
{
    if (m_navMesh == nullptr)
        return std::vector<glm::vec3>();

    std::vector<glm::vec3> path;

    float extents[3] = {2, 4, 2};

    dtPolyRef startRef, endRef;
    float startNearest[3] = {startPos.x, startPos.y, startPos.z}, endNearest[3] = {endPos.x, endPos.y, endPos.z};
    m_navQuery->findNearestPoly(startNearest, extents, &m_filter, &startRef, nullptr);
    m_navQuery->findNearestPoly(endNearest, extents, &m_filter, &endRef, nullptr);

    if (startRef && endRef)
    {
        dtPolyRef polys[256];
        int numPolys;
        float straightPath[256 * 3];
        unsigned char straightPathFlags[256];
        dtPolyRef straightPathPolys[256];
        int numStraightPath;

        m_navQuery->findPath(startRef, endRef, startNearest, endNearest, &m_filter, polys, &numPolys, 256);
        m_navQuery->findStraightPath(startNearest, endNearest, polys, numPolys, straightPath, straightPathFlags,
                                     straightPathPolys, &numStraightPath, 256);

        for (int i = 0; i < numStraightPath; ++i)
        {
            path.emplace_back(straightPath[i * 3], straightPath[i * 3 + 1], straightPath[i * 3 + 2]);
        }
    }

    return path;
}

void PathfindingAgent::Update()
{
    m_navMesh = NavigationMgr::pInstance->navMesh->navMeshes[info.id];


    Object::Update();
}

NavMeshSurface::NavMeshSurface()
{
}

void NavMeshSurface::Update()
{
    Object::Update();
}

AgentInfo::AgentInfo()
{
    id = NavigationMgr::pInstance->agentInfos.size();
}

void NavigationMgr::CalculateAllMeshes()
{
    var scene = obj::Scene::GetMainScene();

    var surfaces = scene->GetObjectsOfType<NavMeshSurface>();

    std::vector<NavMeshObject> meshes;
    for (auto surface : surfaces)
    {
        var meshs = surface->GetObjectsFromTypeRecursive<obj::MeshObject>();
        for (auto mesh : meshs)
        {
            meshes.push_back({mesh->mesh, mesh->GetTransform()});
        }
    }

    navMesh = new NavMesh;
    navMesh->Init(meshes);

    Calculate(navMesh);

}

NavigationMgr::NavigationMgr()
{
    pInstance = this;
    ctx = new rcContext(true);
}

void drawNavMeshPoly(const dtNavMesh* mesh, dtPolyRef ref)
{
    const dtMeshTile* tile = nullptr;
    const dtPoly* poly = nullptr;
    if (dtStatusFailed(mesh->getTileAndPolyByRef(ref, &tile, &poly)))
        return;

    const uint ip = poly - tile->polys;

    if (poly->getType() == DT_POLYTYPE_OFFMESH_CONNECTION)
    {

    }
    else
    {
        const dtPolyDetail* pd = &tile->detailMeshes[ip];
        auto tris = new tmt::debug::DebugTriangle[pd->triCount];
        for (int i = 0; i < pd->triCount; ++i)
        {
            const unsigned char* t = &tile->detailTris[(pd->triBase + i) * 4];
            var tri = tmt::debug::DebugTriangle{};
            for (int j = 0; j < 3; ++j)
            {
                if (t[j] < poly->vertCount)
                {
                    var pt = &tile->verts[poly->verts[t[j]] * 3];

                    tri[j] = tmt::math::convertVec3(pt);
                }
            }

            //tmt::debug::Gizmos::DrawSphere(tri.p1, 0.05f);
            //tmt::debug::Gizmos::DrawSphere(tri.p2, 0.05f);
            //tmt::debug::Gizmos::DrawSphere(tri.p3, 0.05f);

            tris[i] = tri;

        }
        tmt::debug::Gizmos::DrawTriangles(tris, pd->triCount);
    }
}

void NavigationMgr::Update()
{
    const dtNavMesh* mesh = navMesh->navMeshes[0];
    for (int i = 0; i < mesh->getMaxTiles(); ++i)
    {
        var tile = mesh->getTile(i);
        if (!tile->header)
            continue;
        dtPolyRef base = mesh->getPolyRefBase(tile);

        for (int j = 0; j < tile->header->polyCount; ++j)
        {
            const dtPoly* p = &tile->polys[j];
            drawNavMeshPoly(mesh, base | static_cast<dtPolyRef>(j));
        }
    }
}

void NavigationMgr::Calculate(NavMesh* navMesh)
{
    for (auto mesh : navMesh->navMeshes)
    {
        delete mesh;
    }

    navMesh->navMeshes.clear();
    navMesh->navMeshes.resize(agentInfos.size());

    for (int i = 0; i < agentInfos.size(); ++i)
    {
        var agentInfo = agentInfos[i];

        var mesh = navMesh->Calculate(agentInfo);

        navMesh->navMeshes[agentInfo.id] = mesh;
    }
}

NavMesh::NavMesh()
{

}


void ConvertOBBToVerticesAndIndices(const glm::vec3& center, const glm::vec3& extents, const glm::mat4& rotation,
                                    std::vector<float>& vertices, std::vector<int>& indices)
{
    glm::vec3 corners[8];
    corners[0] = glm::vec3(rotation * glm::vec4(center + glm::vec3(-extents.x, -extents.y, -extents.z), 1.0f));
    corners[1] = glm::vec3(rotation * glm::vec4(center + glm::vec3(extents.x, -extents.y, -extents.z), 1.0f));
    corners[2] = glm::vec3(rotation * glm::vec4(center + glm::vec3(extents.x, extents.y, -extents.z), 1.0f));
    corners[3] = glm::vec3(rotation * glm::vec4(center + glm::vec3(-extents.x, extents.y, -extents.z), 1.0f));
    corners[4] = glm::vec3(rotation * glm::vec4(center + glm::vec3(-extents.x, -extents.y, extents.z), 1.0f));
    corners[5] = glm::vec3(rotation * glm::vec4(center + glm::vec3(extents.x, -extents.y, extents.z), 1.0f));
    corners[6] = glm::vec3(rotation * glm::vec4(center + glm::vec3(extents.x, extents.y, extents.z), 1.0f));
    corners[7] = glm::vec3(rotation * glm::vec4(center + glm::vec3(-extents.x, extents.y, extents.z), 1.0f));

    int startIndex = vertices.size() / 3;
    for (const auto& corner : corners)
    {
        vertices.push_back(corner.x);
        vertices.push_back(corner.y);
        vertices.push_back(corner.z);
    }

    int faceIndices[36] = {
        0, 1, 2, 0, 2, 3, // Front face
        4, 5, 6, 4, 6, 7, // Back face
        0, 1, 5, 0, 5, 4, // Bottom face
        2, 3, 7, 2, 7, 6, // Top face
        0, 3, 7, 0, 7, 4, // Left face
        1, 2, 6, 1, 6, 5 // Right face
    };

    for (int i = 0; i < 36; ++i)
    {
        indices.push_back(startIndex + faceIndices[i]);
    }
}


void NavMesh::Init(std::vector<NavMeshObject> meshes)
{
    int vertexOffset = 0;
    for (auto mesh : meshes)
    {
        for (int i = 0; i < mesh.mesh->vertexCount; ++i)
        {
            var vertex = mesh.mesh->vertices[i];
            var pos = glm::vec4(vertex.position, 1.0);
            pos = pos * mesh.modelMatrix;


            vertices.push_back(pos.x);
            vertices.push_back(pos.y);
            vertices.push_back(pos.z);
        }
        for (int i = 0; i < mesh.mesh->indexCount; ++i)
        {
            var idx = static_cast<int>(mesh.mesh->indices[i]);

            indices.push_back(idx + vertexOffset);
        }

        vertexOffset += mesh.mesh->vertexCount;
    }
}

void NavMesh::Init(std::vector<NavMeshOBB> meshes)
{
    for (auto mesh : meshes)
    {
        ConvertOBBToVerticesAndIndices(mesh.center, mesh.extent, mesh.rotation, vertices, indices);
    }
}


// Function to create a navigation mesh
dtNavMesh* CreateNavMesh(const std::vector<float>& vertices, const std::vector<int>& indices, const rcConfig& config)
{
    var ctx = NavigationMgr::pInstance->ctx;
    var vertCount = vertices.size();

    // Step 1: Create a heightfield
    rcHeightfield* heightfield = rcAllocHeightfield();
    if (!heightfield)
    {
        std::cerr << "Failed to allocate heightfield." << std::endl;
        return nullptr;
    }

    if (!rcCreateHeightfield(ctx, *heightfield, config.width, config.height, config.bmin, config.bmax, config.cs,
                             config.ch))
    {
        std::cerr << "Failed to create heightfield." << std::endl;
        return nullptr;
    }


    // Step 2: Rasterize the triangles
    std::vector<unsigned char> triAreas(indices.size() / 3, 0);
    rcMarkWalkableTriangles(ctx, config.walkableSlopeAngle, vertices.data(), vertices.size() / 3, indices.data(),
                            indices.size() / 3, triAreas.data());
    rcRasterizeTriangles(ctx, vertices.data(), vertices.size() / 3, indices.data(), triAreas.data(),
                         indices.size() / 3, *heightfield, config.walkableClimb);

    // Step 3: Filter the walkable surfaces
    rcFilterLowHangingWalkableObstacles(ctx, config.walkableClimb, *heightfield);
    rcFilterLedgeSpans(ctx, config.walkableHeight, config.walkableClimb, *heightfield);
    rcFilterWalkableLowHeightSpans(ctx, config.walkableHeight, *heightfield);

    // Step 4: Create a compact heightfield
    rcCompactHeightfield* compactHeightfield = rcAllocCompactHeightfield();
    if (!compactHeightfield)
    {
        std::cerr << "Failed to allocate compact heightfield." << std::endl;
        return nullptr;
    }

    if (!rcBuildCompactHeightfield(ctx, config.walkableHeight, config.walkableClimb, *heightfield,
                                   *compactHeightfield))
    {
        std::cerr << "Failed to build compact heightfield." << std::endl;
        return nullptr;
    }

    // Step 5: Erode the walkable area
    if (!rcErodeWalkableArea(ctx, config.walkableRadius, *compactHeightfield))
    {
        std::cerr << "Failed to erode walkable area." << std::endl;
        return nullptr;
    }

    // Step 6: Build the distance field
    if (!rcBuildDistanceField(ctx, *compactHeightfield))
    {
        std::cerr << "Failed to build distance field." << std::endl;
        return nullptr;
    }

    // Step 7: Build the regions
    if (!rcBuildRegions(ctx, *compactHeightfield, 0, config.minRegionArea, config.mergeRegionArea))
    {
        std::cerr << "Failed to build regions." << std::endl;
        return nullptr;
    }

    // Step 8: Create a contour set
    rcContourSet* contourSet = rcAllocContourSet();
    if (!contourSet)
    {
        std::cerr << "Failed to allocate contour set." << std::endl;
        return nullptr;
    }

    if (!rcBuildContours(ctx, *compactHeightfield, config.maxSimplificationError, config.maxEdgeLen, *contourSet))
    {
        std::cerr << "Failed to build contours." << std::endl;
        return nullptr;
    }

    // Step 9: Create a poly mesh
    rcPolyMesh* polyMesh = rcAllocPolyMesh();
    if (!polyMesh)
    {
        std::cerr << "Failed to allocate poly mesh." << std::endl;
        return nullptr;
    }

    if (!rcBuildPolyMesh(ctx, *contourSet, config.maxVertsPerPoly, *polyMesh))
    {
        std::cerr << "Failed to build poly mesh." << std::endl;
        return nullptr;
    }

    // Step 10: Create a detail mesh
    rcPolyMeshDetail* polyMeshDetail = rcAllocPolyMeshDetail();
    if (!polyMeshDetail)
    {
        std::cerr << "Failed to allocate poly mesh detail." << std::endl;
        return nullptr;
    }

    if (!rcBuildPolyMeshDetail(ctx, *polyMesh, *compactHeightfield, config.detailSampleDist,
                               config.detailSampleMaxError, *polyMeshDetail))
    {
        std::cerr << "Failed to build poly mesh detail." << std::endl;
        return nullptr;
    }

    for (int i = 0; i < polyMesh->npolys; ++i)
    {
        if (polyMesh->areas[i] == RC_WALKABLE_AREA)
            polyMesh->flags[i] = 0x01;
    }


    // Step 11: Create the navmesh data
    unsigned char* navData = nullptr;
    int navDataSize = 0;
    dtNavMeshCreateParams params = {};
    params.verts = polyMesh->verts;
    params.vertCount = polyMesh->nverts;
    params.polys = polyMesh->polys;
    params.polyAreas = polyMesh->areas;
    params.polyFlags = polyMesh->flags;
    params.polyCount = polyMesh->npolys;
    params.nvp = polyMesh->nvp;
    params.detailMeshes = polyMeshDetail->meshes;
    params.detailVerts = polyMeshDetail->verts;
    params.detailVertsCount = polyMeshDetail->nverts;
    params.detailTris = polyMeshDetail->tris;
    params.detailTriCount = polyMeshDetail->ntris;
    params.walkableHeight = config.walkableHeight;
    params.walkableRadius = config.walkableRadius;
    params.walkableClimb = config.walkableClimb;
    rcVcopy(params.bmin, polyMesh->bmin);
    rcVcopy(params.bmax, polyMesh->bmax);
    params.cs = config.cs;
    params.ch = config.ch;
    params.buildBvTree = true;

    int gw = 0, gh = 0;
    const float* bmin = config.bmin;
    const float* bmax = config.bmax;
    rcCalcGridSize(bmin, bmax, config.cs, &gw, &gh);
    const int ts = config.tileSize;
    const int tw = (gw + ts - 1) / ts;
    const int th = (gh + ts - 1) / ts;

    // Max tiles and max polys affect how the tile IDs are caculated.
    // There are 22 bits available for identifying a tile and a polygon.
    int tileBits = rcMin(static_cast<int>(dtIlog2(bx::nextPow2(tw * th))), 14);
    if (tileBits > 14)
        tileBits = 14;
    int polyBits = 22 - tileBits;
    var m_maxTiles = 1 << tileBits;
    var m_maxPolysPerTile = 1 << polyBits;


    float o[3] = {0, 0, 0};

    dtNavMeshParams nparams = {};
    nparams.tileWidth = config.tileSize * config.cs;
    nparams.tileHeight = config.tileSize * config.cs;
    nparams.maxTiles = m_maxTiles;
    nparams.maxPolys = m_maxPolysPerTile;

    if (!dtCreateNavMeshData(&params, &navData, &navDataSize))
    {
        std::cerr << "Failed to create navmesh data." << std::endl;
        return nullptr;
    }

    // Step 12: Initialize the navmesh
    dtNavMesh* navMesh = dtAllocNavMesh();
    if (!navMesh)
    {
        std::cerr << "Failed to allocate navmesh." << std::endl;
        return nullptr;
    }


    if (dtStatusFailed(navMesh->init(navData, navDataSize, DT_TILE_FREE_DATA, o)))
    {
        std::cerr << "Failed to initialize navmesh." << std::endl;
        return nullptr;
    }


    var t = navMesh->getMaxTiles();
    //dtVcopy(navMesh->getParams()->orig, o);

    return navMesh;
}

dtNavMesh* NavMesh::Calculate(AgentInfo info)
{
    rcConfig config = {};
    config.cs = 0.3f;
    config.ch = 0.2f;
    config.walkableSlopeAngle = info.walkableSlopeAngle;
    config.walkableHeight = static_cast<int>(ceilf(info.height / config.ch));
    config.walkableClimb = static_cast<int>(floorf(info.maxClimb / config.ch));
    config.walkableRadius = static_cast<int>(ceilf(info.size / config.cs));
    config.maxEdgeLen = static_cast<int>(12.0f / config.cs);
    config.maxSimplificationError = 1.3f;
    config.minRegionArea = rcSqr(8); // Note: area = size*size
    config.mergeRegionArea = rcSqr(20); // Note: area = size*size
    config.maxVertsPerPoly = 6;
    config.detailSampleDist = config.cs * 6.0f;
    config.detailSampleMaxError = config.ch * 1.0f;
    config.tileSize = 16;
    config.borderSize = config.walkableRadius + 3;


    rcCalcBounds(vertices.data(), vertices.size() / 3, config.bmin, config.bmax);
    rcCalcGridSize(config.bmin, config.bmax, config.cs, &config.width, &config.height);

    return CreateNavMesh(vertices, indices, config);
}
