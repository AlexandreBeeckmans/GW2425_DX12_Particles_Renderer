#include "DX12LibPCH.h"
#include "dx12lib/CommandList.h"

#include <dx12lib/StructuredBuffer.h>

#include <dx12lib/Device.h>
#include <dx12lib/ResourceStateTracker.h>
#include <dx12lib/d3dx12.h>

using namespace dx12lib;

StructuredBuffer::StructuredBuffer( Device& device, size_t numElements, size_t elementSize )
: Buffer( device,
          CD3DX12_RESOURCE_DESC::Buffer( numElements * elementSize, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS ) )
, m_NumElements( numElements )
, m_ElementSize( elementSize )
{
    m_CounterBuffer = m_Device.CreateByteAddressBuffer( 4 );
}

StructuredBuffer::StructuredBuffer( Device& device, Microsoft::WRL::ComPtr<ID3D12Resource> resource, size_t numElements,
                                    size_t elementSize )
: Buffer( device, resource )
, m_NumElements( numElements )
, m_ElementSize( elementSize )
{
    m_CounterBuffer = m_Device.CreateByteAddressBuffer( 4 );
}


void StructuredBuffer::UploadDataToStructuredBuffer( Device& device, std::shared_ptr<StructuredBuffer>& structuredBuffer, const void* data, size_t dataSize )
{
    // Create an upload buffer (a staging buffer) to hold the data temporarily.
    D3D12_HEAP_PROPERTIES heapProps = {};
    heapProps.Type                  = D3D12_HEAP_TYPE_UPLOAD;

    D3D12_RESOURCE_DESC bufferDesc = {};
    bufferDesc.Dimension           = D3D12_RESOURCE_DIMENSION_BUFFER;
    bufferDesc.Alignment           = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
    bufferDesc.Width               = dataSize;
    bufferDesc.Height              = 1;
    bufferDesc.DepthOrArraySize    = 1;
    bufferDesc.MipLevels           = 1;
    bufferDesc.Format              = DXGI_FORMAT_UNKNOWN;
    bufferDesc.SampleDesc.Count    = 1;
    bufferDesc.SampleDesc.Quality  = 0;
    bufferDesc.Flags               = D3D12_RESOURCE_FLAG_NONE;
    bufferDesc.Layout              = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

    Microsoft::WRL::ComPtr<ID3D12Resource> stagingBuffer;
    HRESULT hr = device.GetD3D12Device()->CreateCommittedResource( &heapProps, D3D12_HEAP_FLAG_NONE, &bufferDesc,
                                                                   D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
                                                                   IID_PPV_ARGS( &stagingBuffer ) );

    if ( FAILED( hr ) )
    {
        // Handle error
        return;
    }

    // Map the staging buffer to CPU memory
    void* mappedData = nullptr;
    hr               = stagingBuffer->Map( 0, nullptr, &mappedData );
    if ( FAILED( hr ) )
    {
        // Handle error
        return;
    }

    // Copy data to the mapped buffer
    memcpy( mappedData, data, dataSize );

    // Unmap the staging buffer to ensure the data is uploaded
    stagingBuffer->Unmap( 0, nullptr );

    // Now, create the structured buffer using the data
    // Using the CreateStructuredBuffer function to create the structured buffer
    structuredBuffer = device.CreateStructuredBuffer( stagingBuffer, dataSize / sizeof( XMFLOAT3 ), sizeof( XMFLOAT3 ) );
}
