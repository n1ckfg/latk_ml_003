#ifndef CONTOUREXTRACTOR_H
#define CONTOUREXTRACTOR_H

#include "../Common/MeshObject.h"
#include "../Common/Drawable.h"

#include <vector>

class ContourNode;

/**\brief Abstract class for a contour extraction algorithm.
 * \details A contour extractor is a pointer to a mesh and a list of contours on that mesh.
 * \author Bastien Wailly <bastien.wailly@inria.fr>, Inria*/
class ContourExtractor {
public:
    virtual ~ContourExtractor();
    
    /**\brief Register a contour to be part of this extractor
     * \param c previously instantiated contour node */
    void add_contour(std::shared_ptr<ContourNode> c){
        contours.push_back(c);
    }
    
    /**\brief Return the type of contour this extractor produces. */
    virtual ContourType type() = 0;

protected:
    ContourExtractor(MeshObject& obj);
    
    MeshObject& mesh;
    std::vector<std::shared_ptr<ContourNode>> contours;
};

#endif /* CONTOUREXTRACTOR_H */

