#ifndef CONTOURCHAIN_H
#define CONTOURCHAIN_H

#include "../Common/Drawable.h"

class ContourNode;
class VertexNode;
class RaycastHelper;

/**
 * \author Bastien Wailly <bastien.wailly@inria.fr>, Inria
 * \details A chain is a list of adjacent contours/vertices. 
 * It represents a "stroke", in the sense of a line sketched at once. 
 * It is a drawable element. 
 * \brief Class for a contour chain (3d polyline)*/
class ContourChain : public Drawable{
private:
    ContourChain(){};
    
    /**\brief ordered list of vertices of the chain*/     
    std::list<std::shared_ptr<VertexNode>> vertices;
    
    /**\brief ordered list of contours of the chain*/     
    std::list<std::shared_ptr<ContourNode>> contours;
    Visibility visibility;
    ContourType type;

public:
    ~ContourChain(){};
    
    /**\brief Create a chain from a contour
     * \param n Chain starting contour
     * \details The chain will have same visibility and type */       
    ContourChain(std::shared_ptr<ContourNode> n);
    
    /**\brief Push given contour to the front of the chain
     * \param c Some contour*/       
    void push_contour_front(std::shared_ptr<ContourNode> c);
    
    /**\brief Push given contour to the back of the chain
     * \param c Some contour*/    
    void push_contour_back(std::shared_ptr<ContourNode> c);
    
    /**\brief Push given vertex at the front of vertices list
     * \param v Some vertex*/     
    void push_vertex_front(std::shared_ptr<VertexNode> v);
    
    /**\brief Push given vertex at the back of vertices list
     * \param v Some vertex*/     
    void push_vertex_back(std::shared_ptr<VertexNode> v);
    
    /**\brief Set the chain's visibility
     * \param b boolean*/     
    void setVisible(bool b);
    
    /**\brief Check the chain's type
     * \param t ContourType to check
     * \return true if chain is of type t*/     
    bool isType(ContourType t){ return type == t; };
    
    /**\brief Get the type of the chain
     * \return the type of the chain*/     
    ContourType get_type(){ return type; };

    /**\brief Compute this chain's visibility.
     * \param voting_factor percentage of contours to use per chain to determine visibility
     * \param ray reference to RaycastHelper used to determine visibility
     * \param tol numerical tolerance for raycast (max distance between hit and object)*/     
    void computeVisibility(double voting_factor, RaycastHelper& ray, double tol);
    
    /**\brief Make a decision regarding visibility information aggregation
     * \param ballots list of visibility values (visible or not)of constituent contours
     * \details Using majoritarian representation*/     
    void vote_for_visibility(std::vector<bool> ballots);

    /**\brief Draw this chain in an SVG document
     * \param doc SVG document
     * \param str Stroke to use (color, width)*/      
    const void Draw(svg::Document& doc, svg::Stroke& str) override;
    
    /**\brief Prepare the lines to draw in the viewer
     * \param lines line matrices to append to
     * \param color color to draw the chain*/      
    const void Draw(Lines& lines, Eigen::RowVector3d color) override;
    
    /**\brief Set the chain's visibility
     * \return true if this chain is visible*/      
    const bool isVisible() override;
    
    Eigen::Vector3d color;
    
};

#endif
