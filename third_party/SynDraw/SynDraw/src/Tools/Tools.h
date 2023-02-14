#ifndef TOOLS_H
#define TOOLS_H

#include <Eigen/Core>
#include <Eigen/Geometry>
#include <string>
#include <vector>
#include <exception>

#include "../Common/Drawable.h"

class MeshObject;
class ViewCamera;
class ContourNode;
class VertexNode;

using namespace Eigen;

/*!
 *  \addtogroup Records
 *  @{
 */
//! Geometric toolset based on Benard et al. tutorial https://arxiv.org/pdf/1810.01175.pdf Appendix C
namespace Tools {
    double orient3d(Vector3d a, Vector3d b, Vector3d c, Vector3d d);
    bool sameside3d(Vector3d a, Vector3d b, Vector3d c, Vector3d d, Vector3d e);
    double frontside3d(Vector3d a, Vector3d b, Vector3d c, Vector3d d);
    bool is_concave3d(Vector3d a, Vector3d b, Vector3d d, Vector3d e);
    double orient2d(Vector2d a, Vector2d b, Vector2d c);
    bool sameside2d(Vector2d a, Vector2d b, Vector2d c, Vector2d d);

    /**\brief compute concavity of a mesh edge
     * \param mesh MeshObject reference
     * \param e edge index
     * \return concavity */
    double concavity(MeshObject& mesh, int e);
    
    /**\brief returns true if mesh edge is concave 
     * \param mesh MeshObject reference
     * \param edge edge index 
     * \return true if mesh edge is concave  */
    bool is_edge_concave(MeshObject& mesh, int e);
    
    /**\brief compute face orientation relative to camera 
     * \param mesh MeshObject reference 
     * \param cam camera to use 
     * \param f mesh face index 
     * \return camera forward dot face normal 
     * \details Camera's view direction is computed based on the ProjectionType */
    double ndotv_face(MeshObject& mesh, ViewCamera& cam, int f);
    
    /**\brief compute back face
     * \param mesh MeshObject reference 
     * \param cam camera to use 
     * \param f face index
     * \return true if face f is backfacing */
    bool is_face_backfacing(MeshObject& mesh, ViewCamera& cam, int f);

    /**\brief compute and get angle between two contours 
     * \param c1 pointer to contour 
     * \param c2 pointer to contour 
     * \return angle in radians in the [0, pi] interval */
    double get_angle(std::shared_ptr<ContourNode> c1, std::shared_ptr<ContourNode> c2);
    
    /**\brief apply tolerance factor to a float value 
     * \param epsilon float to test 
     * \param t tolerance to use 
     * \return true if |epsilon| < t */
    bool isApproxZero(double epsilon, double t);
    
    /**\brief connect a ViewGraph contour node to a vertex node
     * \param c pointer to contour to connect
     * \param v pointer to vertex to connect 
     * \param s contour side to connect v to */
    void connect(std::shared_ptr<ContourNode> c, std::shared_ptr<VertexNode> v, Side s);
    
    /**\brief disconnect all vertices from a contour node 
     * \param c pointer to contour node */
    void disconnect(std::shared_ptr<ContourNode> c);      
}
/*! @} End of Doxygen Groups*/
#endif /* TOOLS_H */
