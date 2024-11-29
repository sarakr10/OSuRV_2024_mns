#!/bin/bash

exit 0

./waf configure

./waf build size upload --app=11 --port=/dev/ttyUSB1
