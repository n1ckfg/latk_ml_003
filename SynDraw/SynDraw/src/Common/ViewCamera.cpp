#include "ViewCamera.h"
#include "../Tools/Tools.h"
#include <iostream>
#include <Eigen/Geometry>

ViewCamera::ViewCamera() {
//    model.setIdentity();
    setView(Vector3f(1.0f, 0.0f, 0.0f), Vector3f(0.0f, 0.0f, 0.0f), Vector3f(0.0f, 1.0f, 0.0f));
    projType = ProjectionType::PERSPECTIVE;
    setProj(-5, 5, -5, 5, 0, 50); 
    viewport = Vector4f(0,0,256,256);
}

ViewCamera::ViewCamera(const ViewCamera& orig):ViewCamera() {
}

ViewCamera::~ViewCamera() {
}

void ViewCamera::setDefault() {
    up = Eigen::Vector3f(0, 1, 0);
    target = Eigen::Vector3f(0, 0, 0);
    position = Eigen::Vector3f(0, 6, 4);
    top = 10;
    bottom = -10;
    left = -10;
    right = 10;
    near = 0.1;
    far = 100;
    computeViewMatrix();
    computeProjectionMatrix();
    updateVP();
}


//void ViewCamera::setModel(Matrix4f m) {
//    model = m;
//
//    updateMVP();
//}

void ViewCamera::setView(Vector3f p, Vector3f t, Vector3f u){
    position = p;
    target = t;
    up = u;
    computeViewMatrix();
    updateVP();
}

void ViewCamera::setProj(float l, float r, float b, float t, float n, float f) {
    left = l;
    right = r;
    top = t;
    bottom = b;
    near = n;
    far = f;
    computeProjectionMatrix();
    updateVP();
}

void ViewCamera::invert_proj() {
    float temp = top;
    top = bottom;
    bottom = temp;
    computeProjectionMatrix();
    updateVP();
}


void ViewCamera::setViewport(Vector4f v) {
    viewport = v;
}

Vector3f ViewCamera::inverse_view(Vector3f vec) {
    //Matrix4f view = viewer.core().view;
    Vector4f v_4;

    v_4 << vec,1;
    v_4 = view.inverse() * v_4;

    return Vector3f(v_4.head(3));
 }

Vector3d ViewCamera::inverse_view2(Vector3d vec) {
    //Matrix4f view = viewer.core().view;
    Vector4d v_4;

    v_4 << vec,1;
    v_4 = view.inverse().cast<double>() * v_4;

    return Vector3d(v_4.head(3));
 }

void ViewCamera::setFromViewer(igl::opengl::glfw::Viewer& viewer) {
    auto c = viewer.core();
//     model = viewer.core().model;
    view = c.view;
    proj = c.proj;
    viewport = c.viewport;


    target = inverse_view(Vector3f(0,0,-1));
    position = inverse_view(Vector3f(0,0,0));
    up = inverse_view(Vector3f(0,1,0)) - position;     

    if(c.orthographic)
        projType = ProjectionType::ORTHOGRAPHIC;
    else
        projType = ProjectionType::PERSPECTIVE;
    
    decompose_projmatrix();
    
    compute_frustum();
}
 
void ViewCamera::decompose_projmatrix() {
    if(projType == ProjectionType::PERSPECTIVE){
        near = proj(3,2) / (proj(2, 2) - 1.0f);
        far = proj(3, 2) / (proj(2, 2) + 1.0f);
        bottom = near * (proj(2, 1) - 1) / proj(1, 1);
        top = near * (proj(2, 1) + 1) / proj(1, 1);
        left = near * (proj(2, 0) - 1) / proj(0, 0);
        right = near * (proj(2, 0) + 1) / proj(0, 0);
    }
    else{
        near   =  (1 + proj(3,2)) / proj(2, 2);
        far    = -(1 - proj(3,2)) / proj(2, 2);
        bottom =  (1 - proj(3,1)) / proj(1, 1);
        top    = -(1 + proj(3,1)) / proj(1, 1);
        left   = -(1 + proj(3,0)) / proj(0, 0);
        right  =  (1 - proj(3,0)) / proj(0, 0);        
    }
}
 
void ViewCamera::save_to_props(std::shared_ptr<Properties> props) {
    props->camera_position = position;
    props->camera_target = target;
    props->camera_up = up;
    props->p_type = projType;
    props->p_near = near;
    props->p_far = far;
    props->p_right = right;
    props->p_left = left;
    props->p_top = top;
    props->p_bottom = bottom;
    props->viewport = viewport;
}


void ViewCamera::computeViewMatrix(){
    Vector3f f = (target - position).normalized();
    Vector3f s = f.cross(up).normalized();
    Vector3f u = s.cross(f);

    view = Matrix4f::Identity();
    view(0, 0) = s(0);
    view(0, 1) = s(1);
    view(0, 2) = s(2);
    view(1, 0) = u(0);
    view(1, 1) = u(1);
    view(1, 2) = u(2);
    view(2, 0) = -f(0);
    view(2, 1) = -f(1);
    view(2, 2) = -f(2);
    view(0, 3) = -s.transpose() * position;
    view(1, 3) = -u.transpose() * position;
    view(2, 3) = f.transpose() * position;
}

void ViewCamera::computeProjectionMatrix(){
    if(projType == ProjectionType::PERSPECTIVE){
        proj = Matrix4f::Zero();
        proj(0, 0) = (2.0f * near) / (right - left);
        proj(1, 1) = (2.0f * near) / (top - bottom);
        proj(0, 2) = (right + left) / (right - left);
        proj(1, 2) = (top + bottom) / (top - bottom);
        proj(2, 2) = -(far + near) / (far - near);
        proj(3, 2) = -1.0f;
        proj(2, 3) = -(2.0f * far * near) / (far - near);
    }
    else{
        proj = Matrix4f::Identity();
        proj(0, 0) = 2.0f / (right - left);
        proj(1, 1) = 2.0f / (top - bottom);
        proj(2, 2) = -2.0f / (far - near);
        proj(0, 3) = -(right + left) / (right - left);
        proj(1, 3) = -(top + bottom) / (top - bottom);
        proj(2, 3) = -(far + near) / (far - near);
    }
    
     compute_frustum();
}

Vector2f ViewCamera::project_point(Vector3f p){
    Vector4f tmp;
    tmp << p, 1;
//    tmp = model * tmp;
    tmp = view * tmp;
    tmp = proj * tmp;

    tmp = tmp.array() / tmp(3);
    tmp = tmp.array() * 0.5f + 0.5f;
    tmp(0) = tmp(0) * (viewport(2) - viewport(0)) + viewport(0);
    tmp(1) = tmp(1) * (viewport(3) - viewport(1)) + viewport(1);

    return tmp.head(2);
}

Vector2d ViewCamera::project_point(Vector3d p){
    Vector3f in = p.cast<float>();
    return project_point(in).cast<double>();
}

void ViewCamera::updateVP() {
    VP = view * proj;
}

RowVector3d ViewCamera::get_view_direction(RowVector3d p) {
    if(projType == ProjectionType::ORTHOGRAPHIC){
        return get_forward().normalized().transpose().cast<double>();
    }
    else if(projType == ProjectionType::PERSPECTIVE){
        return (p - position.transpose().cast<double>()).normalized(); 
    }          
    return RowVector3d();
}

RowVector3d ViewCamera::get_proj_view_direction(RowVector3d p, RowVector3d normal) {
    RowVector3d viewDirection = get_view_direction(p);
    const double d = viewDirection.dot(normal);
    
    const RowVector3d proj_camera = position.cast<double>().transpose() - d * normal;
    RowVector3d r = proj_camera - p;    
    
    return r.normalized();
}


void ViewCamera::print(){
    IOFormat fmt = IOFormat(FullPrecision, 0, "\t", ", ", "", "", "( ", " )");

    if(projType == ProjectionType::PERSPECTIVE)
        std::cout << "projType PERSPECTIVE" << std::endl ;    
    else
        std::cout << "projType ORTHOGRAPHIC" << std::endl ;            
    std::cout << "pos    : " << position.format(fmt) << std::endl;
    std::cout << "target : " << target.format(fmt) << std::endl ;
    std::cout << "up     : " << up.format(fmt) << std::endl ;
    std::cout << "left   : " << left << std::endl ;
    std::cout << "right  : " << right << std::endl ;
    std::cout << "top    : " << top << std::endl ;
    std::cout << "bottom : " << bottom << std::endl ;
    std::cout << "near   : " << near << std::endl ;
    std::cout << "far    : " << far << std::endl ;    
    std::cout << "vport  : " << viewport.format(fmt)<< std::endl << std::endl ;
}

void ViewCamera::setFromProperties(Properties& props) {
//    model.setIdentity();
    target = props.camera_target;
    position = props.camera_position;
    up = props.camera_up;
    
    computeViewMatrix();
    
    projType = props.p_type;
    
    left = props.p_left;
    right = props.p_right;
    top = props.p_top;
    bottom = props.p_bottom;
    near = props.p_near;
    far = props.p_far;
    
    computeProjectionMatrix();
    
    viewport = props.viewport;
}

void ViewCamera::show(igl::opengl::glfw::Viewer& v) {
    RowVector3d topleft = inverse_view2(RowVector3d(left, top, -near));
    RowVector3d topright = inverse_view2(RowVector3d(right, top, -near));
    RowVector3d bottomleft = inverse_view2(RowVector3d(left, bottom, -near));
    RowVector3d bottomright = inverse_view2(RowVector3d(right, bottom, -near));
    RowVector3d pos = position.cast<double>();
    
    RowVector3d red(1, 0, 0);
    RowVector3d black(0, 0, 0);
    
    MatrixXd p1, p2, c;
    p1.resize(8,3); p2.resize(8,3); c.resize(8,3);
    p1.row(0) = p1.row(1) = p1.row(2) = p1.row(3) = pos;
    p1.row(4) = topleft; p1.row(5) = topright;
    p1.row(6) = bottomright; p1.row(7) = bottomleft;
    
    p2.row(0) = p2.row(4) = topright; p2.row(1) = p2.row(5) = bottomright;
    p2.row(2) = p2.row(6) = bottomleft; p2.row(3) = p2.row(7) = topleft;
    
    int i;
    for(i=0; i<4; i++)
        c.row(i) = black;
    for(;i<p1.rows(); i++)
        c.row(i) = red;
    
    v.data().add_edges(p1, p2, c);
}

void ViewCamera::compute_frustum() {
    MatrixXd vp = ( proj * view ).cast<double>();
    // column2 + column3
    frustum.near.A = vp(2, 0) + vp(3, 0);
    frustum.near.B = vp(2, 1) + vp(3, 1);
    frustum.near.C = vp(2, 2) + vp(3, 2);
    frustum.near.D = vp(2, 3) + vp(3, 3);

    // column3 - column2
    frustum.far.A = -vp(2, 0) + vp(3, 0);
    frustum.far.B = -vp(2, 1) + vp(3, 1);
    frustum.far.C = -vp(2, 2) + vp(3, 2);
    frustum.far.D = -vp(2, 3) + vp(3, 3);

    // column1 + column3
    frustum.bottom.A = vp(1, 0) + vp(3, 0);
    frustum.bottom.B = vp(1, 1) + vp(3, 1);
    frustum.bottom.C = vp(1, 2) + vp(3, 2);
    frustum.bottom.D = vp(1, 3) + vp(3, 3);

    // column3 - column1 
    frustum.top.A = -vp(1, 0) + vp(3, 0);
    frustum.top.B = -vp(1, 1) + vp(3, 1);
    frustum.top.C = -vp(1, 2) + vp(3, 2);
    frustum.top.D = -vp(1, 3) + vp(3, 3);

    // column0 + column3
    frustum.left.A = vp(0, 0) + vp(3, 0);
    frustum.left.B = vp(0, 1) + vp(3, 1);
    frustum.left.C = vp(0, 2) + vp(3, 2);
    frustum.left.D = vp(0, 3) + vp(3, 3);

    // column3 - column0
    frustum.right.A = -vp(0, 0) + vp(3, 0);
    frustum.right.B = -vp(0, 1) + vp(3, 1);
    frustum.right.C = -vp(0, 2) + vp(3, 2);
    frustum.right.D = -vp(0, 3) + vp(3, 3);
    
    normalize_plane(frustum.near);
    normalize_plane(frustum.far);
    normalize_plane(frustum.top);
    normalize_plane(frustum.bottom);
    normalize_plane(frustum.left);
    normalize_plane(frustum.right);
}

void ViewCamera::normalize_plane(Plane& pl){
    float mag = sqrt(pl.A *pl.A + pl.B * pl.B + pl.C * pl.C);
    pl.A /= mag;
    pl.B /= mag;
    pl.C /= mag;
    pl.D /= mag;
}

double ViewCamera::point_plane_signed_distance(Vector3d p, Plane pl){
    return pl.A*p(0) + pl.B*p(1) + pl.C*p(2) + pl.D;
}

bool ViewCamera::frustum_culling(Vector3d p) {
    if(point_plane_signed_distance(p, frustum.near) < 0)
        return true;
    if(point_plane_signed_distance(p, frustum.far) < 0)
        return true;
    if(point_plane_signed_distance(p, frustum.top) < 0)
        return true;
    if(point_plane_signed_distance(p, frustum.bottom) < 0)
        return true;
    if(point_plane_signed_distance(p, frustum.left) < 0)
        return true;
    if(point_plane_signed_distance(p, frustum.right) < 0)
        return true;
    
    return false;
}

//bool ViewCamera::viewport_culling(Vector3d p) {
//    Vector2d projected = project_point(p);
//    return projected(0) < viewport(0) || projected(1) < viewport(1) || projected(0) > viewport(2) || projected(1) > viewport(3);
//}