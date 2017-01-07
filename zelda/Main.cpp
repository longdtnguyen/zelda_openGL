#include "Zelda.hpp"

#include <iostream>
using namespace std;

int main( int argc, char **argv ) 
{
	if (argc > 1) {
		std::string luaSceneFile(argv[1]);
		std::string title("Legend of Zelda: the Windwaker - [");
		title += luaSceneFile;
		title += "]";

		CS488Window::launch(argc, argv, new Zelda(luaSceneFile), 1850, 1080, title);

	} else {
		cout << "Must supply Lua file as first argument to program.\n";
	}

	return 0;
}
