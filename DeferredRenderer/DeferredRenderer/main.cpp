#include "Renderer.h"

int main()
{
	Renderer renderer;

	try
	{
		renderer.Initialize();

		renderer.RenderLoop();
	}

	catch (const std::runtime_error& e)
	{
		std::cerr << e.what() << '\n';
		return -1;
	}

	return 0;
}