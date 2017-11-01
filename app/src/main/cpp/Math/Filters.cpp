//
// Created by alex_ on 10/20/2017.
//

#include "Filters.h"

glm::mat3 Filters::GenerateSmoothingMatrix()
{
    glm::mat3  Weight(1);

    float factor = 1.f / 9.f;

    Weight[0] = { factor, factor, factor };
    Weight[1] = { factor, factor, factor };
    Weight[2] = { factor, factor, factor };

    return Weight;
}

glm::mat3 Filters::GenerateSharpenMatrix(bool KeepBackground)
{
    glm::mat3  Weight(1.f);

    float factor = -1;

    float SharpenFactor = 8;

    if (KeepBackground)
    {
        SharpenFactor = 9;
    }


    Weight[0] = { factor, factor, factor };
    Weight[1] = { factor, SharpenFactor, factor };
    Weight[2] = { factor, factor, factor };

    return Weight;
}

glm::mat3 Filters::GenerateSharpenFactor5()
{
    glm::mat3  Weight(1.f);

    Weight[0] = { 0, -1,  0 };
    Weight[1] = { -1, 5, -1 };
    Weight[2] = { 0, -1,  0 };

    return Weight;
}

glm::mat3 Filters::GenerateTestGaussMatrix()
{
    glm::mat3  Weight(1.f);

    Weight[0] = { 0.0113f, 0.0838f, 0.0113f };
    Weight[1] = { 0.0838f, 0.6193f, 0.0838f };
    Weight[2] = { 0.0113f, 0.0838f, 0.0113f };

    return Weight;
}

glm::mat3 Filters::GenerateHighBoost(float BoostFactor)
{
    glm::mat3  Weight(1.f);

    float factor = 9 * BoostFactor - 1;
    Weight[0] = { -1.f, -1.f, -1.f };
    Weight[1] = { -1.f, factor, -1.f };
    Weight[2] = { -1.f, -1.f, -1.f };

    return Weight;
}

glm::mat3 Filters::GenerateLaplacian()
{
    glm::mat3  Weight(1.f);

    Weight[0] = { 0,  -1, 0 };
    Weight[1] = { -1, 4, -1 };
    Weight[2] = { 0,  -1, 0 };

    return Weight;
}