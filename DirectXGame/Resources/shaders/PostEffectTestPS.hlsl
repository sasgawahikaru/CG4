#include "PostEffectTest.hlsli"

Texture2D<float4> tex0 : register(t0);  // 0番スロットに設定されたテクスチャ
Texture2D<float4> tex1 : register(t1);  // 1番スロットに設定されたテクスチャ
SamplerState smp : register(s0);      // 0番スロットに設定されたサンプラー

float4 main(VSOutput input) : SV_TARGET
{
	float4 colortex0 = 1- tex0.Sample(smp,input.uv);
	float4 colortex1 = 0;

	float4 color = colortex0;

	for (float x = -1; x < 2; x++)
	{
		for (float y = -1; y < 2; y++)
		{
			colortex1 += tex1.Sample(smp, input.uv+float2(x/640,y/360));
		}
	}
	colortex1 /= (3 * 3);
//	float4 color = colortex0;
	if (fmod(input.uv.y, 0.1f) < 0.05f)
	{	
		color = colortex1;
	}
	////通常
	return float4(color.rgb, 1);



	////明度
//	return float4(texcolor.rgb * 2.0f, 1);
	////反転
//	return float4(texcolor.rgb * 2.0f, 1);
}