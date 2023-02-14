#include "CurvatureDerivative.h"
#include "../Common/MeshObject.h"

using namespace Eigen;

MatrixXd CurvatureDerivative::compute_coefficients (const VectorXd &principalCurvature_min, const VectorXd &principalCurvature_max,
                                                            const MatrixXd &principalDirection_min, const MatrixXd &principalDirection_max) {
    Matrix3d edgesDirection;
    RowVector3d t, b;
    const long num_faces = mesh.F().rows();
    MatrixXd derivativeOfCurvatureCoeffs = MatrixXd::Zero(mesh.V().rows(), 4);

    for (unsigned int index_face = 0; index_face < num_faces; ++index_face) {
        NTB_coordinates_system(index_face, t, b);

        const auto vertex0 = mesh.V().row(mesh.F()(index_face, 0));
        const auto vertex1 = mesh.V().row(mesh.F()(index_face, 1));
        const auto vertex2 = mesh.V().row(mesh.F()(index_face, 2));

        edgesDirection.row(0) = vertex2 - vertex1;
        edgesDirection.row(1) = vertex0 - vertex2;
        edgesDirection.row(2) = vertex1 - vertex0;

        Matrix3d faceCurvatures = curvatures_in_reference_frame(principalDirection_min, principalDirection_max,
                                    principalCurvature_min, principalCurvature_max,
                                    t, b, index_face);
        const RowVector4d face_derivativeOfcurvature = compute_face_derivative(edgesDirection, faceCurvatures, t, b);

        transfer_to_vertices(principalDirection_min, principalDirection_max, t, b,
                                                  index_face, face_derivativeOfcurvature, derivativeOfCurvatureCoeffs);
    }

    return derivativeOfCurvatureCoeffs;
}

void CurvatureDerivative::NTB_coordinates_system (unsigned int index_face, RowVector3d &t, RowVector3d &b) const {
    const auto vertex0 = mesh.V().row(mesh.F()(index_face, 0));
    const auto vertex1 = mesh.V().row(mesh.F()(index_face, 1));
    const auto vertex2 = mesh.V().row(mesh.F()(index_face, 2));

    const RowVector3d edge0 = vertex2 - vertex1;
    const RowVector3d edge1 = vertex0 - vertex2;

    t = edge0.normalized();
    const RowVector3d n = edge0.cross(edge1);
    b = n.cross(t);
    b.normalize();
}


Matrix3d CurvatureDerivative::curvatures_in_reference_frame (const MatrixXd &principalDirection_min, const MatrixXd &principalDirection_max,
                                                     const VectorXd &principalCurvature_min, const VectorXd &principalCurvature_max,
                                                     const RowVector3d &u, const RowVector3d &v, unsigned int index_face) const {
    Matrix3d faceCurvatures;
    RowVector3d r_new_u, r_new_v;

    for (unsigned int i = 0; i < 3; i++) {
        const int index_vertex = mesh.F()(index_face, i);
        const RowVector3d old_u = principalDirection_min.row(index_vertex);
        const RowVector3d old_v = principalDirection_max.row(index_vertex);
        const double minCurvature = principalCurvature_min(index_vertex);
        const double maxCurvature = principalCurvature_max(index_vertex);

        rotate_coordinates_system(u, v, old_u.cross(old_v), r_new_u, r_new_v);

        double u1 = r_new_u.dot(old_u);
        double v1 = r_new_u.dot(old_v);
        double u2 = r_new_v.dot(old_u);
        double v2 = r_new_v.dot(old_v);

        faceCurvatures(i, 0) = minCurvature * u1*u1 + maxCurvature * v1*v1;
        faceCurvatures(i, 1) = minCurvature * u1*u2 + maxCurvature * v1*v2;
        faceCurvatures(i, 2) = minCurvature * u2*u2 + maxCurvature * v2*v2;

    }

    return faceCurvatures;
}

RowVector4d CurvatureDerivative::compute_face_derivative (const Matrix3d &edgesDirection, const Matrix3d &faceCurvatures,
                                                                      const RowVector3d &t, const RowVector3d &b) const {
    RowVector4d m = RowVector4d::Zero();
    Matrix4d w = Matrix4d::Zero();

    for (unsigned int face_corner = 0; face_corner < 3; ++face_corner) {
        // Variation of curvature along each edge
        RowVector3d derivativeFaceCurvatures = faceCurvatures.row((face_corner - 1) % 3) - faceCurvatures.row((face_corner + 1) % 3);

        double u = edgesDirection.row(face_corner).dot(t);
        double v = edgesDirection.row(face_corner).dot(b);

        w(0,0) += u * u;
        w(0,1) += u * v;
        w(3,3) += v * v;

        m(0) += u * derivativeFaceCurvatures(0);
        m(1) += v * derivativeFaceCurvatures(0) + 2.0 * u * derivativeFaceCurvatures(1);
        m(2) += 2.0 * v * derivativeFaceCurvatures(1) + u * derivativeFaceCurvatures(2);
        m(3) += v * derivativeFaceCurvatures(2);
    }

    w(1,1) = 2.0 * w(0,0) + w(3,3);
    w(1,2) = 2.0 * w(0,1);
    w(2,2) = w(0,0) + 2.0 * w(3,3);
    w(2,3) = w(0,1);

    // Least squares solution
    JacobiSVD<MatrixXd> svd{w, ComputeThinU | ComputeThinV};
    return svd.solve(m.transpose()).transpose();
}

void CurvatureDerivative::transfer_to_vertices (const MatrixXd &principalDirection_min, const MatrixXd &principalDirection_max,
                                                                   const RowVector3d &t, const RowVector3d &b,
                                                                   unsigned int index_face, const RowVector4d &face_derivativeOfcurvature,
                                                                   MatrixXd &derivativeOfCurvatureCoeffs) {
    const MatrixXd &weight_triangleAreas = mesh.triangle_weights();

    for (unsigned int i = 0; i < 3; ++i) {
        const int index_vertex = mesh.F()(index_face, i);
        RowVector4d vertex_derivativeOfCurvatures = compute_coefficients_in_reference_frame(t, b, face_derivativeOfcurvature, principalDirection_min.row(index_vertex),
                                                                                           principalDirection_max.row(index_vertex));
        vertex_derivativeOfCurvatures(0) = fabs(vertex_derivativeOfCurvatures(0));
        vertex_derivativeOfCurvatures(1) = fabs(vertex_derivativeOfCurvatures(1));
        vertex_derivativeOfCurvatures(2) = fabs(vertex_derivativeOfCurvatures(2));
        vertex_derivativeOfCurvatures(3) = fabs(vertex_derivativeOfCurvatures(3));

        derivativeOfCurvatureCoeffs.row(index_vertex) += weight_triangleAreas(index_face, i) * vertex_derivativeOfCurvatures;
    }
}

RowVector4d CurvatureDerivative::compute_coefficients_in_reference_frame(const RowVector3d &old_u, const RowVector3d &old_v,
                                                const RowVector4d old_derivativeOfCurvatures,
                                                const RowVector3d &new_u, const RowVector3d &new_v) const {
    RowVector4d new_derivativeOfCurvatures;
    RowVector3d r_new_u, r_new_v;
    rotate_coordinates_system(new_u, new_v, old_u.cross(old_v), r_new_u, r_new_v);

    double u1 = r_new_u.dot(old_u);
    double v1 = r_new_u.dot(old_v);
    double u2 = r_new_v.dot(old_u);
    double v2 = r_new_v.dot(old_v);

    new_derivativeOfCurvatures(0) = old_derivativeOfCurvatures(0)*u1*u1*u1 +
                   old_derivativeOfCurvatures(1)*3.0*u1*u1*v1 +
                   old_derivativeOfCurvatures(2)*3.0*u1*v1*v1 +
                   old_derivativeOfCurvatures(3)*v1*v1*v1;

    new_derivativeOfCurvatures(1) = old_derivativeOfCurvatures(0)*u1*u1*u2 +
                   old_derivativeOfCurvatures(1)*(u1*u1*v2 + 2.0*u2*u1*v1) +
                   old_derivativeOfCurvatures(2)*(u2*v1*v1 + 2.0*u1*v1*v2) +
                   old_derivativeOfCurvatures(3)*v1*v1*v2;

    new_derivativeOfCurvatures(2) = old_derivativeOfCurvatures(0)*u1*u2*u2 +
                   old_derivativeOfCurvatures(1)*(u2*u2*v1 + 2.0*u1*u2*v2) +
                   old_derivativeOfCurvatures(2)*(u1*v2*v2 + 2.0*u2*v2*v1) +
                   old_derivativeOfCurvatures(3)*v1*v2*v2;

    new_derivativeOfCurvatures(3) = old_derivativeOfCurvatures(0)*u2*u2*u2 +
                   old_derivativeOfCurvatures(1)*3.0*u2*u2*v2 +
                   old_derivativeOfCurvatures(2)*3.0*u2*v2*v2 +
                   old_derivativeOfCurvatures(3)*v2*v2*v2;

    return new_derivativeOfCurvatures;
}

void CurvatureDerivative::rotate_coordinates_system(const RowVector3d &old_u, const RowVector3d &old_v, const RowVector3d &new_norm,
                                                    RowVector3d &new_u, RowVector3d &new_v) const {
    new_u = old_u;
    new_v = old_v;
    const RowVector3d old_norm = old_u.cross(old_v);
    const double ndot = old_norm.dot(new_norm);

    // unlikely
    if (ndot > -1.0) {
        new_u = -new_u;
        new_v = -new_v;
    } else {
        // Perpendicular to old_norm and in the plane of old_norm and new_norm
        const RowVector3d perp_old = new_norm - ndot * old_norm;

        // perp_old - perp_new, with normalization constants folded in
        const RowVector3d dperp = 1.0 / (1.0 + ndot) * (old_norm + new_norm);

        // Subtracts component along perp_old, and adds the same amount along
        // perp_new.  Leaves unchanged the component perpendicular to the
        // plane containing old_norm and new_norm.
        new_u -= dperp * (new_u.dot(perp_old));
        new_v -= dperp * (new_v.dot(perp_old));
    }
}
