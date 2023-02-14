#include <list>
#include <iostream>
#include <vector>
#include <algorithm>

#include "ViewGraph.h"
#include "../Tools/Tools.h"
#include "../Tools/RaycastHelper.h"
#include "../Tools/ContourFilter.h"
#include "../Tools/BSP.h"
#include "../Tools/CrossVertexConverter.h"

ViewGraph::~ViewGraph() {
    contours.clear();
    edge_contours.clear();
    face_contours.clear();
    vertices.clear();
    mesh_edge_contours.clear();
    mesh_vertices.clear();
}

ViewGraph::ViewGraph(MeshObject& obj, ViewCamera& cam, Properties& p):mesh(obj), camera(cam), props(p),
                            sharpFeatures(SharpFeaturesExtractor(mesh, props.sharp_angle)),
                            occlusionContours(OcclusionContoursExtractor(mesh, camera)),
                            suggestiveContours(SuggestiveContoursExtractor(mesh, camera, props.suggestive_speed)),
                            ridges(RidgesExtractor(mesh)), valleys(ValleysExtractor(mesh)),
                            demarcatingCurves(DemarcatingCurvesExtractor(mesh)), boundaries(BoundariesExtractor(mesh,camera)),
                            smoothContours(SmoothOcclusionContoursExtractor(mesh, camera))
{
    if(props.occlusive){   
      if(props.use_smooth)  smoothContours.init();
      else                  occlusionContours.init();
    }
    if(props.suggestive)    suggestiveContours.init();
    if(props.boundaries)    boundaries.init();
    if(props.ridges)        ridges.init();
    if(props.valleys)       valleys.init();
    if(props.demarcating)   demarcatingCurves.init();
}

const vector<shared_ptr<VertexNode>> ViewGraph::get_singularities() {
    vector<shared_ptr<VertexNode>> vert;
    for(auto v: vertices){
        if(v->type() != VertexType::V_JUNCTION)
            vert.push_back(v);
    }
    return vert;
}

const vector<shared_ptr<ContourNode>>& ViewGraph::compute_contours(){
    cout << " Extracting edges data..." << endl;
    compute_edge_contours();

    cout << " Extracting faces data..." << endl;    
    compute_face_contours();

    cout << " Filtering..." << endl;        
    compute_filtering();

    // build contours list
    for(auto e: edge_contours) if(props.extract_hidden or not e->isOccluded()) contours.push_back(e);
    for(auto f: face_contours) if(props.extract_hidden or not f->isOccluded()) contours.push_back(f);

    cout << " Computing image space intersections..." << endl;
    compute_image_space_intersections();

    cout << " Computing visibility..." << endl;
    compute_visibility();

    return contours;    
}

bool ViewGraph::frustum_culling_edge(int e){
    return camera.frustum_culling(mesh.V().row(mesh.E()(e, 0)))  
       and camera.frustum_culling(mesh.V().row(mesh.E()(e, 1)));
}

bool ViewGraph::frustum_culling_face(int f){
    return camera.frustum_culling(mesh.V().row(mesh.F()(f, 0)))
       and camera.frustum_culling(mesh.V().row(mesh.F()(f, 1)))
       and camera.frustum_culling(mesh.V().row(mesh.F()(f, 2)));
}

void ViewGraph::compute_edge_contours() {
//#pragma omp parallel for
    for (unsigned int i = 0; i < mesh.E().rows(); ++i) {
        bool hidden = false;
        bool detection = false;
        ContourType type;
        ContourExtractor* ce;
        
        if(props.frustum_culling and frustum_culling_edge(i))
            continue;
        
        if(props.boundaries and boundaries.is_contour(i)){
            type = ContourType::BOUNDARY;
            ce = &boundaries;
            detection = true;
        }
        else if(props.occlusive and not props.use_smooth and occlusionContours.is_contour(i)){
            hidden = occlusionContours.is_occluded(i);
            type = ContourType::OCCLUSION;
            ce = &occlusionContours;
            detection = true;
        }
        else if(props.sharp and sharpFeatures.is_contour(i)){
            hidden = sharpFeatures.is_occluded(camera, i);
            type = ContourType::SHARP;
            ce = &sharpFeatures;
            detection = true;
        }
        if(props.suggestive)
            suggestiveContours.compute_cross_vertex(i);
        if(props.ridges)
            ridges.compute_cross_vertex(i);
        if(props.valleys)
            valleys.compute_cross_vertex(i);        
        
         if(detection){
//            #pragma omp critical       
//            {
                // new contour detection
                shared_ptr<EdgeContourNode> cnode;
                shared_ptr<VertexNode> vn1, vn2;

                cnode = get_edge_contour_node(i);

                cnode->setType(type);

                if(hidden)
                    cnode -> setVisible(false);

                vn1 = get_vertex_node(mesh.E().row(i)(0));
                Tools::connect(cnode, vn1, Side::HEAD);

                vn2 = get_vertex_node(mesh.E().row(i)(1));
                Tools::connect(cnode, vn2, Side::TAIL);

                ce -> add_contour(cnode);

                is_singularity(vn1, mesh.E().row(i)(0));
                is_singularity(vn2, mesh.E().row(i)(1));
//            }
        }
    }
}
  
VertexType ViewGraph::is_singularity(shared_ptr<VertexNode> vn, int v) {
    if(is_cusp(vn))
        vn -> setType(VertexType::CUSP);
    else if(props.boundaries and is_boundary_cusp(vn, v))
        vn -> setType(VertexType::BOUNDARY_CUSP);
    else if(is_surface_intersection(vn))
        vn -> setType(VertexType::Y_JUNCTION);
    else if(is_bifurcation(vn))
        vn -> setType(VertexType::X_JUNCTION);
    
    return vn->type();
}


// (4) Appendix C.2.1 : boundary curtain fold
bool ViewGraph::is_boundary_cusp(shared_ptr<VertexNode> vn, int v){
    bool is_boundary = false;
    double dist = -1;
    shared_ptr<VertexNode> vert;
    int edge_index;
    
    Vector3d p, q, r, c, e;    
    c = camera.get_position().cast<double>();
    // parse adjacent contours to find the furthest boundary curve
    for(auto e: vn -> adjacent_contours()){
        if(e->isType(ContourType::BOUNDARY)){
            is_boundary = true;
            edge_index = dynamic_pointer_cast<EdgeContourNode>(e)->edge_index();
            shared_ptr<VertexNode> other = e->getHead();
            if(vn == other)
                other = e->getTail();
            
            double d = (other->point3d() - c).norm();
            if(d > dist){
                vert = other;
                dist = d;
            }
        }
    }
    
    if(not is_boundary)
        return false;

    e = vert->point3d();
    
    // parse adjacent faces to test if this boundary curve is occluded
    for(int f: mesh.VF(v)){
        // ignore faces which have this edge
        if(mesh.FE()(f, 0) == edge_index or mesh.FE()(f, 1) == edge_index or mesh.FE()(f, 2) == edge_index)
            continue;
        
        // ignore backfacing faces
        if(Tools::is_face_backfacing(mesh, camera, f))
            continue;
        
        int v0 = mesh.F()(f, 0);
        int v1 = mesh.F()(f, 1);
        int v2 = mesh.F()(f, 2);
        
        if(v0 == v){
            p = mesh.V().row(v0).transpose(); 
            q = mesh.V().row(v1).transpose(); 
            r = mesh.V().row(v2).transpose();
        }
        else if(v1 == v){
            p = mesh.V().row(v1).transpose(); 
            q = mesh.V().row(v2).transpose(); 
            r = mesh.V().row(v0).transpose();
        }
        else if(v2 == v){
            p = mesh.V().row(v2).transpose(); 
            q = mesh.V().row(v0).transpose(); 
            r = mesh.V().row(v1).transpose();
        }
        
        if (
                not Tools::sameside3d(p,q,r,c,e)
                and Tools::sameside3d(c,p,q,e,r)
                and Tools::sameside3d(c,p,r,e,q)
        )
            return true;
    }
    return false;
}

// (3) cusp : vertex where surface sharply folds back on itself
bool ViewGraph::is_cusp(shared_ptr<VertexNode> vn){
    bool concave = false;
    bool convex = false;
    
    for(auto e: vn -> adjacent_contours()){
        if(not e->isType(ContourType::OCCLUSION))
            continue;
        try{
            shared_ptr<EdgeContourNode> ecn = dynamic_pointer_cast<EdgeContourNode>(e);
            
//            if(Tools::concavity(mesh, ecn->edge_index()) > 0){
            if(Tools::is_edge_concave(mesh, ecn->edge_index())){
                concave = true;
            }
            else{
                convex = true;
            }
        }
        catch(const bad_cast& exc){  }
    }

    return concave and convex;
}

// (5) bifurcation : vertex connecting more than 2 contour generators
bool ViewGraph::is_bifurcation(shared_ptr<VertexNode> vn){
    return vn -> adjacent_contours().size() > 2;
}

// (2) YVertex : vertex on the 3d surface, happening at a mesh vertex
bool ViewGraph::is_surface_intersection(shared_ptr<VertexNode> vn){
    bool boundary = false;
    bool occluding = false;
    
    auto cont = vn -> adjacent_contours();
    for(auto c: cont){
        if(c->isType(ContourType::BOUNDARY))
            boundary = true;
        else if(c->isType(ContourType::OCCLUSION))
            occluding = true;
    }
    return boundary and occluding;
}

void ViewGraph::compute_face_contours() {
//#pragma omp parallel for
    for(int i=0; i<mesh.F().rows(); i++){
        if(props.frustum_culling and frustum_culling_face(i))
            continue;
        
        if(props.use_smooth and props.occlusive){
            smoothContours.compute_cross_vertex(i);
        }
        
        if(props.demarcating //and 
                //(props.extract_hidden or not Tools::is_face_backfacing(mesh, camera, i))
        )
            demarcatingCurves.compute_cross_vertex(i);
    }
    
    CrossVertexConverter cvc(mesh, sharpFeatures);
    
    // TODO factor this fuckshit with template or abstraction
    
    if(props.suggestive)
        cvc.convert_cross_vertices(suggestiveContours, camera);
    
    if(props.use_smooth and props.occlusive)
        cvc.convert_cross_vertices(smoothContours, camera);
    
    if(props.demarcating)
        cvc.convert_cross_vertices(demarcatingCurves, camera);
    
    if(props.ridges)
        cvc.convert_cross_vertices(ridges, camera);
    
    if(props.valleys)
        cvc.convert_cross_vertices(valleys, camera);
}

void ViewGraph::filter_faces_contours(CrossValueContoursExtractor& cont, bool hys, bool fil, double thr, double tolthr, double minvis, double maxvis) {    
    ContourFilter filter(camera, thr, tolthr, minvis, maxvis);  
    auto col = filter.filter_contours(cont.get_contours(), hys, fil, not props.extract_hidden);
    
    for(auto c: col){
        face_contours.push_back(c);
        vertices.push_back(c->getHead());
        vertices.push_back(c->getTail());
    }
}

void ViewGraph::compute_filtering()
{    
    if(props.use_smooth and props.occlusive){
        filter_faces_contours(smoothContours, false, false, -1, -1, -1, -1);
    }
    
    if(props.demarcating){
        filter_faces_contours(demarcatingCurves,  props.demarcating_hysteresis, 
                                                props.demarcating_filtering, 
                                                props.demarcating_threshold, 
                                                props.demarcating_tolerance, 
                                                props.demarcating_min_visibility,
                                                props.demarcating_max_visibility);
    }
    
    if(props.suggestive){
        filter_faces_contours(suggestiveContours, props.suggestive_hysteresis, 
                                                props.suggestive_filtering, 
                                                props.suggestive_threshold, 
                                                props.suggestive_tolerance, 
                                                props.suggestive_min_visibility,
                                                props.suggestive_max_visibility);
    }
        
    if(props.ridges){
        filter_faces_contours(ridges, props.ridges_hysteresis, 
                                    props.ridges_filtering, 
                                    props.ridges_threshold, 
                                    props.ridges_tolerance, 
                                    props.ridges_min_visibility,
                                    props.ridges_max_visibility);
    }
    
    if(props.valleys){
        filter_faces_contours(valleys, props.valleys_hysteresis, 
                                    props.valleys_filtering, 
                                    props.valleys_threshold, 
                                    props.valleys_tolerance, 
                                    props.valleys_min_visibility,
                                    props.valleys_max_visibility);
    }
}

// (1) image space intersections
void ViewGraph::compute_image_space_intersections() {
    vector<shared_ptr<bsp_node>> nodes;
    for(shared_ptr<ContourNode> c: contours){
        shared_ptr<bsp_node> b = make_shared<bsp_node>(c);
        nodes.push_back(b);
    }

    // compute intersections using BSP tree
    // intersection information is stored in individual contours (splits)
    // TODO store it elseweyr for optimization ?
    bsp_node::compute_BSP(nodes, camera, props.intersections_tol);
    
    // translate intersection information to view graph population
    vector<shared_ptr<ContourNode>> new_contours;
    vector<shared_ptr<ContourNode>> contours_to_delete;
    for(shared_ptr<ContourNode> c: contours){
        list<float> splits = c->getSplits();
        if(splits.size() == 0)
            continue;

        splits.unique();
        splits.sort();

        // from here we know the current contour must be splitted at least once
        // create first subcontour and connect to father contour head vertex
        Vector3d c_vector = c->tail_vertex3d() - c->head_vertex3d();
        shared_ptr<ContourNode> previous_contour = c->createChild();
        new_contours.push_back(previous_contour);
        Tools::connect(previous_contour, c->getHead(), Side::HEAD);

        for(float t: splits){
            shared_ptr<ContourNode> intersecting_contour = c->getIntersectingContour(t);
            
            // make sure we have a singularity (visibility indicating intersecting contour)
            if(not (intersecting_contour->isType(ContourType::BOUNDARY) or 
               intersecting_contour->isType(ContourType::OCCLUSION) ))
                continue;
            
            // make sure both contours do not share a vertex 
            // in which case the intersecting point can only be this vertex
            if(c->is_sharing_vertex(intersecting_contour))
                continue;
            
            shared_ptr<VertexNode> vnode;

            // make sure we are not on an existing vertex, in which case we update 
            // it as a singularity and move on
            if(t < props.intersections_tol){
                vnode = c->getHead();
                if(not vnode->isSingularity())
                    vnode->setType(VertexType::T_JUNCTION);  
                continue;
            }
            else if((1-t) < props.intersections_tol){
                vnode = c->getTail(); 
                if(not vnode->isSingularity())
                    vnode->setType(VertexType::T_JUNCTION);  
                continue;                
            }
            
            // create new singularity vertex
            Vector3d P = c->head_vertex3d() + t * c_vector;
            vnode = make_shared<VertexNode>(P, camera);

            vnode->setType(VertexType::T_JUNCTION);

            // connect vertex to previous contour
            Tools::connect(previous_contour, vnode, Side::TAIL);

            // create following contour and connect it
            previous_contour = c->createChild();
            new_contours.push_back(previous_contour);
            Tools::connect(previous_contour, vnode, Side::HEAD);

            vertices.push_back(vnode);        
        }

        // connect last contour to father vertex tail
        Tools::connect(previous_contour, c->getTail(), Side::TAIL);
        contours_to_delete.push_back(c);
    }

    // delete all traces of father contours
    for(shared_ptr<ContourNode> c: contours_to_delete){
        Tools::disconnect(c);
        contours.erase(remove(contours.begin(), contours.end(), c), contours.end());
//        if(c->typeinfo() == "edge_contour")
//            shared_ptr<EdgeContourNode> ecn = dynamic_cast<EdgeContourNode>(c);
//            edge_contours.erase(remove(edge_contours.begin(), edge_contours.end(), c), edge_contours.end());        
//        mesh_edge_contours[c->edge_index()] = NULL; // info lost ?
    }

    // add children contours to list
    for(shared_ptr<ContourNode> c: new_contours){
        contours.push_back(c);
    }

   
    // TODO probably should do something to [mesh edge -> contour] map ?
}

void ViewGraph::compute_visibility() {
    RaycastHelper raycaster(mesh, camera);

    auto to_propagate = contours;
    for(shared_ptr<ContourNode> c: to_propagate){
        if(c->parsed_visibility)
            continue;
        // if visibility unknown, then determine it
        if(c->isUndefined()){
            c->parsed_visibility = true;
            c->setVisible(raycaster.isVisible(c, props.raycast_tol));
        }

        // propagate from here
        propagate_visibility(c, c->getHead());
        propagate_visibility(c, c->getTail());
    }
}

void ViewGraph::propagate_visibility(shared_ptr<ContourNode> c, shared_ptr<VertexNode> prev) { 
    // select correct side
    shared_ptr<VertexNode> v = c->getHead();
    if(v == prev)
        v = c->getTail();
    
    // stopping condition : we come to a singularity
    if(v->isSingularity())
        return;

    // propagating
    for(shared_ptr<ContourNode> next: v->adjacent_contours()){
        if(not next->parsed_visibility){
            next->setVisible(c->isVisible());
            next->parsed_visibility = true;
            propagate_visibility(next, v);
        }
    }
}


shared_ptr<EdgeContourNode> ViewGraph::get_edge_contour_node(int e) {
    map<int, shared_ptr<EdgeContourNode>>::iterator it = mesh_edge_contours.find( e );

    if(it != mesh_edge_contours.end()){
        return it->second;
    }

    shared_ptr<EdgeContourNode> cnode = make_shared<EdgeContourNode>(e);
    edge_contours.push_back(cnode);
    mesh_edge_contours[e] = cnode;
    return cnode;
}


shared_ptr<VertexNode> ViewGraph::get_vertex_node(int v) {
    map<int, shared_ptr<VertexNode>>::iterator it = mesh_vertices.find( v );

    if(it != mesh_vertices.end()){
        return it->second;
    }

    shared_ptr<VertexNode> vnode = make_shared<VertexNode>(mesh, camera, v);
    mesh_vertices[v] = vnode;
    vertices.push_back(vnode);
    return vnode;
}