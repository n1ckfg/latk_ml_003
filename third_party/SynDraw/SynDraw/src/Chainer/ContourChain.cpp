#include "ContourChain.h"
#include "../ViewGraph/ContourNode.h"
#include "../ViewGraph/VertexNode.h"
#include "../Tools/RaycastHelper.h"

#include <random>


ContourChain::ContourChain(std::shared_ptr<ContourNode> n) {
    ContourChain();
    contours.push_back(n);
    type = n->get_type();
    visibility = n->isUndefined() ? Visibility::UNKNOWN : (
                    n->isVisible() ? Visibility::VISIBLE : Visibility::OCCLUDED);
}

const bool ContourChain::isVisible() {
    return visibility == Visibility::VISIBLE;
}

void ContourChain::setVisible(bool b) {
    if (b)
        visibility = Visibility::VISIBLE;
    else
        visibility = Visibility::OCCLUDED;
}

void ContourChain::push_contour_back(std::shared_ptr<ContourNode> c) {
    contours.push_back(c);
}

void ContourChain::push_contour_front(std::shared_ptr<ContourNode> c) {
    contours.push_front(c);
}

void ContourChain::push_vertex_back(shared_ptr<VertexNode> v) {
    vertices.push_back(v);
}

void ContourChain::push_vertex_front(shared_ptr<VertexNode> v) {
    vertices.push_front(v);
}

const void ContourChain::Draw(svg::Document& doc, svg::Stroke& str) {
    if (vertices.size() == 0)
        return;

//    float h = doc.get_layout().dimensions.height;

    svg::Fill fill(svg::Color::None);

    svg::Polyline3d line(fill, str, typeToString(type));
    for (auto n : vertices) {
        Vector2d p = n->point2d();
        Vector3d v = n->point3d();
        svg::Point P(p(0), p(1));
        svg::Point3d V(v(0), v(1), v(2));
        line << P;
        line << V;
    }
    doc << line;
}

const void ContourChain::Draw(Lines& lines, Eigen::RowVector3d color){
    if (vertices.size() < 2)
        return;
    
    int i = lines.P1.rows() + 1;

    Vector3d prev = vertices.front()->point3d();
    for (auto v : vertices) {
        if (v == vertices.front())
            continue;
        
        lines.P1.conservativeResize(i, 3);
        lines.P2.conservativeResize(i, 3);
        lines.color.conservativeResize(i, 3);

        lines.P1.row(i - 1) = prev.transpose();
        lines.P2.row(i - 1) = v->point3d().transpose();
        lines.color.row(i - 1) = color;  
        
        prev = v->point3d();
        i++;
    }
}

void ContourChain::computeVisibility(double voting_factor, RaycastHelper& ray, double tol) {
    int N = int(voting_factor * contours.size());
    
    if(N < 1)
        return;
    
    std::random_device rd;
    std::mt19937 rng(rd());

    std::vector<std::shared_ptr<ContourNode>> tmp(contours.begin(), contours.end());
    std::shuffle(tmp.begin(), tmp.end(), rng);
    
    std::vector<bool> res;
    
    for(int i=0; i<N; i++){
        auto c = tmp[i];
        bool b = ray.isVisible(c, tol);
        res.push_back(b);
    }
    
    vote_for_visibility(res);
}

void ContourChain::vote_for_visibility(std::vector<bool> ballots) {
    int sum = 0;
    for(bool b: ballots) if(b) sum++; else sum--;
    
    setVisible(sum>0);
}
