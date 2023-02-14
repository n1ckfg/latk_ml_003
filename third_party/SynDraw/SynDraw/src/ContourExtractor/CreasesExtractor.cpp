#include "CreasesExtractor.h"

#include <math.h>

CreasesExtractor::~CreasesExtractor() {
}


void CreasesExtractor::flip_ifObtuse (const RowVector3d &principalDirection_vertex1, RowVector3d &principalDirection_vertex2,
                            double &extremality_vertex2) const {
    if (principalDirection_vertex1.dot(principalDirection_vertex2) < 0.0) {
        principalDirection_vertex2 *= -1.0;
        extremality_vertex2 *= -1.0;
    }
}


void CreasesExtractor::add_CreaseVertex (unsigned int e, double extrema1, double extrema2, const VectorXd& curvature) {
    int v1 = mesh.E()(e,0);
    int v2 = mesh.E()(e,1);

    RowVector3d position = vector_interpolation(mesh.V().row(v1), mesh.V().row(v2), extrema1, extrema2);
    double valueThreshold = compute_threshold(v1, v2, extrema1, extrema2, curvature);
    add_crossVertex(e, position, valueThreshold);
}


double CreasesExtractor::compute_threshold (unsigned int v1, unsigned int v2, double extrema1, double extrema2, const VectorXd& curvature) {
    const double aex1 = fabs(extrema1);
    const double aex2 = fabs(extrema2);

    double threshold = aex2 * curvature(v1) +
            aex1 * curvature(v2);
    threshold /= aex1 + aex2;
    return fabs(threshold);
}

bool CreasesExtractor::isValueVanish (double discrepancy, double extrema1, double extrema2,
                                     double &new_extrema1, double &new_extrema2) const {
    bool extremalityVanish{false};

    double n1 = extrema1 - discrepancy;
    double n2 = extrema2 - discrepancy;

    if (n1 * n2 < 0) {
        new_extrema1 = n1;
        new_extrema2 = n2;
        extremalityVanish = true;
    } else {
        double p1 = extrema1 + discrepancy;
        double p2 = extrema2 + discrepancy;

        if (p1 * p2 < 0) {
            new_extrema1 = p1;
            new_extrema2 = p2;
            extremalityVanish = true;
        }
    }

    return extremalityVanish;
}