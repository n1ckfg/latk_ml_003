#include "BSP.h"
#include "../ViewGraph/ContourNode.h"
#include "../Tools/Tools.h"

#include <Eigen/Core>

using namespace Eigen;
using namespace std;

bsp_node::bsp_node() {
}

bsp_node::bsp_node(Vector2d p1, Vector2d p2, shared_ptr<ContourNode> c) : P1(p1), P2(p2), contour(c) {
    P1 = p1;
    P2 = p2;

    dir = (P2 - P1).normalized();
    norm = (P2 - P1).norm();
    normal = Vector2d(dir(1), -dir(0));
    line = Hyperplane<double, 2>(normal, P1);
    segment = ParametrizedLine<double, 2>(P1, dir);
}


bsp_node::bsp_node(shared_ptr<ContourNode> c) : bsp_node(c->head_vertex2d(), c->tail_vertex2d(), c) {
}

shared_ptr<ContourNode> bsp_node::getContour() {
    return contour;
}

Vector2d bsp_node::getP1() {
    return P1;
}

Vector2d bsp_node::getP2() {
    return P2;
}

void bsp_node::add_contour_vertex(shared_ptr<bsp_node> lin) {
    Vector2d direction = (contour->tail_vertex2d() - contour->head_vertex2d()).normalized();
    double norm = (contour->tail_vertex2d() - contour->head_vertex2d()).norm();
    ParametrizedLine<double, 2> contour_segment = ParametrizedLine<double, 2>(contour->head_vertex2d(), direction);
    double t = contour_segment.intersectionParameter(lin->line) / norm;

    if (t > 0 and t < 1)
        contour->addSplit(t, lin->contour);
}

Vector3d bsp_node::get_intersection_point_3d(shared_ptr<bsp_node> lin) {
    double t = segment.intersectionParameter(lin->line) / norm;
    Vector3d p3d = contour->head_vertex3d() + t * contour->tail_vertex3d();
    return p3d;
}

Vector2d bsp_node::get_intersection_point_2d(shared_ptr<bsp_node> lin) {
    return segment.intersectionPoint(lin->line);
}

void bsp_node::print() {
    IOFormat fmt = IOFormat(FullPrecision, 0, ", ", "\t; ", "", "", "[", "]");
    cout << "P1  : " << P1.format(fmt) << endl;
    cout << "P2  : " << P2.format(fmt) << endl;
    //            cout << "dir : " << dir.format(fmt) << endl;
    //            cout << "N   : " << normal.format(fmt) << endl;
    //            cout << "len : " << norm << endl;
    //            cout << "Contour : " << contour->edge_index() << endl;
    //            cout << endl;
    flush(cout);
}


void bsp_node::compute_BSP(vector<shared_ptr<bsp_node>>& cnodes, ViewCamera& camera, double tol) {
    if(cnodes.size() <= 1)
        return;

    vector<shared_ptr<bsp_node>> left, right;
    shared_ptr<bsp_node> ab = cnodes[0];

    // let's compare (ab) with all [cd] != [ab] in cnodes
    for(shared_ptr<bsp_node> cd: cnodes){
        if(ab == cd)
            continue;
        
        bool ab_occ = ab->contour->isType(ContourType::OCCLUSION);
        bool ab_bou = ab->contour->isType(ContourType::BOUNDARY);
        bool cd_occ = cd->contour->isType(ContourType::OCCLUSION);
        bool cd_bou = cd->contour->isType(ContourType::BOUNDARY);        
                
        double c_side = Tools::orient2d(ab->getP1(), ab->getP2(), cd->getP1());
        double d_side = Tools::orient2d(ab->getP1(), ab->getP2(), cd->getP2()); 
           
        // both vertex of [cd] are on (ab) => push cd arbitrarily to left side
        if(Tools::isApproxZero(c_side, tol) and Tools::isApproxZero(d_side, tol)){
            left.push_back(cd);
        }
        // one vertex of [cd] is on (ab) => push cd to the side of other vertex
        else if(Tools::isApproxZero(c_side, tol)){
            if(d_side > 0)
                left.push_back(cd);
            else
                right.push_back(cd);
        }
        else if(Tools::isApproxZero(d_side, tol)){
            if(c_side > 0)
                left.push_back(cd);
            else
                right.push_back(cd);            
        }
        // both vertices of [cd] are away from (ab) and on the same side
        else if((c_side>0 and d_side>0)){
            left.push_back(cd);
        }
        else if((c_side<0 and d_side<0)){
            right.push_back(cd);
        }
        // [cd] crosses (ab) strictly (not crossing at a vertex)
        else
        {
            // resulting bsp nodes : split [cd] with intersection point
            Vector2d P = cd->get_intersection_point_2d(ab);
            shared_ptr<bsp_node> cP = make_shared<bsp_node>(cd->getP1(), P, cd->getContour());
            shared_ptr<bsp_node> Pd = make_shared<bsp_node>(P, cd->getP2(), cd->getContour());

            if(c_side >= 0){
                left.push_back(cP);
                right.push_back(Pd);
            }
            else{
                left.push_back(Pd);
                right.push_back(cP);
            }

            // if we have a closed crossing, find out closest contour and split it
            if(not Tools::sameside2d(cd->getP1(),cd->getP2(),ab->getP1(),ab->getP2())){
//                // find both 3d intersection points
//                Vector3d ab_p3d = ab->get_intersection_point_3d(cd);
//                Vector3d cd_p3d = cd->get_intersection_point_3d(ab);
//
//                float dist_ab = (ab_p3d.cast<float>() - camera.get_position()).norm();
//                float dist_cd = (cd_p3d.cast<float>() - camera.get_position()).norm();

                // check farthest point and split edge there
                // store intersection only if closest contour is occluding or boundary
//                if( dist_ab < dist_cd and (ab_occ or ab_bou))
//                if(ab_occ or ab_bou)
                    cd->add_contour_vertex(ab);
//                else if(dist_ab > dist_cd and (cd_occ or cd_bou))
//                if(cd_occ or cd_bou)
                    ab->add_contour_vertex(cd);
            }
        }
    }

    // compute both sub trees
    compute_BSP(left, camera, tol);
    compute_BSP(right, camera, tol);
}
