#include <iostream>
#include <filesystem>
#include <gumbo.h>
#include <string>

int main(int argc, char** argv) {
	std::filesystem::path path = std::filesystem::current_path();
	if(argc > 1) {
		path = path / argv[1];
	}
	if(!std::filesystem::exists(path)) {	
		std::cerr << "Path does not exist: " << path << std::endl;
		return 1;
	}

	std::string html = "<html><body>Ciao</body></html>";
	
	GumboOutput* output = gumbo_parse(html.c_str());
	
	gumbo_destroy_output(&kGumboDefaultOptions, output);

	return 0;
}
