#include "Pipeline/Pipeline.h"
#include <iostream>

InputAssembler::InputAssembler(const Transform::Mesh& mesh)
{
    //if (!mesh.indices.empty()) {
    //    if (mesh.indices.size() % 3 != 0) {
    //        std::cout << "Erreur : nombre d'indices invalide.\n";
    //        return;
    //    }

    //    for (size_t i = 0; i < mesh.indices.size(); i += 3) {
    //        unsigned int a = mesh.indices[i];
    //        unsigned int b = mesh.indices[i + 1];
    //        unsigned int c = mesh.indices[i + 2];

    //        if (a >= mesh.vertices.size() || b >= mesh.vertices.size() || c >= mesh.vertices.size()) {
    //            std::cout << "Erreur : index hors limites.\n";
    //            return;
    //        }

    //    }
    //}
    //else {
    //    if (mesh.vertices.size() % 3 != 0) {
    //        std::cout << "Erreur : nombre de sommets invalide.\n";
    //        return;
    //    }
    //}
}
