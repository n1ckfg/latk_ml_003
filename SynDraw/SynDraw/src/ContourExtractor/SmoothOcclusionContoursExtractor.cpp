#include "SmoothOcclusionContoursExtractor.h"

void SmoothOcclusionContoursExtractor::compute_V_ndotv(){
    // using vertex based normals
    V_ndotv = VectorXd(mesh.V().rows());

    for (unsigned int i = 0; i < mesh.V().rows(); ++i) 
    {
        V_ndotv(i) = mesh.V_normals().row(i).dot(camera.get_view_direction(mesh.V().row(i)));
    }
}

void SmoothOcclusionContoursExtractor::draw_isoline(int v0, int v1, int v2, int f) {
    int e0=-1, e1=-1;
    
    for(int i=0; i<3; i++){
        int e = mesh.FE()(f, i);
        int ev0 = mesh.E()(e, 0);
        int ev1 = mesh.E()(e, 1);
        
        if( (ev0 == v0 or ev0 == v1) and (ev1 == v0 or ev1 == v1) )
            e0 = e;
        else if( (ev0 == v0 or ev0 == v2) and (ev1 == v0 or ev1 == v2) )
            e1 = e;
    }
    
    // How far along each edge?
    double w10 = V_ndotv(v0) / (V_ndotv(v0) - V_ndotv(v1));
    double w20 = V_ndotv(v0) / (V_ndotv(v0) - V_ndotv(v2));

    // Points of zero crossing
    RowVector3d p0 = (1 - w10) * mesh.V().row(v0) + w10 * mesh.V().row(v1);
    RowVector3d p1 = (1 - w20) * mesh.V().row(v0) + w20 * mesh.V().row(v2);
    
    add_crossVertex(e0, p0, -1);
    add_crossVertex(e1, p1, -1);
}

bool SmoothOcclusionContoursExtractor::compute_cross_vertex(int face) {
    int v0 = mesh.F()(face,0),  v1 = mesh.F()(face,1),  v2 = mesh.F()(face,2);
    //backface culling
//    if (V_ndotv(v0) < 0 && V_ndotv(v1) < 0 && V_ndotv(v2) < 0)
//        return false;

    bool pv0 = V_ndotv(v0) > 0, pv1 = V_ndotv(v1) > 0, pv2 = V_ndotv(v2) > 0;

    if (pv0 != pv1 || pv1 != pv2) {
        // Figure out which val has different sign, and draw
        //v0 has a different sign
        if ((pv0 && !pv1 && !pv2) || (!pv0 && pv1 && pv2))
            draw_isoline(v0, v1, v2, face);
        //v1 has a different sign
        else if ((pv1 && !pv0 && !pv2) || (!pv1 && pv0 && pv2))
            draw_isoline(v1, v2, v0, face);
        //v2 has a different sign
        else if ((pv2 && !pv1 && !pv0) || (!pv2 && pv1 && pv0))
            draw_isoline(v2, v0, v1, face);
        
        return true;
    }
    
    return false;
}
