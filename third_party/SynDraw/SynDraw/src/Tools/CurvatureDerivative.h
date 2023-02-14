#ifndef CURVATUREDERIVATIVE_H
#define CURVATUREDERIVATIVE_H

#include <Eigen/Core>

class MeshObject;

/**\author Bastien Wailly <bastien.wailly@inria.fr>, Inria
 * \author Adele Saint-Denis <adele.saint-denis@inria.fr>, Inria
 * \brief utility class to compute Curvature Derivative of a mesh at vertices*/
class CurvatureDerivative {
public:
    CurvatureDerivative(MeshObject& m):mesh(m){};
    virtual ~CurvatureDerivative(){};
    
    /**\brief compute curvature derivative tensor
     * \details Find the coefficients of the derivative of Curvature for each vertex.
     * Each row as the four coefficients of the derivative of Curvature for one vertex.
     *
     * "Demarcating Curves for Shape Illustration"\n 
     * Authors : Michael Kolomenkin, Ilan Shimshoni, Ayellet Tal\n 
     * "The derivatives of the curvature are defined by a 2 x 2 x 2 tensor
     * with four unique numbers :"
     * 
     *   a   b       b   c\n 
     *   b   c       c   d 
     * 
     * The function takes as input the principal curvatures and directions of the mesh
     * \return 2x2x2 curvature derivative tensor
     */
    Eigen::MatrixXd compute_coefficients(const Eigen::VectorXd &principalCurvature_min, const Eigen::VectorXd &principalCurvature_max,
                                                   const Eigen::MatrixXd &principalDirection_min, const Eigen::MatrixXd &principalDirection_max);

    /**\brief The derivative of curvatures are computed in the new ReferenceFrame (new_u, new_v)
     * for each vertex of ONE triangle */
    Eigen::RowVector4d compute_coefficients_in_reference_frame(const Eigen::RowVector3d &old_u, const Eigen::RowVector3d &old_v,
                                                    const Eigen::RowVector4d old_derivativeOfCurvatures,
                                                    const Eigen::RowVector3d &new_u, const Eigen::RowVector3d &new_v) const;    
private:
    MeshObject& mesh;
    
    /**\brief Rotate a coordinate system to be perpendicular to the given normal */
    void rotate_coordinates_system(const Eigen::RowVector3d &old_u, const Eigen::RowVector3d &old_v,
                              const Eigen::RowVector3d &new_norm,
                              Eigen::RowVector3d &new_u, Eigen::RowVector3d &new_v) const;

    /**\brief compute the T and B, eigen vectors of the NTB coordinate system of a face
     * \details N: face normal  T: any face edge  B: N x T */
    void NTB_coordinates_system (unsigned int index_face, Eigen::RowVector3d &t, Eigen::RowVector3d &b) const;

    /**\brief Estimate curvature derivative of a face based on variation of curvature along edges */
    Eigen::RowVector4d compute_face_derivative (const Eigen::Matrix3d &edgesDirection, const Eigen::Matrix3d &faceCurvatures,
                                                              const Eigen::RowVector3d &t, const Eigen::RowVector3d &b) const;

    /**\brief transfer derivative values from face to vertices*/
    void transfer_to_vertices (const Eigen::MatrixXd &principalDirection_min, const Eigen::MatrixXd &principalDirection_max,
                                                            const Eigen::RowVector3d &t, const Eigen::RowVector3d &b, unsigned int index_face,
                                                            const Eigen::RowVector4d &face_derivativeOfcurvature,
                                                            Eigen::MatrixXd &derivativeOfCurvatureCoeffs);

    /**\brief Compute principal curvature of a face in the new ReferenceFrame (u,v)
     *  */
    Eigen::Matrix3d curvatures_in_reference_frame (const Eigen::MatrixXd &principalDirection_min, const Eigen::MatrixXd &principalDirection_max,
                                                         const Eigen::VectorXd &principalCurvature_min, const Eigen::VectorXd &principalCurvature_max,
                                                         const Eigen::RowVector3d &u, const Eigen::RowVector3d &v, unsigned int index_face) const;


};

#endif /* CURVATUREDERIVATIVE_H */

