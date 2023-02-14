#ifndef __GRADIENT_HPP__
#define __GRADIENT_HPP__

#include <queue>


#include "../Common/MeshObject.h"
#include <list>
#include <vector>

/**\author Bastien Wailly <bastien.wailly@inria.fr>, Inria
 * \author Adele Saint-Denis <adele.saint-denis@inria.fr>, Inria
 * \brief utility class to compute a gradient of values on a mesh*/
class Gradient {
public:
    Gradient(){};
    Gradient (MeshObject &m, double r);

    /**\brief compute the pervertex gradient matrix of a perface values list
     * \param values the face based values
     * \return per vertex gradient matrix*/
    Eigen::MatrixXd compute_gradient (const Eigen::VectorXd &values);

    Eigen::MatrixXd compute_gradient_deepMethod(const Eigen::VectorXd &values,
                                     const Eigen::VectorXd &principalCurvature_min, const Eigen::VectorXd &principalCurvature_max) const;
private:
    /* Attributes */
    Eigen::MatrixXd vertices;
    Eigen::MatrixXi faces;
    Eigen::MatrixXd normal_vertices;

    std::vector< std::vector<unsigned int> > vertex_to_vertices;
    double radius;

    MeshObject mesh;

    /* Functions */
    Eigen::MatrixXd faceGradients_toVertices(const Eigen::MatrixXd &gradientPerFace);

    // Get Sphere
    // The candidates are sorted by their distance from the origin vertex
    class compareMinimum;

    using Neightboor = std::pair<unsigned int, double>;
    using ExtraCandidatesList = std::priority_queue<Neightboor, std::vector<Neightboor>, compareMinimum >;


    void putAllNeighBoorsInsideRadiusIntoQueue (unsigned int index_vertex, unsigned int num_selectedNeighboors,
                                                          unsigned int num_neightboors_minimum, const Eigen::Vector3d &vertex_origin,
                                                          bool *visited, std::list<unsigned int> &queue, ExtraCandidatesList &extra_candidates) const;

    void ensure_enoughNeightboors (unsigned int num_neightboors_minimum, const Eigen::Vector3d &vertex_origin,
                                             bool *visited, ExtraCandidatesList &extra_candidates, std::vector<unsigned int> &neightboors) const;

    std::vector<unsigned int> getSphere(unsigned int index_vertex, unsigned int num_neightboors_minimum) const;

    //------------

    void computeReferenceXYaxis(unsigned int index_vertex, Eigen::Vector3d &Xaxis, Eigen::Vector3d &Yaxis) const;

    double distanceBetweenVertices (unsigned int index_vertex1, unsigned int index_vertex2,
                                            const Eigen::VectorXd &principalCurvature_min, const Eigen::VectorXd &principalCurvature_max,
                                            const Eigen::RowVector3d &projectionPlane_vertex1) const;

    Eigen::RowVector3d compute_projectionOnPlane (unsigned int index_vertexProjected, unsigned int index_originFrame,
                                                  const Eigen::Vector3d &Xaxis, const Eigen::Vector3d &Yaxis) const;

    double distanceAtMongeFrame (unsigned int index_centerVertex, const Eigen::Vector3d &projectionPlane,
                                         const Eigen::VectorXd &principalCurvature_min, const Eigen::VectorXd &principalCurvature_max) const;
};

#endif
