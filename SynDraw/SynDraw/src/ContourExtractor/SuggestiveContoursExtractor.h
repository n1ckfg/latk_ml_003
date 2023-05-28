#ifndef SUGGESTIVECONTOURSEXTRACTOR_H
#define SUGGESTIVECONTOURSEXTRACTOR_H

#include <Eigen/Core>
#include "../Common/MeshObject.h"
#include "../Common/ViewCamera.h"
#include "../Tools/Gradient.h"
#include "../ViewGraph/FaceContourNode.h"
#include "CrossStructures.h"
#include "SharpFeaturesExtractor.h"
#include "CrossValueContoursExtractor.h"

#include <vector>
#include <unordered_map>

/**\brief Extractor class for suggestive contours, following [DeCarlo 2003].
 * \details Suggestive contours are located at zero crossings of the radial curvature. 
 * They are therefore view-dependent, contrary to all other contours.
 * \author Bastien Wailly <bastien.wailly@inria.fr>, Inria*/
class SuggestiveContoursExtractor : public CrossValueContoursExtractor{
public:
    SuggestiveContoursExtractor(MeshObject& m, ViewCamera& c, double s);    
    virtual ~SuggestiveContoursExtractor();
    
    /**\brief Check if mesh edge contains a suggestive cross vertex. 
     * \details  Section 3.1 */
    bool compute_cross_vertex(int e) override;
    
    /**\brief init mesh before extraction, i.e. compute radial and gaussian curvature. */
    void init(){
        mesh.compute_radial_curvature(camera);
        mesh.gaussian_curvature();
    }
    
    ContourType type() override { return ContourType::SUGGESTIVE; } ;
    

private:
    ViewCamera& camera;
    
    double maximum_speed = 1.0;
    
    /**\brief Interactive Rendering of Suggestive Contours with Temporal Coherence, [DeCarlo 2004]
    * \details Section 3.3\n
    * We further discard suggestive contours that move too quickly across the surface */
    double get_speed (unsigned int v1, unsigned int v2, const RowVector3d &vdir);
    
    void add_crossVertex(unsigned int index_edge, const RowVector3d& position, double valueThreshold) override;
    
    bool vertex_visible (int i) {
        RowVector3d v = camera.get_view_direction(mesh.V().row(i));
        return (v.dot(mesh.V_normals().row(i)) <= 0);
    }

    double ndotv (int v1, int v2, const RowVector3d &viewDirection); 
    RowVector3d projected_view_direction(int i);
};

#endif /* SUGGESTIVECONTOURSEXTRACTOR_H */

