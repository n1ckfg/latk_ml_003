#include <igl/PI.h>
#include <iostream>

#include "DemarcatingCurvesExtractor.h"

DemarcatingCurvesExtractor::DemarcatingCurvesExtractor(MeshObject& m) : CrossValueContoursExtractor(m) {

}

DemarcatingCurvesExtractor::~DemarcatingCurvesExtractor() {
}

/* Section 3.2 : Computing Demarcating Curves on meshes */
bool DemarcatingCurvesExtractor::compute_cross_vertex(int index_face) {
    auto indices_verticesSimilarity = find_verticesWithCurvatureGradientSimilarity(index_face);

    /* Section 3.2 : Calculating Demarcating Curves
     * Faces whose three gradient vectors differ considerably are eliminated from further consideration */
    const bool faceToBeDiscarded = (indices_verticesSimilarity.size() == 0);

    if (!faceToBeDiscarded) {
        auto curvatureGradientAngles_oneFace = compute_curvatureGradientAnglesForOneFace(index_face, indices_verticesSimilarity,
                mesh.derivative_of_curvature(),
                mesh.principal_direction_min(), mesh.principal_direction_max());

        map<unsigned int, double> curvatureInCurvatureGradientDirection =
                compute_curvatureInCurvatureGradientDirectionOneFace(index_face, curvatureGradientAngles_oneFace);

        find_demarcatingCurvePoints_forOneFace(index_face, curvatureInCurvatureGradientDirection);
    }
    return true;
}

/* Section 3.2 : Calculating Demarcating Curves */
void DemarcatingCurvesExtractor::add_crossVertex(unsigned int index_vertex1, unsigned int index_vertex2, unsigned int index_edge,
        double curvatureInCurvatureGradientDirection_vertex1, double curvatureInCurvatureGradientDirection_vertex2) {

    /* The exact location of the demarcating curve point
     * is obtained by linear interpolation of the curvature values */
    const RowVector3d position = vector_interpolation(mesh.V().row(index_vertex1), mesh.V().row(index_vertex2),
            curvatureInCurvatureGradientDirection_vertex1,
            curvatureInCurvatureGradientDirection_vertex2);

    /* strength parameter */
    const double valueThreshold = (fabs(curvatureInCurvatureGradientDirection_vertex1) +
            fabs(curvatureInCurvatureGradientDirection_vertex2)) * 0.5;

#pragma omp critical
    {
        CrossValueContoursExtractor::add_crossVertex(index_edge, position, valueThreshold);
    }
}

void DemarcatingCurvesExtractor::find_demarcatingCurvePoints_forOneFace(unsigned int index_face,
        const map<unsigned int, double> &curvatureInCurvatureGradientDirection) {
    const MatrixXi &Edges = mesh.E();
    const MatrixXi &Face_to_Edges = mesh.FE();

    for (unsigned int i = 0; i < 3; ++i) {
        const unsigned int index_edge = Face_to_Edges(index_face, i);

        const unsigned int index_vertex1 = Edges(index_edge, 0);
        const unsigned int index_vertex2 = Edges(index_edge, 1);

        /* Section 3.2
         * The zero crossings of k(gp) on the mesh faces are computed according to
         * Definition 3.2 to create the demarcating curves */
        const bool demarcatingCurveVertexFound = curvatureInCurvatureGradientDirection.at(index_vertex1) *
                curvatureInCurvatureGradientDirection.at(index_vertex2) < 0.0;
        if (demarcatingCurveVertexFound) {
            add_crossVertex(index_vertex1, index_vertex2, index_edge,
                    curvatureInCurvatureGradientDirection.at(index_vertex1), curvatureInCurvatureGradientDirection.at(index_vertex2));
        }
    }
}

vector<unsigned int> DemarcatingCurvesExtractor::find_verticesWithCurvatureGradientSimilarity(unsigned int f) const {
    const double discrepancyMaximum = igl::PI / 4.0;
    vector<unsigned int> indices_verticesSimilarity;

    for (unsigned int i = 0; i < 3; ++i) {
        unsigned int v1 = mesh.F()(f, i);
        unsigned int v2 = mesh.F()(f, (i + 1) % 3);
        const double discrepancy_curvatureGradient = fabs(mesh.curvature_gradient_angles()(v1) - mesh.curvature_gradient_angles()(v2));

        if (discrepancy_curvatureGradient < discrepancyMaximum) {
            indices_verticesSimilarity.push_back(i);
        }
    }

    return indices_verticesSimilarity;
}

ArrayXd DemarcatingCurvesExtractor::compute_curvatureGradientAnglesForOneFace(unsigned int index_face, const vector<unsigned int> &indices_verticesSimilarity,
        const MatrixXd &derivativeOfCurvatures,
        const MatrixXd &principalDirection_min, const MatrixXd &principalDirection_max) {
    ArrayXd curvatureGradientAngles_oneFace{3};

    /* Section 3.2
     * For faces in which the gradients of only two vertices are similar,
     * the average gradient of the two similar vertices is selected
     * and the curvature of the third vertex is computed in this direction
     * Obviously, when this gradient is used for the third vertex,
     * it should be rotated so as to coincide with the vertex’s tangent plane */
    if (indices_verticesSimilarity.size() == 1) {
        const unsigned int index_vertexSimilarFace_one = indices_verticesSimilarity[0];
        const unsigned int index_vertexSimilarFace_two = (index_vertexSimilarFace_one + 1) % 3;
        const unsigned int index_differentVertexFace = (index_vertexSimilarFace_one + 2) % 3;

        const unsigned int index_vertexSimilar_one = mesh.F()(index_face, index_vertexSimilarFace_one);
        const unsigned int index_vertexSimilar_two = mesh.F()(index_face, index_vertexSimilarFace_two);

        curvatureGradientAngles_oneFace(index_vertexSimilarFace_one) = mesh.curvature_gradient_angles()(index_vertexSimilar_one);
        curvatureGradientAngles_oneFace(index_vertexSimilarFace_two) = mesh.curvature_gradient_angles()(index_vertexSimilar_two);

        curvatureGradientAngles_oneFace(index_differentVertexFace) = compute_averageCurvatureGradientDirection
                (index_vertexSimilarFace_one, index_vertexSimilarFace_two, index_differentVertexFace,
                derivativeOfCurvatures, principalDirection_min, principalDirection_max);
    } else {
        for (unsigned int i = 0; i < 3; ++i) {
            unsigned int index_vertex = mesh.F()(index_face, i);
            curvatureGradientAngles_oneFace(i) = mesh.curvature_gradient_angles()(index_vertex);
        }
    }

    return curvatureGradientAngles_oneFace;
}

double DemarcatingCurvesExtractor::compute_averageCurvatureGradientDirection(unsigned int index_vertex1, unsigned int index_vertex2, unsigned int index_vertexAverage,
        const MatrixXd &derivativeOfCurvatures,
        const MatrixXd &principalDirection_min, const MatrixXd &principalDirection_max) {

    RowVector4d derivativeOfCurvaturesVertex1_inVertexAverage =
            mesh.curvature_derivative_computer().compute_coefficients_in_reference_frame(principalDirection_min.row(index_vertex1),
            principalDirection_max.row(index_vertex1), derivativeOfCurvatures.row(index_vertex1),
            principalDirection_min.row(index_vertexAverage), principalDirection_max.row(index_vertexAverage));

    RowVector4d derivativeOfCurvaturesVertex2_inVertexAverage =
            mesh.curvature_derivative_computer().compute_coefficients_in_reference_frame(principalDirection_min.row(index_vertex2),
            principalDirection_max.row(index_vertex2), derivativeOfCurvatures.row(index_vertex2),
            principalDirection_min.row(index_vertexAverage), principalDirection_max.row(index_vertexAverage));

    RowVector4d newDerivativeOfCurvatures_inVertexAverage = (derivativeOfCurvaturesVertex1_inVertexAverage +
            derivativeOfCurvaturesVertex2_inVertexAverage) * 0.5;

    double newCurvatureGradientAngle_inVertexAverage =
            mesh.compute_curvatureGradientAngle_oneVertex(newDerivativeOfCurvatures_inVertexAverage(0), newDerivativeOfCurvatures_inVertexAverage(1),
            newDerivativeOfCurvatures_inVertexAverage(2), newDerivativeOfCurvatures_inVertexAverage(3));

    return newCurvatureGradientAngle_inVertexAverage;
}

/* Section 3.1 : Defining demarcating Curves
 * The normal curvature in direction v is k(v) = vt * II * v
 * where II is the second fundamental form
 *
 * Definition 3.2
 * p is as a demarcating curve point if the following holds at p : k(gp) = gpt * II * gp = 0  */
map<unsigned int, double> DemarcatingCurvesExtractor::compute_curvatureInCurvatureGradientDirectionOneFace(unsigned int index_face, const Array3d &curvatureGradientAngles) const {
    map<unsigned int, double> curvatureInCurvatureGradientDirection;

    for (unsigned int face_corner = 0; face_corner < 3; ++face_corner) {
        unsigned int index_vertex = mesh.F()(index_face, face_corner);
        const RowVector2d curvatureGradient = compute_vector(curvatureGradientAngles(face_corner));
        curvatureInCurvatureGradientDirection[index_vertex] = curvatureGradient * mesh.second_fundamental_form()[index_vertex] * curvatureGradient.transpose();
    }

    return curvatureInCurvatureGradientDirection;
}








