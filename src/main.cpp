#include "app/Application.h"

#include <exception>
#include <iostream>
#include <string>

int main()
{
    try
    {
        Application app;
        app.run();
    }
    catch (const std::exception& exception)
    {
        std::cerr << "Fatal error: " << exception.what() << '\n';
        return 1;
    }

    return 0;
}
