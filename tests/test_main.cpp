#include "test_runner.h"

#include <iostream>

int main()
{
    auto& cases = TestRegistry::cases();
    int passed = 0;

    std::cout << "Running " << cases.size() << " test cases\n";

    for (const auto& tc : cases)
    {
        TestRegistry::currentName() = tc.name;
        const int beforeFails = TestRegistry::failures();
        tc.fn();
        const int afterFails = TestRegistry::failures();
        if (beforeFails == afterFails)
        {
            std::cout << "  PASS " << tc.name << '\n';
            ++passed;
        }
        else
        {
            std::cout << "  ---- " << tc.name << " (above)\n";
        }
    }

    std::cout << '\n'
              << "Summary: " << passed << "/" << cases.size() << " passed, "
              << TestRegistry::failures() << " failure(s)\n";

    return TestRegistry::failures() == 0 ? 0 : 1;
}
