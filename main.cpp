#include <iostream>
#include <App.hpp>

int main(int argc, char** argv)
{
	std::string username;
	if (argc > 1)
		username = argv[1];
	else{
		std::cout<<"Enter your name: ";
		std::cin >> username;
	}

	App app(username);
	app.greet();

    return 0;
}