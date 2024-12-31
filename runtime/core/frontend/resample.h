// Copyright [2024] <WeNet>
#ifndef KALDI_FEAT_RESAMPLE_H_
#define KALDI_FEAT_RESAMPLE_H_

#include <math.h>
#include <cassert>
#include <cstdlib>
#include <string>
#include <vector>

#include "base/kaldi-error.h"

// using namespace std;

namespace wenet {

#ifndef M_PI
#define M_PI 3.1415926535897932384626433832795
#endif

#ifndef M_2PI
#define M_2PI 6.283185307179586476925286766559005
#endif

class LinearResample {
 public:
  LinearResample(int32 samp_rate_in_hz, int32 samp_rate_out_hz,
                 float filter_cutoff_hz, int32 num_zeros);

  void Resample(const std::vector<float>& input, bool flush, std::vector<float>* output);

  void Reset();

  inline int32 GetInputSamplingRate() { return samp_rate_in_; }
  inline int32 GetOutputSamplingRate() { return samp_rate_out_; }

 private:
  int64 GetNumOutputSamples(int64 input_num_samp, bool flush) const;

  inline void GetIndexes(int64 samp_out, int64* first_samp_in,
                         int32* samp_out_wrapped) const;

  void SetRemainder(const std::vector<float>& input);

  void SetIndexesAndWeights();

  float FilterFunc(float) const;

  int32 samp_rate_in_;
  int32 samp_rate_out_;
  float filter_cutoff_;
  int32 num_zeros_;

  int32 input_samples_in_unit_;   ///< The number of input samples in the
  int32 output_samples_in_unit_;  ///< The number of output samples in the

  std::vector<int32> first_index_;

  std::vector<std::vector<float> > weights_;

  int64 input_sample_offset_;      ///< The number of input samples we have
  int64 output_sample_offset_;     ///< The number of samples we have already
  std::vector<float> input_remainder_;  ///< A small trailing part of the
};

}  // namespace wenet
#endif  // KALDI_FEAT_RESAMPLE_H_
