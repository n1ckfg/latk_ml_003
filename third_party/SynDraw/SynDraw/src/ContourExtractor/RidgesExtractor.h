#ifndef RIDGESEXTRACTOR_H
#define RIDGESEXTRACTOR_H

#include "CreasesExtractor.h"

    
/**\brief Extractor class for Ridges, following [Ohtake 2004].
 * \details Ridges are contours where the curvature gradient is the highest.
 * \author Bastien Wailly <bastien.wailly@inria.fr>, Inria*/
class RidgesExtractor : public CreasesExtractor{
public:
    RidgesExtractor(MeshObject& m);
    virtual ~RidgesExtractor();
    
    /**\brief Check if an edge contains a Ridge cross vertex 
     * \param e mesh edge index 
     * \return true if edge e contains a ridge vertex. 
     * \details Section 3 : Tracing Valleys
     * Next we check the following conditions:\n
     * kmax(v) > |k min (v)| for v = v1 , v2 and emax (v1) * emax (v2) < 0 */ 
    bool compute_cross_vertex(int e) final override;
    
    /**\brief init mesh before ridges extraction (compute gradient maximas)*/
    void init(){
        extremas = mesh.maxima();
    }
    
    ContourType type() override { return ContourType::RIDGES; } ;
    
private:
    
    /**\brief Section 3 : Tracing Ridges
     * \details
    * Finally we apply a simple derivative test :\n
    * emax (vi) (v 3−i − v i ) * t max (vi) > 0 with i = 1 or 2 */
    bool is_extrema (unsigned int v1, unsigned int v2,
                                     double extrema1, double extrema2,
                                     const RowVector3d &pdir_max1, const RowVector3d &pdir_max2);

};

#endif /* RIDGESEXTRACTOR_H */

