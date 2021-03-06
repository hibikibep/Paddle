/* Copyright (c) 2018 PaddlePaddle Authors. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License. */

#include <fstream>
#include <iostream>
#include "paddle/fluid/inference/tests/api/tester_helper.h"

namespace paddle {
namespace inference {
namespace analysis {

void SetConfig(AnalysisConfig *cfg) {
  cfg->param_file = FLAGS_infer_model + "/params";
  cfg->prog_file = FLAGS_infer_model + "/model";
  cfg->use_gpu = false;
  cfg->device = 0;
  cfg->enable_ir_optim = true;
  cfg->specify_input_name = true;
}

void SetInput(std::vector<std::vector<PaddleTensor>> *inputs) {
  SetFakeImageInput(inputs, FLAGS_infer_model);
}

// Easy for profiling independently.
void profile(bool use_mkldnn = false) {
  AnalysisConfig cfg;
  SetConfig(&cfg);
  cfg._use_mkldnn = use_mkldnn;
  std::vector<PaddleTensor> outputs;

  std::vector<std::vector<PaddleTensor>> input_slots_all;
  SetInput(&input_slots_all);
  TestPrediction(cfg, input_slots_all, &outputs, FLAGS_num_threads);
}

TEST(Analyzer_resnet50, profile) { profile(); }
#ifdef PADDLE_WITH_MKLDNN
TEST(Analyzer_resnet50, profile_mkldnn) { profile(true /* use_mkldnn */); }
#endif

// Check the fuse status
TEST(Analyzer_resnet50, fuse_statis) {
  AnalysisConfig cfg;
  SetConfig(&cfg);
  int num_ops;
  auto predictor = CreatePaddlePredictor<AnalysisConfig>(cfg);
  auto fuse_statis = GetFuseStatis(
      static_cast<AnalysisPredictor *>(predictor.get()), &num_ops);
  LOG(INFO) << "num_ops: " << num_ops;
}

// Compare result of NativeConfig and AnalysisConfig
void compare(bool use_mkldnn = false) {
  AnalysisConfig cfg;
  SetConfig(&cfg);
  cfg._use_mkldnn = use_mkldnn;

  std::vector<std::vector<PaddleTensor>> input_slots_all;
  SetInput(&input_slots_all);
  CompareNativeAndAnalysis(cfg, input_slots_all);
}

TEST(Analyzer_resnet50, compare) { compare(); }
#ifdef PADDLE_WITH_MKLDNN
TEST(Analyzer_resnet50, compare_mkldnn) { compare(true /* use_mkldnn */); }
#endif

}  // namespace analysis
}  // namespace inference
}  // namespace paddle
