#include "CrossVertexConverter.h"
#include "../ViewGraph/FaceContourNode.h"
#include "../ContourExtractor/CrossValueContoursExtractor.h"
#include "../ContourExtractor/SharpFeaturesExtractor.h"
#include "../Common/ViewCamera.h"
#include "../Tools/Tools.h"

using namespace std;

void CrossVertexConverter::convert_cross_vertices(CrossValueContoursExtractor& cont, ViewCamera& cam) {
    vector<CrossVertex> face_vertices;
    face_vertices.reserve(3);

    for(int f=0; f<mesh.F().rows(); ++f){
        if (sharp.is_face_sharp(f)) {
            for (int i = 0; i < 3; ++i) {
                const int e = mesh.FE()(f, i);
                if (cont.crossVertices().count(e) > 0) {
                    CrossVertex c = cont.crossVertices()[e];
                    face_vertices.push_back(c);
                }
            }

            if (face_vertices.size() == 2) {
                /* "Ridge-Valley Lines on Meshes via Implicit Surface Fitting"
                 * Section 3 : Tracing Ridges
                 * If two ridge vertices are detected on edges of a mesh triangle,
                 * they are connected by a straight segment. */
                auto vertices = make_pair(face_vertices[0], face_vertices[1]);
                CrossEdge crossEdge = {vertices, f, CrossEdgeType::REGULAR};
                create_face_contour(crossEdge, cont, cam);
            } 
            else if (face_vertices.size() == 3) {
                create_face_center_vertex(f, face_vertices, cont, cam);
            }
        }
        face_vertices.clear();
    }
}
/* "Ridge-Valley Lines on Meshes via Implicit Surface Fitting"
 * Section 3 : Tracing Ridges
 * If all three edges of a mesh triangle contain ridge vertices,
 * the vertices are connected with the centroid of the triangle formed by the vertices. */
void CrossVertexConverter::create_face_center_vertex (int f, const vector<CrossVertex> &face_vertices, CrossValueContoursExtractor& cont, ViewCamera& cam) {
    CrossVertex center_face = get_face_center(f, face_vertices);

    // Calculate the view dependent parameters of the cross value of the center of the face
    center_face.gradient = center_face.normal = RowVector3d::Zero();
    for (const auto &cv : face_vertices) {
        center_face.gradient += cv.gradient;
        center_face.normal += cv.normal;
    }
    center_face.gradient /= 3.0;
    center_face.normal /= 3.0;

    for (int i = 0; i < 3; ++i) {
        auto vertices = make_pair(center_face, face_vertices[i]);
        CrossEdge crossEdge = {vertices, f, CrossEdgeType::CENTROID};        
        create_face_contour(crossEdge, cont, cam);
    }
}

CrossVertex CrossVertexConverter::get_face_center (int f, const vector<CrossVertex> &crossVertex_forFace) {
    CrossVertex center_face;
    center_face.position = (mesh.V().row(mesh.F()(f, 0)) + mesh.V().row(mesh.F()(f, 1)) + mesh.V().row(mesh.F()(f, 2))) / 3.0;

    // Calculate the absolute curvature of the center of the face
    center_face.valueThreshold = 0.0;
    for (auto &crossVertex : crossVertex_forFace) {
        center_face.valueThreshold += crossVertex.valueThreshold;
    }
    center_face.valueThreshold /= 3.0;

    return center_face;
}

void CrossVertexConverter::create_face_contour(CrossEdge& e, CrossValueContoursExtractor& cont, ViewCamera& cam){
    shared_ptr<FaceContourNode> fc = make_shared<FaceContourNode>(e.face_index);
    fc->setType(cont.type());  

    shared_ptr<VertexNode> vh;
    if(e.type == CrossEdgeType::REGULAR)
        vh = cont.get_crossvertex_node(e.vertices.first, cam);
    else if(e.type == CrossEdgeType::CENTROID)
        vh = cont.get_centroidvertex_node(e.face_index, e.vertices.first.position, cam);

    shared_ptr<VertexNode> vt = cont.get_crossvertex_node(e.vertices.second, cam);

    Tools::connect(fc, vh, Side::HEAD);
    Tools::connect(fc, vt, Side::TAIL); 

    // set contour as hidden if not an interpolated occlusion contour and if on a backface
    if(cont.type() != ContourType::OCCLUSION and Tools::is_face_backfacing(mesh, cam, fc->face_index()))
        fc->setVisible(false);

    fc->setCrossEdge(e);
    
    cont.add_contour(fc);
    cont.add_vertex(vh);
    cont.add_vertex(vt);     
}