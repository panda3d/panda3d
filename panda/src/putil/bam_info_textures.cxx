#include "pandaFramework.h"
#include "bamReader.h"
#include "texture.h"
#include "textureAttrib.h"
#include <filesystem>
#include <set>
#include <iostream>

int main(int argc, char **argv) {
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " <bam-file> [--list-textures]" << std::endl;
        return 1;
    }

    std::string bam_file = argv[1];
    bool list_textures_flag = false;

    for (int i = 2; i < argc; ++i) {
        if (std::string(argv[i]) == "--list-textures") {
            list_textures_flag = true;
        }
    }

    if (!list_textures_flag) {
        std::cout << "No options specified. Use --list-textures to list textures." << std::endl;
        return 0;
    }

    // Load BAM file
    BamReader loader;
    PandaNode *root_node = loader.read_node(bam_file);
    if (!root_node) {
        std::cerr << "Failed to load BAM file: " << bam_file << std::endl;
        return 1;
    }

    // Collect textures
    std::set<std::string> textures_found;
    root_node->traverse([&](NodePath node) {
        CPT(TextureAttrib) tex_attrib = node.node()->get_attrib(TextureAttrib::get_class_type());
        if (tex_attrib) {
            for (int i = 0; i < tex_attrib->get_num_on_stages(); ++i) {
                Texture *tex = tex_attrib->get_on_texture(i);
                if (tex) {
                    textures_found.insert(tex->get_name().c_str());
                }
            }
        }
    });

    // Print results
    std::cout << "Textures referenced in BAM:\n";
    for (auto &t : textures_found) {
        if (std::filesystem::exists(t)) {
            std::cout << t << " [FOUND]\n";
        } else {
            std::cout << t << " [MISSING]\n";
        }
    }

    return 0;
}
