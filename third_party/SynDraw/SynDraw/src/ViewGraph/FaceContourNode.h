#ifndef FACECONTOURNODE_H
#define FACECONTOURNODE_H

#include "ContourNode.h"
#include "../ContourExtractor/CrossStructures.h"

/**\brief Class for contours lying on mesh faces. 
 * \author Bastien Wailly <bastien.wailly@inria.fr>, Inria
 * \details Implements ContourNode. A face contour is composed of 2 vertices lying on mesh edges, 
 * called cross vertices. Two CrossVertex make up a CrossEdge. */
class FaceContourNode : public ContourNode {
public:
    FaceContourNode(int f);
    FaceContourNode(const FaceContourNode& orig);
    virtual ~FaceContourNode();
    
    /**\brief get mesh edge index where this contour lie 
     * \return edge index */    
    const int face_index() const { return _face_index; };
    
    /**\brief store original cross edge making up this contour */
    void setCrossEdge(CrossEdge c){ _crossEdge = c; };
    
    /**\brief get original cross edge making up this contour */    
    CrossEdge& getCrossEdge(){ return _crossEdge; };
    
    /**\brief create another face contour sharing properties 
     * \return pointer to newly created face contour
     * \details Same face index, visibility and type. 
     * Vertices are uninitialized */    
    std::shared_ptr<ContourNode> createChild() const;

    /**\brief get this class identifier for dynamic allocation 
     * \return string identifier */    
    const std::string typeinfo() const{ return "face_contour"; };
    
    bool visibility_filtered = false;
    bool hysteresis_filtered = false;

private:
    int _face_index;
    
    CrossEdge _crossEdge;

};

#endif /* FACECONTOURNODE_H */

