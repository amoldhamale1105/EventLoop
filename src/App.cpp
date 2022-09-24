#include <App.hpp>
#include <random>

App::App() : name("user")
{
	initialize();
}

App::App(const std::string& name) : name(name)
{
	initialize();
}

App::~App()
{
	std::cout<<"App exiting..."<<std::endl;
}

void App::initialize()
{
	srand(time(nullptr));
	
	greetings[0] = "Hello "+name+", how are you doing today?";
	greetings[1] = "Welcome "+name+"!";
	greetings[2] = "Hi "+name+", what's up?";
	greetings[3] = "How's it going "+name+"?";
	greetings[4] = "Good to see you "+name+"!";
}

void App::greet()
{
	std::cout<<greetings[rand()%5]<<std::endl;
}
