#include <gtest/gtest.h>
#include <utils-cpp/string_hash.h>
#include <set>
#include <list>

TEST(UtilsCpp, string_hash_test)
{
    int triggers1 = 0;
    int triggers2 = 0;

    std::set<uint32_t> hashes;
    std::list<const char*> strings = {"Test string", "Another string", "str", "", "1", "2", "1 "};
    for (const auto x : strings) {
        const auto isInserted = hashes.insert(string_hash(x)).second;
        ASSERT_TRUE(isInserted);

        switch (string_hash(x)) {
            case string_hash("Test string"): triggers1++; break;
            case string_hash("str"):         triggers2++; break;
        }
    }

    ASSERT_EQ(triggers1, 1);
    ASSERT_EQ(triggers2, 1);
}
