struct VertexShaderOutput
{
    float2 TexCoord : TEXCOORD;
    float4 Position : SV_Position;
};

struct GeometryShaderOutput
{
    float2 TexCoord : TEXCOORD;
    float4 Position : SV_Position;
};

cbuffer Constants : register(b1)
{
    float FOV;
    float ParticleSize;
};

struct Mat
{
    matrix ModelViewProjectionMatrix;
};
ConstantBuffer<Mat> MVPMatrix : register(b0);


void CreateTriangle(float4 v1, float4 v2, float4 v3, inout TriangleStream<GeometryShaderOutput> triangleStream)
{
    GeometryShaderOutput OUT;
    //top-left
    OUT.Position = v1;
    OUT.TexCoord = float2(0.0f, 0.0f);
    triangleStream.Append(OUT);

    //top-right
    OUT.Position = v2;
    OUT.TexCoord = float2(1.0f, 0.0f);
    triangleStream.Append(OUT);

    //bottom-right
    OUT.Position = v3;
    OUT.TexCoord = float2(1.0f, 1.0f);
    triangleStream.Append(OUT);
}

float4 CreateVertexFromBasePoint(float x, float y)
{
    return mul(MVPMatrix.ModelViewProjectionMatrix, float4(x, y, 0, 1.0f));
}


[maxvertexcount(6)]
void main(inout TriangleStream<GeometryShaderOutput> triangleStream)
{
    // quad vertices
    float4 topLeft = CreateVertexFromBasePoint(-ParticleSize, ParticleSize);
    float4 topRight = CreateVertexFromBasePoint(ParticleSize, ParticleSize);
    float4 bottomLeft = CreateVertexFromBasePoint(-ParticleSize, -ParticleSize);
    float4 bottomRight = CreateVertexFromBasePoint(ParticleSize, -ParticleSize);

    //----------------------------------------------------------
    //first triangle
    CreateTriangle(topLeft, topRight, bottomRight, triangleStream);
    //----------------------------------------------------------

    //----------------------------------------------------------
    //second triangle
    CreateTriangle(topLeft, bottomLeft, bottomRight, triangleStream);
    //----------------------------------------------------------
}

