#pragma once
namespace DSP
{
// 4-point, 3rd-order B-spline (z-form)
float inline bspline_43z(const float* y, const float x)
{
    auto z = x - 1.f / 2.f;
    auto even1 = y[3] + y[0];
    auto odd1 = y[3] - y[0];
    auto even2 = y[2] + y[1];
    auto odd2 = y[2] - y[1];
    auto c0 = 1.f / 48.f * even1 + 23.f / 48.f * even2;
    auto c1 = 1.f / 8.f * odd1 + 5.f / 8.f * odd2;
    auto c2 = 1.f / 4.f * (even1 - even2);
    auto c3 = 1.f / 6.f * odd1 - 1.f / 2.f * odd2;
    return ((c3 * z + c2) * z + c1) * z + c0;
}

// 4-point, 3rd-order B-spline (x-form)
float inline bspline_43x(const float* y, const float x)
{
    auto ym1py1 = y[0] + y[2];
    auto c0 = 1.f / 6.f * ym1py1 + 2.f / 3.f * y[1];
    auto c1 = 1.f / 2.f * (y[2] - y[0]);
    auto c2 = 1.f / 2.f * ym1py1 - y[1];
    auto c3 = 1.f / 2.f * (y[1] - y[2]) + 1.f / 6.f * (y[3] - y[0]);
    return ((c3 * x + c2) * x + c1) * x + c0;
}

}
