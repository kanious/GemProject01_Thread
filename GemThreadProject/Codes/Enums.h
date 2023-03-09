#ifndef _ENUMS_H_
#define _ENUMS_H_

// Header for defining global enums

enum eSCENETAG
{
	SCENE_3D,
	SCENE_END
};

enum eLAYERTAG
{
	LAYER_STATIC_OBJECT,
	LAYER_INTERACTIVE_OBJECT,
	LAYER_CHARACTER,
	LAYER_ENEMY,
	LAYER_EVENT_OBJECT,
	LAYER_UI,
	LAYER_CAMERA,
	LAYER_END
};

enum eOBJTAG
{
	OBJ_CHARACTER,
	OBJ_BACKGROUND,
	OBJ_CAMERA,
	OBJ_DYNAMIC,
	OBJ_ITEM,
	OBJ_PROJECTILE,
	OBJ_END
};

enum eAIType
{
	AI_SEEK,
	AI_FLEE,
	AI_PURSUE,
	AI_EVADE,
	AI_APPROACH,
	AI_END
};

enum eAnimType
{
	ANIM_MOVING,
	ANIM_ROTATING,
	ANIM_SCALING,
	ANIM_END
};

enum eEaseType
{
	EASE_NONE,
	EASE_IN,
	EASE_OUT,
	EASE_INOUT,
	EASE_END
};

enum eDir
{
	DIR_LEFT,
	DIR_RIGHT,
	DIR_UP,
	DIR_DOWN,
	DIR_NONE
};

enum eState
{
	STATE_CHASING_MOVE,
	STATE_CHASING_TURN,
	STATE_CHASING_ATTACK,
	STATE_ROAMING_MOVE,
	STATE_ROAMING_TURN,
	STATE_DYING,
	STATE_DEAD,
	STATE_NONE
};


#endif //_ENUMS_H_