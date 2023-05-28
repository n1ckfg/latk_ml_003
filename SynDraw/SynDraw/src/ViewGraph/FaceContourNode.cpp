#include "FaceContourNode.h"

FaceContourNode::FaceContourNode(int f):ContourNode(),_face_index(f) {
}

FaceContourNode::FaceContourNode(const FaceContourNode& orig) {
}

FaceContourNode::~FaceContourNode() {
}

std::shared_ptr<ContourNode> FaceContourNode::createChild() const {
    std::shared_ptr<FaceContourNode> child = std::make_shared<FaceContourNode>(_face_index);

    child->type = this->type;
    child->visibility = this->visibility;
    child->head = NULL;
    child->tail = NULL;

    return child;
}