#ifndef SMOOTHOCCLUSIONCONTOURSEXTRACTOR_H
#define SMOOTHOCCLUSIONCONTOURSEXTRACTOR_H

#include "CrossValueContoursExtractor.h"
#include "../Common/ViewCamera.h"

/**\brief Work in Progress. Extractor for interpolated occlusion contours, following [Benard, 2014]
 * \details A contour extractor is a pointer to a mesh and a list of contours on that mesh.
 * \author Bastien Wailly <bastien.wailly@inria.fr>, Inria*/
class SmoothOcclusionContoursExtractor : public CrossValueContoursExtractor{
public:
    SmoothOcclusionContoursExtractor(MeshObject& m, ViewCamera& cam):CrossValueContoursExtractor(m), camera(cam){};
    virtual ~SmoothOcclusionContoursExtractor(){};
    
    bool compute_cross_vertex(int face) override;
    
    void init(){
        compute_V_ndotv();
    }
    
    ContourType type() override { return ContourType::OCCLUSION; };    
    
private:
    ViewCamera& camera;
    
    MatrixXd V_ndotv;
    
    void compute_V_ndotv();
    void draw_isoline(int v0, int v1, int v2, int f);
};

#endif /* SMOOTHOCCLUSIONCONTOURSEXTRACTOR_H */

