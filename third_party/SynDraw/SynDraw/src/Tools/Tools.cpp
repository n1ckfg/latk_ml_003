#include "Tools.h"
#include "../Common/MeshObject.h"
#include "../Common/ViewCamera.h"
#include "../ViewGraph/ContourNode.h"
#include "../ViewGraph/VertexNode.h"

#include <iostream>

// is d left or right of the plane defined by a, b, c ?
double Tools::orient3d(Vector3d a, Vector3d b, Vector3d c, Vector3d d){
    Matrix4d mat;
    
    mat << a.transpose(), 1.0,
           b.transpose(), 1.0,
           c.transpose(), 1.0,
           d.transpose(), 1.0;
    
    return mat.determinant();
}

// returns true if d and e are on the same side of triangle abc
bool Tools::sameside3d(Vector3d a, Vector3d b, Vector3d c, Vector3d d, Vector3d e){
    return (orient3d(a,b,c,d) > 0) == (orient3d(a,b,c,e) > 0);
}

// returns a positive number if d is front facing triangle abc
double Tools::frontside3d(Vector3d a, Vector3d b, Vector3d c, Vector3d d){
    return orient3d(d,b,c,a);
}

// returns true if edge ab made by triangles abd and bae is concave
bool Tools::is_concave3d(Vector3d a, Vector3d b, Vector3d d, Vector3d e){
    return Tools::frontside3d(a, b, d, e) > 0;
}

double Tools::concavity(MeshObject& mesh, int e){
    int f1 = mesh.EF()(e, 0);
    int f2 = mesh.EF()(e, 1);
    
    Vector3d centre_f1 = (mesh.V().row(mesh.F()(f1,0)) + mesh.V().row(mesh.F()(f1, 1)) + mesh.V().row(mesh.F()(f1, 2))) / 3;
    Vector3d centre_f2 = (mesh.V().row(mesh.F()(f2,0)) + mesh.V().row(mesh.F()(f2, 1)) + mesh.V().row(mesh.F()(f2, 2))) / 3;
    
    Vector3d w = centre_f2 - centre_f1;
    
    return w.dot(mesh.F_normals().row(f1));
}

bool Tools::is_edge_concave(MeshObject& mesh, int edge){
    int va = mesh.E()(edge, 0);
    int vb = mesh.E()(edge, 1);
    
    RowVector3i vf1 = mesh.F().row(mesh.EF()(edge, 0));
    RowVector3i vf2 = mesh.F().row(mesh.EF()(edge, 1));

    int vd, ve;
    for(int i=0; i<3; i++){
        if(vf1(i) != va and vf1(i) != vb)
            vd = vf1(i);
        
        if(vf2(i) != va and vf2(i) != vb)
            ve = vf2(i);
    }
    
    Vector3d a = mesh.V().row(va).transpose();
    Vector3d b = mesh.V().row(vb).transpose();
    Vector3d d = mesh.V().row(vd).transpose();
    Vector3d e = mesh.V().row(ve).transpose();
    
    return is_concave3d(a, b, d, e);
}

double Tools::ndotv_face(MeshObject& mesh, ViewCamera& cam, int f){
    Vector3d centre = (mesh.V().row(mesh.F()(f,0)) + mesh.V().row(mesh.F()(f, 1)) + mesh.V().row(mesh.F()(f, 2))) / 3;
    Vector3d viewDir = cam.get_view_direction(centre).transpose();
    return viewDir.dot(mesh.F_normals().row(f));
}

bool Tools::is_face_backfacing(MeshObject& mesh, ViewCamera& cam, int f){
     return ndotv_face(mesh, cam, f) > 0; 
}

double Tools::orient2d(Vector2d a, Vector2d b, Vector2d c){
    Matrix3d mat;
    
    mat << a.transpose(), 1.0,
           b.transpose(), 1.0,
           c.transpose(), 1.0;
    
    return mat.determinant();
}

bool Tools::sameside2d(Vector2d a, Vector2d b, Vector2d c, Vector2d d){
    return (orient2d(a,b,c) > 0) == (orient2d(a,b,d) > 0);
}

double Tools::get_angle(std::shared_ptr<ContourNode> c1, std::shared_ptr<ContourNode> c2){
    std::shared_ptr<VertexNode> a = c1->getHead();
    std::shared_ptr<VertexNode> b = c1->getTail();
    std::shared_ptr<VertexNode> c = c2->getHead();
    std::shared_ptr<VertexNode> d = c2->getTail();
    
    Vector2d u, v;
    if(a == c or b == d){
        u = b->point2d() - a->point2d();
        v = c->point2d() - d->point2d();
    }
    else{
        u = b->point2d() - a->point2d();
        v = d->point2d() - c->point2d();        
    }
    
    u.normalize();
    v.normalize();
    
    double cosa = u.dot(v);
    if(cosa < -1) cosa = -1;
    else if(cosa > 1) cosa = 1;
    return acos(cosa);
}

bool Tools::isApproxZero(double d, double tol){ 
    return abs(d) < tol; 
}  


void Tools::connect(shared_ptr<ContourNode> c, shared_ptr<VertexNode> v, Side side) {
    if(v == NULL){
        cout << "% Error : connecting a NULL vertex ! Aborting..." << endl;
        return;
    }
    if(c == NULL){
        cout << "% Error : connecting a NULL contour ! Aborting..." << endl;
        return;
    }

    v -> addConnection(c);
    if(side == Side::HEAD){
        c -> setHead(v);
    }
    else if(side == Side::TAIL){
        c -> setTail(v);
    }
}

void Tools::disconnect(shared_ptr<ContourNode> c) {
    c->getHead()->delConnection(c);
    c->getTail()->delConnection(c);
    c->setHead(NULL);
    c->setTail(NULL);
}
