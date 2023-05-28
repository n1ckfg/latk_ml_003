#ifndef VIEWCAMERA_H
#define VIEWCAMERA_H

#include <Eigen/Core>
#include <Eigen/Geometry>
#include <igl/opengl/glfw/Viewer.h>
#include "../Tools/Properties.h"
#include "../Tools/ProjectionType.h"


using namespace Eigen;

/**\brief 3D Camera parameters
 * \author Bastien Wailly <bastien.wailly@inria.fr>, Inria
 * \details Camera class holding camera parameters and accessors.*/
class ViewCamera {
public:
    ViewCamera();
    ViewCamera(const ViewCamera& orig);
    virtual ~ViewCamera();

//    /**\brief set model matrix
//     * \details model matrix is currently not used.
//     * \param m model matrix to push
//     */
//    void setModel(Matrix4f m);
    
    /**\brief set view matrix
     * \details coordinates are expressed in world frame
     * \param p position of camera
     * \param t position of camera target
     * \param u up vector of camera*/    
    void setView(Vector3f p, Vector3f t, Vector3f u);
    
    /**\brief set proj matrix from frustum values
     * \details The current type of projection is took into account to compute proj matrix
     * \param l frustum left plane
     * \param r frustum right plane
     * \param t frustum top plane
     * \param b frustum bottom plane
     * \param n frustum near plane
     * \param f frustum far plane*/    
    void setProj(float l, float r, float t, float b, float n, float f);
    
    /**\brief Invert top and bottom values of projection frustum
     * \details this is used to counteract the viewport y axis inversion */
    void invert_proj();
    
    /**\brief set viewport 
     * \details the viewport will determine the "size" (and frame) of output SVG.
     * \param v viewport vector4*/    
    void setViewport(Vector4f v);
    
    /**\brief set camera parameters from viewer instance 
     * \details compute all parameters so that they match the viewer's camera
     * \param viewer reference to viewer instance*/    
    void setFromViewer(igl::opengl::glfw::Viewer& viewer);

    /**\brief get the current frustum top plane value
     * \return proj.top*/
    float get_top(){ return top; };
    
    /**\brief get the current frustum bottom plane value
     * \return proj.bottom*/    
    float get_bottom(){ return bottom; };
    
    /**\brief get the current frustum near plane value
     * \return proj.near*/    
    float get_near(){ return near; };
    
    /**\brief get the current frustum far plane value
     * \return proj.far*/    
    float get_far(){ return far; };
    
    /**\brief get the current frustum left plane value
     * \return proj.left*/    
    float get_left(){ return left; };
    
    /**\brief get the current frustum right plane value
     * \return proj.right*/    
    float get_right(){ return right; };
        
    /**\brief get the current camera up vector
     * \return view.up*/
    const Vector3f& get_up(){ return up; };
    
    /**\brief get the current camera position
     * \return view.position*/    
    const Vector3f& get_position(){ return position; };
    
    /**\brief get the current camera target position
     * \return view.target*/    
    const Vector3f& get_target(){ return target; };
    
    /**\brief get the current forward vector
     * \details forward = target - position
     * \return view.forward*/    
    Vector3f get_forward(){ return target - position; };
    
    /**\brief get the current projection type
     * \details either orthographic or perspective
     * \return projection type*/    
    const ProjectionType& get_projType(){ return projType; };
    
    /**\brief get the current projection matrix
     * \return 4x4 projection matrix*/    
    const Matrix4f& get_proj(){ return proj; };
    
    /**\brief get the current view matrix
     * \return 4x4 view matrix*/      
    const Matrix4f& get_view(){ return view; };
    
    /**\brief get the current viewport vector
     * \return viewport values*/     
    const Vector4f& get_viewport(){ return viewport; }    
    
    /**\brief get the normalized view direction to a point
     * \details current projection type is took into account
     * \param p the point whose direction is returned
     * \return normalized direction vector to p*/      
    RowVector3d get_view_direction(RowVector3d p);
    
    /**\brief compute and get the normalized view direction to a point projected onto the radial plane
     * \details used by suggestive contours exclusively (for now)
     * \param p the point whose direction is returned
     * \param normal the normal of the mesh at the point
     * \return normalized direction vector to p, projected on radial plane*/     
    RowVector3d get_proj_view_direction(RowVector3d p, RowVector3d normal);
    
    /**\brief update properties from this camera
     * \param props Properties to update */
    void save_to_props(std::shared_ptr<Properties> props);
    
    /**\brief set this camera from a Properties instance
     * \param props the Properties used to update the camera */
    void setFromProperties(Properties& props);    

    /**\brief project 3d point (float) and get viewport coordinates
     * \param p the point to project
     * \return projected point (viewport frame) */
    Vector2f project_point(Vector3f p);
    
    /**\brief project 3d point (double) and get viewport coordinates
     * \param p the point to project
     * \return projected point (viewport frame) */
    Vector2d project_point(Vector3d p);    
    
    /**\brief apply frustum culling to 3d point
     * \param p the 3d point to test 
     * \return true if point is culled */
    bool frustum_culling(Vector3d p);
    
    /**\brief set camera instance to default */
    void setDefault();
    
    /**\brief print camera properties on stdout */
    void print();
    
    /**\brief show camera frustum in viewer 3d scene */
    void show(igl::opengl::glfw::Viewer& v);

private:
    /**\brief Structure to hold Plane equation factors
     * \details equation is Ax + Bx + Cx + D = 0 */
    struct Plane
    {
        double A, B, C, D;
    };

    /**\brief Structure to hold frustum planes */
    struct Frustum
    {
        Plane top, bottom, right, left, near, far;
    };    

    // View info
    Vector3f target;
    Vector3f position;
    Vector3f up;
    Matrix4f view;

    // Projection info
    ProjectionType projType;
    float left, right, top, bottom, near, far;
    Matrix4f proj;

    // Viewport info
    Vector4f viewport;
    
    // Frustum planes for clipping
    Frustum frustum;

    Matrix4f VP;
    
    /**\brief compute frustum from current matrices*/
    void compute_frustum();
    
    /**\brief compute signed distance between a given plane and a point
     * \param p point in world space
     * \param pl given plane 
     * \return signed distance*/
    double point_plane_signed_distance(Vector3d p, Plane pl);
    
    /**\brief normalize Plane equation
     * \param p in-out Plane reference*/
    void normalize_plane(Plane& p);
    
    /**\brief extract projection parameters from proj matrix */
    void decompose_projmatrix();

    /**\brief get inverse view coordinates of a point/vector
     * \param vec point coordinates in camera frame 
     * \return coordinates of vec in world space */
    Vector3f inverse_view(Vector3f vec);
    
    /**\brief get inverse view coordinates of a point/vector
     * \param vec point coordinates in camera frame 
     * \return coordinates of vec in world space */    
    Vector3d inverse_view2(Vector3d vec);
    
    void computeProjectionMatrix();
    void computeViewMatrix();
    void updateVP();    
};

#endif /* VIEWCAMERA_H */
