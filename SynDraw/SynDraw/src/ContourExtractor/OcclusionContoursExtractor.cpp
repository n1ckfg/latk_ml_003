#include "OcclusionContoursExtractor.h"
#include "../Tools/Tools.h"

#include <iostream>

OcclusionContoursExtractor::~OcclusionContoursExtractor() {
}

OcclusionContoursExtractor::OcclusionContoursExtractor(MeshObject& obj, ViewCamera& cam):ContourExtractor(obj), camera(cam) {
}

bool OcclusionContoursExtractor::is_contour(int i) {
    int f1 = mesh.EF()(i, 0);
    int f2 = mesh.EF()(i, 1);
    
    if(f1 == -1 or f2 == -1)
        return false;
    
    return ndotv(f2) * ndotv(f1) <= 0;
}


void OcclusionContoursExtractor::compute_F_ndotv(){
    ndotv = VectorXd(mesh.F().rows());

    for (unsigned int i = 0; i < mesh.F().rows(); ++i) 
    {
        ndotv(i) = Tools::ndotv_face(mesh, camera, i);
    }
}

bool OcclusionContoursExtractor::is_occluded(int e) {
    return Tools::concavity(mesh, e) > 0;
}
