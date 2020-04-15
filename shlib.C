#include <iostream>

extern "C" void so_entry (const std::string& arg)
{
	std::cout << "so_entry called with " << arg << std::endl;
}
