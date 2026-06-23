#include <concepts>
#include <array>
#include <cstdint>

namespace FIR {

template <typename T>
concept fir_value_t = requires (T value) {
  value + value;
  value += value;
  value - value;
  value -= value;
  value * value;
  value / value;
};

template <
  fir_value_t T, fir_value_t V,
  size_t ImpulseCharSize, size_t WindowSize
>
class FirWindowedRuntimeConvolve final {
public:
  void set_impulse_characteristic(
    std::array<V, ImpulseCharSize> &&new_characteristics
  ) {
    h = new_characteristics;
  }
  T step(T new_value) {
    window[counter] = new_value;
    y_out = 0;
    for (long j = counter, k = 0; (j != -1) && (k < WindowSize); j--, k++) {
      y_out += window[j] * h[k];
      if (is_window_filled) {
        for (
          long x = 1, y = WindowSize - 1;
          (x < WindowSize) && (y != -1);
          x++, y--
        ) {
          y_out += window[x] * h[y];
        }
        is_window_filled = false;
      }
    }
    if (++counter >= WindowSize) {
      counter = 0;
      is_window_filled = true;
    }
    return y_out;
  }
  T operator() (const T new_value) {
    return step(new_value);
  }
private:
  T y_out{0};
  std::array<V, WindowSize> window;
  size_t counter{0};
  bool is_window_filled{false};
  std::array<V, ImpulseCharSize> h;
};

} /* namespace FIR */
