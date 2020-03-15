#ifndef _MODULEAI_H_
#define _MODULEAI_H_
#define ENGINE_EXPORTS

#include "Module/Module.h"
#include "AI/NavMesh.h"
#include <MathGeoLib/MathGeoLib.h>

class PanelNavMesh;
class ComponentCamera;

class ModuleAI : public Module
{
public:
	ModuleAI() = default;
	~ModuleAI() = default;

	bool Init() override;
	update_status Update() override;
	bool CleanUp() override;

	void RenderNavMesh(ComponentCamera& camera);

	bool FindPath();
	ENGINE_API bool IsPointWalkable(float3& target_position);

public:
	std::vector<float3> debug_path;

	//Starting and ending points
	//PathFinding
	float3 start_position;
	float3 end_position;

	bool start_initialized = false;
	bool end_initialized = false;

private:
	NavMesh nav_mesh = NavMesh();

	friend PanelNavMesh;
};

#endif //_MODULEAI_H_