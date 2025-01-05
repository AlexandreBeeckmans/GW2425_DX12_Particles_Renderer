#pragma once
#include <string>


class MemoryCounter
{
public:
    MemoryCounter();

    void Update(int sampleCount) const;

private:
    static size_t GetCPUMemoryUsage();
    static size_t GetGPUMemoryUsage();

    static void ClearFile( const std::string& fileLocation );

    std::string m_CPULocation { "logCPU.txt" };
    std::string m_GPULocation { "logGPU.txt" };
};