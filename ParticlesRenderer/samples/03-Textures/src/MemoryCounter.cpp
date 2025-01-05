#include <MemoryCounter.h>

#include <windows.h>
#include <psapi.h>
#include "../nvml/nvml.h"
#include <fstream>
#include <iostream>

MemoryCounter::MemoryCounter()
{
    ClearFile( m_CPULocation );
    ClearFile( m_GPULocation );
}

void MemoryCounter::Update( int sampleCount ) const
{
    if ( std::ofstream file {m_CPULocation, std::ios::app} )
    {
        size_t memoryInBytes { GetCPUMemoryUsage() };
        size_t memoryInMegaBytes { memoryInBytes / ( 1024 * 1024 ) };
        //file << "[" << sampleCount << "]: " << memoryInBytes << " B\n";
        //file << "[" << sampleCount << "]: " << memoryInMegaBytes << " MB\n";
        file << memoryInBytes << "\n";
    }

    if ( std::ofstream file { m_GPULocation, std::ios::app } )
    {
        size_t memoryInBytes { GetGPUMemoryUsage() };
        size_t memoryInMegaBytes { memoryInBytes / ( 1024 * 1024 ) };
        //file << "[" << sampleCount << "]: " << memoryInBytes << " B\n";
        //file << "[" << sampleCount << "]: " << memoryInMegaBytes << " MB\n";
        file << memoryInBytes << "\n";
    }
}

size_t MemoryCounter::GetCPUMemoryUsage()
{
    PROCESS_MEMORY_COUNTERS_EX pmc{};
    if ( GetProcessMemoryInfo( GetCurrentProcess(), reinterpret_cast<PROCESS_MEMORY_COUNTERS*>(&pmc), sizeof( pmc ) ) )
    {
        return pmc.PrivateUsage;  // Returns memory usage in bytes
    }
    return 0;
}

size_t MemoryCounter::GetGPUMemoryUsage()
{
    nvmlInit();
    nvmlDevice_t device;
    nvmlDeviceGetHandleByIndex( 0, &device );

    nvmlMemory_t memory;
    nvmlDeviceGetMemoryInfo( device, &memory );

    nvmlShutdown();
    return memory.used;
     // Returns GPU memory usage in bytes
}

void MemoryCounter::ClearFile( const std::string& fileLocation )
{
    if ( std::ofstream file { fileLocation, std::ios::trunc } )
    {}
}