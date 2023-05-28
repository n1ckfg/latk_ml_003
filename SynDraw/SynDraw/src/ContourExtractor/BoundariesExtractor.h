#ifndef BOUNDARIESEXTRACTOR_H
#define BOUNDARIESEXTRACTOR_H

#include "OcclusionContoursExtractor.h"


/**\brief Extractor for boundary contours (mesh frontiers).
 * \author Bastien Wailly <bastien.wailly@inria.fr>, Inria*/
class BoundariesExtractor : public OcclusionContoursExtractor{
public:
    BoundariesExtractor(MeshObject& obj, ViewCamera& cam):OcclusionContoursExtractor(obj, cam){};
    virtual ~BoundariesExtractor(){};
    
    /**\brief Check if an edge is a boundary (onyl one adjacent face). 
     * \param e mesh edge index
     * \param return true if edge e is a boundary. */
    bool is_contour(int e) override;
    
    ContourType type() override { return ContourType::BOUNDARY; } ;


private:

};

#endif /* BOUNDARIESEXTRACTOR_H */

