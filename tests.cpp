#include <iostream>
#include "digital_filters.hpp"
#include "fir.hpp"

// using namespace DigitalFilters;
using DigitalFilters::ExponentialFilter;

int main() {
  /* RunningMedianFilter<unsigned char, 3> rmf;
  
  std::cout << (int) rmf.step(9) << '\n'; // 0
  std::cout << (int) rmf.step(1) << '\n'; // 1
  std::cout << (int) rmf.step(5) << '\n'; // 5 */

  /* ExponentialFilter<float, float> ef(0.42);
  ef.step(146.0f);
  ef.step(2.71f); */

  /* Exponential2FieldsFilter<float, float> eff(0.1, 0.5, 42);
  eff(1);
  eff.step(69); */

// #define FIR_0
// // #define FIR_1

// #ifdef FIR_0
//   float data_to_filter[] = {1.793f, 6.3f, 9.432f, 5.32f, 2.923f, 6.41f, 5.01f};
//   FIR::FirWindowedRuntimeConvolve<float, float, 4, 2> fir;
//   fir.set_impulse_characteristic({0.1f, 0.05f, 0.4f, 0.8f});
// #elif defined(FIR_1)
//   float data_to_filter[] = {593.21f, 439.8f, 603.03f, 399.722f, 432.37f, 410.1f, 583.016f};
//   FIR::FirWindowedRuntimeConvolve<float, float, 6, 3> fir;
//   fir.set_impulse_characteristic({6.32f, 4.02f, 3.94f, 3.51f, 2.05f, 1.42f});
// #endif

//   for (auto &data : data_to_filter) {
//     std::cout << fir(data) << '\n';
//   }

  ExponentialFilter<float, float> e2f(-1.0f);

  return 0;
}
