#include "EdgeContourNode.h"

EdgeContourNode::EdgeContourNode(int e): ContourNode(), _edge_index(e) {
}

EdgeContourNode::EdgeContourNode(const EdgeContourNode& orig) {
}

EdgeContourNode::~EdgeContourNode() {
}

std::shared_ptr<ContourNode> EdgeContourNode::createChild() const {
    std::shared_ptr<EdgeContourNode> child = std::make_shared<EdgeContourNode>(_edge_index);

    child->type = this->type;
    child->visibility = this->visibility;
    child->head = NULL;
    child->tail = NULL;

    return child;
}