// Copyright [2024] <WeNet>
#include <algorithm>
#include <limits>

#include "frontend/resample.h"

namespace wenet {

float VecVec(const vector<float>& ra, const vector<float>& rb) {
  int adim = ra.size();
  assert(adim == rb.size());
  const float* a_data = &ra[0];
  const float* b_data = &rb[0];
  float sum = 0.0;
  for (int i = 0; i < adim; i++) sum += a_data[i] * b_data[i];
  return sum;
}

template <class I>
I Gcd(I m, I n) {
  if (m == 0 || n == 0) {
    assert(!(m == 0 && n == 0));
    return (m == 0 ? (n > 0 ? n : -n) : (m > 0 ? m : -m));
  }
  assert(std::numeric_limits<I>::is_integer);
  while (1) {
    m %= n;
    if (m == 0) return (n > 0 ? n : -n);
    n %= m;
    if (n == 0) return (m > 0 ? m : -m);
  }
}

template <class I>
I Lcm(I m, I n) {
  assert(m > 0 && n > 0);
  I gcd = Gcd(m, n);
  return gcd * (m / gcd) * (n / gcd);
}

LinearResample::LinearResample(int32 samp_rate_in_hz, int32 samp_rate_out_hz,
                               float filter_cutoff_hz, int32 num_zeros)
    : samp_rate_in_(samp_rate_in_hz),
      samp_rate_out_(samp_rate_out_hz),
      filter_cutoff_(filter_cutoff_hz),
      num_zeros_(num_zeros) {
  assert(samp_rate_in_hz > 0.0 && samp_rate_out_hz > 0.0 &&
         filter_cutoff_hz > 0.0 && filter_cutoff_hz * 2 <= samp_rate_in_hz &&
         filter_cutoff_hz * 2 <= samp_rate_out_hz && num_zeros > 0);

  // base_freq is the frequency of the repeating unit, which is the gcd
  // of the input frequencies.
  int32 base_freq = Gcd(samp_rate_in_, samp_rate_out_);
  input_samples_in_unit_ = samp_rate_in_ / base_freq;
  output_samples_in_unit_ = samp_rate_out_ / base_freq;

  SetIndexesAndWeights();
  Reset();
}

int64 LinearResample::GetNumOutputSamples(int64 input_num_samp,
                                          bool flush) const {
  int32 tick_freq = Lcm(samp_rate_in_, samp_rate_out_);
  int32 ticks_per_input_period = tick_freq / samp_rate_in_;

  int64 interval_length_in_ticks = input_num_samp * ticks_per_input_period;
  if (!flush) {
    float window_width = num_zeros_ / (2.0 * filter_cutoff_);
    int32 window_width_ticks = floor(window_width * tick_freq);
    interval_length_in_ticks -= window_width_ticks;
  }
  if (interval_length_in_ticks <= 0) return 0;
  int32 ticks_per_output_period = tick_freq / samp_rate_out_;
  int64 last_output_samp = interval_length_in_ticks / ticks_per_output_period;
  if (last_output_samp * ticks_per_output_period == interval_length_in_ticks)
    last_output_samp--;
  int64 num_output_samp = last_output_samp + 1;
  return num_output_samp;
}

void LinearResample::SetIndexesAndWeights() {
  first_index_.resize(output_samples_in_unit_);
  weights_.resize(output_samples_in_unit_);

  double window_width = num_zeros_ / (2.0 * filter_cutoff_);

  for (int32 i = 0; i < output_samples_in_unit_; i++) {
    double output_t = i / static_cast<double>(samp_rate_out_);
    double min_t = output_t - window_width, max_t = output_t + window_width;
    int32 min_input_index = ceil(min_t * samp_rate_in_),
          max_input_index = floor(max_t * samp_rate_in_),
          num_indices = max_input_index - min_input_index + 1;
    first_index_[i] = min_input_index;
    weights_[i].resize(num_indices);
    for (int32 j = 0; j < num_indices; j++) {
      int32 input_index = min_input_index + j;
      double input_t = input_index / static_cast<double>(samp_rate_in_),
             delta_t = input_t - output_t;
      weights_[i][j] = FilterFunc(delta_t) / samp_rate_in_;
    }
  }
}

void LinearResample::GetIndexes(int64 samp_out, int64* first_samp_in,
                                int32* samp_out_wrapped) const {
  int64 unit_index = samp_out / output_samples_in_unit_;
  *samp_out_wrapped =
      static_cast<int32>(samp_out - unit_index * output_samples_in_unit_);
  *first_samp_in =
      first_index_[*samp_out_wrapped] + unit_index * input_samples_in_unit_;
}

void LinearResample::Resample(const vector<float>& input, bool flush,
                              vector<float>* output) {
  int32 input_dim = input.size();
  int64 tot_input_samp = input_sample_offset_ + input_dim,
        tot_output_samp = GetNumOutputSamples(tot_input_samp, flush);

  assert(tot_output_samp >= output_sample_offset_);

  output->resize(tot_output_samp - output_sample_offset_);

  for (int64 samp_out = output_sample_offset_; samp_out < tot_output_samp;
       samp_out++) {
    int64 first_samp_in;
    int32 samp_out_wrapped;
    GetIndexes(samp_out, &first_samp_in, &samp_out_wrapped);
    const vector<float>& weights = weights_[samp_out_wrapped];
    int32 first_input_index =
        static_cast<int32>(first_samp_in - input_sample_offset_);
    float this_output;
    if (first_input_index >= 0 &&
        first_input_index + weights.size() <= input_dim) {
      // Subvector<float> input_part(input, first_input_index, weights.size());
      vector<float> input_part(
          input.begin() + first_input_index,
          input.begin() + first_input_index + weights.size());
      this_output = VecVec(input_part, weights);
    } else {  // Handle edge cases.
      this_output = 0.0;
      for (int32 i = 0; i < weights.size(); i++) {
        float weight = weights[i];
        int32 input_index = first_input_index + i;
        int32 input_index1 = input_remainder_.size() + input_index;
        if (input_index < 0 && input_index1 >= 0) {
          this_output += weight * input_remainder_[input_index1];
        } else if (input_index >= 0 && input_index < input_dim) {
          this_output += weight * input[input_index];
        } else if (input_index >= input_dim) {
          assert(flush);
        }
      }
    }
    int32 output_index = static_cast<int32>(samp_out - output_sample_offset_);
    (*output)[output_index] = this_output;
  }

  if (flush) {
    Reset();  // Reset the internal state.
  } else {
    SetRemainder(input);
    input_sample_offset_ = tot_input_samp;
    output_sample_offset_ = tot_output_samp;
  }
}

void LinearResample::SetRemainder(const vector<float>& input) {
  vector<float> old_remainder(input_remainder_);
  int32 max_remainder_needed =
      ceil(samp_rate_in_ * num_zeros_ / filter_cutoff_);
  input_remainder_.resize(max_remainder_needed);
  for (int32 index = -input_remainder_.size(); index < 0; index++) {
    int32 input_index = index + input.size();
    if (input_index >= 0)
      input_remainder_[index + input_remainder_.size()] = input[input_index];
    else if (input_index + old_remainder.size() >= 0)
      input_remainder_[index + input_remainder_.size()] =
          old_remainder[input_index + old_remainder.size()];
  }
}

void LinearResample::Reset() {
  input_sample_offset_ = 0;
  output_sample_offset_ = 0;
  input_remainder_.resize(0);
}

float LinearResample::FilterFunc(float t) const {
  float window,  // raised-cosine (Hanning) window of width
      filter;    // sinc filter function
  if (fabs(t) < num_zeros_ / (2.0 * filter_cutoff_))
    window = 0.5 * (1 + cos(M_2PI * filter_cutoff_ / num_zeros_ * t));
  else
    window = 0.0;  // outside support of window function
  if (t != 0)
    filter = sin(M_2PI * filter_cutoff_ * t) / (M_PI * t);
  else
    filter = 2 * filter_cutoff_;  // limit of the function at t = 0
  return filter * window;
}

}  // namespace wenet
