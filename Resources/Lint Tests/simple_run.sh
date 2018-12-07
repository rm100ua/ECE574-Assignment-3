#!/usr/bin/env bash
echo -e '[0;31m#########################Starting Tests #########################[0m'
././../../cmake-build-debug/src/hlysn ././../if_tests/test_if3.c $1 test_if3.v
sleep 2s
././../../cmake-build-debug/src/hlysn ././../if_tests/test_if4.c $1 test_if4.v
sleep 2s
././../../cmake-build-debug/src/hlysn ././../if_tests/test_if1.c $1 test_if1.v
sleep 2s
././../../cmake-build-debug/src/hlysn ././../if_tests/test_if2.c $1 test_if2.v
sleep 2s
././../../cmake-build-debug/src/hlysn ././../latency_tests/hls_lat_test3.c $1 hls_lat_test3.v
sleep 2s
././../../cmake-build-debug/src/hlysn ././../latency_tests/hls_lat_test4.c $1 hls_lat_test4.v
sleep 2s
././../../cmake-build-debug/src/hlysn ././../latency_tests/hls_lat_test6.c $1 hls_lat_test6.v
sleep 2s
././../../cmake-build-debug/src/hlysn ././../latency_tests/hls_lat_test1.c $1 hls_lat_test1.v
sleep 2s
././../../cmake-build-debug/src/hlysn ././../latency_tests/hls_lat_test2.c $1 hls_lat_test2.v
sleep 2s
././../../cmake-build-debug/src/hlysn ././../latency_tests/hls_lat_test5.c $1 hls_lat_test5.v
sleep 2s
././../../cmake-build-debug/src/hlysn ././../for_tests/test_for1.c $1 test_for1.v
sleep 2s
././../../cmake-build-debug/src/hlysn ././../for_tests/test_for2.c $1 test_for2.v
sleep 2s
././../../cmake-build-debug/src/hlysn ././../for_tests/test_for_if.c $1 test_for_if.v
sleep 2s
././../../cmake-build-debug/src/hlysn ././../for_tests/test_for4.c $1 test_for4.v
sleep 2s
././../../cmake-build-debug/src/hlysn ././../for_tests/test_for3.c $1 test_for3.v
sleep 2s
././../../cmake-build-debug/src/hlysn ././../error_tests/error3.c $1 test error3.v
sleep 2s
././../../cmake-build-debug/src/hlysn ././../error_tests/error2.c $1 test error2.v
sleep 2s
././../../cmake-build-debug/src/hlysn ././../error_tests/error1.c $1 test error1.v
sleep 2s
././../../cmake-build-debug/src/hlysn ././../standard_tests/hls_test7.c $1 hls_test7.v
sleep 2s
././../../cmake-build-debug/src/hlysn ././../standard_tests/hls_test1.c $1 hls_test1.v
sleep 2s
././../../cmake-build-debug/src/hlysn ././../standard_tests/hls_test6.c $1 hls_test6.v
sleep 2s
././../../cmake-build-debug/src/hlysn ././../standard_tests/hls_test2.c $1 hls_test2.v
sleep 2s
././../../cmake-build-debug/src/hlysn ././../standard_tests/hls_test5.c $1 hls_test5.
sleep 2s
././../../cmake-build-debug/src/hlysn ././../standard_tests/hls_test3.c $1 hls_test3.v
sleep 2s
././../../cmake-build-debug/src/hlysn ././../standard_tests/hls_test8.c $1 hls_test8.v
sleep 2s
././../../cmake-build-debug/src/hlysn ././../standard_tests/hls_test4.c $1 hls_test4.v