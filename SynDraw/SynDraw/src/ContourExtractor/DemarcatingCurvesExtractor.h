#ifndef DEMARCATINGCURVESEXTRACTOR_H
#define DEMARCATINGCURVESEXTRACTOR_H

#include <vector>
#include <map>

#include "CrossValueContoursExtractor.h"
#include "../Tools/CurvatureDerivative.h"

/**\brief Extractor for demarcating curves [Kolomenkin, 2008]
 * \details Demarcating curves use 2nd order curvature derivative to generate lines where it crosses zero.
 * \author Bastien Wailly <bastien.wailly@inria.fr>, Inria
 * \author Adele Saint-Denis <adele.saint-denis@inria.fr>, Inria */
class DemarcatingCurvesExtractor : public CrossValueContoursExtractor{
public:
    DemarcatingCurvesExtractor(MeshObject& m);;
    virtual ~DemarcatingCurvesExtractor();
    
    /**\brief Compute cross vertices for a single mesh facet. 
     * \param f mesh face index 
     * \details Section 3.2 : Computing Demarcating Curves on meshes.\n
     * Faces whose three gradient vectors differ considerably are eliminated from further consideration */
    bool compute_cross_vertex(int f) override;
    
    /**\brief initialize mesh before extraction (compute curvature derivative)*/
    void init(){
        mesh.curvature_derivative_computer();
    }
    
    ContourType type() override { return ContourType::DEMARCATING; } ;

    
private:
    
    /**\brief Calculating Demarcating Curves section 3.2 \
     * \details The exact location of the demarcating curve point is obtained by linear interpolation of the curvature values */
    void add_crossVertex (unsigned int index_vertex1, unsigned int index_vertex2, unsigned int index_edge,
                                         double curvatureInCurvatureGradientDirection_vertex1, double curvatureInCurvatureGradientDirection_vertex2);
    
    /**\brief Section 3.1 : Defining demarcating Curves
     * \details
     * The normal curvature in direction v is k(v) = vt * II * v\n
     * where II is the second fundamental form\n
     *
     * Definition 3.2\n
     * p is as a demarcating curve point if the following holds at p : k(gp) = gpt * II * gp = 0  */    
    std::map<unsigned int, double> compute_curvatureInCurvatureGradientDirectionOneFace (unsigned int index_face, const Array3d &curvatureGradientAngles) const;
    
    /**\brief The zero crossings of k(gp) on the mesh faces are computed according to Definition 3.2 to create the demarcating curves */    
    void find_demarcatingCurvePoints_forOneFace (unsigned int index_face, const map<unsigned int, double> &curvatureInCurvatureGradientDirection);

    /**\brief Section 3.2
     * \details For faces in which the gradients of only two vertices are similar,
     * the average gradient of the two similar vertices is selected
     * and the curvature of the third vertex is computed in this direction
     * Obviously, when this gradient is used for the third vertex,
     * it should be rotated so as to coincide with the vertex’s tangent plane */
    ArrayXd compute_curvatureGradientAnglesForOneFace (unsigned int index_face, const vector<unsigned int> &indices_verticesSimilarity,
                                                                    const MatrixXd &derivativeOfCurvatures,
                                                                    const MatrixXd &principalDirection_min, const MatrixXd &principalDirection_max);

    std::vector<unsigned int> find_verticesWithCurvatureGradientSimilarity (unsigned int index_face) const;
    double compute_valueToMaximize (double angle, double A, double B, double C, double D) const;
    double compute_averageCurvatureGradientDirection (unsigned int index_vertex1, unsigned int index_vertex2, unsigned int index_vertexAverage,
                                                                   const MatrixXd &derivativeOfCurvatures,
                                                                   const MatrixXd &principalDirection_min, const MatrixXd &principalDirection_max);
    
    inline Eigen::RowVector2d compute_vector (double angle) const {
        return Eigen::RowVector2d{cos(angle), sin(angle)};
    }    
};

#endif /* DEMARCATINGCURVESEXTRACTOR_H */

