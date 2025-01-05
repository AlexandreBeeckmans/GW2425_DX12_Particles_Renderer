struct PixelShaderInput
{
    float2 TexCoord   : TEXCOORD;
};


Texture2D DiffuseTexture            : register( t2 );
SamplerState LinearRepeatSampler    : register(s0);

float4 main( PixelShaderInput IN ) : SV_Target
{
    float4 texColor = DiffuseTexture.Sample( LinearRepeatSampler, IN.TexCoord );
    return float4(texColor.r, texColor.g, texColor.b, 0);
}