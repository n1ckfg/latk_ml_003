#include <igl/read_triangle_mesh.h>
#include <igl/per_face_normals.h>
#include <igl/edge_topology.h>
#include <igl/avg_edge_length.h>

#include "MeshObject.h"
#include "ViewCamera.h"
#include "../Tools/Gradient.h"
#include "../Tools/CurvatureDerivative.h"
#include <algorithm>

MeshObject::MeshObject(): gaussian_computed(false),
                          radial_computed(false),
                          principal_computed(false),
                          minima_computed(false),
                          maxima_computed(false),
                          weights_computed(false),
                          derivative_computed(false)
{    
}

MeshObject::MeshObject(const MeshObject& orig) {
}

MeshObject::~MeshObject() {
//    delete _curvderv;
}

MeshObject::MeshObject(Properties& props):MeshObject(){
    path_to_mesh = props.mesh_in;
    sizeRing = props.ring_size;
    scaleFactor = props.curvature_scale;
    gradient_radius = props.gradient_radius;

    if(not igl::read_triangle_mesh(path_to_mesh, _V, _F))
        throw 42;
}

MeshObject::MeshObject(Properties& props, bool load):MeshObject(props){
    if(load){
        this->load();
    }
}

void MeshObject::load() {
    std::cout << " Computing mesh topology..." << std::endl;
    igl::edge_topology(_V, _F, _E, _FE, _EF);
    avg_length =  igl::avg_edge_length(_V, _F);

    // vertex to edges
    _VE.clear();
    for (unsigned int e = 0; e < _E.rows(); ++e) {
        _VE[_E.row(e)(0)].insert(e);
        _VE[_E.row(e)(1)].insert(e);
    }

    // vertex to faces
    _VF.clear();
    for (unsigned int f = 0; f < _F.rows(); ++f) {
        _VF[_F.row(f)(0)].insert(f);
        _VF[_F.row(f)(1)].insert(f);
    }        

    std::cout << " Computing mesh normals..." << std::endl;
    igl::per_face_normals(_V, _F, _F_normals);
    igl::per_vertex_normals(_V, _F, _V_normals);

    std::cout << " V \tF \tE" << std::endl;    
    std::cout << " " << _V.rows() << " \t" << _F.rows() << " \t" << _E.rows() << std::endl;  

    curvatureScale = scaleFactor * avg_length; 
}


void MeshObject::compute_triangle_weights() {
    VectorXd pointAreas;
    MatrixXd cornerAreas;
    const unsigned int num_faces = _F.rows();
    _triangle_weights.resize(num_faces, 3);

    compute_pointCornerAreas(pointAreas, cornerAreas);

#pragma omp parallel for
    for (unsigned int f = 0; f < num_faces; ++f) {
        for (unsigned int i = 0; i < 3; ++i) {
            const unsigned int v = _F(f, i);
            _triangle_weights(f, i) = cornerAreas(f, i) / pointAreas(v);
        }
    }
}

void MeshObject::compute_pointCornerAreas (VectorXd &pointAreas, MatrixXd &cornerAreas) {
    const unsigned int num_faces = _F.rows();
    pointAreas = VectorXd::Zero(_V.rows());
    cornerAreas.resize(num_faces, 3);

#pragma omp parallel for
    for (unsigned int f = 0; f < num_faces; ++f) {
        const unsigned int v0 = _F(f, 0);
        const unsigned int v1 = _F(f, 1);
        const unsigned int v2 = _F(f, 2);

        const RowVector3d edge0 = _V.row(v2) - _V.row(v1);
        const RowVector3d edge1 = _V.row(v0) - _V.row(v2);
        const RowVector3d edge2 = _V.row(v1) - _V.row(v0);

        const double faceArea = compute_triangleAera(v0, v1, v2);
        const double edge_squareNorm[3] = { edge0.squaredNorm(), edge1.squaredNorm(), edge2.squaredNorm() };

        Array3d barycentricWeightsCircumcenter;
        barycentricWeightsCircumcenter(0) = edge_squareNorm[0] * (edge_squareNorm[1] + edge_squareNorm[2] - edge_squareNorm[0]);
        barycentricWeightsCircumcenter(1) = edge_squareNorm[1] * (edge_squareNorm[2] + edge_squareNorm[0] - edge_squareNorm[1]);
        barycentricWeightsCircumcenter(2) = edge_squareNorm[2] * (edge_squareNorm[0] + edge_squareNorm[1] - edge_squareNorm[2]);

        update_cornerAreas(f, faceArea, edge0, edge1, edge2, edge_squareNorm, barycentricWeightsCircumcenter, cornerAreas);

#pragma omp critical
{
        pointAreas(v0) += cornerAreas(f, 0);
        pointAreas(v1) += cornerAreas(f, 1);
        pointAreas(v2) += cornerAreas(f, 2);
}
    }
}

const MatrixXd& MeshObject::triangle_weights() {
    if (not weights_computed) {
        compute_triangle_weights();
        weights_computed = true;
    }
    return _triangle_weights;
}

double MeshObject::compute_triangleAera (unsigned int v0, unsigned int v1, unsigned int v2) const {
    const RowVector3d &vertex1 = _V.row(v0);
    const RowVector3d &vertex2 = _V.row(v1);
    const RowVector3d &vertex3 = _V.row(v2);

    const RowVector3d edge0 = vertex2 - vertex1;
    const RowVector3d edge1 = vertex3 - vertex1;

    const double areaParallelogram = (edge0.cross(edge1)).norm();
    return areaParallelogram * 0.5;
}


void MeshObject::update_cornerAreas (unsigned int index_face, double faceArea,
                                         const RowVector3d &edge0, const RowVector3d &edge1, const RowVector3d &edge2,
                                         const double *edge_squareNorm, const Array3d &barycentricWeightsCircumcenter, MatrixXd &cornerAreas) {
    if (barycentricWeightsCircumcenter(0) <= 0.0f) {
        cornerAreas(index_face, 1) = -0.25f * edge_squareNorm[2] * faceArea / edge0.dot(edge2);
        cornerAreas(index_face, 2) = -0.25f * edge_squareNorm[1] * faceArea / edge0.dot(edge1);
        cornerAreas(index_face, 0) = faceArea - cornerAreas(index_face, 1) - cornerAreas(index_face, 2);
    } else if (barycentricWeightsCircumcenter(1) <= 0.0f) {
        cornerAreas(index_face, 2) = -0.25f * edge_squareNorm[0] * faceArea / edge1.dot(edge0);
        cornerAreas(index_face, 0) = -0.25f * edge_squareNorm[2] * faceArea / edge1.dot(edge2);
        cornerAreas(index_face, 1) = faceArea - cornerAreas(index_face, 2) - cornerAreas(index_face, 0);
    } else if (barycentricWeightsCircumcenter(2) <= 0.0f) {
        cornerAreas(index_face, 0) = -0.25f * edge_squareNorm[1] * faceArea / edge2.dot(edge1);
        cornerAreas(index_face, 1) = -0.25f * edge_squareNorm[0] * faceArea / edge2.dot(edge0);
        cornerAreas(index_face, 2) = faceArea - cornerAreas(index_face, 0) - cornerAreas(index_face, 1);
    } else {
        const double scale = 0.5 * faceArea / barycentricWeightsCircumcenter.sum();
        for (unsigned int faceCorner = 0; faceCorner < 3; ++faceCorner) {
            cornerAreas(index_face, faceCorner) = scale * ( barycentricWeightsCircumcenter((faceCorner + 1) % 3) +
                                         barycentricWeightsCircumcenter((faceCorner - 1) % 3) );
        }
    }
}

const VectorXd& MeshObject::gaussian_curvature() {
    if (not gaussian_computed) {
        std::cout << " Computing gaussian curvature..." << std::endl;
        igl::gaussian_curvature(_V, _F, _gaussian_curvature);
        gaussian_computed = true;
    }
    return _gaussian_curvature;
}

const VectorXd& MeshObject::principal_curvature_max() {
    query_principal();
    return _principal_curvature_max;
}

const MatrixXd& MeshObject::principal_direction_min() {
    query_principal();
    return _principal_direction_min;
}

const MatrixXd& MeshObject::principal_direction_max() {
    query_principal();
    return _principal_direction_max;
}

const VectorXd& MeshObject::principal_curvature_min() {
    query_principal();
    return _principal_curvature_min;
}

const VectorXd& MeshObject::maxima() {
    if (not maxima_computed) {
        compute_maxima();
        maxima_computed = true;
    }
    return _maxima;
}

const VectorXd& MeshObject::minima() {
    if (not minima_computed) {
        compute_minima();
        minima_computed = true;
    }
    return _minima;
}

// valleys
void MeshObject::compute_minima() {
    query_principal();
    
    std::cout << " Computing minimalities..." << std::endl;            
    
    Gradient gradientCalculator(*this, gradient_radius);    
    MatrixXd gradient = gradientCalculator.compute_gradient(_principal_curvature_min);
    
    _minima.resize(_V.rows());
    
    for(int i=0; i<_V.rows(); ++i){
        _minima[i] = gradient.row(i).dot(_principal_direction_min.row(i));
    }
}

// ridges
void MeshObject::compute_maxima() {
    query_principal();
    
    std::cout << " Computing maximalities..." << std::endl;
    
    Gradient gradientCalculator(*this, gradient_radius);    
    MatrixXd gradient = gradientCalculator.compute_gradient(_principal_curvature_max);
    
    _maxima.resize(_V.rows());
    
    for(int i=0; i<_V.rows(); ++i){
        _maxima[i] = gradient.row(i).dot(_principal_direction_max.row(i));
    }
}

// suggestive contours
void MeshObject::compute_radial_curvature(ViewCamera& cam){
    query_principal();
    
    std::cout << " Computing radial curvature..." << std::endl;
    
    Gradient gradientCalculator(*this, gradient_radius);
    _radial_curvature.resize(_V.rows());
    for (int i = 0; i < _V.rows(); ++i) {
        const RowVector3d proj_dir = cam.get_proj_view_direction(_V.row(i), _V_normals.row(i));
        double cos_sq = proj_dir.dot(_principal_direction_max.row(i));
        
        cos_sq *= cos_sq;
        const double sin_sq = 1.0 - cos_sq;

        double r = _principal_curvature_max(i) * cos_sq + _principal_curvature_min(i) * sin_sq;        
        _radial_curvature(i) = curvatureScale * r;
    }

    _radial_curvature_gradient = gradientCalculator.compute_gradient(_radial_curvature);  
    
    radial_computed = true;
}

const CurvatureDerivative& MeshObject::curvature_derivative_computer() {
    query_derivative();
    return *_curvderv;
}

const ArrayXd& MeshObject::curvature_gradient_angles() {
    query_derivative();
    return _curvatureGradientAngles;
}

const std::vector<Matrix2d>& MeshObject::second_fundamental_form() {
    query_derivative();
    return _secondFundamentalForm;
}

const MatrixXd MeshObject::derivative_of_curvature() {
    query_derivative();
    return _derivativeOfCurvatures;
}

void MeshObject::query_principal() {
    if (not principal_computed) {
        std::cout << " Computing principal curvature..." << std::endl;
        igl::principal_curvature(_V, _F, _principal_direction_min, _principal_direction_max,
                _principal_curvature_min, _principal_curvature_max, sizeRing);
        principal_computed = true;
    }
}

// demarcating
void MeshObject::query_derivative() {
    if(not derivative_computed){
        std::cout << " Computing derivative of curvature..." << std::endl;
        _curvderv = new CurvatureDerivative(*this);
        _derivativeOfCurvatures = _curvderv->compute_coefficients(principal_curvature_min(), principal_curvature_max(),
                                                                         principal_direction_min(), principal_direction_max());
        _secondFundamentalForm = compute_secondFundamentalForm(principal_curvature_min(), principal_curvature_max());
        _curvatureGradientAngles = compute_curvatureGradientAngles(_derivativeOfCurvatures); 
        derivative_computed = true;
    }
}

/**\details Section 3.1 : Defining Demarcating Curves
 * For a smooth surface, the normal curvature in direction v is k(v) = vt * II * v
 * where the symmetric matrix II is the second fundamental form
 *
 * Plus : "Estimating Curvatures and Their Derivatives on Triangle Meshes"
 * Authors : Szymon Rusinkiewicz
 * Section 2 : Background and Previous work
 * The symmetric matrix II [..] can be diagonalized by a rotation
 * of the local coordinate system to give :
 *
 *        k1  0
 *        0   k2
 *
 * where k1 and k2 are the principal curvatures
 * [and the local system are the principal directions] */
vector<Matrix2d> MeshObject::compute_secondFundamentalForm(const VectorXd &principalCurvature_min, const VectorXd &principalCurvature_max) {
    const unsigned int num_vertices = _V.rows();
    vector<Matrix2d> secondFundamentalForm;

    for (unsigned int index_vertex = 0; index_vertex < num_vertices; ++index_vertex) {
        Matrix2d secondFundamentalForm_oneVertex;
        secondFundamentalForm_oneVertex << principalCurvature_min(index_vertex), 0,
                                           0, principalCurvature_max(index_vertex);

        secondFundamentalForm.push_back(secondFundamentalForm_oneVertex);
    }

    return secondFundamentalForm;
}

// cbrt : cubic Root
ArrayXd MeshObject::compute_curvatureGradientAngles (const MatrixXd &derivativeOfCurvatureCoeffs) {
    const ArrayXd a = derivativeOfCurvatureCoeffs.col(0);
    const ArrayXd b = derivativeOfCurvatureCoeffs.col(1);
    const ArrayXd c = derivativeOfCurvatureCoeffs.col(2);
    const ArrayXd d = derivativeOfCurvatureCoeffs.col(3);

    const unsigned int num_curvatureGradient_values = derivativeOfCurvatureCoeffs.rows();
    ArrayXd curvatureGradientAngles{num_curvatureGradient_values};

#pragma omp parallel for
    for (unsigned int i = 0; i < num_curvatureGradient_values; ++i) {
        curvatureGradientAngles(i) = compute_curvatureGradientAngle_oneVertex(a(i), b(i), c(i), d(i));
    }

    return curvatureGradientAngles;
}

/** \details Definition 3.1
 * The curvature gradient is the tangent direction of the maximum normal curvature variation
 * Plus Section 3.2 : Calculation of gp  */
double MeshObject::compute_curvatureGradientAngle_oneVertex (double a, double b, double c, double d) {
    double A, B, C, D;
    double A_normalized, B_normalized, C_normalized;
    double QdividedByTwo, AdividedbyThree;
    double curvatureGradientAngle;

    compute_cubicFormulaTerms(a, b, c, d, A, B, C, D);
    compute_normalizedCubicEquation(A, B, C, D, A_normalized, B_normalized, C_normalized);
    double discriminant = compute_discriminantCubicFormula(A_normalized, B_normalized, C_normalized, QdividedByTwo, AdividedbyThree);

    if (discriminant > 0.0) {
        /* "If there is a single root,
         * the extremal angle corresponding to the maximum is used to determine gp" */
        const double delta_squareRoot = sqrt(discriminant);
        const double u = cbrt( - QdividedByTwo + delta_squareRoot );
        const double v = cbrt( - QdividedByTwo - delta_squareRoot );
        double sinus_square_curvatureGradientAngle = u + v - AdividedbyThree;
        curvatureGradientAngle = find_curvatureGradientAngle_fromSinusSquareAngle(sinus_square_curvatureGradientAngle, a, b, c, d);
    } else {
        // The equation has several roots
        /* "Otherwise, the function in Equation 3 is smoothed
         * with a Gaussian before selecting the global maximum" */
        curvatureGradientAngle = find_curvatureGradientAngle_bySampling(a, b, c, d);
    }

    return curvatureGradientAngle;
}

void MeshObject::compute_cubicFormulaTerms(double a, double b, double c, double d,
                                            double &A, double &B, double &C, double &D) const {
    const double B_square = b * b;
    const double minusThreeCPlusA = -3.0 * c + a;
    const double threeBMinusD = 3.0 * b - d;
    const double twoCMinusA = 2.0 * c - a;
    const double twoBthreeBMinusD = 2.0 * b * threeBMinusD;

    const double threeBMinusD_square = threeBMinusD * threeBMinusD;
    const double twoCMinusA_square = twoCMinusA * twoCMinusA;

    A = minusThreeCPlusA * minusThreeCPlusA + threeBMinusD_square;
    B = 2.0 * twoCMinusA * minusThreeCPlusA - threeBMinusD_square - twoBthreeBMinusD;
    C = twoCMinusA_square + twoBthreeBMinusD + B_square;
    D = - B_square;
}


void MeshObject::compute_normalizedCubicEquation (double A, double B, double C, double D, double &newA, double &newB, double &newC) {
    if ( A != 0.0 ) {
        newA = B / A;
        newB = C / A;
        newC = D / A;
    } else {
        std::cerr << "Warning : A cubic formula is actually quadratic" << std::endl;
        newA = A;
        newB = B;
        newC = C;
    }
}

double MeshObject::compute_discriminantCubicFormula (double A, double B, double C, double &QdividedByTwo, double &AdividedbyThree) const {
    const double A_square = A * A;
    AdividedbyThree = A / 3.0;

    const double P = B - A_square / 3.0;
    const double Q = 2.0 * (A_square * A) / 27.0 - ( AdividedbyThree * B ) + C;

    QdividedByTwo = Q / 2.0;

    return QdividedByTwo * QdividedByTwo + (P * P * P) / 27.0;
}


double MeshObject::find_curvatureGradientAngle_fromSinusSquareAngle (double sinus_square_curvatureGradientAngle,
                                                                            double a, double b, double c, double d) {
    Vector2d possible_sinusCurvatureGradientAngle;
    possible_sinusCurvatureGradientAngle(0) =  sqrt(sinus_square_curvatureGradientAngle);
    possible_sinusCurvatureGradientAngle(1) = - possible_sinusCurvatureGradientAngle(0);

    Vector2d possible_curvatureGradientAngles = possible_sinusCurvatureGradientAngle.array().asin();

    double value0 = compute_valueToMaximize(possible_curvatureGradientAngles(0), a, b, c, d);
    double value1 = compute_valueToMaximize(possible_curvatureGradientAngles(1), a, b, c, d);

    const unsigned int index_maxValue = value0 > value1 ? 0 : 1;

    return possible_curvatureGradientAngles(index_maxValue);
}

double MeshObject::find_curvatureGradientAngle_bySampling (double a, double b, double c, double d) const {
    const unsigned int num_samples = 2000;
    const unsigned int windowSize = 5;

    const ArrayXd angles = ArrayXd::LinSpaced(num_samples, -igl::PI, igl::PI);
    const ArrayXd functionToMaximize = compute_functionToMaximize(angles, a, b, c, d);

    const ArrayXd weights = compute_gaussianFilterForConvolution(windowSize);
    const ArrayXd functionToMaximize_smoothed = compute_gaussianSmoothedFunction(weights, functionToMaximize);

    unsigned int index_maxAngle;
    functionToMaximize_smoothed.maxCoeff(&index_maxAngle);
    return angles(index_maxAngle);
}

ArrayXd MeshObject::compute_functionToMaximize (const ArrayXd &angles, double a, double b, double c, double d) const {
    const ArrayXd cosTheta = angles.cos();
    const ArrayXd cosTheta_square = cosTheta * cosTheta;

    const ArrayXd sinTheta = angles.sin();
    const ArrayXd sinTheta_square = sinTheta * sinTheta;

    ArrayXd functionToMaximize = a * cosTheta_square * cosTheta + 3.0 * b * cosTheta_square * sinTheta;
    return functionToMaximize + 3.0 * c * cosTheta * sinTheta_square + d * sinTheta_square * sinTheta;
}

double MeshObject::compute_valueToMaximize (double angle, double A, double B, double C, double D) const {
    const double cosTheta = cos(angle);
    const double cosTheta_square = cosTheta * cosTheta;

    const double sinTheta = sin(angle);
    const double sinTheta_square = sinTheta * sinTheta;

    double valueToMaximize = A * cosTheta_square * cosTheta + 3 * B * cosTheta_square * sinTheta;
    return valueToMaximize + 3 * C * cosTheta * sinTheta_square + D * sinTheta_square * sinTheta;
}

ArrayXd MeshObject::compute_gaussianFilterForConvolution (unsigned int windowSize) const {
    ArrayXd weights{windowSize};
    const unsigned int half_windowSize = windowSize / 2.0;

    weights(half_windowSize) = exp(half_windowSize);

    for (unsigned int i = 1; i <= half_windowSize; ++i) {
        weights(half_windowSize + i) = exp(half_windowSize - i);
        weights(half_windowSize - i) = weights(half_windowSize + i);
    }

    weights /= weights.sum();

    return weights;
}

ArrayXd MeshObject::compute_gaussianSmoothedFunction (const ArrayXd &gaussianFilter, const ArrayXd &function) const {
    const unsigned int windowSize = gaussianFilter.rows();
    const unsigned int half_windowSize = windowSize / 2;
    const unsigned int num_samples = function.rows();
    ArrayXd gaussianSmoothedFunction = ArrayXd::Zero(num_samples);

    for (unsigned int i = 0; i < half_windowSize; ++i) {
        gaussianSmoothedFunction(i) = function(i);
        gaussianSmoothedFunction(num_samples - half_windowSize + i) = function(i);
    }

    for (unsigned int i = half_windowSize; i < num_samples - half_windowSize; ++i) {
        for (unsigned int k = 0; k < windowSize; ++k) {
            gaussianSmoothedFunction(i) += gaussianFilter(k) * function(i - half_windowSize + k);
        }
    }

    return gaussianSmoothedFunction;
}

