#!/bin/bash

echo "Updating code on Coala..."
scp /usr/local/coala/gen_exported.h root@192.168.2.14:/usr/src/coala/gen
scp /usr/local/coala/gen_exported.cpp root@192.168.2.14:/usr/src/coala/gen
echo "Code updated! :-)"

# /usr/local/bin/python2.7 << END
# import requests
# import codecs
# ##sess = requests.Session()
# ##sess.auth = ("*", "*")
# url = "http://192.168.2.14:10000/?dataupload="
# gen_h = codecs.open('/usr/local/coala/gen_exported.h', 'r', 'UTF-8')
# gen_h_request = requests.post( url + 'gen_h', files={'gen_exported.h': gen_h})
# gen_cpp = codecs.open('/usr/local/coala/gen_exported.cpp', 'r', 'UTF-8')
# gen_cpp_request = requests.post( url + 'gen_cpp', files={'gen_exported.cpp': gen_cpp})
# END
