#ifndef _FUNCTION_H_
#define _FUNCTION_H_

#include <cstdlib>
#include <string>
#include "OpenGLDefines.h"
#include "Enums.h"

// Header for defining global functions

static int GetRandNum(int min, int max)
{
	return (rand() % (max - min + 1)) + min;
}

static int GetRandNum_Big(int min, int max)
{
	return ((int)(((double)((rand() << 15) | rand())) / ((RAND_MAX << 15 | RAND_MAX) + 1) * (max - min + 1)) + min);
}

static void PrintVector(glm::vec3 vec, std::string title = "")
{
	std::cout << title << ": " << vec.x << ", " << vec.y << ", " << vec.z << std::endl;
}

static float safe_acos(float value)
{
	if (value <= -1.0f)
		return glm::radians(180.f);
	else if (value >= 1.0f)
		return 0.f;
	else
		return glm::acos(value);
}

static float GetAngle(glm::vec3 vDir1, glm::vec3 vDir2)
{
	float fDot = glm::dot(vDir1, vDir2);
	float fAngleGap = safe_acos(fDot);
	return glm::degrees(fAngleGap);
}

static eLAYERTAG GetLayerTagByName(std::string name)
{
	if ("Layer_camera" == name)
		return LAYER_CAMERA;
	else if ("Layer_static" == name)
		return LAYER_STATIC_OBJECT;
	else if ("Layer_interactive" == name)
		return LAYER_INTERACTIVE_OBJECT;
	else if ("Layer_character" == name)
		return LAYER_CHARACTER;
	else if ("Layer_enemy" == name)
		return LAYER_ENEMY;
	else if ("Layer_event" == name)
		return LAYER_EVENT_OBJECT;
	else if ("Layer_ui" == name)
		return LAYER_UI;

	return LAYER_END;
}

#endif //_FUNCTION_H_