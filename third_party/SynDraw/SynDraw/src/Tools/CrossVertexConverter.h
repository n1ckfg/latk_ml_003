#ifndef CROSSVERTEXCONVERTER_H
#define CROSSVERTEXCONVERTER_H

#include <vector>

#include "../ContourExtractor/CrossStructures.h"
class FaceContourNode;
class SharpFeaturesExtractor;
class MeshObject;
class ViewCamera;
class CrossValueContoursExtractor;


/**\author Bastien Wailly <bastien.wailly@inria.fr>, Inria
 * \brief FaceContourNode Factory class + subgraph computer 
 * \details This class is taking a face contour extractor as input. It will take all its 
 * discovered cross vertices and convert them into a graph of contours, which
 * will be used to update the view graph.*/
class CrossVertexConverter {

public:
    /**\brief create a converter instance on a mesh
     * \param m the mesh
     * \param sh the sharp features extractor instance (used for face sharpness info)*/
    CrossVertexConverter(MeshObject& m, SharpFeaturesExtractor& sh):mesh(m), sharp(sh){};
    virtual ~CrossVertexConverter(){};
    
    /**\brief create the graph of a given contour extractor
     * \details the graph is stored inside the extractor instance
     * \param cont the contour extractor which contains the list of cross vertices
     * \param cam camera used for extraction*/
    void convert_cross_vertices(CrossValueContoursExtractor& cont, ViewCamera& cam);
    
    /**\brief create an instance of ContourNode based on arguments
     * \param e the CrossEdge (proto-contour)
     * \param cont the extractor we are */
    void create_face_contour(CrossEdge& e, CrossValueContoursExtractor& cont, ViewCamera& cam);    

private:
    MeshObject& mesh;
    SharpFeaturesExtractor& sharp;
    
    /**\brief handle the case where we have 3 cross vertices on a face
     * \details we simply create 3 contours connected to each cross vertex and to
     * the center of the face.*/
    void create_face_center_vertex (int f, const std::vector<CrossVertex> &face_vertices, CrossValueContoursExtractor& cont, ViewCamera& cam);
    
    /**\brief get the coordinates of a face center point (barycenter)*/
    CrossVertex get_face_center (int index_face, const std::vector<CrossVertex> &crossVertex_forFace);    
  
};

#endif /* CROSSVERTEXCONVERTER_H */

