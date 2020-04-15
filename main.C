#include <dlfcn.h>
#include <string>

void* load_shlib(std::string path)
{
	// also crashes with just RTLD_LOCAL
	return dlopen(path.c_str(), RTLD_NOW | RTLD_GLOBAL | RTLD_DEEPBIND);
}

int main(int argc, char** argv)
{
	void* opened = load_shlib(argv[1]);

	auto* entry = (void (*) (const std::string&)) dlsym(opened, "so_entry");
	(*entry)(argv[2]);
}
