#ifndef MESHDRAWING_H
#define MESHDRAWING_H

#include "ViewCamera.h"
#include "MeshObject.h"
#include "../ViewGraph/ViewGraph.h"
#include "../Chainer/ContourChainer.h"
#include "../Tools/Properties.h"
#include "../Tools/InputParser.h"

#include <igl/opengl/glfw/Viewer.h>
#include <vector>
#include <string>

/**\brief Drawing generator container 
 * \author Bastien Wailly <bastien.wailly@inria.fr>, Inria
 * \details Container class that allows one to extract contours from a mesh and a camera.
 *  One of the resulting output is the view graph of the drawing, containing all contours, vertices and singularities. */
class MeshDrawing {
public:
    MeshDrawing(){};
    /** \brief Constructor from properties list (mesh and camera are in there) 
     *  \param p Properties attributes instance*/
    MeshDrawing(std::shared_ptr<Properties> p);
    virtual ~MeshDrawing();
        
    /** \brief extract all contours, view graph, chains and visibility */
    bool compute(bool camera_changed = false);
    
    /** \brief Loads the mesh from disk */
    void load_mesh(); 
    
    /** \brief load the mesh in the viewer 
     *  \param v Reference to igl::opengl::glfw::Viewer instance*/
    void update_viewer_mesh(igl::opengl::glfw::Viewer& v);
    
    /** \brief update viewer from properties 
     *  \param v Reference to igl::opengl::glfw::Viewer instance*/
    void update_viewer_from_properties(igl::opengl::glfw::Viewer& v);

    /** \brief draw / redraw objects in viewer
     *  \param v Reference to igl::opengl::glfw::Viewer instance*/    
    void update_viewer_content(igl::opengl::glfw::Viewer& v);    

    /** \brief reset drawing camera to default values */
    void set_camera_default();    
    
    /** \brief update drawing camera from properties */
    void update_camera_from_props();
    
    /** \brief set drawing camera from the viewer's camera
     *  \param v Reference to igl::opengl::glfw::Viewer instance*/     
    void update_camera_from_viewer(igl::opengl::glfw::Viewer& v, bool b);    
    
    /** \brief save current viewer GUI to drawing properties
     *  \param v Reference to igl::opengl::glfw::Viewer instance*/     
    void update_props_from_viewer(igl::opengl::glfw::Viewer& v);

    
    /** \brief draw singularity vertices in the viewer
     *  \param v Reference to igl::opengl::glfw::Viewer instance*/         
    void show_singularities(igl::opengl::glfw::Viewer& v);
    
    /** \brief set color of the mesh to gaussian curvature values
     *  \param v Reference to igl::opengl::glfw::Viewer instance*/     
    void show_gaussian_curvature(igl::opengl::glfw::Viewer& v );
    
    /** \brief set color of the mesh to principal max curvature values
     *  \param v Reference to igl::opengl::glfw::Viewer instance*/     
    void show_principal_curvature_max(igl::opengl::glfw::Viewer& v);
    
    /** \brief set color of the mesh to principal min curvature values
     *  \param v Reference to igl::opengl::glfw::Viewer instance*/     
    void show_principal_curvature_min(igl::opengl::glfw::Viewer& v);
    
    /** \brief set color of the mesh to radial curvature values
     *  \param v Reference to igl::opengl::glfw::Viewer instance
     *  \param camera_changed flag to indicate if we should recompute radial curvature*/     
    void show_radial_curvature(igl::opengl::glfw::Viewer& v, bool camera_changed);
    
    /** \brief set color of the mesh to curvature minimality values
     *  \param v Reference to igl::opengl::glfw::Viewer instance*/     
    void show_minimality(igl::opengl::glfw::Viewer& v);
    
    /** \brief set color of the mesh to curvature maximality values
     *  \param v Reference to igl::opengl::glfw::Viewer instance*/     
    void show_maximality(igl::opengl::glfw::Viewer& v);
    
    /** \brief reset mesh status and update properties
     *  \param rad reset radial curvature
     *  \param pri reset principal curvature
     *  \param ext reset extremalities
     *  \param gau reset gaussian curvature
     *  \param der reset curvature derivative
     *  \param sf new value of curvature scale factor
     *  \param gr new value of gradient radius
     *  \details The next time we query the mesh data, the data that was reset will be recomputed.*/     
    void reset_mesh(bool rad, bool pri, bool ext, bool gau, bool der, int sr, double sf, double gr);
    
    /** \brief make sure the chains have been computed */
    void ensure_chains(); 
    
    /** \brief recompute chains */
    void rebuild_chains();
    
    /** \brief update lines to be drawn in viewer from extracted contours */
    void compute_lines();   
    
    /** \brief update lines to be drawn in viewer from computed chains */
    void compute_chains();
    
    /** \brief reset all computed lines */
    void reset();
    
    /** \brief create an SVG at the given path
     *  \param path_to_svg path to SVG document 
     *  \details all precomputed lines will be drawn in SVG. 
     *   If chains have been computed, their color depends on their type*/    
    void create_svg_file(string path_to_svg);   
    
private:
    MeshObject mesh;
    ViewCamera camera;    
    std::shared_ptr<Properties> props;    
    std::shared_ptr<ViewGraph> graph;
    
    Lines o_lines, sh_lines, s_lines, r_lines, v_lines, d_lines, b_lines, ch_lines;  

    std::vector<std::shared_ptr<ContourNode>> contours;
    std::list<std::shared_ptr<ContourChain>> chains;
    std::vector<std::shared_ptr<VertexNode>> singularities;
    
    bool chains_computed;
    
    /** \brief extract contours given current camera and mesh */    
    void build_contours();
    
    /** \brief compute chains from previously extracted contours */
    void build_chains();     
    
    /** \brief add all current chains to an SVG document */
    void add_chains_to_svg(svg::Document& svg);
    
    /** \brief add all current contours to an SVG document */
    void add_contours_to_svg(svg::Document& svg);
    
    /** \brief add all current singularities to an SVG document */
    void add_singularities_to_svg(svg::Document& svg);
    
    /** \brief compute PARULA color map from a list of values 
     *  \param values values to be color mapped 
     *  \return color map from input values */
    Eigen::MatrixXd compute_colors (const VectorXd &values) const;
    
    /** \brief estimate min and max value of a values list 
     *  \param values input values
     *  \param minValue output estimated min value
     *  \param maxValue output estimated max value*/
    void estimateRangeOfValues (const VectorXd &values, double &minValue, double &maxValue) const;
};

#endif /* MESHDRAWING_H */

