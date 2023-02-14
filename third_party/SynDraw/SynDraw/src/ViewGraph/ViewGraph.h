#ifndef VIEWGRAPH_H
#define VIEWGRAPH_H

#include <Eigen/Core>
#include <iostream>
#include <vector>
#include <map>

#include "VertexNode.h"
#include "EdgeContourNode.h"
#include "FaceContourNode.h"
#include "../Common/MeshObject.h"
#include "../Common/ViewCamera.h"
#include "../Tools/Properties.h"

#include "../ContourExtractor/OcclusionContoursExtractor.h"
#include "../ContourExtractor/SharpFeaturesExtractor.h"
#include "../ContourExtractor/SuggestiveContoursExtractor.h"
#include "../ContourExtractor/RidgesExtractor.h"
#include "../ContourExtractor/ValleysExtractor.h"
#include "../ContourExtractor/DemarcatingCurvesExtractor.h"
#include "../ContourExtractor/BoundariesExtractor.h"
#include "../ContourExtractor/SmoothOcclusionContoursExtractor.h"

using namespace std;

/**\brief Class for contour graph used to compute the set of 3D lines from a mesh and a camera as described in [Benard 2019].
 * \author Bastien Wailly <bastien.wailly@inria.fr>, Inria
 * \details Holds all info relative to a graph of 3D lines. It is computed thanks to : a camera, a mesh, input parameters.\n
 * The graph is composed of : \n
 *  - Vertex Nodes : 3d points 
 *  - Contour Nodes : connection between 2 vertices */
class ViewGraph {
public:
    /**\brief Create a view graph with a given mesh, camera and input parameters. 
     * \param obj the mesh
     * \param cam the camera 
     * \param p input properties */
    ViewGraph(MeshObject& obj, ViewCamera& cam, Properties& p);
    virtual ~ViewGraph();
    
    /**\brief Compute and return the set of 3D contours.
     * \return list of labelled ContourNode representing lines in 3D space. */
    const vector<shared_ptr<ContourNode>>& compute_contours();  
    
    /**\brief Return singularities of the graph. Singularities are vertice nodes where visibility of the graph might change.
     * \return list of VertexNode that are singularities. */
    const vector<shared_ptr<VertexNode>> get_singularities();
    
private:
    MeshObject& mesh;
    ViewCamera& camera;
    Properties& props;
    
    // contour extractors
    BoundariesExtractor boundaries;
    SharpFeaturesExtractor sharpFeatures;
    OcclusionContoursExtractor occlusionContours;
    SuggestiveContoursExtractor suggestiveContours;
    ValleysExtractor valleys;
    RidgesExtractor ridges;
    DemarcatingCurvesExtractor demarcatingCurves;
    SmoothOcclusionContoursExtractor smoothContours;
    
    // the graph itself
    vector<shared_ptr<ContourNode>> contours;
    vector<shared_ptr<VertexNode>> vertices; 

    // differentiate edge and face contours
    vector<shared_ptr<EdgeContourNode>> edge_contours;
    vector<shared_ptr<FaceContourNode>> face_contours;
    vector<shared_ptr<FaceContourNode>> unfiltered_face_contours;    

    // data structures factories for unicity
    map<int, shared_ptr<EdgeContourNode>> mesh_edge_contours;
    map<int, shared_ptr<VertexNode>> mesh_vertices;   
    
    /**\brief Factory method for edge contour nodes to insure unicity and ease adjacency.
     * \param e edge index in the mesh
     * \details Returns the EdgeContourNode instance corresponding to this mesh edge. 
     * Will create an EdgeContourNode pointer if mesh edge e has no assigned node yet.\n
     * FaceContourNode instances may not be not unique per face, so we need this only for contours lying on edges.*/
    shared_ptr<EdgeContourNode> get_edge_contour_node(int e);
    
    /**\brief Factory method for vertex nodes to insure unicity and ease adjacency.
     * \param v vertex index in the mesh
     * \details Returns the VertexNode instance corresponding to this mesh edge. 
     * Will create a VertexNode pointer if mesh edge e has no assigned node yet.*/    
    shared_ptr<VertexNode> get_vertex_node(int v);  
    
    /**\brief Check if a VertexNode is a singularity and set its type accordingly, then return the type. 
     * \param vn VertexNode pointer 
     * \param v Vertex mesh index (if applicable) 
     * \return VertexType value */
    VertexType is_singularity(shared_ptr<VertexNode> vn, int v = -1);
    
    /**\brief Check if a VertexNode is a boundary cusp, as described in [Benard 2019] chapter 4.3, type (3)
     * \param vn VertexNode instance
     * \param v vertex index, because boundary cusps are all mesh vertices.
     * \return true if vertex vn is a boundary cusp */
    bool is_boundary_cusp(shared_ptr<VertexNode> vn, int v);
    
    /**\brief Check if a VertexNode is a regular cusp (curtain fold), as described in [Benard 2019] chapter 4.3 , type (4)
     * \param vn VertexNode instance
     * \return true if vertex vn is a cusp */    
    bool is_cusp(shared_ptr<VertexNode> v);
    
    /**\brief Check if a VertexNode is a bifurcation, as described in [Benard 2019] chapter 4.3, type (5)
     * \param vn VertexNode instance
     * \return true if vertex vn is a bifurcation */    
    bool is_bifurcation(shared_ptr<VertexNode> v);
    
    /**\brief Check if a VertexNode is a surface intersection, as described in [Benard 2019] chapter 4.3, type (2)
     * \param vn VertexNode instance
     * \return true if vertex vn is an intersection on the mesh */    
    bool is_surface_intersection(shared_ptr<VertexNode> vn);
    
    /**\brief Compute all image space intersections given the current setup. 
     * \details Image space intersections are singularity vertices described in [Benard 2019] chapter 4.3, type (1).\n
     * They are computed using a BSP, and stored in individual contour instances. */
    void compute_image_space_intersections();
    
    /**\brief Parse mesh edges and compute contours lying on edges */
    void compute_edge_contours();
    
    /**\brief Parse mesh faces and compute contours lying on faces */    
    void compute_face_contours();
    
    /**\brief Apply filtering to a subset of face contours with given parameters
     * \details see ContourFilter for details.
     * \param cont ContourExtractor containing the subset of contours 
     * \param hys set to true to use hysteresis thresholding (HT)
     * \param fil set to true to use visibility thresholding (VT)
     * \param thr threshold for HT 
     * \param tolthr tolerance for HT 
     * \param minvis minimum visibility
     * \param maxvis maximum visibility */
    void filter_faces_contours(CrossValueContoursExtractor& cont, bool hys, bool fil, double thr, double tolthr, double minvis, double maxvis);   
    
    /**\brief Compute filtering step of generation pipeline. See ContourFilter for details. */
    void compute_filtering();

    /**\brief Compute visibility step of generation pipeline, after all singularities are computed. */
    void compute_visibility();
    
    /**\brief Recursively propagate the visibility of a contour to all adjacent contours.
     * \param c previous contour that was set 
     * \param v previous vertex of the contour c that was parsed for adjacency */
    void propagate_visibility(shared_ptr<ContourNode> c, shared_ptr<VertexNode> v);
    
    /**\brief Apply frustum culling to an edge 
     * \param e edge index */
    bool frustum_culling_edge(int e);
    
    /**\brief Apply frustum culling to a face 
     * \param f face index */    
    bool frustum_culling_face(int f);
};

#endif /* VIEWGRAPH_H */

