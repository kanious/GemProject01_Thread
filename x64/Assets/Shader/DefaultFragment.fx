#version 460

in vec4 fColour;
in vec4 fNormal;
in vec4 fVtxWorldPos;
in vec4 fTexUV;
in vec4 fTangent;
in vec4 fBinormal;
in mat4 fInvMat;

uniform sampler2D diffTexture;

uniform bool isSelected;
uniform bool isTransparency;

out vec4 daColor;
void main()
{
	vec4 diffColour = texture(diffTexture, fTexUV.xy);

	//if (isTransparency)
	//	daColor.a = 1.f;

	if (isSelected)
		daColor = vec4(diffColour.x + 0.3f, diffColour.y, diffColour.z + 0.3f, diffColour.a);
	else
		daColor = diffColour;
} 