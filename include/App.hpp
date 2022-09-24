#pragma once
#include <iostream>
#include <array>

class App
{
    private:
        std::string name;
        std::array<std::string, 5> greetings;

        void initialize();
    
    public:
        App();
        App(const std::string& name);
        virtual ~App();

        void greet();
};
