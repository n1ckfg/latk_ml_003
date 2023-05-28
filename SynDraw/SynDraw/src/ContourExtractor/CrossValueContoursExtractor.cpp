#include "CrossValueContoursExtractor.h"
#include "../ViewGraph/VertexNode.h"

CrossValueContoursExtractor::~CrossValueContoursExtractor() {
    _edge_cross_vertices.clear();
    _face_centroid_vertices.clear();
    _crossVertices.clear();
}

void CrossValueContoursExtractor::add_crossVertex(unsigned int index_edge, const RowVector3d &position, double valueThreshold) {
    const unsigned int v1 = mesh.E()(index_edge, 0);
    const unsigned int v2 = mesh.E()(index_edge, 1);
    
    CrossVertex crossVertex;
    crossVertex.edge_index = index_edge;
    crossVertex.position = position;
    crossVertex.valueThreshold = valueThreshold;
    
    _crossVertices[index_edge] = crossVertex;
}



shared_ptr<VertexNode> CrossValueContoursExtractor::get_centroidvertex_node(int f, RowVector3d v, ViewCamera& camera) {
    map<int, shared_ptr<VertexNode>>::iterator it = _face_centroid_vertices.find( f );

    if(it != _face_centroid_vertices.end()){
        return it->second;
    }

    shared_ptr<VertexNode> vnode = make_shared<VertexNode>(v.transpose(), camera);
    _face_centroid_vertices[f] = vnode;
    _vertices.push_back(vnode);
    return vnode;   
}

shared_ptr<VertexNode> CrossValueContoursExtractor::get_crossvertex_node(const CrossVertex& cv, ViewCamera& camera) {
    map<int, shared_ptr<VertexNode>>::iterator it = _edge_cross_vertices.find( cv.edge_index );

    if(it != _edge_cross_vertices.end()){
        return it->second;
    }

    shared_ptr<VertexNode> vnode = make_shared<VertexNode>(cv.position.transpose(), camera);
    _edge_cross_vertices[cv.edge_index] = vnode;
    _vertices.push_back(vnode);
    return vnode;  
}

