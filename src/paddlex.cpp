//   Copyright (c) 2020 PaddlePaddle Authors. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "include/paddlex/paddlex.h"

using namespace InferenceEngine;

namespace PaddleX {

void Model::create_predictor(const std::string& model_dir,
                            const std::string& cfg_dir,
                            std::string device) {
    Core ie;
    network_ = ie.ReadNetwork(model_dir, model_dir.substr(0, model_dir.size() - 4) + ".bin");
    network_.setBatchSize(1);
    InputInfo::Ptr input_info = network_.getInputsInfo().begin()->second;

    input_info->getPreProcess().setResizeAlgorithm(RESIZE_BILINEAR);
    input_info->setLayout(Layout::NCHW);
    input_info->setPrecision(Precision::FP32);
    executable_network_ = ie.LoadNetwork(network_, device);
    load_config(cfg_dir);
}

bool Model::load_config(const std::string& cfg_dir) {
  YAML::Node config = YAML::LoadFile(cfg_dir);
  type = config["_Attributes"]["model_type"].as<std::string>();
  name = config["Model"].as<std::string>();
  bool to_rgb = true;
  if (config["TransformsMode"].IsDefined()) {
    std::string mode = config["TransformsMode"].as<std::string>();
    if (mode == "BGR") {
      to_rgb = false;
    } else if (mode != "RGB") {
      std::cerr << "[Init] Only 'RGB' or 'BGR' is supported for TransformsMode"
                << std::endl;
      return false;
    }
  }
  // 构建数据处理流
  transforms_.Init(config["Transforms"], to_rgb);
  // 读入label lis
  for (const auto& item : config["_Attributes"]["labels"]) {
    int index = labels.size();
    labels[index] = item.as<std::string>();
  }

  return true;
}

bool Model::preprocess(cv::Mat* input_im, ImageBlob* inputs) {
  if (!transforms_.Run(input_im, inputs)) {
    return false;
  }
  return true;
}

bool Model::predict(const cv::Mat& im, ClsResult* result, double &total_time, int count_num) {
  inputs_.clear();
  if (type == "detector") {
    std::cerr << "Loading model is a 'detector', DetResult should be passed to "
                 "function predict()!"
              << std::endl;
    return false;
  } else if (type == "segmenter") {
    std::cerr << "Loading model is a 'segmenter', SegResult should be passed "
                 "to function predict()!"
              << std::endl;
    return false;
  }
  // 处理输入图像
  InferRequest infer_request = executable_network_.CreateInferRequest();
  std::string input_name = network_.getInputsInfo().begin()->first;
  inputs_.blob = infer_request.GetBlob(input_name);
  cv::Mat im_clone = im.clone();
  if (!preprocess(&im_clone, &inputs_)) {
    std::cerr << "Preprocess failed!" << std::endl;
    return false;
  }
  
  std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
  infer_request.Infer();
  std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
  std::chrono::duration<double> time_used = std::chrono::duration_cast<std::chrono::duration<double>>(end-start);
  std::cout << "time use:" << time_used.count() << "s" << std::endl;
  if (count_num >= 20){
    total_time = total_time + time_used.count();
  }  

  std::string output_name = network_.getOutputsInfo().begin()->first;
  output_ = infer_request.GetBlob(output_name);
  MemoryBlob::CPtr moutput = as<MemoryBlob>(output_);
  auto moutputHolder = moutput->rmap();
  float* outputs_data = moutputHolder.as<float *>();

  // 对模型输出结果进行后处理
  auto ptr = std::max_element(outputs_data, outputs_data+sizeof(outputs_data));
  result->category_id = std::distance(outputs_data, ptr);
  result->score = *ptr;
  result->category = labels[result->category_id];
  //for (int i=0;i<sizeof(outputs_data);i++){
  //    std::cout <<  labels[i] << std::endl;
  //    std::cout <<  outputs_[i] << std::endl;
  //    }
}

/*bool Model::predict(const std::vector<cv::Mat>& im_batch,
                    std::vector<SegResult>* result,
                    int thread_num) {


  if (type == "classifier") {
    std::cerr << "Loading model is a 'classifier', ClsResult should be passed "
                 "to function predict()!" << std::endl;
    return false;
  } else if (type == "detector") {
    std::cerr << "Loading model is a 'detector', DetResult should be passed to "
                 "function predict()!" << std::endl;
    return false;
  }

*/

bool Model::predict(const cv::Mat& im, SegResult* result) {
  result->clear();
  inputs_.clear();
  if (type == "classifier") {
    std::cerr << "Loading model is a 'classifier', ClsResult should be passed "
                 "to function predict()!" << std::endl;
    return false;
  } else if (type == "detector") {
    std::cerr << "Loading model is a 'detector', DetResult should be passed to "
                 "function predict()!" << std::endl;
    return false;
  }
  //
  InferRequest infer_request = executable_network_.CreateInferRequest();
  std::string input_name = network_.getInputsInfo().begin()->first;
  inputs_.blob = infer_request.GetBlob(input_name);
  
  //
  cv::Mat im_clone = im.clone();
  if (!preprocess(&im_clone, &inputs_)) {
    std::cerr << "Preprocess failed!" << std::endl;
    return false;
  }

  //
  infer_request.Infer();
   
  std::string output_name_label = network_.getOutputsInfo().begin()->first;
  Blob::Ptr output_label = infer_request.GetBlob(output_name_label);
  MemoryBlob::CPtr moutput_label = as<MemoryBlob>(output_label);
  TensorDesc blob_label = moutput_label->getTensorDesc();
  std::vector<size_t> output_label_shape = blob_label.getDims();
  int size = 1;
  for (auto& i : output_label_shape) {
    size *= static_cast<int>(i);
    result->label_map.shape.push_back(static_cast<int>(i));
  }
  result->label_map.data.resize(size);
  auto moutputHolder_label = moutput_label->rmap();
  int64_t* label_data = moutputHolder_label.as<int64_t *>();
  memcpy(result->label_map.data.data(),label_data,moutput_label->byteSize());
  
  std::string output_name_score = network_.getOutputsInfo().end()->first;
  Blob::Ptr output_score = infer_request.GetBlob(output_name_score);
  MemoryBlob::CPtr moutput_score = as<MemoryBlob>(output_score);
  TensorDesc blob_score = moutput_score->getTensorDesc();
  std::vector<size_t> output_score_shape = blob_score.getDims();
  size = 1;
  for (auto& i : output_score_shape) {
    size *= static_cast<int>(i);
    result->score_map.shape.push_back(static_cast<int>(i));
  }
  result->score_map.data.resize(size);
  auto moutputHolder_score = moutput_score->rmap();
  float* score_data = moutputHolder_score.as<float *>();
  memcpy(result->score_map.data.data(),score_data,moutput_score->byteSize());

  std::vector<uint8_t> label_map(result->label_map.data.begin(),
                                 result->label_map.data.end());
  cv::Mat mask_label(result->label_map.shape[1],
                     result->label_map.shape[2],
                     CV_8UC1,
                     label_map.data());

  cv::Mat mask_score(result->score_map.shape[2],
                     result->score_map.shape[3],
                     CV_32FC1,
                     result->score_map.data.data());
  int idx = 1;
  int len_postprocess = inputs_.im_size_before_resize_.size();
  for (std::vector<std::string>::reverse_iterator iter =
           inputs_.reshape_order_.rbegin();
       iter != inputs_.reshape_order_.rend();
       ++iter) {
    if (*iter == "padding") {
      auto before_shape = inputs_.im_size_before_resize_[len_postprocess - idx];
      inputs_.im_size_before_resize_.pop_back();
      auto padding_w = before_shape[0];
      auto padding_h = before_shape[1];
      mask_label = mask_label(cv::Rect(0, 0, padding_h, padding_w));
      mask_score = mask_score(cv::Rect(0, 0, padding_h, padding_w));
    } else if (*iter == "resize") {
      auto before_shape = inputs_.im_size_before_resize_[len_postprocess - idx];
      inputs_.im_size_before_resize_.pop_back();
      auto resize_w = before_shape[0];
      auto resize_h = before_shape[1];
      cv::resize(mask_label,
                 mask_label,
                 cv::Size(resize_h, resize_w),
                 0,
                 0,
                 cv::INTER_NEAREST);
      cv::resize(mask_score,
                 mask_score,
                 cv::Size(resize_h, resize_w),
                 0,
                 0,
                 cv::INTER_LINEAR);
    }
    ++idx;
  }
  result->label_map.data.assign(mask_label.begin<uint8_t>(),
                                mask_label.end<uint8_t>());
  result->label_map.shape = {mask_label.rows, mask_label.cols};
  result->score_map.data.assign(mask_score.begin<float>(),
                                mask_score.end<float>());
  result->score_map.shape = {mask_score.rows, mask_score.cols};
  return true;
}
  
}  // namespce of PaddleX
