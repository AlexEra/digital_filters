#pragma once

#include <stdint.h>
#include <algorithm>
#include <array>
#include <cmath>
#include <concepts>


namespace DigitalFilters {

template <typename T>
concept vals_to_filter = requires (T value) {
  value + value;
  value += value;
  value - value;
  value -= value;
  value * value;
  value / value;
};

template <typename L>
concept expo_coef_type = std::is_same<L, float>::value || std::is_same<L, double>::value;

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
  RunningSlidingAvg(T_out init_value = 0) : y_out{init_value} { } // ctor
  RunningSlidingAvg(const RunningSlidingAvg& other) : // copy ctor
    y_out{other.y_out}, y_prev{other.y_prev},
    idx{other.idx}, start_sliding{other.start_sliding} { }
  RunningSlidingAvg(const RunningSlidingAvg&& other) : // move ctor
    y_out{other.y_out}, y_prev{other.y_prev},
    idx{other.idx}, start_sliding{other.start_sliding} { }
  RunningSlidingAvg& operator=(const RunningSlidingAvg& other) { // copy operator
    if (&other != this) {
      y_out = other.y_out;
      y_prev = other.y_prev;
      idx = other.idx;
      start_sliding = other.start_sliding;
    }
    return *this;
  }
  RunningSlidingAvg& operator=(const RunningSlidingAvg&& other) { // move operator
    if (&other != this) {
      y_out = other.y_out;
      y_prev = other.y_prev;
      idx = other.idx;
      start_sliding = other.start_sliding;
    }
    return *this;
  }
  ~RunningSlidingAvg() { } // dtor

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
  T_out y_out;
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
  T step(const T new_value) {
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

template <vals_to_filter T, expo_coef_type L>
class ExponentialFilter final {
public:
  ExponentialFilter(L alpha_coefficient = 0) : // ctor
  alpha{((alpha_coefficient > 1) || (alpha_coefficient < 0)) ? 0 : alpha_coefficient},
  alpha_inv{((alpha_coefficient > 1) || (alpha_coefficient < 0)) ? 1 : alpha_coefficient} { }
  ExponentialFilter(const ExponentialFilter& other) :
  alpha{other.alpha}, alpha_inv{other.alpha_inv} { } // copy ctor
  ExponentialFilter(const ExponentialFilter&& other) :
  alpha{other.alpha}, alpha_inv{other.alpha_inv} { } // move ctor
  ExponentialFilter& operator=(const ExponentialFilter& other) { // copy operator
    if (&other != this) {
      alpha = other.alpha;
      alpha_inv = other.alpha_inv;
    }
    return *this;
  }
  ExponentialFilter& operator=(const ExponentialFilter&& other) { // move operator
    if (&other != this) {
      alpha = other.alpha;
      alpha_inv = other.alpha_inv;
    }
    return *this;
  }
  ~ExponentialFilter() {} // dtor
  bool set_alpha(L new_alpha) {
    if ((new_alpha < 0) || (new_alpha > 1)) {
      return false;
    }
    alpha = new_alpha;
    alpha_inv = 1 - alpha;
    return true;
  }
  T step(T new_value) {
    y_prev = new_value * alpha + (1 - alpha) * y_prev;
    return y_prev;
  }
  T operator() (const T new_value) {
    return step(new_value);
  }

private:
  L alpha;
  L alpha_inv;
  T y_prev{0};
};

template <vals_to_filter T, expo_coef_type L>
class Exponential2FieldsFilter final {
public:
  Exponential2FieldsFilter(L k_0 = 0, L k_1 = 1, L sharpness = 0.0) :
  k_0{k_0}, k_1{k_1}, sharpness{fabs(sharpness)} { } // ctor
  Exponential2FieldsFilter(const Exponential2FieldsFilter& other) :
  k_0{other.k_0}, k_1{other.k_1}, sharpness{other.sharpness} { } // copy ctor
  Exponential2FieldsFilter(const Exponential2FieldsFilter&& other) :
  k_0{other.k_0}, k_1{other.k_1}, sharpness{other.sharpness} { } // move ctor
  Exponential2FieldsFilter& operator=(const Exponential2FieldsFilter& other) { // copy operator
    if (&other != this) {
      sharpness = other.sharpness;
      k_0 = other.k_0;
      k_1 = other.k_1;
    }
    return *this;
  }
  Exponential2FieldsFilter& operator=(const Exponential2FieldsFilter&& other) { // move operator
    if (&other != this) {
      sharpness = other.sharpness;
      k_0 = other.k_0;
      k_1 = other.k_1;
    }
    return *this;
  }
  void set_alpha(L new_k_0, L new_k_1) {
    k_0 = new_k_0;
    k_1 = new_k_1;
  }
  void set_sharpness(L new_sharpness) {
    sharpness = fabs(new_sharpness);
  }
  T step(T new_value) {
    T delta = new_value - last_value;
    last_value += (fabs(delta) <= sharpness) ? k_0 * delta : k_1 * delta;
    return last_value;
  }
  void reset(void) { last_value = 0; }
  T operator() (const T new_value) {
    return step(new_value);
  }

private:
  L sharpness;
  L k_0;
  L k_1;
  T last_value{0};
};

template<vals_to_filter ValueType, vals_to_filter CoefficientsType>
class SimpleKalmanFilter final {
public:
  SimpleKalmanFilter(
    CoefficientsType noise = 0,
    CoefficientsType change_speed = 0
  ) :
    err_measure{noise},
    q{change_speed},
    err_estimate{err_measure} { } // ctor
  SimpleKalmanFilter(const SimpleKalmanFilter& other) :
  err_measure{other.err_measure}, q{other.q},
  err_estimate{other.err_estimate},
  last_estimate{other.last_estimate},
  gain{other.gain}, current_estimate{other.current_estimate} { } // copy ctor
  SimpleKalmanFilter(const SimpleKalmanFilter&& other) :
  err_measure{other.err_measure}, q{other.q},
  err_estimate{other.err_estimate},
  last_estimate{other.last_estimate},
  gain{other.gain}, current_estimate{other.current_estimate} { } // move ctor
  SimpleKalmanFilter& operator=(const SimpleKalmanFilter& other) { // copy operator
    if (&other != this) {
      err_measure = other.err_measure;
      q = other.q;
      err_estimate = other.err_estimate;
      last_estimate = other.last_estimate;
      gain = other.gain;
      current_estimate = other.current_estimate;
    }
    return *this;
  }
  SimpleKalmanFilter& operator=(const SimpleKalmanFilter&& other) { // move operator
    if (&other != this) {
      err_measure = other.err_measure;
      q = other.q;
      err_estimate = other.err_estimate;
      last_estimate = other.last_estimate;
      gain = other.gain;
      current_estimate = other.current_estimate;
    }
    return *this;
  }
  ~SimpleKalmanFilter() {} // dtor

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
  CoefficientsType err_measure, q, err_estimate,
    last_estimate{0}, gain{0}, current_estimate{0};
};

} // namespace DigitalFilters
