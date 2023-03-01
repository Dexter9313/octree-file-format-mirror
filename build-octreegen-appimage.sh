#!/bin/bash

docker run --rm -v $(pwd):/project ubuntu:13.10 /bin/bash -c "cd /project/octreegen ; ./build-ubuntu-13.10.sh"

