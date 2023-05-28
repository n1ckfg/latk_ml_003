#include "SharpFeaturesExtractor.h"
#include "../Tools/Tools.h"

#include <iostream>

SharpFeaturesExtractor::~SharpFeaturesExtractor() {
}

SharpFeaturesExtractor::SharpFeaturesExtractor(MeshObject& obj,float angle):ContourExtractor(obj) {
    threshold_angle = angle;
}

//
//const std::vector<ContourNode*>& SharpFeaturesExtractor::extract_contours(ViewGraph& graph) {
//    VectorXd N1, N2;
//    double dot_adjacentFaces;
//    
//    contours.clear();
//    
//    for (unsigned int i = 0; i < mesh.E().rows(); ++i) {
//        N1 = mesh.F_normals().row(mesh.EF()(i, 0));
//        N2 = mesh.F_normals().row(mesh.EF()(i, 1));
//        dot_adjacentFaces = N1.dot(N2); 
//        
//        if (dot_adjacentFaces <  cos((threshold_angle * M_PI) / 180)) { 
//            graph.insert_mesh_edge(i, CREASE, compute_visibility(i));
//        }
//    }
//    
//    return contours;
//}

bool SharpFeaturesExtractor::is_contour(int i) {
    int f1 = mesh.EF()(i, 0);
    int f2 = mesh.EF()(i, 1);
    
    if(f1 == -1 or f2 == -1){
        return false;
    }
    else{
        VectorXd N1 = mesh.F_normals().row(mesh.EF()(i, 0));
        VectorXd N2 = mesh.F_normals().row(mesh.EF()(i, 1));
        double dot_adjacentFaces = N1.dot(N2); 
        if( dot_adjacentFaces <  cos((threshold_angle * M_PI) / 180) ){
            sharpVertices.insert(mesh.E()(0, 0));
            sharpVertices.insert(mesh.E()(0, 1));
            return true;
        }        
    }

    return false;
}


// all edges on backfaces are not visible
bool SharpFeaturesExtractor::is_occluded(ViewCamera& cam, int i) {
    return Tools::is_face_backfacing(mesh, cam, mesh.EF()(i, 0)) and Tools::is_face_backfacing(mesh, cam, mesh.EF()(i, 1));
}
