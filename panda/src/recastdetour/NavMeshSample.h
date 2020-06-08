#ifndef NAVMESHSAMPLE_H
#define NAVMESHSAMPLE_H

#ifndef CPPPARSER
#include "Recast.h"
#else
class rcContext;
struct rcHeightfield;
struct rcCompactHeightfield;
struct rcContourSet;
struct rcPolyMesh;
struct rcConfig;
struct rcPolyMeshDetail;
#endif

#include "InputGeom.h"
#include <string>
#include "geom.h"
#include "geomNode.h"
#include "nodePath.h"

class NavMeshSample
{
protected:
	class InputGeom* m_geom;
	class dtNavMesh* m_navMesh;
	class dtNavMeshQuery* m_navQuery;
	class dtCrowd* m_crowd;

	float m_cellSize;
	float m_cellHeight;
	float m_agentHeight;
	float m_agentRadius;
	float m_agentMaxClimb;
	float m_agentMaxSlope;
	float m_regionMinSize;
	float m_regionMergeSize;
	float m_edgeMaxLen;
	float m_edgeMaxError;
	float m_vertsPerPoly;
	float m_detailSampleDist;
	float m_detailSampleMaxError;
	int m_partitionType;

	bool m_filterLowHangingObstacles;
	bool m_filterLedgeSpans;
	bool m_filterWalkableLowHeightSpans;


	rcContext* m_ctx;

	dtNavMesh* loadAll(const std::string& path);
	void saveAll(const std::string& path, const dtNavMesh* mesh);

	unsigned char* m_triareas;
	rcHeightfield* m_solid;
	rcCompactHeightfield* m_chf;
	rcContourSet* m_cset;
	rcPolyMesh* m_pmesh;
	rcConfig m_cfg;
	rcPolyMeshDetail* m_dmesh;

	void cleanup();

public:

	NavMeshSample();
	~NavMeshSample();
	void setContext(rcContext* ctx) { m_ctx = ctx; }
	
	virtual void collectSettings(struct BuildSettings& settings);

	void resetCommonSettings();
	void handleCommonSettings();

	bool handleBuild();
	void LoadGeom(const std::string& filepath)
	{ 
		m_geom = new InputGeom;
		m_geom->load(m_ctx, filepath);
	}
	void LoadGeom(NodePath node)
	{
		m_geom = new InputGeom;
		m_geom->load(m_ctx, node);
	}

	PT(GeomNode) getPolyMeshGeom();



private:
	// Explicitly disabled copy constructor and copy assignment operator.
	NavMeshSample(const NavMeshSample&);
	NavMeshSample& operator=(const NavMeshSample&);
};

enum SamplePartitionType
{
	SAMPLE_PARTITION_WATERSHED,
	SAMPLE_PARTITION_MONOTONE,
	SAMPLE_PARTITION_LAYERS,
};

#endif // NAVMESHSAMPLE_H