#define MAX_PARTICLES_PER_GROUP 1

StructuredBuffer<matrix> Matrices : register(t0);

struct MeshOutput
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD;
};

cbuffer Constants : register(b1)
{
    float FOV;
    float ParticleSize;
    float ParticlesAmount;
};

float4 CreateVertexPositionFromMatrix(matrix inputMatrix, float x, float y)
{
    return mul(inputMatrix, float4(x, y, 0.0f, 1.0f));
}

MeshOutput CreateVertexFromBasePoint(matrix inputMatrix, float2 position, float2 uv)
{
    MeshOutput OUT;
    OUT.Position = CreateVertexPositionFromMatrix(inputMatrix, position.x, position.y);
    OUT.TexCoord = float2(uv.x, uv.y);
    return OUT;
}

[numthreads(MAX_PARTICLES_PER_GROUP, 1, 1)]
[outputtopology("triangle")]
void main(out indices uint3 triangles[2 * MAX_PARTICLES_PER_GROUP], out vertices MeshOutput vertices[4 * MAX_PARTICLES_PER_GROUP], uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint groupStart = dispatchThreadID.x * MAX_PARTICLES_PER_GROUP;
    uint groupEnd = min(groupStart + MAX_PARTICLES_PER_GROUP, ParticlesAmount);

    //Amount of triangles and vertices definition
    uint numParticles = groupEnd - groupStart;
    uint numTriangles = 2 * numParticles;
    uint numVertices = 4 * numParticles;
    SetMeshOutputCounts(numVertices, numTriangles);

    for (uint i = groupStart; i < groupEnd; ++i)
    {
        matrix inputMatrix = Matrices[i];

        //Indexes Definition
        uint baseIndex = 4 * (i - groupStart);
        uint baseTriangleIndex = 2 * (i - groupStart);

        //Triangles Definition
        triangles[baseTriangleIndex] = uint3(baseIndex, baseIndex + 1, baseIndex + 2);
        triangles[baseTriangleIndex + 1] = uint3(baseIndex, baseIndex + 2, baseIndex + 3);

        // Vertices Definition
        //top-left
        vertices[baseIndex] = CreateVertexFromBasePoint(inputMatrix, float2(-ParticleSize, ParticleSize), float2(0.0f, 1.0f));

        //top-right
        vertices[baseIndex + 1] = CreateVertexFromBasePoint(inputMatrix, float2(ParticleSize, ParticleSize), float2(1.0f, 1.0f));

        //bottom-right
        vertices[baseIndex + 2] = CreateVertexFromBasePoint(inputMatrix, float2(ParticleSize, -ParticleSize), float2(1.0f, 0.0f));

        //bottom-left
        vertices[baseIndex + 3] = CreateVertexFromBasePoint(inputMatrix, float2(-ParticleSize, -ParticleSize), float2(0.0f, 0.0f));
    }
}