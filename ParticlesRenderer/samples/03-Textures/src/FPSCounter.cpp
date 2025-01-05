#include <FPSCounter.h>
#include <fstream>
#include <iostream>
#include <numeric>

FPSCounter::FPSCounter()
{
    if ( std::ofstream logFile { m_FileLocation, std::ios::trunc } )
    {}
    std::cout << m_CurrentSampleId << " : Particles Amount [100]: ";

    if ( std::ofstream logFile { m_FileLocation, std::ios::app } )
    {
        //logFile << m_CurrentSampleId << " : Particles Amount [100]: ";
    }
}

bool FPSCounter::Update( float deltaTime )
{
    if ( m_CurrentSampleId >= m_SamplesAmount ) return false;


    m_AccumulatedTime += deltaTime;
    float fps = 1.0f / deltaTime;

    m_ValuesThisSecond.push_back(fps);

    if ( m_AccumulatedTime < 1.0f ) return true;

    m_AccumulatedTime -= 1.0f;
    m_ValuesThisSample.push_back( GetAverageThisSecond() );
    m_ValuesThisSecond.clear();

    std::cout << m_ValuesThisSample[m_ValuesThisSample.size() - 1] << " ";
    WriteValueToFile( m_ValuesThisSample[m_ValuesThisSample.size() - 1] );

    return true;
}

void FPSCounter::UpdateSample(int sampleAmount)
{
    m_ValuesThisSample.clear();
    ++m_CurrentSampleId;
    std::cout << "\n" << m_CurrentSampleId << " : Particles Amount [" << sampleAmount << "]: ";
    if ( std::ofstream logFile { m_FileLocation, std::ios::app } )
    {
        //logFile << "\n" << m_CurrentSampleId << " : Particles Amount [" << sampleAmount << "]: ";
        logFile << "\n";
    }
}

float FPSCounter::GetAverageThisSecond() const
{
    float sum = std::accumulate( m_ValuesThisSecond.cbegin(), m_ValuesThisSecond.cend(), 0.0f );
    return sum / static_cast<float>(m_ValuesThisSecond.size());
}

void FPSCounter::WriteValueToFile(float value) const
{
    if ( std::ofstream logFile { m_FileLocation, std::ios::app } )
    {
        logFile << value;
        if (m_ValuesThisSample.size() < 10)
        {
            logFile << " ";
        }
    }
}