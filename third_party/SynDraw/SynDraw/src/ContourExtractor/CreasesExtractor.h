#ifndef CREASESEXTRACTOR_H
#define CREASESEXTRACTOR_H

#include "CrossStructures.h"
#include "CrossValueContoursExtractor.h"

/**\brief Abstract class for creases extraction, following [Ohtake 2004].
 * \details Creases can be ridges or valleys. This class exists because they share algorithms.
 * \author Bastien Wailly <bastien.wailly@inria.fr>, Inria*/
class CreasesExtractor : public CrossValueContoursExtractor{
public:
    CreasesExtractor(MeshObject& m):CrossValueContoursExtractor(m){};
    virtual ~CreasesExtractor();

protected:
    VectorXd extremas;
    CrossVerticesMap _crossVertices; 
    

    /**\brief Section 3 : Tracing Ridges. 
     * We use linear interpolation to approximate a zero-crossing of emax on [v1, v2] (the edge) */    
    void add_CreaseVertex (unsigned int e, double extrema1, double extrema2, const VectorXd& curvature);
    
/**\brief Section 3 : Tracing Ridge : Thresholding
 * \details We use linear interpolation to estimate kmax at the vertices 
 * of the polyline approximation of the ridge line */    
    double compute_threshold (unsigned int v1, unsigned int v2, double extrema1, double extrema2, const VectorXd& curvature);
    bool isValueVanish (double discrepancy, double extrema1, double extrema2, double &new_extrema1, double &new_extrema2) const;
    
/**\brief Flip if the angle between the two directions is obtuse
 * \details Section 3 : Tracing Ridges\n
 * We flip tmax(v2) if the angle between tmax(v1) and tmax(v2) is obtuse\n
 * tmax (v2) ← −t max (v2) and emax(v2) ← − emax (v2)\n
 * tmax : principal direction extremum (max for the ridges and min for the valleys) */    
    void flip_ifObtuse (const RowVector3d &principalDirection_vertex1, RowVector3d &principalDirection_vertex2,
                            double &extremality_vertex2) const;
};

#endif /* CREASESEXTRACTOR_H */

