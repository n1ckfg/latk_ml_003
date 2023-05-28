#ifndef MESHOBJECT_H
#define MESHOBJECT_H

#include <Eigen/Core>
#include <string>
#include <map>
#include <set>
#include <iostream>
#include <igl/principal_curvature.h>
#include <igl/gaussian_curvature.h>
#include "../Tools/Properties.h"
#include "ViewCamera.h"

using namespace Eigen;

class ViewCamera;
class CurvatureDerivative;

/**\author Bastien Wailly <bastien.wailly@inria.fr>, Inria
 * \brief Class holding all mesh related data and algorithms.
 * \details Algorithms are called if and only if there is a need for it (i.e. there is a request leading to it). 
 * Every request for data will call the corresponding algorithm if it's the first time or if mesh was reset, 
 * and then return a reference to the full matrix holding data. */
class MeshObject {
public:
    MeshObject();
    MeshObject(Properties& p);
    MeshObject(Properties& props, bool load);
    MeshObject(const MeshObject& orig);
    virtual ~MeshObject();
    
    /**\brief Init mesh data
     * \details compute normals and topology info */     
    void load();
    
    /**\brief vertices info (indexed by vertex_id)
     * \return reference to vertices coordinates matrix */
    const MatrixXd& V() const { return _V; }
    
    /**\brief faces info (indexed by face_id)
     * \return reference to matrix of vertex_id*/    
    const MatrixXi& F() const { return _F; }
    
    /**\brief per-face normals info (indexed by face_id)
     * \return reference to matrix of stacked normals */    
    const MatrixXd& F_normals() const { return _F_normals; }
    
    /**\brief per-vertex normals info (indexed by vertex_id) 
     * \return reference to matrix of stacked normals */    
    const MatrixXd& V_normals() const { return _V_normals; }
    
    /**\brief edges info (indexed by edge_id)
     * \return reference to matrix of vertex_id */    
    const MatrixXi& E() const { return _E; }
    
    /**\brief edge-to-face info (indexed by edge_id)
     * \return reference to matrix of face_id */    
    const MatrixXi& EF() const { return _EF; }
    
    /**\brief face-to-edge info (indexed by face_id)
     * \return reference to matrix of edge_id */    
    const MatrixXi& FE() const { return _FE; }
    
    /**\brief vertex-to-edge info (indexed by vertex_id)
     * \return set of adjacent edge_id */     
    const std::set<int>& VE(int v) { return _VE[v]; }
    
    /**\brief face-to-edge info (indexed by vertex_id)
     * \return set of adjacent face_id */     
    const std::set<int>& VF(int v) { return _VF[v]; }
    
    /**\brief average length of all edges
     * \return length */     
    double get_edge_avg_length() { return avg_length; }

    
    // Radial curvature
    
    /**\brief compute radial curvature matrix with given camera 
     * \param cam reference to camera to use for computation */     
    void compute_radial_curvature(ViewCamera& cam);
    
    /**\brief get flag for radial curvature
     * \return true if radial curvature has been previously computed */     
    bool is_radial_computed(){ return radial_computed; }   
    
    /**\brief get radial curvature info
     * \return per-vertex radial curvature vector */     
    const VectorXd& radial_curvature() const { return _radial_curvature; }
    
    /**\brief get radial curvature gradient info
     * \return per-vertex radial curvature gradient matrix */    
    const MatrixXd& radial_curvature_gradient() { return _radial_curvature_gradient; }
    
    /**\brief reset radial curvature flag */    
    void reset_radial(){ radial_computed = false; }

    
    // gaussian curvature
    
    /**\brief get gaussian curvature info
     * \return per-vertex gaussian curvature vector */    
    const VectorXd& gaussian_curvature();
    
    /**\brief reset gaussian curvature flag */        
    void reset_gaussian(){ gaussian_computed = false; }    

    
    // principal curvature
    
    /**\brief get principal min curvature info
     * \return per-vertex principal min curvature vector */      
    const VectorXd& principal_curvature_min();
    
    /**\brief get principal max curvature info
     * \return per-vertex principal max curvature vector */    
    const VectorXd& principal_curvature_max();
    
    /**\brief get principal min direction info
     * \return per-vertex principal min direction matrix */    
    const MatrixXd& principal_direction_min();
    
    /**\brief get principal max direction info
     * \return per-vertex principal max direction vector */    
    const MatrixXd& principal_direction_max();
    
    /**\brief reset principal curvature flag */            
    void reset_principal(){ principal_computed = false; }    

    
    // extremalities
    
    /**\brief get minimalities info
     * \return per-vertex minimalities vector */     
    const VectorXd& minima();
    
    /**\brief get maximalities info
     * \return per-vertex maximalities vector */     
    const VectorXd& maxima();
    
    /**\brief reset both extremalities flags */
    void reset_extremas(){ minima_computed = false; }        

    
    // curvature derivative
    
    /**\brief compute or return curvature derivative second fundamental form
     * \return second fundamental form 2x2 matrix*/      
    const std::vector<Matrix2d>& second_fundamental_form();
    
    /**\brief compute or return gradient angles
     * \return per-vertex array of gradient angles (rad)*/      
    const ArrayXd& curvature_gradient_angles();
    
    /**\brief get the CurvatureDerivative instance reference used to compute CD 
     * \return reference to CurvatureDerivative*/      
    const CurvatureDerivative& curvature_derivative_computer();
    
    /**\brief compute or return curvature derivative
     * \return per-vertex matrix of curvature derivative*/      
    const MatrixXd derivative_of_curvature();
    
    /**\brief compute or return gradient angle for one vertex
     * \return gradient angle (rad)*/      
    double compute_curvatureGradientAngle_oneVertex (double a, double b, double c, double d); 
    
    /**\brief reset curvature derivative flag*/      
    void reset_derivative(){ derivative_computed = false; }

    /**\brief update parameters
     * \param sr size of ring
     * \param sf curvature scale factor
     * \param gr gradient radius*/          
    void setProperties(int sr, double sf, double gr){
        sizeRing = sr;
        scaleFactor = sf;
        gradient_radius = gr;
    }
    
    /**\brief compute triangle weights to use for curvature derivative
     * \return per-face weights matrix */
    const MatrixXd& triangle_weights();    
    
private:
    std::string path_to_mesh;

    // Triangular Mesh 
    MatrixXd _V;
    MatrixXi _F;

    // Face and Vertex normals
    MatrixXd _F_normals;
    MatrixXd _V_normals;

    // Edge topology
    MatrixXi _E;
    MatrixXi _FE, _EF;
    std::map<int, std::set<int>> _VE;
    std::map<int, std::set<int>> _VF;
    double avg_length;

    // curvature
    int sizeRing;
    double gradient_radius;
    double scaleFactor;
    double curvatureScale;
    VectorXd _gaussian_curvature;
    VectorXd _radial_curvature;
    MatrixXd _radial_curvature_gradient;
    VectorXd _principal_curvature_min, _principal_curvature_max;
    MatrixXd _principal_direction_max, _principal_direction_min;
    VectorXd _minima, _maxima;
    
    // curvature derivative
    std::vector<Matrix2d> _secondFundamentalForm;
    ArrayXd _curvatureGradientAngles;
    CurvatureDerivative* _curvderv;
    MatrixXd _derivativeOfCurvatures;    

    /* One row corresponds to the weights of a triangle
    Each coefficient of a row corresponds to the weight of a triangle corner vertex */
    MatrixXd _triangle_weights;

    bool radial_computed;    
    bool gaussian_computed, principal_computed, minima_computed, maxima_computed, weights_computed, derivative_computed;

    /**\brief compute principal curvature if not already computed*/
    void query_principal();
    
    /**\brief compute triangle weights */    
    void compute_triangle_weights();
    
    /**\brief compute extremalities */    
    void compute_minima();
    
    /**\brief compute extremalities */        
    void compute_maxima();
    
    /**\brief compute curvature derivative if not already computed*/        
    void query_derivative();
    
    void compute_pointCornerAreas(VectorXd &pointAreas, MatrixXd &cornerAreas);
    double compute_triangleAera(unsigned int index_vertex0, unsigned int index_vertex1, unsigned int index_vertex2) const;
    void update_cornerAreas(unsigned int index_face, double faceArea,
            const RowVector3d &edge0, const RowVector3d &edge1, const RowVector3d &edge2,
            const double *edge_squareNorm, const Array3d &barycentricWeightsCircumcenter, MatrixXd &cornerAreas);
    void compute_cubicFormulaTerms(double a, double b, double c, double d, double &A, double &B, double &C, double &D) const;
    void compute_normalizedCubicEquation (double A, double B, double C, double D, double &newA, double &newB, double &newC);
    double find_curvatureGradientAngle_fromSinusSquareAngle (double sinus_square_curvatureGradientAngle,double a, double b, double c, double d);
    double find_curvatureGradientAngle_bySampling (double a, double b, double c, double d) const;
    double compute_discriminantCubicFormula (double A, double B, double C, double &QdividedByTwo, double &AdividedbyThree) const;
    double compute_valueToMaximize (double angle, double A, double B, double C, double D) const;
    ArrayXd compute_functionToMaximize (const ArrayXd &angles, double a, double b, double c, double d) const;
    ArrayXd compute_gaussianFilterForConvolution (unsigned int windowSize) const;
    ArrayXd compute_gaussianSmoothedFunction (const ArrayXd &gaussianFilter, const ArrayXd &function) const; 
    ArrayXd compute_curvatureGradientAngles (const MatrixXd &derivativeOfCurvatureCoeffs);    
    std::vector<Matrix2d> compute_secondFundamentalForm(const VectorXd &princ_curv_min, const VectorXd &princ_curv_max);
    
};

#endif /* MESHOBJECT_H */

