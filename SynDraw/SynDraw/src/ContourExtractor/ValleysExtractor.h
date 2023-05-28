#ifndef VALLEYSEXTRACTOR_H
#define VALLEYSEXTRACTOR_H

#include "CreasesExtractor.h"

/**\brief Extractor class for Valleys, following [Ohtake 2004].
 * \details Valleys are contours where the curvature gradient is the lowest.
 * \author Bastien Wailly <bastien.wailly@inria.fr>, Inria*/
class ValleysExtractor : public CreasesExtractor{
public:
    ValleysExtractor(MeshObject& m);
    virtual ~ValleysExtractor();
    
    /**\brief Check if an edge contains a Valley cross vertex 
     * \param e mesh edge index 
     * \return true if edge e contains a valley vertex. 
     * \details Section 3 : Tracing Ridges
     * Next we check the following conditions:\n
     * kmax(v) > |k min (v)| for v = v1 , v2 and emax (v1) * emax (v2) < 0 */     
    bool compute_cross_vertex(int e) final override;
    
    /**\brief init mesh before ridges extraction (compute gradient minimas)*/    
    void init(){
        extremas = mesh.minima();
    }
    
    ContourType type() override { return ContourType::VALLEYS; } ;
    
private: 
    /**\brief Section 3 : Tracing Ridges
     * \details
    * Finally we apply a simple derivative test :\n
    * emax (vi) (v 3−i − v i ) * t max (vi) > 0 with i = 1 or 2 */    
    bool is_extrema (unsigned int v1, unsigned int v2,
                                     double extrema1, double extrema2,
                                     const RowVector3d &pdir_max1, const RowVector3d &pdir_max2);    

};

#endif /* VALLEYSEXTRACTOR_H */

