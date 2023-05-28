#ifndef SHARPFEATURESEXTRACTOR_H
#define SHARPFEATURESEXTRACTOR_H

#include "ContourExtractor.h"
#include "../Common/ViewCamera.h"
#include <unordered_set>

/**\brief Extractor class for sharp features.
 * \details Sharp features are (convex) edges whose dihedral angle is inferior to a threshold.
 * \author Bastien Wailly <bastien.wailly@inria.fr>, Inria*/
class SharpFeaturesExtractor : public ContourExtractor{
public:
    SharpFeaturesExtractor(MeshObject& obj, float angle);
    virtual ~SharpFeaturesExtractor();
    
    /**\brief Check if a mesh edge qualifies as sharp edge. 
     * \param e mesh edge index */
    bool is_contour(int e);
    
    /**\brief fast visibility computation for mesh edge e 
     * \details Described in [Benard, 2019] chapt. 4.2 
     * \return true if edge e is not visible by the camera */
    bool is_occluded(ViewCamera& cam, int e);   
    
    ContourType type() override { return ContourType::SHARP; } ;
    
    /**\brief Check if a face is sharp. 
     * \details A face is considered sharp if its 3 vertices are not part of sharp edges. */
    inline bool is_face_sharp (unsigned int index_face) {
        return (sharpVertices.count(mesh.F()(index_face, 0)) == 0) &&
               (sharpVertices.count(mesh.F()(index_face, 1)) == 0) &&
               (sharpVertices.count(mesh.F()(index_face, 2)) == 0);
    }    
private:
    std::unordered_set<unsigned int> sharpVertices;
    float threshold_angle;
};

#endif /* SHARPFEATURESEXTRACTOR_H */

