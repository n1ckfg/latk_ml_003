#ifndef RAYCASTHELPER_H
#define RAYCASTHELPER_H

#include <igl/embree/EmbreeIntersector.h>
#include <igl/Hit.h>

#include "../Common/MeshObject.h"
#include "../Common/ViewCamera.h"
#include "../ViewGraph/EdgeContourNode.h"
#include "../ViewGraph/FaceContourNode.h"

/**\brief Raycast utility class to determine contour visibility
 * \author Bastien Wailly <bastien.wailly@inria.fr>, Inria */
class RaycastHelper {
public:
    /**\brief init a raycaster on a mesh with a given camera*/
    RaycastHelper(MeshObject& mesh, ViewCamera& cam);
    RaycastHelper(const RaycastHelper& orig);
    virtual ~RaycastHelper();
    
    /**\brief check if contour is visible by launching a ray
     * \param node the ContourNode to consider
     * \param tol numerical tolerance (max distance between hit and contour)
     * \param interpolated true if we are using interpolated occlusion contours
     * \return true if contour is not occluded (ray hit contour within tol)*/
    bool isVisible(std::shared_ptr<ContourNode> node, double tol, bool interpolated = false);
    
private:
    RaycastHelper();
    MeshObject mesh;
    ViewCamera camera;
    
    igl::embree::EmbreeIntersector embree;
    
    /**\brief launch a ray to a point
     * \param p 3d point ray target
     * \return Hit values*/
    igl::Hit launch_ray(Vector3d p);    
    
};

#endif /* RAYCASTHELPER_H */

