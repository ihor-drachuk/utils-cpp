/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/utils-cpp
 * Contact:  ihor-drachuk-libs@pm.me  */

#include <gtest/gtest.h>
#include <utils-cpp/vm_detector.h>

TEST(utils_cpp, VmDetectorTest)
{
    utils_cpp::detectSupervisor();
    utils_cpp::detectVmOnly();
}
