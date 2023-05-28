#ifndef OCCLUSIONCONTOURSEXTRACTOR_H
#define OCCLUSIONCONTOURSEXTRACTOR_H

#include <Eigen/Core>

#include "ContourExtractor.h"
#include "../Common/ViewCamera.h"

using namespace Eigen;

/**\brief ContourExtractor sub-class for standard occlusion contours.
 * \details Occlusion contours are mesh edges where facets normal shifts from looking towards the camera (N.V) to looking in the same general direction of the camera.
 * \author Bastien Wailly <bastien.wailly@inria.fr>, Inria*/
class OcclusionContoursExtractor : public ContourExtractor{
public:
    OcclusionContoursExtractor(MeshObject& obj, ViewCamera& cam);
    virtual ~OcclusionContoursExtractor();
    
    /**\brief Check if an edge is an occlusion contour 
     * \param i mesh edge index 
     * \return true if edge i is a silhouette/occlusion */
    virtual bool is_contour(int i);
    
    /**\brief Fast visibility test for occlusion contours 
     * \param e mesh edge index
     * \return true if mesh edge e is occluded 
     * \details Described in [Benard, 2019] chapt. 4.2 */
    bool is_occluded(int e);      
    
    /**\brief extractor init */
    void init(){
        compute_F_ndotv();
    }

    ContourType type() override { return ContourType::OCCLUSION; } ;

protected:
    ViewCamera& camera;
    MatrixXd ndotv;
    
    /**\brief compute view dependent normals of mesh */
    void compute_F_ndotv();

};

#endif /* OCCLUSIONCONTOURSEXTRACTOR_H */

