#pragma once

#include <stdint.h>
#include <algorithm>
#include <array>
#include <cmath>
#include <concepts>


namespace DigitalFilters {

// TODO: add exponential filters

template <typename T>
concept vals_to_filter = requires (T value) {
  value + value;
  value += value;
  value - value;
  value -= value;
  value * value;
  value / value;
};

template<vals_to_filter T, size_t N>
class RunningAverageFilter final {
public:
  T operator() (const T new_value) {
    return step(new_value);
  }
  void start(void) {
    index = 0;
    average = 0;
    for (auto &v: vals) { v = 0; }
  }
  T step(const T new_data) {
    if (++index >= N) {
      index = 0;
    }
    average -= vals[index];
    average += new_data;
    vals[index] = new_data;
    return (average / N);
  }
private:
  T average{0};
  size_t index{0};
  std::array<T, N> vals;
};

template<vals_to_filter T_in, vals_to_filter T_out, size_t N>
class RunningSlidingAvg {
public:
  // RunningSlidingAvg() { };
  RunningSlidingAvg(float init_value = 0): y_out{init_value} { };

  T_out operator() (T_in new_value) {
    return step(new_value);
  }

  void start(void) {
    y_out = 0;
    y_prev = 0;
    idx = 0;
    start_sliding = false;
  }

  T_out step(T_in new_data) {
    static_assert(N); // to prevent zero dividing
    y_out = (float)new_data / N;
    if ((idx >= N) && !start_sliding) {
      start_sliding = true;
    }
    y_out += y_prev;
    if (start_sliding) {
      y_out -= y_prev / N;
    }
    y_prev = y_out;
    if (!start_sliding) {
      idx++;
    }
    return y_out;
  }

  void reset(void) {
    y_out = y_prev = 0;
    idx = 0;
    start_sliding = false;
  }

private:
  T_out y_out{0};
  T_out y_prev{0};
  size_t idx{0};
  volatile bool start_sliding{false};
};

template<vals_to_filter T, size_t N>
class MedianFilter {
public:
  T operator() (const T new_value) {
    return step(new_value);
  }
  void start(void) {
    index = 0;
    for (auto &v: values) { v = 0; }
  }
  virtual T step(const T new_value) {
    values[index++] = new_value;
    if (index >= N) {
      index = 0;
      std::sort(
        values.begin(), values.end(),
        [] (T first, T second) {
          return first > second;
        }
      );
      out = values[N >> 1];
    }
    return out;
  }
protected:
  T out{0};
  uint8_t index{0};
  std::array<T, N> values;
};

template<vals_to_filter T, size_t N>
class RunningMedianFilter : public MedianFilter<T, N> {
public:
  T step(const T new_value) {
    this->values[this->index++] = new_value;
    if (this->index >= N) {
      this->index = 0;
    }
    std::sort(
      this->values.begin(), this->values.end(),
      [] (T first, T second) {
        return first > second;
      }
    );
    return this->values[N >> 1];
  }
};

template<vals_to_filter ValueType, vals_to_filter CoefficientsType>
class SimpleKalmanFilter final {
public:
  SimpleKalmanFilter() { };
  SimpleKalmanFilter(
    CoefficientsType noise,
    CoefficientsType change_speed
  ) :
    err_measure{noise},
    q{change_speed},
    err_estimate{err_measure}
    { };
  ValueType operator() (ValueType new_value) {
    return step(new_value);
  }
  void start(void) {
    last_estimate = 0;
  }
  void set_coefficients(
    CoefficientsType noise,
    CoefficientsType change_speed
  ) {
    err_measure = noise;
    q = change_speed;
    err_estimate = err_measure;
  }
  ValueType step(ValueType new_value) {
    gain = err_estimate / (err_estimate + err_measure);
    current_estimate = last_estimate + 
      gain * ((CoefficientsType) new_value - last_estimate);
    err_estimate = (1.0 - gain) * err_estimate
      + fabs(last_estimate - current_estimate) * q;
    last_estimate = current_estimate;
    return (ValueType) current_estimate;
  }
private:
  CoefficientsType err_measure{0};
  CoefficientsType q{0};
  CoefficientsType err_estimate{0};
  CoefficientsType last_estimate{0};
  CoefficientsType gain{0}, current_estimate{0};
};

} // namespace DigitalFilters
