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
    } else if (node->type == GUMBO_NODE_TEXT) {
        std::string text = node->v.text.text;
		if (text.find_first_not_of(" \t\n\r") != std::string::npos) {
            std::cout << prefix << (isLast ? "└─" : "├─")
                      << "\"" << text << "\"\n";
        }
    }
}


// Find the title of the website by looking for the <title> tag in the DOM tree and return its content, if not found return the second option passed
std::string findTitle(const GumboNode* node, const std::string& defaultTitle) {
	if (!node) return defaultTitle;

	if (node->type == GUMBO_NODE_ELEMENT && node->v.element.tag == GUMBO_TAG_TITLE) {
		const GumboVector* children = &node->v.element.children;
		for (unsigned int i = 0; i < children->length; i++) {
			const GumboNode* child = static_cast<GumboNode*>(children->data[i]);
			if (child->type == GUMBO_NODE_TEXT) {
				return child->v.text.text;
			}
		}
	}

	if (node->type == GUMBO_NODE_ELEMENT) {
		const GumboVector* children = &node->v.element.children;
		for (unsigned int i = 0; i < children->length; i++) {
			const GumboNode* child = static_cast<GumboNode*>(children->data[i]);
			std::string title = findTitle(child, defaultTitle);
			if (title != defaultTitle) {
				return title;
			}
		}
	}

	return defaultTitle;
}

// Find all the local resources linked in the index.html file by looking for the <a>, <link>, <img>, <script>, <source>, <iframe>, <video> and <audio> tags and return their paths in a vector
void findLocalResourcesLinked(const GumboNode* node, const std::filesystem::path& baseDir, std::vector<std::filesystem::path>& resources){
    if (!node) return;

    static std::unordered_set<std::filesystem::path> seen;

    if (node->type == GUMBO_NODE_ELEMENT) {
        const GumboElement& el = node->v.element;

        for (unsigned int i = 0; i < el.attributes.length; i++) {
            auto* attr = static_cast<GumboAttribute*>(el.attributes.data[i]);
            std::string name(attr->name);
            std::string value(attr->value);

            bool isResource = false;
            if ((el.tag == GUMBO_TAG_A || el.tag == GUMBO_TAG_LINK) && name == "href") isResource = true;
            else if ((el.tag == GUMBO_TAG_IMG || el.tag == GUMBO_TAG_SCRIPT ||
                      el.tag == GUMBO_TAG_SOURCE || el.tag == GUMBO_TAG_IFRAME ||
                      el.tag == GUMBO_TAG_VIDEO || el.tag == GUMBO_TAG_AUDIO) &&
                     name == "src") isResource = true;
            else if (el.tag == GUMBO_TAG_VIDEO && name == "poster") isResource = true;

            if (isResource && !value.starts_with("http://") && !value.starts_with("https://") && !value.starts_with("//")) {
                std::filesystem::path fullPath = baseDir / value;

                if (std::filesystem::exists(fullPath) && seen.insert(fullPath).second) {
                    resources.push_back(fullPath);
                } else if (!std::filesystem::exists(fullPath)) {
                    std::cout << "Warning: resource not found -> " << fullPath << "\n";
                }
            }
        }

        const GumboVector* children = &el.children;
        for (unsigned int i = 0; i < children->length; i++) {
            findLocalResourcesLinked(static_cast<GumboNode*>(children->data[i]), baseDir, resources);
        }
    }
}

// Website structure
struct Website {
	std::string title;
	std::filesystem::path indexPath;
	std::vector<std::filesystem::path> resourcesPaths;
	GumboOutput* indexDomTree;
	std::vector<GumboOutput*> htmlFilesDomTrees;
};


/* MAIN */
int main(int argc, char** argv) {

	// Get the current path and if an argument is passed, use that as the path instead
	std::filesystem::path path = std::filesystem::current_path();
	if(argc > 1) {
		path = path / argv[1];
	}
	if(!std::filesystem::exists(path)) {	
		std::cerr << "Path does not exist: " << path << std::endl;
		return 1;
	}
	DirectoryManager dm;
	std::vector<std::filesystem::path> indexHtmlFilesPaths = dm.findFilesByName(path, "index.html");

	// If no index.html files are found, print a message and exit
	if(indexHtmlFilesPaths.empty()) {
		std::cout << "No .html files found in path: " << path << std::endl;
		return 0;
	}
	FileManager fm;

	// Create a vector of Website structures and store the paths of the index.html files in it
	std::vector<Website> websites;
	websites.resize(indexHtmlFilesPaths.size());
	for(size_t i = 0; i < indexHtmlFilesPaths.size(); i++) {
		websites[i].indexPath = indexHtmlFilesPaths[i];
	}

	// Parse the index.html files using Gumbo and store the outputs in a vector
	for(size_t i = 0; i < indexHtmlFilesPaths.size(); i++) {
		std::string htmlContent = fm.copyContent(indexHtmlFilesPaths[i]);
		websites[i].indexDomTree = gumbo_parse(htmlContent.c_str());
	}


	// Create a directory called "output" in the current path and if it already exists, print a message and exit
	if(!dm.createDirectory(path, "output")){
		for(Website& website : websites) {
			gumbo_destroy_output(&kGumboDefaultOptions, website.indexDomTree);
		}
		return 1;
	}
	std::filesystem::path outputPath = path / "output";


	// For every index.html file found, create a directory with the title of that file and move the file into that directory
	// If the file is not found, call it Website(n) where n is the number of the website
	// If a directory with the same name already exists, add a suffix to the name until a unique name is found
	unsigned int websiteCount = 0;

	for (auto& website : websites) {
		std::string baseTitle = findTitle(website.indexDomTree->root, "Website" + std::to_string(websiteCount));
		website.title = baseTitle;
		std::string title = baseTitle;

		std::filesystem::path websiteDir;
		int suffix = 1;
		while (true) {
			websiteDir = outputPath / title;
			if (dm.createDirectory(outputPath, title)) break;
			title = baseTitle + std::to_string(suffix++);
		}
		fm.createFile(websiteDir / "index.html", serializeHtmlToString(website.indexDomTree->root));
		websiteCount++;
	}

	for(Website& website : websites) {
		gumbo_destroy_output(&kGumboDefaultOptions, website.indexDomTree);
	}
	return 0;
}