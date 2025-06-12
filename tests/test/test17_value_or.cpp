/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/utils-cpp
 * Contact:  ihor-drachuk-libs@pm.me  */

#include <gtest/gtest.h>
#include <utils-cpp/value_or.h>
#include <optional>
#include <string>
#include <memory>

namespace {

struct TestStruct
{
    int intField {42};
    std::string stringField {"default"};
    double doubleField {3.14};
};

struct NestedStruct
{
    TestStruct nested;
    int value {123};
};

struct MoveOnlyStruct
{
    std::unique_ptr<int> ptr;

    MoveOnlyStruct() : ptr(std::make_unique<int>(42)) {}
    MoveOnlyStruct(const MoveOnlyStruct&) = delete;
    MoveOnlyStruct& operator=(const MoveOnlyStruct&) = delete;
    MoveOnlyStruct(MoveOnlyStruct&&) = default;
    MoveOnlyStruct& operator=(MoveOnlyStruct&&) = default;
};

struct Level3
{
    std::optional<std::string> optString;
    double data {3.14};
};

struct Level2
{
    std::optional<Level3> optLevel3;
    int value {42};
};

struct Level1
{
    std::optional<Level2> optLevel2;
    std::string name {"level1"};
};

} // namespace

TEST(utils_cpp, value_or_basic_test)
{
    std::optional<TestStruct> optStruct;

    // Test with empty optional
    auto result1 = utils_cpp::value_or(optStruct, &TestStruct::intField, 999);
    EXPECT_EQ(result1, 999);

    auto result2 = utils_cpp::value_or(optStruct, &TestStruct::stringField, std::string("fallback"));
    EXPECT_EQ(result2, "fallback");

    auto result3 = utils_cpp::value_or(optStruct, &TestStruct::stringField, "fallback");
    EXPECT_EQ(result3, "fallback");

    auto result4 = utils_cpp::value_or(optStruct, &TestStruct::doubleField, 1.23);
    EXPECT_DOUBLE_EQ(result4, 1.23);
}

TEST(utils_cpp, value_or_with_value_test)
{
    TestStruct testData;
    testData.intField = 100;
    testData.stringField = "test_value";
    testData.doubleField = 2.71;

    std::optional<TestStruct> optStruct = testData;

    // Test with optional containing value
    auto result1 = utils_cpp::value_or(optStruct, &TestStruct::intField, 999);
    EXPECT_EQ(result1, 100);

    auto result2 = utils_cpp::value_or(optStruct, &TestStruct::stringField, std::string("fallback"));
    EXPECT_EQ(result2, "test_value");

    auto result3 = utils_cpp::value_or(optStruct, &TestStruct::doubleField, 1.23);
    EXPECT_DOUBLE_EQ(result3, 2.71);
}

TEST(utils_cpp, value_or_const_optional_test)
{
    TestStruct testData;
    testData.stringField = "const_test";

    const std::optional<TestStruct> constOptStruct = testData;

    auto result = utils_cpp::value_or(constOptStruct, &TestStruct::stringField, std::string("fallback"));
    EXPECT_EQ(result, "const_test");

    const std::optional<TestStruct> emptyConstOpt;
    auto result2 = utils_cpp::value_or(emptyConstOpt, &TestStruct::stringField, std::string("fallback"));
    EXPECT_EQ(result2, "fallback");
}

TEST(utils_cpp, value_or_rvalue_optional_test)
{
    auto createOptional = []() -> std::optional<TestStruct> {
        TestStruct data;
        data.stringField = "rvalue_test";
        return {data};
    };

    auto result = utils_cpp::value_or(createOptional(), &TestStruct::stringField, std::string("fallback"));
    EXPECT_EQ(result, "rvalue_test");

    auto createEmpty = []() -> std::optional<TestStruct> {
        return std::nullopt;
    };

    auto result2 = utils_cpp::value_or(createEmpty(), &TestStruct::stringField, std::string("fallback"));
    EXPECT_EQ(result2, "fallback");
}

TEST(utils_cpp, value_or_different_types_test)
{
    std::optional<TestStruct> optStruct;

    // Test with C-string fallback for string field
    auto result1 = utils_cpp::value_or(optStruct, &TestStruct::stringField, "c_string_fallback");
    EXPECT_EQ(result1, "c_string_fallback");

    // Test with different numeric types
    auto result2 = utils_cpp::value_or(optStruct, &TestStruct::intField, 3.5); // double fallback for int field
    EXPECT_DOUBLE_EQ(result2, 3.5);
}

TEST(utils_cpp, value_or_nested_struct_test)
{
    std::optional<NestedStruct> optNested;

    // Test accessing field of nested struct - this is the actual nested access
    // Note: This tests accessing NestedStruct::nested field, not a field within the nested struct
    TestStruct fallbackStruct;
    fallbackStruct.intField = 999;
    fallbackStruct.stringField = "fallback_nested";

    auto result1 = utils_cpp::value_or(optNested, &NestedStruct::nested, fallbackStruct);
    EXPECT_EQ(result1.intField, 999);
    EXPECT_EQ(result1.stringField, "fallback_nested");

    // Test with value present
    NestedStruct data;
    data.nested.intField = 123;
    data.nested.stringField = "actual_nested";
    optNested = data;

    auto result2 = utils_cpp::value_or(optNested, &NestedStruct::nested, fallbackStruct);
    EXPECT_EQ(result2.intField, 123);
    EXPECT_EQ(result2.stringField, "actual_nested");

    // Test accessing simple field too
    auto result3 = utils_cpp::value_or(optNested, &NestedStruct::value, 456);
    EXPECT_EQ(result3, 123); // default value from NestedStruct constructor
}

TEST(utils_cpp, value_or_move_semantics_test)
{
    // Test with move-only types
    std::optional<MoveOnlyStruct> optMoveOnly;

    // Empty optional - should return fallback
    auto fallback = std::make_unique<int>(999);
    auto result1 = utils_cpp::value_or(std::move(optMoveOnly), &MoveOnlyStruct::ptr, std::move(fallback));
    EXPECT_NE(result1, nullptr);
    EXPECT_EQ(*result1, 999);

    // Optional with value - should return field value
    MoveOnlyStruct moveData;
    *moveData.ptr = 555;
    optMoveOnly = std::move(moveData);

    auto fallback2 = std::make_unique<int>(888);
    auto result2 = utils_cpp::value_or(std::move(optMoveOnly), &MoveOnlyStruct::ptr, std::move(fallback2));
    EXPECT_NE(result2, nullptr);
    EXPECT_EQ(*result2, 555);
}

TEST(utils_cpp, value_or_constexpr_test)
{
    // Test constexpr functionality with simple types
    constexpr auto test_constexpr = []() constexpr {
        struct SimpleStruct { int value = 42; };
        std::optional<SimpleStruct> opt; // empty
        // Use the public API for constexpr testing
        return utils_cpp::value_or(opt, &SimpleStruct::value, 999);
    };

    constexpr auto result = test_constexpr();
    static_assert(result == 999, "Constexpr test failed");
    EXPECT_EQ(result, 999);

    // Test runtime unified interface
    struct SimpleStruct { int value = 42; };
    std::optional<SimpleStruct> opt;
    auto runtime_result = utils_cpp::value_or(opt, &SimpleStruct::value, 999);
    EXPECT_EQ(runtime_result, 999);
}

TEST(utils_cpp, value_or_reference_fallback_test)
{
    std::optional<TestStruct> optStruct;

    // Test with reference fallback
    std::string fallbackString = "reference_fallback";
    auto result = utils_cpp::value_or(optStruct, &TestStruct::stringField, fallbackString);
    EXPECT_EQ(result, "reference_fallback");

    // Modify original and verify copy semantics
    fallbackString = "modified";
    EXPECT_EQ(result, "reference_fallback"); // Should still be the original value
}

TEST(utils_cpp, value_or_type_deduction_test)
{
    std::optional<TestStruct> optStruct;

    // Test that return type is properly deduced
    auto intResult = utils_cpp::value_or(optStruct, &TestStruct::intField, 3.14);
    static_assert(std::is_same_v<decltype(intResult), double>, "Type deduction failed for int/double");
    EXPECT_DOUBLE_EQ(intResult, 3.14);

    auto stringResult = utils_cpp::value_or(optStruct, &TestStruct::stringField, "literal");
    static_assert(std::is_same_v<decltype(stringResult), std::string>, "Type deduction failed for string/literal");
    EXPECT_EQ(stringResult, "literal");
}

// =====================================================================================
// Tests for unified value_or interface (single + chained access)
// =====================================================================================

TEST(utils_cpp, value_or_unified_interface_test)
{
    // Test that the unified interface works for both single and chained access
    std::optional<TestStruct> optStruct;

    // Single field access (existing functionality preserved)
    auto result1 = utils_cpp::value_or(optStruct, &TestStruct::intField, 999);
    EXPECT_EQ(result1, 999);

    auto result2 = utils_cpp::value_or(optStruct, &TestStruct::stringField, "fallback");
    EXPECT_EQ(result2, "fallback");

    // Test with actual values
    TestStruct testData;
    testData.intField = 42;
    testData.stringField = "test";
    optStruct = testData;

    auto result3 = utils_cpp::value_or(optStruct, &TestStruct::intField, 999);
    EXPECT_EQ(result3, 42);

    auto result4 = utils_cpp::value_or(optStruct, &TestStruct::stringField, "fallback");
    EXPECT_EQ(result4, "test");

    // Now test chained access with the same unified interface
    std::optional<Level1> optLevel1;

    auto result5 = utils_cpp::value_or(optLevel1, &Level1::optLevel2, &Level2::optLevel3, &Level3::optString, "chained_fallback");
    EXPECT_EQ(result5, "chained_fallback");
}

// =====================================================================================
// Tests for chained optional access
// =====================================================================================

TEST(utils_cpp, value_or_chained_basic_test)
{
    // Test the main feature you requested! Now with unified interface
    std::optional<Level1> optLevel1;

    // Empty chain - should return fallback
    auto result1 = utils_cpp::value_or(optLevel1, &Level1::optLevel2, &Level2::optLevel3, &Level3::optString, "fallback");
    EXPECT_EQ(result1, "fallback");

    // Partially filled chain - Level1 exists but Level2 is empty
    Level1 level1;
    level1.optLevel2 = std::nullopt; // Empty Level2
    optLevel1 = level1;

    auto result2 = utils_cpp::value_or(optLevel1, &Level1::optLevel2, &Level2::optLevel3, &Level3::optString, "fallback");
    EXPECT_EQ(result2, "fallback");

    // Fully filled chain
    Level3 level3;
    level3.optString = "success";
    Level2 level2;
    level2.optLevel3 = level3;
    level1.optLevel2 = level2;
    optLevel1 = level1;

    auto result3 = utils_cpp::value_or(optLevel1, &Level1::optLevel2, &Level2::optLevel3, &Level3::optString, "fallback");
    EXPECT_EQ(result3, "success");
}

TEST(utils_cpp, value_or_mixed_chain_test)
{
    // Test chain with non-optional intermediate values
    std::optional<Level1> optLevel1;

    // Test accessing non-optional field in the chain
    auto result1 = utils_cpp::value_or(optLevel1, &Level1::optLevel2, &Level2::value, 999);
    EXPECT_EQ(result1, 999);

    Level1 level1;
    Level2 level2;
    level2.value = 555;
    level1.optLevel2 = level2;
    optLevel1 = level1;

    auto result2 = utils_cpp::value_or(optLevel1, &Level1::optLevel2, &Level2::value, 999);
    EXPECT_EQ(result2, 555);
}

TEST(utils_cpp, value_or_fluent_api_test)
{
    // Test the fluent API approach
    std::optional<Level1> optLevel1;

    // Empty chain
    auto result1 = utils_cpp::chained_value_or(optLevel1)
                      .field(&Level1::optLevel2)
                      .field(&Level2::optLevel3)
                      .field(&Level3::optString)
                      .value_or("fluent_fallback");
    EXPECT_EQ(result1, "fluent_fallback");

    // Filled chain
    Level3 level3;
    level3.optString = "fluent_success";
    Level2 level2;
    level2.optLevel3 = level3;
    Level1 level1;
    level1.optLevel2 = level2;
    optLevel1 = level1;

    auto result2 = utils_cpp::chained_value_or(optLevel1)
                      .field(&Level1::optLevel2)
                      .field(&Level2::optLevel3)
                      .field(&Level3::optString)
                      .value_or("fluent_fallback");
    EXPECT_EQ(result2, "fluent_success");
}



TEST(utils_cpp, value_or_real_world_chaining_example)
{
    // Real-world example: accessing deeply nested optional config
    struct ConnectionInfo {
        std::optional<std::string> host;
        int port;
    };

    struct DatabaseConfig {
        std::optional<ConnectionInfo> connection;
    };

    struct Config {
        std::optional<DatabaseConfig> database;
    };

    std::optional<Config> config;

    // Get host with fallback - unified interface with fallback last!
    auto host = utils_cpp::value_or(config,
                                   &Config::database,
                                   &DatabaseConfig::connection,
                                   &ConnectionInfo::host,
                                   "localhost");
    EXPECT_EQ(host, "localhost");

    // Fill the config
    Config cfg;
    DatabaseConfig dbCfg;
    ConnectionInfo connInfo;
    connInfo.host = "prod-server.com";
    connInfo.port = 5432;
    dbCfg.connection = connInfo;
    cfg.database = dbCfg;
    config = cfg;

    // Now get the actual values
    auto actualHost = utils_cpp::value_or(config,
                                         &Config::database,
                                         &DatabaseConfig::connection,
                                         &ConnectionInfo::host,
                                         "localhost");
    EXPECT_EQ(actualHost, "prod-server.com");

    auto port = utils_cpp::value_or(config,
                                   &Config::database,
                                   &DatabaseConfig::connection,
                                   &ConnectionInfo::port,
                                   3306);
    EXPECT_EQ(port, 5432);
}
