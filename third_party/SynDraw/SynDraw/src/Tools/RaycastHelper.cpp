#include "RaycastHelper.h"

RaycastHelper::RaycastHelper() {
}

RaycastHelper::RaycastHelper(MeshObject& m, ViewCamera& cam){
    embree.init(m.V().template cast<float>(), m.F().template cast<int>(), true);
    mesh = m;
    camera = cam;
}

RaycastHelper::RaycastHelper(const RaycastHelper& orig) {
}

RaycastHelper::~RaycastHelper() {
}

bool RaycastHelper::isVisible(std::shared_ptr<ContourNode> node, double tol, bool interpolated) {
    Vector3d V1 = node->head_vertex3d();
    Vector3d V2 = node->tail_vertex3d();
    Vector3d midPoint3d = (V1 + V2) / 2.0f;
    
    igl::Hit hit = launch_ray(midPoint3d);
    Vector3f hitPoint = camera.get_position() + hit.t * (midPoint3d.cast<float>() - camera.get_position()).normalized();
    double dist_mid = (midPoint3d.cast<float>() - camera.get_position()).norm();
    double dist_hit = (hitPoint - camera.get_position()).norm();
    
    // >0 means visible
    double signed_dist = dist_hit - dist_mid;

    // filter out autointersections
    if(node->typeinfo() == "edge_contour"){
        std::shared_ptr<EdgeContourNode> ecn = std::dynamic_pointer_cast<EdgeContourNode>(node);
        
        // check signed distance and adjacent faces id
        if(signed_dist < tol and hit.id != mesh.EF().row(ecn->edge_index())(0) and hit.id != mesh.EF().row(ecn->edge_index())(1)){
            return false;
        }    
    }
    else if(node->typeinfo() == "face_contour"){
        std::shared_ptr<FaceContourNode> fcn = std::dynamic_pointer_cast<FaceContourNode>(node);
        
        // check if interpolated occlusion contours. we don't care about distance
        if(node->get_type() == ContourType::OCCLUSION and interpolated and hit.id == fcn->face_index())
            return true;
        
        // check signed distance and face id
        if(signed_dist < tol and hit.id != fcn->face_index()){
            return false;
        }    
    }    
    
    return true;
}

igl::Hit RaycastHelper::launch_ray(Vector3d Pf) {
    RowVector3f rayDir, rayStart;
    RowVector3f P = Pf.transpose().cast<float>();

    if(camera.get_projType() == ProjectionType::ORTHOGRAPHIC){
        float l = (P - camera.get_position().transpose()).norm();
        rayStart = (P - l * (camera.get_forward().transpose().normalized()));
        rayDir = camera.get_forward().normalized();
    }
    else{
        rayDir = (P-camera.get_position().transpose()).normalized();  
        rayStart = camera.get_position();
    }    
        
    igl::Hit hit;
    embree.intersectBeam(rayStart, rayDir, hit);
    return hit;
}    
