#ifndef EDGECONTOURNODE_H
#define EDGECONTOURNODE_H

#include "ContourNode.h"

/**\brief Class for contours lying on mesh edges. 
 * \author Bastien Wailly <bastien.wailly@inria.fr>, Inria
 * \details Implements ContourNode */
class EdgeContourNode : public ContourNode {
public:
    EdgeContourNode(int e);
    EdgeContourNode(const EdgeContourNode& orig);
    virtual ~EdgeContourNode();
    
    /**\brief get mesh edge index where this contour lie 
     * \return edge index */
    const int edge_index() const { return _edge_index; };
 
    /**\brief create another edge contour sharing properties 
     * \return pointer to newly created edge contour
     * \details Same edge index, visibility and type. 
     * Vertices are uninitialized */
    std::shared_ptr<ContourNode> createChild() const;
    
    /**\brief get this class identifier for dynamic allocation 
     * \return string identifier */
    const std::string typeinfo() const{ return "edge_contour"; };    
        
private:
    int _edge_index;
};

#endif /* EDGECONTOURNODE_H */

