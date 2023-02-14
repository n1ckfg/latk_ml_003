#include "VertexNode.h"
#include "ContourNode.h"
#include <algorithm>
#include <iostream>

VertexNode::VertexNode(){
    
}
VertexNode::VertexNode(MeshObject& m, ViewCamera& cam, int v): _type(VertexType::V_JUNCTION) {
    _point3d = m.V().row(v).transpose();
    _point2d = cam.project_point(_point3d);
}

VertexNode::VertexNode(Vector3d point, ViewCamera& cam): _point3d(point), _type(VertexType::V_JUNCTION) {
    _point2d = cam.project_point(_point3d);

}

VertexNode::VertexNode(const VertexNode& orig) {
}

VertexNode::~VertexNode() {
}

void VertexNode::addConnection(std::shared_ptr<ContourNode> c) {
    connections.insert(c);
}

void VertexNode::delConnection(std::shared_ptr<ContourNode> c) {
    connections.erase(c);
}


void VertexNode::setType(VertexType t){
//    if(_type != VertexType::UNKNOWN)
//        std::cout << "Warning: overriding vertex type." << std::endl;
    _type = t;
}

const bool VertexNode::isVisible() {
    // TODO find a solution to this crap
    for (auto c : connections)
        if (c->isVisible())
            return true;
    return false;
}

const void VertexNode::Draw(igl::opengl::glfw::Viewer& v, Eigen::RowVector3d color) {
    v.data().add_points(point3d().transpose(), color);
}

const void VertexNode::Draw(svg::Document& doc, svg::Stroke& str) {
    svg::Color c(0,0,0);
    if(_type == VertexType::CUSP){
        c = svg::Color(255, 0, 0);
    }else if(_type == VertexType::Y_JUNCTION){
        c = svg::Color(0, 255, 0);
    }else if(_type == VertexType::X_JUNCTION){
        c = svg::Color(255, 26, 220);
    }else if(_type == VertexType::T_JUNCTION){
        c = svg::Color(0, 0, 255);
    }else if(_type == VertexType::BOUNDARY_CUSP){
        c = svg::Color(255, 150, 80);
    }    
    svg::Fill fill(c);
    
    svg::Point o = { _point2d(0), _point2d(1) };
    
    svg::Circle circle(o, 2, fill, str);
    
    doc << circle;
}
