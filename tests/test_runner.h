#pragma once

#include <cmath>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <string>
#include <vector>

// Minimal in-process test framework. No external deps. Each TEST_CASE registers
// itself at static-init time; main() iterates through registry.

struct TestCase
{
    const char* name;
    std::function<void()> fn;
};

class TestRegistry
{
public:
    static std::vector<TestCase>& cases()
    {
        static std::vector<TestCase> instance;
        return instance;
    }

    static int& failures()
    {
        static int count = 0;
        return count;
    }

    static const char*& currentName()
    {
        static const char* name = "(none)";
        return name;
    }
};

#define TEST_CASE(testName) \
    static void test_##testName(); \
    static const bool _registered_##testName = []{ \
        TestRegistry::cases().push_back({#testName, test_##testName}); \
        return true; \
    }(); \
    static void test_##testName()

#define EXPECT_TRUE(cond) \
    do { \
        if (!(cond)) { \
            std::cerr << "  FAIL " << __FILE__ << ":" << __LINE__ \
                      << " EXPECT_TRUE(" #cond ")\n"; \
            ++TestRegistry::failures(); \
            return; \
        } \
    } while (0)

#define EXPECT_FALSE(cond) EXPECT_TRUE(!(cond))

#define EXPECT_EQ(a, b) \
    do { \
        const auto _lhs = (a); \
        const auto _rhs = (b); \
        if (!(_lhs == _rhs)) { \
            std::cerr << "  FAIL " << __FILE__ << ":" << __LINE__ \
                      << " EXPECT_EQ(" #a ", " #b ")\n" \
                      << "    lhs = " << _lhs << "\n" \
                      << "    rhs = " << _rhs << "\n"; \
            ++TestRegistry::failures(); \
            return; \
        } \
    } while (0)

#define EXPECT_NEAR(a, b, eps) \
    do { \
        const double _lhs = static_cast<double>(a); \
        const double _rhs = static_cast<double>(b); \
        if (std::abs(_lhs - _rhs) > (eps)) { \
            std::cerr << "  FAIL " << __FILE__ << ":" << __LINE__ \
                      << " EXPECT_NEAR(" #a ", " #b ", " #eps ")\n" \
                      << "    lhs = " << _lhs << "\n" \
                      << "    rhs = " << _rhs << "\n"; \
            ++TestRegistry::failures(); \
            return; \
        } \
    } while (0)
