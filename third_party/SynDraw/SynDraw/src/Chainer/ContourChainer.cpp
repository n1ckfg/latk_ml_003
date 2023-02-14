#include "ContourChainer.h"
#include "../Tools/Tools.h"
#include "../ViewGraph/ContourNode.h"
#include "../ViewGraph/VertexNode.h"

#include <iostream>
#include <cmath>

const double pi = std::acos(-1);

ContourChainer::ContourChainer() {
}

ContourChainer::ContourChainer(const ContourChainer& orig) {
}

ContourChainer::~ContourChainer() {
}

ContourChainer::ContourChainer(const std::vector<std::shared_ptr<ContourNode>>& contours, double a):input_contours(contours){
    angle = pi/180.0 * a;
//    input_contours = contours;
}

std::list<std::shared_ptr<ContourChain>>& ContourChainer::build_chains() {
    tochain.clear();
    for(auto c: input_contours){
        tochain.insert(c);
    }

    while(tochain.size() > 0){
        std::shared_ptr<ContourNode> c = *(tochain.begin());
        std::shared_ptr<ContourChain> ch = make_shared<ContourChain>(c);
        // chain recursively on the tail side
        chain(ch, c, c->getHead(), Side::TAIL, true);
        // chain recursively on the head side
        chain(ch, c, c->getTail(), Side::HEAD, true);
        chains.push_back(ch);
    }
    
    return chains;
}

void ContourChainer::chain(std::shared_ptr<ContourChain> ch, std::shared_ptr<ContourNode> c, std::shared_ptr<VertexNode> prev, Side side, bool init) {
    // select correct side and add vertices
    std::shared_ptr<VertexNode> v = c->getHead();
    if(v == prev)
        v = c->getTail();

    if(side == Side::HEAD){
        ch->push_vertex_front(v);
        if(not init)
            ch->push_contour_front(c);
    }
    else{
        ch->push_vertex_back(v);
        if(not init)
            ch->push_contour_back(c);
    }

    // this contour is chained
    tochain.erase(c);

    // stop chaining if current vertex is singularity
    if(v->isSingularity())
        return;

    // find next contour to chain
    std::shared_ptr<ContourNode> next;
    double a = angle;
    bool found = false;
    for(auto candidate: v->adjacent_contours()){
        if( candidate != c and // candidate must be a different contour
            candidate->isSimilar(c)){ // candidate must be similar to current contour  
            double tmp = Tools::get_angle(c, candidate);
            if( tmp <= a and // candidate must have the lowest angle among candidates                
                tochain.find(candidate) != tochain.end()) // candidate must be in the list of contours not yet in a chain
            {
                next = candidate;
                a = tmp;
                found = true;
            }
        }
    }
    
    // recursive call
    if(found)
        chain(ch, next, v, side, false);
}

void ContourChainer::compute_chains_visibility(RaycastHelper& ray, double tol, double voting_factor) {
    for(auto c: chains){
        c->computeVisibility(voting_factor, ray, tol);
    }
}
