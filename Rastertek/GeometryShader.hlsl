struct GSInput
{
	float4 pos : SV_POSITION;
	float4 color : COLOR;
};


struct PixelInputType
{
	float4 position : SV_POSITION;
	float4 color : COLOR0;
};

[maxvertexcount(18)]
void main(
	point GSInput input[1] : SV_POSITION, 
	inout TriangleStream<PixelInputType> output
)
{
	float offset = 1.0f;
	PixelInputType element;

	float4 position = input[0].pos;
	float4 color = input[0].color;
    float4 otherColor1 = float4(0.0f, 0.0f, 1.0f, 1.0f);
	float4 otherColor2 = float4(0.0f, 1.0f, 0.0f, 1.0f);

	element.position = position;
	element.color = color;
	output.Append(element);
	
	element.position = position;
	element.position.x = position.x + offset * 0.5f;
	element.position.y = position.y + offset;
	element.color = otherColor1;
	output.Append(element);

	element.position = position;
	element.position.x = position.x + offset;
	element.color = otherColor2;
	output.Append(element);

	output.RestartStrip();
	
}