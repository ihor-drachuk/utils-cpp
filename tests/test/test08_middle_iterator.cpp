#include <gtest/gtest.h>
#include <utils-cpp/middle_iterator.h>
#include <vector>

namespace {

struct data_pack
{
    std::vector<int> container;
    std::vector<int> expectedResult;
};

struct data_pack_change
{
    std::vector<int> container;
    std::vector<int> expectedResult;
};

class middle_iterator_basic : public testing::TestWithParam<data_pack>
{
public:
};

class middle_iterator_change : public testing::TestWithParam<data_pack_change>
{
public:
};


} // namespace

TEST_P(middle_iterator_basic, Test)
{
    auto dataPack = GetParam();
    auto it = middle_iterator(dataPack.container.cbegin(), dataPack.container.cend());
    std::vector<int> iterated;

    while (it.isValid())
        iterated.push_back(*it++);

    ASSERT_EQ(iterated, dataPack.expectedResult);
}

INSTANTIATE_TEST_SUITE_P(
    Test,
    middle_iterator_basic,
    testing::Values(
            data_pack{{1,2,3,4,5}, {3,2,4,1,5}},
            data_pack{{1,2,3,4},   {3,2,4,1}},
            data_pack{{1,2,3},     {2,1,3}},
            data_pack{{1,2},       {2,1}},
            data_pack{{1},         {1}},
            data_pack{{},          {}}
    )
);

TEST_P(middle_iterator_change, Test)
{
    auto dataPack = GetParam();
    auto it = middle_iterator(dataPack.container.begin(), dataPack.container.end());

    for (int i = 0; i < 3; i++)
        *it++ = 0;

    ASSERT_EQ(dataPack.container, dataPack.expectedResult);
}

INSTANTIATE_TEST_SUITE_P(
    Test,
    middle_iterator_change,
    testing::Values(
            data_pack_change{{1,2,3,4,5}, {1,0,0,0,5}},
            data_pack_change{{1,2,3,4},   {1,0,0,0}},
            data_pack_change{{1,2,3},     {0,0,0}}
    )
);
