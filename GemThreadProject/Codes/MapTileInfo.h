#ifndef _MAPTILEINFO_H_
#define _MAPTILEINFO_H_

#include "EngineDefines.h"
#include "glm\vec3.hpp"

class MapTileInfo
{
public:
	glm::vec3		m_vPos;
	_bool			m_bBlock;
	_uint			m_iTileX;
	_uint			m_iTileZ;
};

#endif //_MAPTILEINFO_H_