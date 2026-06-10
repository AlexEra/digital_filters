#include <iostream>
#include "digital_filters.hpp"

int main() {
  /* DigitalFilters::RunningMedianFilter<unsigned char, 3> rmf;
  
  std::cout << (int) rmf.step(9) << '\n'; // 0
  std::cout << (int) rmf.step(1) << '\n'; // 1
  std::cout << (int) rmf.step(5) << '\n'; // 5 */

  DigitalFilters::ExponentialFilter<float, float> ef(0.42);
  ef.step(146.0f);
  ef.step(2.71f);
  return 0;
}
