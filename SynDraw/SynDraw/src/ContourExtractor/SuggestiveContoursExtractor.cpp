#include "SuggestiveContoursExtractor.h"
#include "../Tools/ContourFilter.h"

SuggestiveContoursExtractor::~SuggestiveContoursExtractor() {
}

SuggestiveContoursExtractor::SuggestiveContoursExtractor(MeshObject& m, ViewCamera& c, double s):CrossValueContoursExtractor(m), camera(c), maximum_speed(s) {
}

/* "Suggestive Contours for Conveying Shape"
// * Section 3.1 */
bool SuggestiveContoursExtractor::compute_cross_vertex(int e) {
    int v1 = mesh.E()(e, 0);
    int v2 = mesh.E()(e, 1);
    
    // filter in edges with negative gaussian curvature
//    if( mesh.gaussian_curvature()(v1) <= 0 or mesh.gaussian_curvature()(v2) <= 0 ){
        // discarding not visible edges
        if( vertex_visible(v1) or vertex_visible(v2) ){
            // check for kr zero crossing
            if(mesh.radial_curvature()(v1) * mesh.radial_curvature()(v2) <= 0.0){
                const RowVector3d position = vector_interpolation(mesh.V().row(v1), mesh.V().row(v2), mesh.radial_curvature()(v1), mesh.radial_curvature()(v2));
                const RowVector3d sug_vertex_viewdir = camera.get_view_direction(position);                
                // temporal coherence
                const double speed = get_speed(v1, v2, sug_vertex_viewdir);
                if (speed < maximum_speed){
                /* Suggestive Contours for Conveying Shape
                 * Section 2.3
                 * Remove parts of the suggestive contours generator
                 * that are too unstable to give meaningful information about the shape */                    
                    const double d = ndotv(v1, v2, sug_vertex_viewdir);
                    add_crossVertex(e, position, d);
                    return true;
                }
            }
//        }
    }
    return false;
}


double SuggestiveContoursExtractor::get_speed (unsigned int v1, unsigned int v2, const RowVector3d &vdir) {
    const double gcurv = value_interpolation(mesh.gaussian_curvature()(v1), mesh.gaussian_curvature()(v2), mesh.radial_curvature()(v1), mesh.radial_curvature()(v2));
    double result;
    if (gcurv >= 0) {
        result = 0.0;
    } else {
        const RowVector3d proj_dir = vector_interpolation(projected_view_direction(v1), projected_view_direction(v2),
                                                                             mesh.radial_curvature()(v1), mesh.radial_curvature()(v2));

        const RowVector3d g_rcurv = vector_interpolation(mesh.radial_curvature_gradient().row(v1), mesh.radial_curvature_gradient().row(v2),
                                                                                mesh.radial_curvature()(v1), mesh.radial_curvature()(v2));
        result = 2.0 * sqrt(-gcurv) / proj_dir.norm();
        result /= g_rcurv.norm();
        result *= vdir.norm();
    }
    return result;
}

RowVector3d SuggestiveContoursExtractor::projected_view_direction(int i){
    return camera.get_proj_view_direction(mesh.V().row(i), mesh.V_normals().row(i));
}

double SuggestiveContoursExtractor::ndotv(int v1, int v2, const RowVector3d &viewDirection) {
    RowVector3d normal = vector_interpolation(mesh.V_normals().row(v1), mesh.V_normals().row(v2),
                                                   mesh.radial_curvature()(v1), mesh.radial_curvature()(v2));
    normal.normalize();
    return normal.dot(viewDirection.normalized());
}

void SuggestiveContoursExtractor::add_crossVertex(unsigned int index_edge, const RowVector3d& position, double valueThreshold) {
    const unsigned int v1 = mesh.E()(index_edge, 0);
    const unsigned int v2 = mesh.E()(index_edge, 1);
    
    CrossVertex crossVertex;
    crossVertex.edge_index = index_edge;
    crossVertex.position = position;
    crossVertex.valueThreshold = valueThreshold;
        
    crossVertex.gradient = vector_interpolation(mesh.radial_curvature_gradient().row(v1), mesh.radial_curvature_gradient().row(v2),
                                                  mesh.radial_curvature()(v1),              mesh.radial_curvature()(v2));
    crossVertex.gradient.normalize();
    crossVertex.normal = vector_interpolation(mesh.V_normals().row(v1),   mesh.V_normals().row(v2),
                                                mesh.radial_curvature()(v1),        mesh.radial_curvature()(v2));
    crossVertex.normal.normalize();   
    
    _crossVertices[index_edge] = crossVertex;    
}
