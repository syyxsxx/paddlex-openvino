# copyright (c) 2020 PaddlePaddle Authors. All Rights Reserve.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import argparse
import sys
import os
import cv2 as cv
import numpy as np
from six import text_type as _text_type
from openvino.inference_engine import IECore
from utils import logging 


def arg_parser():
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--model_dir",
        "-m",
        type=str,
        default=None,
        help="path to openvino model .xml file")
    parser.add_argument(
        "--device",
        "-d",
        type=str,
        default='CPU',
        help="Specify the target device to infer on:[CPU, GPU, FPGA, HDDL, MYRIAD,HETERO]"
             "Default value is CPU")
    parser.add_argument(
        "--img",
        "-i",
        type=str,
        default=None,
        help="Path to a folder with images or path to an image files")

    parser.add_argument(
        "--cfg_dir",
        "-c",
        type=str,
        default=None,
        help="Path to PaddelX model yml file")

    return parser


def main():
    parser = arg_parser()
    args = parser.parse_args()
    model_xml = args.model_dir
    model_bin = os.path.splitext(model_xml)[0] + ".bin"
    with open(cfg_dir) as f:
        info = yaml.load(f.read(), Loader=yaml.Loader)

    #initialization for specified device
    logging.info("Creating Inference Engine")
    ie = IECore()
    logging.info("Loading network files:\n\t{}\n\t{}".format(model_xml, model_bin))
    net = ie.read_network(model=model_xml,weights=model_bin)

    logging.info("Perparing input blobs")
    input_blob = next(iter(net.input_info))
    out_blob = next(iter(net.outputs))
    net.batch_size = 1

    #Read and pre-process input images
    n, c, h, w = net.input_info[input_blob].input_data.shape
    images = np.ndarray(shape=(n,c,h,w))
    for i in range(n):
        image = cv.imread(args.img)
        image = cv.resize(image,(w,h))
        image = image.transpose((2,0,1))
        images[i] = image
    
    logging.info("Batch size is {}".format(n))
    exec_net = ie.load_network(network=net,device_name=args.device)

    #Start sync inference
    logging.info("Starting inference in synchronous mode")
    res = exec_net.infer(inputs={input_blob:images})
    
    #Processing output blob
    logging.info("Processing output blob")
    res = res[out_blob]
    print("res: ",res)


if __name__ == "__main__":
    main()

