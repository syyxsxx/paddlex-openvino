#include <glog/logging.h>
#include <omp.h>

#include <algorithm>
#include <chrono>  // NOLINT
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <utility>

#include "include/paddlex/paddlex.h"
#include "include/paddlex/visualize.h"

using namespace std::chrono;  // NOLINT

DEFINE_string(model_dir, "", "Path of openvino model xml file");
DEFINE_string(cfg_dir, "", "Path of PaddleX model yaml file");
DEFINE_string(image, "", "Path of test image file");
DEFINE_string(image_list, "", "Path of test image list file");
DEFINE_string(device, "CPU", "Device name");
DEFINE_string(save_dir, "", "Path to save visualized image");
DEFINE_int32(batch_size, 1, "Batch size of infering");
DEFINE_double(threshold,
              0.5,
              "The minimum scores of target boxes which are shown");

int main(int argc, char** argv) {
  google::ParseCommandLineFlags(&argc, &argv, true);
  if (FLAGS_model_dir == "") {
    std::cerr << "--model_dir need to be defined" << std::endl;
    return -1;
  }
  if (FLAGS_cfg_dir == "") {
    std::cerr << "--cfg_dir need to be defined" << std::endl;
    return -1;
  }
  if (FLAGS_image == "" & FLAGS_image_list == "") {
    std::cerr << "--image or --image_list need to be defined" << std::endl;
    return -1;
  }

  //
  PaddleX::Model model; 
  model.Init(FLAGS_model_dir, FLAGS_cfg_dir, FLAGS_device);

  int imgs = 1;
  auto colormap = PaddleX::GenerateColorMap(model.labels.size());
  // 进行预测
  if (FLAGS_image_list != "") {
    std::ifstream inf(FLAGS_image_list);
    if(!inf){
      std::cerr << "Fail to open file " << FLAGS_image_list << std::endl;
      return -1;
    }
    std::string image_path;
    model.total_time_ = 0.0f;
    model.count_num_ = 0;
    while (getline(inf, image_path)) {
      PaddleX::DetResult result;
      cv::Mat im = cv::imread(image_path, 1);
      model.predict(im, &result);
      if(FLAGS_save_dir != ""){
        cv::Mat vis_img =
          PaddleX::Visualize(im, result, model.labels, colormap, FLAGS_threshold);  
        std::string save_path =
          PaddleX::generate_save_path(FLAGS_save_dir, FLAGS_image);      
        cv::imwrite(save_path, vis_img);
        std::cout << "Visualized output saved as " << save_path << std::endl;
      }
      model.count_num_++;
    }
    std::cout << "im per ms: " << model.total_time_*10 << std::endl;
  }else {
  PaddleX::DetResult result;
  cv::Mat im = cv::imread(FLAGS_image, 1);
  model.predict(im, &result);
  for (int i = 0; i < result.boxes.size(); ++i) {
      std::cout << "image file: " << FLAGS_image << std::endl;
      std::cout << ", predict label: " << result.boxes[i].category
                << ", label_id:" << result.boxes[i].category_id
                << ", score: " << result.boxes[i].score
                << ", box(xmin, ymin, w, h):(" << result.boxes[i].coordinate[0]
                << ", " << result.boxes[i].coordinate[1] << ", "
                << result.boxes[i].coordinate[2] << ", "
                << result.boxes[i].coordinate[3] << ")" << std::endl;
    }
    if(FLAGS_save_dir != ""){
    // 可视化
      cv::Mat vis_img =
          PaddleX::Visualize(im, result, model.labels, colormap, FLAGS_threshold);
      std::string save_path =
          PaddleX::generate_save_path(FLAGS_save_dir, FLAGS_image);
      cv::imwrite(save_path, vis_img);
      result.clear();
      std::cout << "Visualized output saved as " << save_path << std::endl;
    }
  }
  return 0;
}


