#ifndef MESHCONTOUR_H
#define MESHCONTOUR_H

#include <Eigen/Core>
#include <set>
#include <list>
#include <map>
#include <string>

#include "../Common/Drawable.h"

class VertexNode;

using namespace Eigen;

/**\brief Class for contour node in a ViewGraph
 * \author Bastien Wailly <bastien.wailly@inria.fr>, Inria
 * \details Holds all info relative to a line in 3D and its 2D projected counterpart. 
 * The line is identified with : \n
 *  - a head VertexNode, 
 *  - a tail VertexNode,
 *  - a ContourType,
 *  - a Visibility relative to a ViewCamera. */
class ContourNode : public Drawable{
public:    
    ContourNode(const ContourNode& orig){}
    virtual ~ContourNode(){}

    /**\brief get head vertex coordinates */
    const Vector3d& head_vertex3d();
    
    /**\brief get tail vertex coordinates */
    const Vector3d& tail_vertex3d();
    
    /**\brief get projected head point coordinates */
    const Vector2d& head_vertex2d();
    
    /**\brief get projected tail point coordinates */
    const Vector2d& tail_vertex2d();
    
    /**\brief set head vertex node 
     * \param node vertex to use */
    void setHead(std::shared_ptr<VertexNode> node);
    
    /**\brief set tail vertex node 
     * \param node vertex to use */
    void setTail(std::shared_ptr<VertexNode> node);
    
    /**\brief get head vertex pointer 
     * \return pointer to VertexNode */
    std::shared_ptr<VertexNode> getHead(){ return head; }
    
    /**\brief get tail vertex pointer 
     * \return pointer to VertexNode */
    std::shared_ptr<VertexNode> getTail(){ return tail; }
    
    /**\brief return true if contour is occluded */
    bool isOccluded(){ return visibility == Visibility::OCCLUDED; }
    
    /**\brief return true if contour has not yet being tested for visibility */
    bool isUndefined(){ return visibility == Visibility::UNKNOWN; }
    
    /**\brief set contour visibility 
     * \param v visibility (true for visible, false for hidden) */
    void setVisible(bool v);
    
    /**\brief set contour type 
     * \param t contour type */
    void setType(ContourType t){ type = t; }
      
    /**\brief get contour type 
     * \return ContourType */
    ContourType get_type(){ return type; }
    
    /**\brief test type of contour
     * \param t type to test
     * \return true if contour is of type t */
    bool isType(ContourType t){ return type == t; }
    
    /**\brief check for similarity with another contour 
     * \param c other contour 
     * \return true if contours are similar */
    bool isSimilar(std::shared_ptr<ContourNode> c);
    
    /**\brief check whether this contour contains a given vertex 
     * \param vertex VertexNode to test 
     * \return true if vertex is head or tail of this contour */  
    bool has_vertex(std::shared_ptr<VertexNode> vertex){
        return head == vertex or tail == vertex;
    }
    
    /**\brief check whether this contour is connected to another contour 
     * \param contour ContourNode to test 
     * \return true if both contours share a vertex */
    bool is_sharing_vertex(std::shared_ptr<ContourNode> contour){
        return head == contour->head or tail == contour->head or head == contour->tail or tail == contour->tail;
    }

    /**\brief get all intersections points of this contour 
     * \return list of splits in [0 1] interval 
     * \details 0 is the head, 1 is the tail */
    std::list<float> getSplits();
    
    /**\brief get reference to intersecting contour 
     * \param t split value in [0 1]
     * \return pointer to contour intersecting this at t */
    std::shared_ptr<ContourNode> getIntersectingContour(float t){ return intersections[t]; }
    
    /**\brief add an intersection to this contour 
     * \param s split value in [0 1] : where this contour is split
     * \param c reference to the other intersecting contour */
    void addSplit(float s, std::shared_ptr<ContourNode> c){ intersections[s] = c; }      

    /**\brief draw this contour in an SVG Document 
     * \param doc initialized SVG document 
     * \param str Stroke to use to draw */
    const void Draw(svg::Document& doc, svg::Stroke& str) override;
    
    /**\brief store this contour in viewer edge buffer 
     * \param lines the buffer storing contours 
     * \param color color to draw this contour in */
    const void Draw(Lines& lines, RowVector3d color) override;
    
    /**\brief get visibility status of this contour 
     * \return true if this contour is visible, false otherwise (including when unknown) */
    const bool isVisible() override { return visibility == Visibility::VISIBLE; }  
    
    /**\brief Pure virtual method describing herited class 
     * \return string identifying herited class (see EdgeContourNode, FaceContourNode) */
    virtual const std::string typeinfo() const = 0;
    
    /**\brief Pure virtual method to clone this contour 
     * \return pointer to newly created contour with same type */
    virtual std::shared_ptr<ContourNode> createChild() const = 0;
    
    /**\brief flag for filtering step */
    bool parsed_visibility = false;
    
protected:
    // abstract class
    ContourNode();

    ContourType type;
    Visibility visibility;

    std::shared_ptr<VertexNode> head;
    std::shared_ptr<VertexNode> tail;
    
    std::map<float, std::shared_ptr<ContourNode>> intersections;
};

#endif /* MESHCONTOUR_H */
