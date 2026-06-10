#include <iostream>
#include "digital_filters.hpp"
#include "fir.hpp"

using namespace DigitalFilters;

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

  FIR::FirWindowedRuntimeConvolve<float, float, 64, 8> fir;

  return 0;
}
