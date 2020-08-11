# coding: utf8
"""
http utils
"""
import logging
from baidubce.bce_client_configuration import BceClientConfiguration
from baidubce.auth.bce_credentials import BceCredentials
from baidubce.services.bos.bos_client import BosClient

"""
config object
"""
import json
import sys

class HubConfig(object):
    """
    HubConfig class
    """
    _instance = None
    _configs = {}
    """
    constructor
    """
    def __new__(cls, *args, **kw):
        if cls._instance is None:
            cls._instance = object.__new__(cls, *args, **kw)
        return cls._instance

    def load(self, config_file):
        """
        load a config file
        """
        with open(config_file) as fp:
            HubConfig._configs = json.load(fp)


# es_config = {
#     "server":"http://182.61.177.7:8200",
#     "index":"paddlehub-test",
#     "module_index": "_doc",
#     "need_auth" : True,
#     "auth_str" : "Basic c3VwZXJ1c2VyOlBhZGRsZTY2Ng==",
#     "default_page_size": 100,
#     "host":"http://182.61.177.7",
#     "port":"9200",
#     "upload_folder": "/file"
# }
bos_config = {
        "bucket": "paddlex",
        "bos_host": "bj.bcebos.com",
        "access_key_id": "ea9d0e09aeb343599c5310be740fc3fe",
        "secret_access_key": "db5d06e638b84b0c89efce0d784f9536"
    }
#
str1 = "AI Book 快速体验</a>"
str2 = "AI Studio 快速体验</a>"
str3_1 = "hub install"
str3_2 = "```"
g_filename = ""


config = HubConfig()


#bos_config = config.bos_config

# 设置BosClient的Host，Access Key ID和Secret Access Key
bos_host = str(bos_config['bos_host'])
access_key_id = str(bos_config['access_key_id'])
secret_access_key = str(bos_config['secret_access_key'])
bos_bucket = str(bos_config['bucket'])

# 设置日志文件的句柄和日志级别
logger = logging.getLogger('baidubce.http.bce_http_client')
fh = logging.FileHandler("bos.log")
fh.setLevel(logging.DEBUG)

# 设置日志文件输出的顺序、结构和内容
formatter = logging.Formatter('%(asctime)s - %(name)s - %(levelname)s - %(message)s')
fh.setFormatter(formatter)
logger.setLevel(logging.DEBUG)
logger.addHandler(fh)


def upload_to_bos(src, category='test'):
    # 创建BceClientConfiguration
    b_config = BceClientConfiguration(credentials=BceCredentials(access_key_id, secret_access_key), endpoint=bos_host)
    # 新建BosClient
    bos_client = BosClient(b_config)
    bos_client.put_object_from_file(bos_bucket, "%s/%s" % (category, src))
    url = 'https://bj.bcebos.com/%s/%s/%s' % (bos_bucket, category, src)
    return url


def upload_to_bos_from_raw(raw, name, category='test'):
    # 创建BceClientConfiguration
    # print("in1")
    b_config = BceClientConfiguration(credentials=BceCredentials(access_key_id, secret_access_key), endpoint=bos_host)
    # 新建BosClient
    bos_client = BosClient(b_config)
    bos_client.put_object_from_string(bos_bucket, "%s/%s" % (category, name), raw)
    url = 'https://bj.bcebos.com/%s/%s/%s' % (bos_bucket, category, name)
    return url

#
if __name__ == "__main__":
    # print(sys.argv)
    if len(sys.argv) != 3:
        print("Usage: python bos_tuils.py file_path file_name")
        sys.exit(1)
    with open(sys.argv[1],
              'rb') as fp:
        url = upload_to_bos_from_raw(raw=fp.read(),
                                     name=sys.argv[2],
                                               category="deploy/armopencv")
        print(url)
