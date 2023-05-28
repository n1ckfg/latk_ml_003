#ifndef BSP_H
#define BSP_H

#include <Eigen/Core>
#include <Eigen/Geometry> 
#include <vector>

#include "../Common/ViewCamera.h"

class ContourNode;

/**\brief Binary Space Partition graph node and algorithm
 * \author Bastien Wailly <bastien.wailly@inria.fr>, Inria
 * \details Each node is a contour or a contour part in image space. We compute the BSP to find all intersecting contours.
 * Once We know all intersections, we can add singularities (T vertices) into the graph when it applies. */
class bsp_node{  
private:
    Eigen::Vector2d P1;
    Eigen::Vector2d P2;            
    Eigen::Vector2d dir;
    Eigen::Vector2d normal;
    double norm;
    
    /**\brief the infinite reference line on which this contour resides 
     * \details this line is used when we are using this node as reference for space partitionning*/
    Eigen::Hyperplane<double, 2> line;
    
    /**\brief the line segment on which this contour resides 
     * \details this segment is used when we compare this node to another reference line*/
    Eigen::ParametrizedLine<double, 2> segment;
    
    /*\brief contour of which this node is a part */
    std::shared_ptr<ContourNode> contour;

public:
    // Constructors
    ~bsp_node(){};
    bsp_node(std::shared_ptr<ContourNode> c);
    bsp_node(Eigen::Vector2d p1, Eigen::Vector2d p2, std::shared_ptr<ContourNode> c); 

    /**\brief Compute BSP of a list of contours, given a viewpoint.
     * \details The input list of contours must have been converted to a list of nodes.
     *          There is no output. The intersection informations are stored in each 
     *          ContourNode instance under the attribute splits.
     * \param cnodes list of bsp nodes previously created
     * \param camera camera used to project points
     * \param tol numerical tolerance for vertex equality*/
    static void compute_BSP(std::vector<std::shared_ptr<bsp_node>>& cnodes, ViewCamera& camera, double tol);
    
    /**\brief get first point of contour part
     * \return image space coordinates vector*/
    Eigen::Vector2d getP1();
    
    /**\brief get first point of contour part
     * \return image space coordinates vector*/
    Eigen::Vector2d getP2();
    
    /**\brief get instance of ContourNode of which this node is from
     * \return pointer to contour in view graph*/
    std::shared_ptr<ContourNode> getContour();

    /**\brief get image space intersection between current segment and another reference line
     * \param lin reference line (2-hyperplane)
     * \return 2d coordinates of the intersection*/
    Eigen::Vector2d get_intersection_point_2d(std::shared_ptr<bsp_node> lin);

    /**\brief get object-space coordinates of image-space intersection between current segment and another reference line
     * \details the 3d coordinates are interpolated between the contour's vertices
     * \param lin reference line (2-hyperplane)
     * \return 2d coordinates of the intersection*/
    Eigen::Vector3d get_intersection_point_3d(std::shared_ptr<bsp_node> lin);
    
    /**\brief indicate an intersection between this node's segment and another reference line
     * \param lin reference line (2-hyperplane)
     * \details the intersection information is store into the ContourNode instance.
     * later in the process, the ContourNode will be converted to a list of ContourNode
     * using this data.*/
    void add_contour_vertex(std::shared_ptr<bsp_node> lin);
    
    /**\brief print this node info on stdout*/
    void print();

    private: 
    bsp_node();;  

};

#endif /* BSP_H */

