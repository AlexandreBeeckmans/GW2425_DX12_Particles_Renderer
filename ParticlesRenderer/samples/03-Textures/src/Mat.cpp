#include "../inc/Mat.h"

#include <random>

float Math::GetRandomInRange( float min, float max )
{
    std::random_device                    rd {};
    std::mt19937                          gen { rd() };
    std::uniform_real_distribution<float> dis { min, max };

    return dis( gen );
}