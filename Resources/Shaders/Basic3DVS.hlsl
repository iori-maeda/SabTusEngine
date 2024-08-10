
struct Output
{
	float4 position : SV_POSITION;
};

struct Input
{
	float4 position : POSITION0;
};


Output main( Input input)
{
	Output output;
	output.position = input.position;
	return output;
}