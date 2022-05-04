#########################################################################
# File Name: stress_test_start.sh
# Author: Winter
#Created Time:2022年05月03日 星期二 20时57分35秒
#########################################################################
#!/bin/bash
g++ stress-test/stress_test.cpp -o ./bin/stress --std=c++11 -pthread
./bin/stress
