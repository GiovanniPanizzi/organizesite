#include <iostream>
#include <filesystem>
#include <gumbo.h>
#include "DirectoryManager.hpp"
#include "FileManager.hpp"

void printDom(const GumboNode* node, const std::string& prefix = "", bool isLast = true) {
    if (!node) return;

    if (node->type == GUMBO_NODE_ELEMENT) {
        std::cout << prefix << (isLast ? "└─" : "├─") << gumbo_normalized_tagname(node->v.element.tag) << "\n";

        const GumboVector* children = &node->v.element.children;
        for (unsigned int i = 0; i < children->length; ++i) {
            const GumboNode* child = static_cast<GumboNode*>(children->data[i]);
            if (child->type == GUMBO_NODE_ELEMENT || child->type == GUMBO_NODE_TEXT) {
                bool lastChild = (i == children->length - 1);
                std::string newPrefix = prefix + (isLast ? "  " : "│ ");
                printDom(child, newPrefix, lastChild);
            }
        }
    } 
	else if (node->type == GUMBO_NODE_TEXT) {
        std::string text = node->v.text.text;
		if (text.find_first_not_of(" \t\n\r") != std::string::npos) {
            std::cout << prefix << (isLast ? "└─" : "├─")
                      << "\"" << text << "\"\n";
        }
    }
}

int main(int argc, char** argv) {
	std::filesystem::path path = std::filesystem::current_path();
	if(argc > 1) {
		path = path / argv[1];
	}
	if(!std::filesystem::exists(path)) {	
		std::cerr << "Path does not exist: " << path << std::endl;
		return 1;
	}

	DirectoryManager dm;
	std::vector<std::filesystem::path> indexHtmlFilesPaths = dm.findFilesByExtention(path, ".html");
	if(indexHtmlFilesPaths.empty()) {
		std::cout << "No .html files found in path: " << path << std::endl;
		return 0;
	}

	FileManager fm;
	std::string html = fm.copyContent(indexHtmlFilesPaths[0]);

	GumboOutput* output = gumbo_parse(html.c_str());

	printDom(output->root);
	
	gumbo_destroy_output(&kGumboDefaultOptions, output);

	return 0;
}
