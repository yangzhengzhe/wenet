// Copyright (c) 2017 Personal (Binbin Zhang)
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "frontend/feature_pipeline.h"

#include <algorithm>
#include <utility>

namespace wenet {

FeaturePipeline::FeaturePipeline(const FeaturePipelineConfig& config)
    : config_(config),
      feature_dim_(config.num_bins),
      fbank_(config.num_bins, config.sample_rate, config.frame_length,
             config.frame_shift, config.low_freq, config.pre_emphasis,
             config.scale_input_to_unit, config.log_floor, config.log_base,
             config.window_type, config.mel_type, config.norm_type),
      num_frames_(0),
      input_finished_(false) {}

void FeaturePipeline::AcceptWaveform(const float* pcm, const int size) {
  std::vector<std::vector<float>> feats;
  std::vector<float> waves;
  waves.insert(waves.end(), remained_wav_.begin(), remained_wav_.end());
  waves.insert(waves.end(), pcm, pcm + size);
  int num_frames = fbank_.Compute(waves, &feats);
  feature_queue_.Push(std::move(feats));
  num_frames_ += num_frames;

  int left_samples = waves.size() - config_.frame_shift * num_frames;
  remained_wav_.resize(left_samples);
  std::copy(waves.begin() + config_.frame_shift * num_frames, waves.end(),
            remained_wav_.begin());
  // We are still adding wave, notify input is not finished
  finish_condition_.notify_one();
}

void FeaturePipeline::AcceptWaveform(const int16_t* pcm, const int size) {
  auto* float_pcm = new float[size];
  for (size_t i = 0; i < size; i++) {
    float_pcm[i] = static_cast<float>(pcm[i]);
  }
  this->AcceptWaveform(float_pcm, size);
  delete[] float_pcm;
}

void FeaturePipeline::AcceptWaveform(const int16_t* pcm, const int size,
                                     const int sample_rate) {
  std::vector<float> ori_wav(size);
  std::transform(pcm, pcm + size, ori_wav.begin(),
                 [](int16_t sample) { return static_cast<float>(sample); });

  const float* wav = nullptr;
  int wav_size = 0;
  std::vector<float> resampled_wav;

  MaybeCreateResampler(static_cast<float>(sample_rate));
  if (resampler_ == nullptr) {
    wav = ori_wav.data();
    wav_size = ori_wav.size();
  } else {
    resampler_->Resample(ori_wav, false, &resampled_wav);
    if (!resampled_wav.empty()) {
      wav = resampled_wav.data();
      wav_size = resampled_wav.size();
    } else {
      throw std::runtime_error("Resampling failed: resampled_wav is empty.");
    }
  }
  std::cout << "Output wav info:" << std::endl;
  std::cout << "in size:" << size << " out size:" << wav_size << std::endl;
  this->AcceptWaveform(wav, wav_size);
}

void FeaturePipeline::set_input_finished() {
  CHECK(!input_finished_);
  {
    std::lock_guard<std::mutex> lock(mutex_);
    input_finished_ = true;
  }
  finish_condition_.notify_one();
}

bool FeaturePipeline::ReadOne(std::vector<float>* feat) {
  if (!feature_queue_.Empty()) {
    *feat = std::move(feature_queue_.Pop());
    return true;
  } else {
    std::unique_lock<std::mutex> lock(mutex_);
    while (!input_finished_) {
      // This will release the lock and wait for notify_one()
      // from AcceptWaveform() or set_input_finished()
      finish_condition_.wait(lock);
      if (!feature_queue_.Empty()) {
        *feat = std::move(feature_queue_.Pop());
        return true;
      }
    }
    CHECK(input_finished_);
    // Double check queue.empty, see issue#893 for detailed discussions.
    if (!feature_queue_.Empty()) {
      *feat = std::move(feature_queue_.Pop());
      return true;
    } else {
      return false;
    }
  }
}

bool FeaturePipeline::Read(int num_frames,
                           std::vector<std::vector<float>>* feats) {
  feats->clear();
  if (feature_queue_.Size() >= num_frames) {
    *feats = std::move(feature_queue_.Pop(num_frames));
    return true;
  } else {
    std::unique_lock<std::mutex> lock(mutex_);
    while (!input_finished_) {
      // This will release the lock and wait for notify_one()
      // from AcceptWaveform() or set_input_finished()
      finish_condition_.wait(lock);
      if (feature_queue_.Size() >= num_frames) {
        *feats = std::move(feature_queue_.Pop(num_frames));
        return true;
      }
    }
    CHECK(input_finished_);
    // Double check queue.empty, see issue#893 for detailed discussions.
    if (feature_queue_.Size() >= num_frames) {
      *feats = std::move(feature_queue_.Pop(num_frames));
      return true;
    } else {
      *feats = std::move(feature_queue_.Pop(feature_queue_.Size()));
      return false;
    }
  }
}

void FeaturePipeline::Reset() {
  if (resampler_ != nullptr) resampler_->Reset();
  input_finished_ = false;
  num_frames_ = 0;
  remained_wav_.clear();
  feature_queue_.Clear();
}

void FeaturePipeline::MaybeCreateResampler(float sample_rate) {
  float expected_sample_rate = config_.sample_rate;
  if (resampler_ != nullptr) {
    CHECK_EQ(resampler_->GetInputSamplingRate(), sample_rate);
    CHECK_EQ(resampler_->GetOutputSamplingRate(), expected_sample_rate);
  } else if (sample_rate != expected_sample_rate) {
    resampler_.reset(
        new LinearResample(sample_rate, expected_sample_rate, 3800, 64));
    LOG(INFO) << "Convert sample rate from: " << sample_rate << " to "
              << expected_sample_rate;
  }
}

}  // namespace wenet
