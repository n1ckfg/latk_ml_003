#ifndef CROSSEDGEEXTRACTOR_H
#define CROSSEDGEEXTRACTOR_H

#include "../ContourExtractor/CrossStructures.h"
#include "../ContourExtractor/SharpFeaturesExtractor.h"
#include "../Common/ViewCamera.h"

#include <unordered_set>

#include <vector>

class FaceContourNode;

/**\brief Contour filtering utility class
 * \author Bastien Wailly <bastien.wailly@inria.fr>, Inria
 * \details Utility class used to filter a list of contours.
 * 
 * 3 kinds of filtering can be applied : 
 * 
 * - filtering based on visibility (edge normal)
 * - hysteresis filtering based on curvature gradient
 * - filtering based on occlusions
 * */
class ContourFilter {
public:
    /**\brief create a filter instance with given parameters
     * \param cam the camera used
     * \param thr threshold for gradient-based filtering
     * \param tolthr tolerance for gradient-based filtering
     * \param minv low threshold for visibility filtering
     * \param maxv high threshold for visibility filtering*/
    ContourFilter(ViewCamera& cam, double thr, double tolthr, double minv, double maxv);
    
    /**\brief apply filtering to a list of ContourNode 
     * \param hysteresis apply hysteresis thresholding ?
     * \param visibility apply visibility filtering ?
     * \param hidden apply occlusion filtering ?
     * \return filtered list of ContourNode*/
    std::vector<std::shared_ptr<FaceContourNode>> filter_contours(std::vector<std::shared_ptr<FaceContourNode>>& contoursIn,  bool hysteresis=true, bool visibility=true, bool hidden=false);

private:
    double threshold;
    double tolerance;
    double minVisibility;
    double maxVisibility;
    
    ViewCamera& camera;
    std::vector<std::shared_ptr<FaceContourNode>> buffer;
    
    /**\brief apply hysteresis thresholding on a list of input ContourNode
     * \details the method is described in [Canny 1986] and used in [Panozzo 2010] section 3.1 Object-space algorithm.
     * the filtering information is stored inside each ContourNode instance. 
     * We pass v and h for optimization purposes.
     * \param contoursIn input contours
     * \param v are we using visibility filtering also ?
     * \param h are we using occlusion filtering also ?*/
    void apply_hysteresis(std::vector<std::shared_ptr<FaceContourNode>>& contoursIn, bool v, bool h);
    
    /**\brief apply visibility_filtering to a list of ContourNode
     * \details for more info, check [DeCarlo 2003] section 2.3 Viewpoint dependence and stability.
     * the filtering information is stored inside each ContourNode instance.
     * \param contoursIn input contours*/
    void apply_visibility(std::vector<std::shared_ptr<FaceContourNode>>& contoursIn);
    
    /**\brief check for visibility filtering of a contour
     * \param contour the contour to check
     * \return true if contour is filtered*/
    bool is_visibility_filtered(std::shared_ptr<FaceContourNode>& contour);
    
    /**\brief check for hysteresis filtering of a contour
     * \param contour the contour to check
     * \return true if the contour is NOT filtered*/
    static inline bool validVertex(float valueCrossVertex, float threshold) {
        return valueCrossVertex <= threshold;
    }    
  
    /**\brief compute visibility of a CrossVertex
     * \return normal.dot(projected_view_direction)*/
    double compute_visibility (const CrossVertex &crossVertex) const;
};

#endif /* CROSSEDGEEXTRACTOR_H */

