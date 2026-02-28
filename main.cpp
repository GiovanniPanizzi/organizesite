#include <iostream>
#include <filesystem>
#include <gumbo.h>
#include "DirectoryManager.hpp"
#include "FileManager.hpp"
#include <set>


/* HELPERS */
const std::set<GumboTag> VOID_TAGS = {
    GUMBO_TAG_AREA, GUMBO_TAG_BASE, GUMBO_TAG_BR, GUMBO_TAG_COL,
    GUMBO_TAG_EMBED, GUMBO_TAG_HR, GUMBO_TAG_IMG, GUMBO_TAG_INPUT,
    GUMBO_TAG_LINK, GUMBO_TAG_META, GUMBO_TAG_PARAM, GUMBO_TAG_SOURCE,
    GUMBO_TAG_TRACK, GUMBO_TAG_WBR
};

std::string serializeHtmlToString(const GumboNode* node, int indent = 0) {
    if (!node) return "";

    std::string result;
    std::string indentStr(indent, '\t');

    switch (node->type) {
        case GUMBO_NODE_ELEMENT: {
            const GumboElement& el = node->v.element;
            std::string tagName = gumbo_normalized_tagname(el.tag);

            result += indentStr + "<" + tagName;
            for (unsigned int i = 0; i < el.attributes.length; i++) {
                auto* attr = static_cast<GumboAttribute*>(el.attributes.data[i]);
                result += " " + std::string(attr->name) + "=\"" + std::string(attr->value) + "\"";
            }

            if (VOID_TAGS.count(el.tag)) {
                result += ">\n";
                return result;
            }

            result += ">\n";

            for (unsigned int i = 0; i < el.children.length; i++) {
                const GumboNode* child = static_cast<GumboNode*>(el.children.data[i]);
                result += serializeHtmlToString(child, indent + 1);
            }

            result += indentStr + "</" + tagName + ">\n";
            break;
        }

        case GUMBO_NODE_TEXT: {
            std::string text = node->v.text.text;
            if (text.find_first_not_of(" \t\n\r") != std::string::npos) {
                result += indentStr + text + "\n";
            }
            break;
        }

        case GUMBO_NODE_COMMENT: {
            result += indentStr + "<!--" + node->v.text.text + "-->\n";
            break;
        }

        default:
            break;
    }

    return result;
}

void printDom(const GumboNode* node, const std::string& prefix = "", bool isLast = true) {
    if (!node) return;

    if (node->type == GUMBO_NODE_ELEMENT) {
        std::cout << prefix << (isLast ? "└─" : "├─") << gumbo_normalized_tagname(node->v.element.tag) << "\n";

        const GumboVector* children = &node->v.element.children;
        for (unsigned int i = 0; i < children->length; i++) {
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


/* MAIN */

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

	std::string serializedHtml = serializeHtmlToString(output->root);

	std::cout << "Serialized HTML:\n" << serializedHtml << std::endl;
	
	gumbo_destroy_output(&kGumboDefaultOptions, output);

	return 0;
}
