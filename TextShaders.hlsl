Texture2D t1 : register(t0);
SamplerState s1 : register(s0);

struct VSInput
{
	float4 pos : POSITION;
	float4 texCoord: TEXCOORD;
	float4 color: COLOR;
};

struct PSInput
{
	float4 pos: SV_POSITION;
	float4 color: COLOR;
	float2 texCoord: TEXCOORD;
};

PSInput VSMain(VSInput input, uint vertexID : SVVertexID)
{
	PSInput result;

	// vert id 0 = 0000, uv = (0, 0)
	// vert id 1 = 0001, uv = (1, 0)
	// vert id 2 = 0010, uv = (0, 1)
	// vert id 3 = 0011, uv = (1, 1)
	float2 uv = float2(vertexID & 1, (vertexID >> 1) & 1);

	// 정점 기준으로 정점의 위치 설정
	result.pos = float4(input.pos.x + (input.pos.z * uv.x), input.pos.y - (input.pos.w * uv.y), 0, 1);
	result.color = input.color;

	//정점 기준으로 텍스쳐 좌표를 설정합니다.
	result.texCoord = float2(input.texCoord.x + (input.texCoord.z * uv.x), input.texCoord.y + (input.texCoord.w * uv.y));

	return result;
}

float4 PSMain(PSInput input) : SV_TARGET
{
	return float4(input.color.rgb, input.color.a * t1.Sample(s1, input.texCoord).a);
}

