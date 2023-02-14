#include "ValleysExtractor.h"

ValleysExtractor::ValleysExtractor(MeshObject& m):CreasesExtractor(m){
}

ValleysExtractor::~ValleysExtractor() {
}

bool ValleysExtractor::compute_cross_vertex(int e) {
    const unsigned int v1 = mesh.E()(e, 0);
    const unsigned int v2 = mesh.E()(e, 1);

    RowVector3d dirmin1 = mesh.principal_direction_min().row(v1);
    RowVector3d dirmin2 = mesh.principal_direction_min().row(v2);

    double extrema2 = extremas[v2];

    flip_ifObtuse(dirmin1, dirmin2, extrema2);

    bool is_cross_vertex = mesh.principal_curvature_min()(v1) < - fabs(mesh.principal_curvature_max()(v1));
    is_cross_vertex &= mesh.principal_curvature_min()(v2) < - fabs(mesh.principal_curvature_max()(v2));
    is_cross_vertex &= (extremas[v1] * extrema2) < 0;
    is_cross_vertex &= is_extrema(v1, v2, extremas[v1], extrema2, dirmin1, dirmin2);

    if (is_cross_vertex) {        
        add_CreaseVertex(e, extremas[v1], extrema2, mesh.principal_curvature_min());
        return true;
    }  
    return false;
}

bool ValleysExtractor::is_extrema(unsigned int v1, unsigned int v2, double extrema1, double extrema2, const RowVector3d& pdir_min1, const RowVector3d& pdir_min2) {
    RowVector3d sub_edgeVertices = mesh.V().row(v2) - mesh.V().row(v1);
    double value_checked = sub_edgeVertices.dot(pdir_min1);
    value_checked *= extrema1;

    bool attainsAmaximumFound = value_checked < 0;

    if (!attainsAmaximumFound) {
        sub_edgeVertices = mesh.V().row(v1) - mesh.V().row(v2);
        value_checked = sub_edgeVertices.dot(pdir_min2);
        value_checked *= extrema2;
        attainsAmaximumFound = value_checked < 0;
    }
    return attainsAmaximumFound;
}
