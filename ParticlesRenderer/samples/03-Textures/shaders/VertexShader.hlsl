struct Mat
{
    matrix ModelViewProjectionMatrix;
};

ConstantBuffer<Mat> MVPMatrix : register(b0);

struct VertexPositionNormalTexture
{
    float3 Position  : POSITION;
    float2 TexCoord  : TEXCOORD;
};

struct VertexShaderOutput
{
    float2 TexCoord   : TEXCOORD;
    float4 Position   : SV_Position;
};

VertexShaderOutput main(VertexPositionNormalTexture IN)
{
    VertexShaderOutput OUT;

    OUT.Position = mul(MVPMatrix.ModelViewProjectionMatrix, float4(IN.Position, 1.0f));
    
    OUT.TexCoord = IN.TexCoord;

    return OUT;
}