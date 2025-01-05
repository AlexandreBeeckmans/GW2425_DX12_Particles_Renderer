#pragma once
#include <string>
#include <vector>

class FPSCounter
{
public:
    FPSCounter();
    bool Update( float deltaTime );

    void UpdateSample(int sampleAmount);

private:
    float GetAverageThisSecond() const;
    void  WriteValueToFile( float value ) const;


    float m_AccumulatedTime{0.0f};

    static constexpr int m_SamplesAmount{12};
    int m_CurrentSampleId { 0 };

    std::vector<float> m_ValuesThisSecond{};
    std::vector<float> m_ValuesThisSample{};

    std::string m_FileLocation { "log.txt" };
};