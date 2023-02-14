#include "ContourFilter.h"
#include "../ViewGraph/FaceContourNode.h"

#include <forward_list>

#include <iostream>

ContourFilter::ContourFilter(ViewCamera& cam, double thr, double tolthr, double minVis, double maxVis):
        camera(cam), threshold(thr), tolerance(tolthr), minVisibility(minVis), maxVisibility(maxVis)
{
}


std::vector<std::shared_ptr<FaceContourNode>> ContourFilter::filter_contours(std::vector<std::shared_ptr<FaceContourNode>>& contoursIn, bool hysteresis, bool visibility, bool hidden){
    buffer.clear();

    if(visibility)
        // flag contours based on their visibility (ndotv)
        apply_visibility(contoursIn);
    
    if(hysteresis)
        // run multiple passes to filter based on hysteresis thresholding
        apply_hysteresis(contoursIn, visibility, hidden);
        
    for(auto c: contoursIn){
        if(     (not visibility or not c->visibility_filtered)
            and (not hidden or not c->isOccluded())
            and (not hysteresis or not c->hysteresis_filtered))
        {
            buffer.push_back(c);
        }
    }
    
    return buffer;
}


/* "Suggestive Contours for Conveying Shape"
 * Section 3.1
 * "We use the idea of hysteresis thresholding [Canny 1986] to increase granularity */
void ContourFilter::apply_hysteresis(std::vector<std::shared_ptr<FaceContourNode>>& contoursIn, bool visibility, bool hidden) {
    std::cout << "    Applying hysteresis..." << std::endl;        
    
    std::unordered_set<CrossVertex> crossVertices_selected;
    std::forward_list<unsigned int> indicesCrossEdges_notSelected;
    
    // first pass with less tolerant threshold + visibility
    for (unsigned int i = 0; i < contoursIn.size(); ++i) {
        auto &xe = contoursIn.at(i)->getCrossEdge();
        auto &xv1 = xe.vertices.first;
        auto &xv2 = xe.vertices.second;

        // ignore contours that have already been filtered by visibility or are not visible at all
        if( (visibility and contoursIn.at(i)->visibility_filtered)
            or (hidden and contoursIn.at(i)->isOccluded()) )
        {
            continue;
        }

        if ( validVertex(xv1.valueThreshold, threshold) || validVertex(xv2.valueThreshold, threshold) ) {
            crossVertices_selected.insert(xe.vertices.first);
            crossVertices_selected.insert(xe.vertices.second);
            contoursIn.at(i)->hysteresis_filtered=false;
        } 
        else {
            indicesCrossEdges_notSelected.push_front(i);
            contoursIn.at(i)->hysteresis_filtered=true;
        }
    }    

    // make passes with more tolerant threshold until it converges
    bool changed;
    do {
        changed = false;

        auto it = indicesCrossEdges_notSelected.begin();
        while (it != indicesCrossEdges_notSelected.end()) {
            auto &xe = contoursIn.at(*it)->getCrossEdge();

            // Check that the edge is validated with the more tolerant threshold
            bool crossEdgeToAdd = (validVertex(xe.vertices.first.valueThreshold, threshold + tolerance)) ||
                                  (validVertex(xe.vertices.second.valueThreshold,  threshold + tolerance));
            // Check that the edge is connected to an already accepted edge
            crossEdgeToAdd &= (crossVertices_selected.count(xe.vertices.first) > 0) || (crossVertices_selected.count(xe.vertices.second) > 0);

            if (crossEdgeToAdd) {
                crossVertices_selected.insert(xe.vertices.first);
                crossVertices_selected.insert(xe.vertices.second);
                contoursIn.at(*it)->hysteresis_filtered=false;
                changed = true;

                unsigned int index_crossEdgeToRemove = *it;
                ++it;
                indicesCrossEdges_notSelected.remove(index_crossEdgeToRemove);
            } 
            else {
                ++it;
            }
        }    
    } while (changed);  
}


bool ContourFilter::is_visibility_filtered(std::shared_ptr<FaceContourNode>& contour){
    CrossEdge crossEdge = contour->getCrossEdge();
    double d_v1 = compute_visibility(crossEdge.vertices.first);
    double d_v2 = compute_visibility(crossEdge.vertices.second);
    if (d_v1 >= minVisibility || d_v2 >= minVisibility) {
        if(d_v1 <= maxVisibility || d_v2 <= maxVisibility){
            return false;
        }        
    }

    return true;
}

double ContourFilter::compute_visibility (const CrossVertex &crossVertex) const {
    RowVector3d projviewdir = camera.get_proj_view_direction(crossVertex.position, crossVertex.normal);
    projviewdir.normalize();
    double derivativeCrossValue = (crossVertex.gradient).dot(projviewdir);
    return derivativeCrossValue;
}

void ContourFilter::apply_visibility(std::vector<std::shared_ptr<FaceContourNode>>& contoursIn) {
    std::cout << "    Applying visibility filter..." << std::endl;
    
    for (unsigned int i = 0; i < contoursIn.size(); ++i){
        if(is_visibility_filtered(contoursIn.at(i))){
            contoursIn.at(i)->visibility_filtered = true;
        }
        else{
            contoursIn.at(i)->visibility_filtered = false;
        }
    }
}
