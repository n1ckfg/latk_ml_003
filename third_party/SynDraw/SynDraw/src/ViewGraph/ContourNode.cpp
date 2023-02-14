#include "ContourNode.h"
#include "VertexNode.h"

#include <iostream>

ContourNode::ContourNode(): visibility(Visibility::UNKNOWN), head(NULL), tail(NULL) {
}

void ContourNode::setHead(std::shared_ptr<VertexNode> node) {
    head = node;
}

void ContourNode::setTail(std::shared_ptr<VertexNode> node) {
    tail = node;
}

const Vector2d& ContourNode::head_vertex2d() {
    return head->point2d();
}

const Vector3d& ContourNode::head_vertex3d() {
    return head->point3d();
}

const Vector3d& ContourNode::tail_vertex3d() {
    return tail->point3d();
}

const Vector2d& ContourNode::tail_vertex2d() {
    return tail->point2d();
}

// TODO might want to expose or control that as it is the chaining rule
bool ContourNode::isSimilar(std::shared_ptr<ContourNode> c) {
    // not similar if visibility of both contours is known and not the same
    if( visibility != Visibility::UNKNOWN and
        c->visibility != Visibility::UNKNOWN and
        visibility != c->visibility)
        return false;

    // not similar if of different type
    if(type != c->type)
        return false;

    return true;
}

void ContourNode::setVisible(bool v) {
//    if(visibility != ContourVisibility::UNKNOWN)
//        std::cout << "Warning: Overwriting node visibility !" << std::endl;
    if (v) visibility = Visibility::VISIBLE;
    else visibility = Visibility::OCCLUDED;
}

const void ContourNode::Draw(svg::Document& doc, svg::Stroke& str) {
    Vector2d p1 = head->point2d();
    Vector2d p2 = tail->point2d();
    Vector3d P1 = head->point3d();
    Vector3d P2 = tail->point3d();
    
    // why is viewport mirrored vertically ?!
    float h = doc.get_layout().dimensions.height;
    
    svg::Line3d line( svg::Point(p1(0), h-p1(1)), 
                    svg::Point(p2(0), h-p2(1)), 
                    svg::Point3d(P1(0), P1(1), P1(2)),
                    svg::Point3d(P2(0), P2(1), P2(2)), str, typeToString(type));
    doc << line;
}

const void ContourNode::Draw(Lines& lines, RowVector3d color) {
    int i = lines.P1.rows() + 1;

    lines.P1.conservativeResize(i, 3);
    lines.P2.conservativeResize(i, 3);
    lines.color.conservativeResize(i, 3);

    lines.P1.row(i - 1) = head_vertex3d().transpose();
    lines.P2.row(i - 1) = tail_vertex3d().transpose();
    lines.color.row(i - 1) = color;
}

std::list<float> ContourNode::getSplits() {
    std::list<float> v;
    for (auto it = intersections.begin(); it != intersections.end(); ++it) {
        v.push_back(it->first);
    }
    return v;
}
