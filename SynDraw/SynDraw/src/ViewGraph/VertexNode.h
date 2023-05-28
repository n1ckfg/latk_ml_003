#ifndef SINGULARITYNODE_H
#define SINGULARITYNODE_H

#include <Eigen/Core>
#include <set>

#include "../Common/ViewCamera.h"
#include "../Common/MeshObject.h"
#include "../Common/Drawable.h"

class ContourNode;

/**\brief Class for vertex node in a ViewGraph
 * \author Bastien Wailly <bastien.wailly@inria.fr>, Inria
 * \details Holds all info relative to a vertex in 3D and its 2D projected counterpart. 
 * The vertex is identified with : \n
 *  - a 3D set of coordinates and their 2D projection,
 *  - a set of connected ContourNode,
 *  - a VertexType, to store singularity info. */
class VertexNode : public Drawable{
public:
    /**\brief Create a vertex node with a mesh and an index 
     * \param m given mesh 
     * \param cam camera used for projection 
     * \param v vertex index */
    VertexNode(MeshObject& m, ViewCamera& cam, int v);
    
    /**\brief Create a vertex node with a set of coordinates
     * \param point 3D coordinates
     * \param cam camera used for projection */    
    VertexNode(Vector3d point, ViewCamera& cam);
    
    VertexNode(const VertexNode& orig);   
    virtual ~VertexNode();  
    
    /**\brief get vertex 3D coordinates */
    const Vector3d& point3d(){ return _point3d; }
    
    /**\brief get vertex 2D coordinates */    
    const Vector2d& point2d(){ return _point2d; }
    
    /**\brief add a contour connection to this vertex 
     * \param c the contour to connect */
    void addConnection(std::shared_ptr<ContourNode> c);
    
    /**\brief remove a contour connection from this vertex
     * \param c the contour to disconnect */    
    void delConnection(std::shared_ptr<ContourNode> c);
    
    /**\brief get the set of all connected contours
     * \return set of pointers to ContourNode */    
    const std::set<std::shared_ptr<ContourNode>>& adjacent_contours(){ return connections; }
    
    /**\brief set vertex type 
     * \param t the new type */
    void setType(VertexType t);
    
    /**\brief get this contour type
     * \return VertexType value */    
    const VertexType type(){ return _type; }
    
    /**\brief check if this vertex is a singularity based on its current type 
     * \return true if it is a singularity */
    bool isSingularity(){ return _type != VertexType::V_JUNCTION; }
    
    /**\brief check vertex current visibility status 
     * \return true if visible 
     * \details a vertex is visible if it is connected to at least one visible contour */
    const bool isVisible() override;    
    
    /**\brief draw this vertex in an SVG Document 
     * \param doc initialized SVG document 
     * \param str Stroke to use to draw */    
    const void Draw(svg::Document& doc, svg::Stroke& str) override;
    
    /**\brief store this vertex in viewer buffer 
     * \param lines the buffer
     * \param color color to draw this vertex in 
     * \deprecated */    
    const void Draw(Lines& lines, Eigen::RowVector3d color) override {}
    
    /**\brief draw this vertex in viewer
     * \param v the viewer to draw in
     * \param color color to draw this vertex in */ 
    const void Draw(igl::opengl::glfw::Viewer& v, Eigen::RowVector3d color);

private:
    VertexNode();
    
    VertexType _type;
        
    std::set<std::shared_ptr<ContourNode>> connections;
    
    Vector3d _point3d;
    Vector2d _point2d;
};

#endif /* SINGULARITYNODE_H */

