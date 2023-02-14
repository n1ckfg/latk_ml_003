#include "RidgesExtractor.h"

RidgesExtractor::RidgesExtractor(MeshObject& m):CreasesExtractor(m){
}

RidgesExtractor::~RidgesExtractor() {
}

bool RidgesExtractor::compute_cross_vertex(int e) {
    const unsigned int v1 = mesh.E()(e, 0);
    const unsigned int v2 = mesh.E()(e, 1);

    RowVector3d dirmax1 = mesh.principal_direction_max().row(v1);
    RowVector3d dirmax2 = mesh.principal_direction_max().row(v2);

    double extrema2 = extremas[v2];

    flip_ifObtuse(dirmax1, dirmax2, extrema2);

    /* Section 3 : Tracing Ridges
     * Next we check the following conditions:
     * kmax(v) > |k min (v)| for v = v1 , v2 and emax (v1) * emax (v2) < 0 */
    bool is_cross_vertex = mesh.principal_curvature_max()(v1) > fabs(mesh.principal_curvature_min()(v1));
    is_cross_vertex &= mesh.principal_curvature_max()(v2) > fabs(mesh.principal_curvature_min()(v2));
    is_cross_vertex &= (extremas[v1] * extrema2) < 0;
    is_cross_vertex &= is_extrema(v1, v2, extremas[v1], extrema2, dirmax1, dirmax2);

    if (is_cross_vertex) {    
        add_CreaseVertex(e, extremas[v1], extrema2, mesh.principal_curvature_max());
        return true;
    }    
    return false;
}

/* Section 3 : Tracing Ridges
 * Finally we apply a simple derivative test :
 * emax (vi) (v 3−i − v i ) * t max (vi) > 0 with i = 1 or 2 */
bool RidgesExtractor::is_extrema (unsigned int v1, unsigned int v2,
                                     double extrema1, double extrema2,
                                     const RowVector3d &pdir_max1, const RowVector3d &pdir_max2) {

    RowVector3d sub_edgeVertices = mesh.V().row(v2) - mesh.V().row(v1);
    double value_checked = sub_edgeVertices.dot(pdir_max1);
    value_checked *= extrema1;

    bool attainsAmaximumFound = value_checked > 0;

    if (!attainsAmaximumFound) {
        sub_edgeVertices = mesh.V().row(v1) - mesh.V().row(v2);
        value_checked = sub_edgeVertices.dot(pdir_max2);
        value_checked *= extrema2;
        attainsAmaximumFound = value_checked > 0;
    }
    return attainsAmaximumFound;
}
