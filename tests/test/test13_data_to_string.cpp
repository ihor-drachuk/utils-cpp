/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/utils-qt
 * Contact:  ihor-drachuk-libs@pm.me  */

#include <gtest/gtest.h>
#include <utils-cpp/data_to_string.h>

using namespace utils_cpp;

template<typename T>
static void transform_digits_inplace(T& c)
{
    for (size_t i = 0; i < c.size(); i++)
        if (std::isdigit(c[i]))
            c[i] = i % 10;
}

TEST(utils_cpp, data_to_string_Empty)
{
    auto str = utils_cpp::data_to_string(nullptr, 0);
    ASSERT_EQ(str, "");
}

TEST(utils_cpp, data_to_string_String)
{
    char data[] = "Test";
    auto str = utils_cpp::data_to_string(data, sizeof(data));
    ASSERT_EQ(str, "Test<00>");
}

TEST(utils_cpp, data_to_string_Data)
{
    char data[5];
    data[0] = 1;
    data[1] = 2;
    data[2] = 0;
    data[3] = 3;
    reinterpret_cast<unsigned char*>(data)[4] = 255;

    auto str = utils_cpp::data_to_string(data, sizeof(data));
    ASSERT_STREQ(str.c_str(), "<01 02 00 03 FF>");
}

TEST(utils_cpp, data_to_string_Mixed_Long_SD)
{
    std::string someData = "1234My567Data12";
    transform_digits_inplace(someData);

    auto str = utils_cpp::data_to_string(someData.data(), someData.size());
    ASSERT_STREQ(str.c_str(), "<00 01 02 03>My<06 07 08>Data<03 04>");
}


TEST(utils_cpp, data_to_string_Mixed_Middle_ST)
{
    std::string someData = "AA1BB2CC3DD4EE5";
    transform_digits_inplace(someData);
    auto str = utils_cpp::data_to_string(someData.data(), someData.size());
    ASSERT_STREQ(str.c_str(), "AA<02>BB<05>CC<08>DD<01>EE<04>");
}


TEST(utils_cpp, data_to_string_Mixed_Short_ST)
{
    std::string someData = "A1B2C3D4E5";
    transform_digits_inplace(someData);
    auto str = utils_cpp::data_to_string(someData.data(), someData.size());
    ASSERT_STREQ(str.c_str(), "A<01>B<03>C<05>D<07>E\t");
}

TEST(utils_cpp, data_to_string_Borders)
{
    std::string someData = "1MyData2";
    transform_digits_inplace(someData);

    auto str = utils_cpp::data_to_string(someData.data(), someData.size());
    ASSERT_STREQ(str.c_str(), "<00>MyData<07>");
}
