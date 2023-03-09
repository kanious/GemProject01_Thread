#version 460

uniform mat4 matWorld;

layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aTexCoords;

out vec2 TexCoords;

void main()
{
	//gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0);

	vec4 vPos = vec4(aPos.x, aPos.y, 0.0, 1.0);
	gl_Position = matWorld * vPos;

	TexCoords = aTexCoords;
}