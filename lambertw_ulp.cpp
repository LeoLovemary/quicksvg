#include <cmath>
#include <iostream>
#include <boost/multiprecision/float128.hpp>
#include <boost/math/constants/constants.hpp>
#include <boost/math/special_functions/lambert_w.hpp>
#include "quicksvg/ulp_plot.hpp"

using boost::math::lambert_w0;
using boost::math::lambert_wm1;
using boost::math::lambert_w0_prime;
using boost::math::lambert_wm1_prime;
using boost::multiprecision::float128;
using boost::math::constants::exp_minus_one;

int main()
{
    double divider = -0.3667;
    int samples = 15000;
    double a = -exp_minus_one<double>();
    double b = divider;
    std::string title = "ULP accuracy of double precision Lambert W₀ on (-1/e, -0.3667)";
    std::cout << title << "\n";
    title = "";
    std::string filename = "examples/ulp_lambert_w0_1e_3667.svg";
    auto flo = [](double x)->double { return lambert_w0<double>(x); };
    auto fhi = [](float128 x)->float128 { return lambert_w0<float128>(x); };

    int clip = 3;
    int horizontal_lines = 5;
    int vertical_lines = 5;
    quicksvg::ulp_plot<decltype(flo), double, decltype(fhi), float128>(flo, fhi, a, b, title, filename, samples, 1100, clip, horizontal_lines, vertical_lines);


    a = divider;
    b = 0;
    title = "ULP accuracy of double precision Lambert W₀ on (-0.3667,0)";
    std::cout << title << "\n";
    filename = "examples/ulp_lambert_w0_3667_0.svg";
    quicksvg::ulp_plot<decltype(flo), double, decltype(fhi), float128>(flo, fhi, a, b, title, filename, samples);

    a = 0;
    b = 1000000;
    title = "ULP accuracy of double precision Lambert W₀ on [0, 10⁶)";
    filename = "examples/ulp_lambert_w0_0_mil.svg";
    std::cout << title << "\n";
    quicksvg::ulp_plot<decltype(flo), double, decltype(fhi), float128>(flo, fhi, a, b, title, filename, samples, 1100, -1, horizontal_lines, vertical_lines);

    a = -exp_minus_one<double>();
    b = divider;
    title = "ULP accuracy of double precision Lambert W₋₁ on [-1/e, -0.3667)";
    filename = "examples/ulp_lambert_wm1_1e_3667.svg";
    std::cout << title << "\n";
    quicksvg::ulp_plot<decltype(flo), double, decltype(fhi), float128>(flo, fhi, a, b, title, filename, samples);

    a = divider;
    b = 0;
    title = "ULP accuracy of double precision Lambert W₋₁ on [-0.3667, 0)";
    filename = "examples/ulp_lambert_wm1_3667_0.svg";
    std::cout << title << "\n";
    quicksvg::ulp_plot<decltype(flo), double, decltype(fhi), float128>(flo, fhi, a, b, title, filename, samples);

    a = -exp_minus_one<double>();
    b = divider;
    title = "ULP accuracy of double precision Lambert W₀\u2032 on [-1/e, -0.3667)";
    filename = "examples/ulp_lambert_w0_prime_1e_3667.svg";
    std::cout << title << "\n";
    quicksvg::ulp_plot<decltype(flo), double, decltype(fhi), float128>(flo, fhi, a, b, title, filename, samples);

    a = divider;
    b = 0;
    title = "ULP accuracy of double precision Lambert W₀\u2032 on [-0.3667, 0)";
    filename = "examples/ulp_lambert_w0_prime_3667_0.svg";
    std::cout << title << "\n";
    quicksvg::ulp_plot<decltype(flo), double, decltype(fhi), float128>(flo, fhi, a, b, title, filename, samples);


    a = 0;
    b = 1000000;
    title = "ULP accuracy of double precision Lambert W₀\u2032 on [0, 10⁶)";
    filename = "examples/ulp_lambert_w0_prime_0_mil.svg";
    std::cout << title << "\n";
    quicksvg::ulp_plot<decltype(flo), double, decltype(fhi), float128>(flo, fhi, a, b, title, filename, samples);

    a = -exp_minus_one<double>();
    b = divider;
    title = "ULP accuracy of double precision Lambert W₋₁\u2032 on [-1/e, -0.3667)";
    filename = "examples/ulp_lambert_wm1_prime_1e_3667.svg";
    std::cout << title << "\n";
    quicksvg::ulp_plot<decltype(flo), double, decltype(fhi), float128>(flo, fhi, a, b, title, filename, samples);

    a = divider;
    b = 0;
    title = "ULP accuracy of double precision Lambert W₋₁\u2032 on [-0.3667, 0)";
    filename = "examples/ulp_lambert_wm1_prime_3667_0.svg";
    std::cout << title << "\n";
    quicksvg::ulp_plot<decltype(flo), double, decltype(fhi), float128>(flo, fhi, a, b, title, filename, samples);

}
