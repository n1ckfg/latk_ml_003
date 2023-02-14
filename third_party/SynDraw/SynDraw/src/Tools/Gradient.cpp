#include "Gradient.h"

#include <igl/grad.h>
#include <igl/massmatrix.h>


using namespace std;
using namespace igl;
using namespace Eigen;

//--------------------------------------------------------------------------------

Gradient::Gradient(MeshObject& m, double r)
{
    mesh = m;
    vertices = mesh.V();
    faces = mesh.F();
    normal_vertices = mesh.V_normals();
    radius = r;
}

/* First Method */


/*
 * The gradient per vertex is computed has the weighted sum of the triangles
 * in the neightbourhood of the vertex
 */
MatrixXd Gradient::faceGradients_toVertices (const MatrixXd &gradientPerFace) {
    const unsigned int num_vertices = vertices.rows();
    const unsigned int num_faces = faces.rows();
    MatrixXd weight_triangleAreas = mesh.triangle_weights();
    MatrixXd gradientPerVertex = MatrixXd::Zero(num_vertices, 3);

    for (unsigned int index_face = 0; index_face < num_faces; ++index_face) {
        unsigned int index_vertex0 = faces(index_face, 0);
        unsigned int index_vertex1 = faces(index_face, 1);
        unsigned int index_vertex2 = faces(index_face, 2);

        RowVector3d face_contribution = gradientPerFace.row(index_face);

        gradientPerVertex.row(index_vertex0) += weight_triangleAreas(index_face, 0) * face_contribution;
        gradientPerVertex.row(index_vertex1) += weight_triangleAreas(index_face, 1) * face_contribution;
        gradientPerVertex.row(index_vertex2) += weight_triangleAreas(index_face, 2) * face_contribution;
    }
    return gradientPerVertex;
}


MatrixXd Gradient::compute_gradient (const VectorXd &values) {

    SparseMatrix<double> gradient_operator;
    grad(vertices,faces,gradient_operator);

    MatrixXd gradientCurvature_perFace = Map<const MatrixXd>((gradient_operator * values).eval().data(),faces.rows(),3);
    return faceGradients_toVertices(gradientCurvature_perFace);
}


/* Second Method */

// ------------ Get Sphere

class Gradient::compareMinimum {
public:
    bool operator() (const Neightboor &elementLeft, const Neightboor &elementRight) const {
        return elementLeft.second > elementRight.second;
    }
};



void Gradient::putAllNeighBoorsInsideRadiusIntoQueue (unsigned int index_vertex, unsigned int num_selectedNeighboors,
                                                      unsigned int num_neightboors_minimum, const Vector3d &vertex_origin,
                                                      bool *visited, list<unsigned int> &queue, ExtraCandidatesList &extra_candidates) const {
    const unsigned int num_neightboors = vertex_to_vertices[index_vertex].size();

    for (unsigned int i = 0; i < num_neightboors; ++i) {
        const unsigned int index_neighbor = vertex_to_vertices[index_vertex][i];
        if (!visited[index_neighbor]) {
            const Vector3d neighboor = vertices.row(index_neighbor);
            const double distance = (vertex_origin - neighboor).norm();
            if (distance < radius) {
                queue.push_back(index_neighbor);
            } else if (num_selectedNeighboors < num_neightboors_minimum) {
                // Stock candidates in case there not enough neighboors at the end
                extra_candidates.push(Neightboor(index_neighbor, distance));
            }
            visited[index_neighbor] = true;
        }
    }
}


void Gradient::ensure_enoughNeightboors (unsigned int num_neightboors_minimum, const Vector3d &vertex_origin,
                                         bool *visited, ExtraCandidatesList &extra_candidates, vector<unsigned int> &neightboors) const {
    while (!extra_candidates.empty() && neightboors.size() < num_neightboors_minimum) {
        // Take the closest candidate
        Neightboor candidate = extra_candidates.top();
        extra_candidates.pop();
        neightboors.push_back(candidate.first);

        const unsigned int num_neightboors = vertex_to_vertices[candidate.first].size();
        for (unsigned int i = 0; i < num_neightboors; ++i) {
            unsigned int index_neighbor = vertex_to_vertices[candidate.first][i];
            if (!visited[index_neighbor]) {
                const Vector3d neighbor = vertices.row(index_neighbor);
                double distance = (vertex_origin - neighbor).norm();
                extra_candidates.push(Neightboor(index_neighbor,distance));
                visited[index_neighbor] = true;
            }
        }
    }
}


vector<unsigned int> Gradient::getSphere(unsigned int index_vertex, unsigned int num_neightboors_minimum) const {
    vector<unsigned int> neightboors;
    ExtraCandidatesList extra_candidates;
    list<unsigned int> queue;
    const Vector3d vertex = vertices.row(index_vertex);

    const unsigned int bufsize = vertices.rows();
    neightboors.reserve(bufsize);

    bool* visited = (bool*)calloc(bufsize,sizeof(bool));
    visited[index_vertex] = true;

    // We start with the vertex
    putAllNeighBoorsInsideRadiusIntoQueue(index_vertex, neightboors.size(), num_neightboors_minimum, vertex,
                                          visited, queue, extra_candidates);

    // While all the neightboors in the radius has not been taken yet
    while (!queue.empty()) {
        // The neightboors to visit is inside the radius
        unsigned int toVisit = queue.front();
        queue.pop_front();
        neightboors.push_back(toVisit);
        putAllNeighBoorsInsideRadiusIntoQueue(toVisit, neightboors.size(), num_neightboors_minimum, vertex,
                                              visited, queue, extra_candidates);
    }

    ensure_enoughNeightboors(num_neightboors_minimum, vertex, visited, extra_candidates, neightboors);

    free(visited);

    return neightboors;
}

//------------------------------------------

void Gradient::computeReferenceXYaxis(unsigned int index_vertex, Vector3d &Xaxis, Vector3d &Yaxis) const {
    const Vector3d point = vertices.row(index_vertex);
    const Vector3d pointNormal = normal_vertices.row(index_vertex);
    const Vector3d longest_v = vertices.row(vertex_to_vertices[index_vertex][0]);
    Vector3d projection = longest_v - point;
    projection = pointNormal * (projection.dot(pointNormal));
    projection = longest_v - projection;

    Xaxis = (projection - point).normalized();
    Yaxis = (pointNormal.cross(Xaxis)).normalized();
}


double Gradient::distanceAtMongeFrame (unsigned int index_centerVertex, const Vector3d &projectionPlane,
                                     const VectorXd &principalCurvature_min, const VectorXd &principalCurvature_max) const {
    const double uj = projectionPlane(0);
    const double vj = projectionPlane(1);

	const double k1 = principalCurvature_max(index_centerVertex);
	const double k2 = principalCurvature_min(index_centerVertex);

	const double a = pow((k1*uj*uj + k2*vj*vj), 2);
	const double b = uj*uj + vj*vj;
	const double c = sqrt(a/b);

    double distance = (sqrt(a + b) + b/sqrt(a)*log(c + sqrt(1.0+ a/b))) / 2.0;
    return distance;
}


RowVector3d Gradient::compute_projectionOnPlane (unsigned int index_vertexProjected, unsigned int index_originFrame,
                                               const Vector3d &Xaxis, const Vector3d &Yaxis) const {
    const RowVector3d originFrame = vertices.row(index_originFrame);
    const RowVector3d vTang = vertices.row(index_vertexProjected) - originFrame;

    double x = vTang.dot(Xaxis);
    double y = vTang.dot(Yaxis);
    double z = 0.0;

    RowVector3d point = RowVector3d{ x, y, z };
    point.normalize();

    return point;
}


double Gradient::distanceBetweenVertices (unsigned int index_vertex1, unsigned int index_vertex2,
                                        const VectorXd &principalCurvature_min, const VectorXd &principalCurvature_max,
                                        const RowVector3d &projectionPlane_vertex1) const {

    const double distance_frameCenteredAtVertex1 = distanceAtMongeFrame(index_vertex1, projectionPlane_vertex1,
                          principalCurvature_min, principalCurvature_max);

    /* Distance frame Centered At vertex 2 */
    Vector3d XaxisReferenceFrame_vertex2, YaxisReferenceFrame_vertex2;
    computeReferenceXYaxis(index_vertex2, XaxisReferenceFrame_vertex2, YaxisReferenceFrame_vertex2);

    RowVector3d projectionOnPlane = compute_projectionOnPlane(index_vertex1, index_vertex2, XaxisReferenceFrame_vertex2, YaxisReferenceFrame_vertex2);
    

    const double distance_frameCenteredAtVertex2 = distanceAtMongeFrame(index_vertex2, projectionOnPlane,
                          principalCurvature_min, principalCurvature_max);

    return 0.5 * (distance_frameCenteredAtVertex1 + distance_frameCenteredAtVertex2);
}


MatrixXd Gradient::compute_gradient_deepMethod (const VectorXd &values,
                                   const VectorXd &principalCurvature_min, const VectorXd &principalCurvature_max) const {
    const unsigned int num_vertices = vertices.rows();
    MatrixXd set_gradients{num_vertices, 3};
    Vector3d XaxisReferenceFrame, YaxisReferenceFrame, ZaxisReferenceFrame;

    for (unsigned int index_vertex = 0; index_vertex < num_vertices; ++index_vertex) {
        auto neightBoors = getSphere(index_vertex, 6);
        const unsigned int num_neightBours = neightBoors.size();
        MatrixXd setEquation_leftPart{num_neightBours, 3};
        VectorXd setEquation_rightPart{num_neightBours};

        computeReferenceXYaxis(index_vertex, XaxisReferenceFrame, YaxisReferenceFrame);

        for (unsigned int i = 0; i < num_neightBours; ++i) {
            unsigned int index_neightBoor = neightBoors[i];

            RowVector3d projectionNeightBourPlane = compute_projectionOnPlane(index_neightBoor, index_vertex,
                                                                              XaxisReferenceFrame, YaxisReferenceFrame);

            const double distance = distanceBetweenVertices(index_vertex, index_neightBoor,
                                            principalCurvature_min, principalCurvature_max, projectionNeightBourPlane);

            // Conversion de l'origine du repere objet dans le repère local
            RowVector3d vTang = -vertices.row(index_vertex);
            double x = vTang.dot(XaxisReferenceFrame);
            double y = vTang.dot(YaxisReferenceFrame);
            double z = vTang.dot(ZaxisReferenceFrame);
            RowVector3d origin{x, y, z};

            // Conversion de la projection dans le repère objet
            vTang = projectionNeightBourPlane - origin;
            x = vTang.dot(RowVector3d{1.0, 0.0, 0.0});
            y = vTang.dot(RowVector3d{0.0, 1.0, 0.0});
            z = vTang.dot(RowVector3d{0.0, 0.0, 1.0});

            setEquation_rightPart(i) = values(index_neightBoor) - values(index_vertex);
            setEquation_rightPart(i) /= distance;
            setEquation_leftPart.row(i) = projectionNeightBourPlane.normalized();
        }

        JacobiSVD<MatrixXd> svd{setEquation_leftPart, ComputeThinU | ComputeThinV};
        set_gradients.row(index_vertex) = svd.solve(setEquation_rightPart);

    }

    return set_gradients;
}

