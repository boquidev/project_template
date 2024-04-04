#include "shader_header.h"

float4 ps(VS_OUTPUT_DEFAULT input, uint tid : SV_PrimitiveID) : SV_TARGET
{
	float4 texcolor = color_texture.Sample( sampler0, input.texcoord );

	// clip(texcolor.a *input.color.a -0.5);

	float4 result = float4(
		texcolor.r * input.color.r, 
		texcolor.g * input.color.g,
		texcolor.b * input.color.b,
		texcolor.a * input.color.a);
		

	// return float4(1,0,0,1);
	// return float4(input.vertex_world_pos.xyz, 1);
	return result;
}