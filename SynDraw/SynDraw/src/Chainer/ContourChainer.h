#ifndef CONTOURCHAINER_H
#define CONTOURCHAINER_H

#include <list>
#include <set>
#include <vector>

#include "ContourChain.h"

class ContourNode;
class VertexNode;
class RaycastHelper;

/**\brief ContourChain Factory class 
 * \author Bastien Wailly <bastien.wailly@inria.fr>, Inria
 * \details Utility class to convert a list of ContourNode to a list of ContourChain. */
class ContourChainer {
public:
    /**\brief Construct chainer from list of contours and max chain angle.
     * \param contours all contours to build chains from
     * \param a max angle between two edges (in degrees)*/       
    ContourChainer(const std::vector<std::shared_ptr<ContourNode>>& contours, double a);
    ContourChainer(const ContourChainer& orig);
    virtual ~ContourChainer();

    /**\brief Compute contours chaining
     * \return list of chains
     * \details From all the contours passed at construction, build the chained contours by recursively parsing adjacent contours.*/       
    std::list<std::shared_ptr<ContourChain>>& build_chains();
    
    /**\brief Determine the visibility of the chain.
     * \param ray reference to RaycastHelper used to determine visibility
     * \param tol numerical tolerance for raycast (max distance between hit and object)
     * \param voting_factor percentage of contours to use per chain to determine visibility
     * \details Determines the visibility of the chain by parsing some of its contours and using raycasts when needed*/       
    void compute_chains_visibility(RaycastHelper& ray, double tol, double voting_factor);

private:
    ContourChainer();    
    
    const std::vector<std::shared_ptr<ContourNode>> input_contours;
    
    std::set<std::shared_ptr<ContourNode>> tochain;
    std::list<std::shared_ptr<ContourChain>> chains;
    double angle;

    /**\brief Recursively chain a contours with one of its neighbours at a vertex
     * \param ch the current chain
     * \param c the next contour to chain
     * \param prev the previous parsed vertex
     * \param side the side on which we are parsing current contour
     * \param init flag for first use of the method
     * \details This method is used to parse contours by their adjacency and side to determine whether or not we can push it to a given chain.
     * We go from a random contour and parse neighbouring contours both directions, chaining until a singularity is hit, angle is too high, or the contours are of different types.*/   
    void chain(std::shared_ptr<ContourChain> ch, std::shared_ptr<ContourNode> c, std::shared_ptr<VertexNode> prev, Side side, bool init);
};

#endif /* CONTOURCHAINER_H */
