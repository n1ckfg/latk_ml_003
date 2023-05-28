#ifndef CROSSVALUECONTOURSEXTRACTOR_H
#define CROSSVALUECONTOURSEXTRACTOR_H

#include "ContourExtractor.h"
#include "CrossStructures.h"
#include "../Tools/CrossVertexConverter.h"

class VertexNode;
class ViewGraph;
class ViewCamera;

/**\brief Abstract ContourExtractor for cross value lines (i.e. lines lying in faces, cf CrossVertex, CrossEdge, FaceContourNode).
 * \author Bastien Wailly <bastien.wailly@inria.fr>, Inria*/
class CrossValueContoursExtractor : public ContourExtractor{

/**\brief CrossVertexConverter need to access extractors to edit their list of contours. */
friend void CrossVertexConverter::create_face_contour(CrossEdge& e, CrossValueContoursExtractor& cont, ViewCamera& cam);

public:
    CrossValueContoursExtractor(MeshObject& m):ContourExtractor(m){};
    virtual ~CrossValueContoursExtractor();
    
    /**\brief Check if a mesh edge contains a cross vertex for this extractor. 
     * \param e mesh edge index 
     * \return true if edge e has a cross vertex. */
    virtual bool compute_cross_vertex(int e) = 0;
    
    /**\brief Get the list of all cross vertices for this extractor. 
     * \return Map from mesh edge e to cross vertex v, with all entries being a vertex v lying on an edge e. */
    CrossVerticesMap& crossVertices() { return _crossVertices; };
    
    /**\brief Get the list of contours that were previously computed. 
     * \return vector of pointers to instantiated FaceContourNode. */
    std::vector< std::shared_ptr<FaceContourNode> >& get_contours(){ return _contours; };
    
protected:
    CrossVerticesMap _crossVertices;
    
    std::vector< std::shared_ptr<FaceContourNode> > _contours;
    std::vector< std::shared_ptr<VertexNode> > _vertices;
    std::map<int, std::shared_ptr<VertexNode> > _edge_cross_vertices;
    std::map<int, std::shared_ptr<VertexNode> > _face_centroid_vertices;  
    
    /**\brief Factory method converting CrossVertex indicating a vertex on an edge to a VertexNode representing a Vertex in the view graph. 
     * \details This particular method handles the case where we have 2 and only 2 CrossVertex on a face. We simply join them to create a contour.\n
     * There can only be one and only one CrossVertex per mesh edge per extractor. This Factory ensures that.
     * \param cv the CrossVertex to add in the graph
     * \param cam the camera used for extraction
     * \return Either the newly created or the previously instantiated VertexNode. */
    shared_ptr<VertexNode> get_crossvertex_node(const CrossVertex& cv, ViewCamera& cam);
    
    /**\brief Factory method converting CrossVertex indicating a vertex on an edge to a VertexNode representing a Vertex in the view graph. 
     * \details This particular method handles the case where we have 3 CrossVertex on a face. We create 3 contours joining all 3 vertices to the center of the face.\n
     * There can only be one and only one CrossVertex per mesh edge per extractor. This Factory ensures that.
     * \param f the face we are considering
     * \param v the 3d coordinates of the vertex we are considering
     * \param cam the camera used for extraction
     * \return Either the newly created or the previously instantiated VertexNode. */    
    shared_ptr<VertexNode> get_centroidvertex_node(int f, RowVector3d v, ViewCamera& cam);
    
    /**\brief Add a vertex to this extractor vertices list. 
     * \param v the instantiated VertexNode */
    void add_vertex(std::shared_ptr<VertexNode> v){ _vertices.push_back(v); }
    
    /**\brief Add a contour to this extractor contours list. 
     * \param v the instantiated FaceContourNode */    
    void add_contour(std::shared_ptr<FaceContourNode> c){ _contours.push_back(c); }

    /**\brief Interpolate vectors v1 and v2 with weights w1 and w2. 
     * \return interpolated vector */
    inline RowVector3d vector_interpolation(const RowVector3d &v1, const RowVector3d &v2, double w1, double w2) const {
        const double a_w1 = fabs(w1);
        const double a_w2 = fabs(w2);
        RowVector3d result = a_w2 * v1 + a_w1 * v2;
        result /= a_w1 + a_w2;
        return result;
    }
    
    /**\brief Interpolate values v1 and v2 with weights w1 and w2.
     * \return interpolated value */
    inline double value_interpolation (double v1, double v2, double w1, double w2) const {
        const double a1 = fabs(w1);
        const double a2 = fabs(w2);

        double result = a2 * v1 + a1 * v2;
        result /= a1 + a2;
        return result;
    }    
        
    /**\brief Register a new CrossVertex in this extractor. 
     * \param e mesh edge index 
     * \param position the 3d coordinates of the vertex 
     * \param valueThreshold the value we are assigning for hysteresis thresholding (see ContourFilter) */
    virtual void add_crossVertex(unsigned int e, const RowVector3d &position, double valueThreshold);

};

#endif /* CROSSVALUECONTOURSEXTRACTOR_H */

