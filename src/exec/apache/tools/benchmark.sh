#!/bin/sh
#ab -c 200 -n 5000 http://localhost/hello.k | grep "Requests per second"
ab -c 10 -n 2000 http://localhost/hello.k | grep "Requests per second"
