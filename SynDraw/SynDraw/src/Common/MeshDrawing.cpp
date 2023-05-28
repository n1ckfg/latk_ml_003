#include "MeshDrawing.h"
#include "../Tools/SVGCreator.h"
#include "../Tools/RaycastHelper.h"
#include "../Lib/simple_svg.hpp"
#include "../Tools/Tools.h"

#include <igl/colormap.h>
#include <igl/parula.h>
#include <iostream>
#include <sstream>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <chrono>

#include "../Tools/Properties.h"

MeshDrawing::~MeshDrawing() {
}

MeshDrawing::MeshDrawing(std::shared_ptr<Properties> p):props(p) {
    std::srand(std::time(nullptr));
    chains_computed = false;   
}

void MeshDrawing::load_mesh() {
    std::cout << "Reading mesh..." << std::endl;
    std::cout << props->mesh_in << std::endl;    
    try{
        mesh = MeshObject(*props, false);
    }
    catch(int e){
        std::cout << "Error: cannot read mesh file." << std::endl;
        throw;
    }
    mesh.load();
}


void MeshDrawing::update_camera_from_props() {
    camera.setFromProperties(*props); 
}

//void MeshDrawing::reload_props() {
//    props->reload_from_file();
////    camera.setFromProperties(props);
//}

void MeshDrawing::set_camera_default() {
    camera.setDefault();
}


bool MeshDrawing::compute(bool camera_changed) {  
    auto start = std::chrono::high_resolution_clock::now();
    
    if(camera_changed)
        mesh.reset_radial();
    std::cout << "Building contours..." << std::endl;
    build_contours();
    chains_computed = false;
    
    if(props->chaining){
        std::cout << "Building chains..." << std::endl;                
        build_chains();
        compute_chains();
    }else{
        compute_lines();
    }
    
    auto t1 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> ext = t1 - start;
            
    std::cout << "Done! Elapsed: " << ext.count() << std::endl;
    
    return true;
}

void MeshDrawing::build_contours() {
    graph = std::make_shared<ViewGraph>(mesh, camera, *props);
    contours = graph->compute_contours();
    singularities = graph->get_singularities();
}

void MeshDrawing::build_chains() {
    ContourChainer chainer = ContourChainer(contours, props->chain_angle);    
    chains = chainer.build_chains();
    RaycastHelper ray(mesh, camera);
    chainer.compute_chains_visibility(ray, props->raycast_tol, props->voting_factor);
    chains_computed = true;        
}

void MeshDrawing::ensure_chains() {
    if(not chains_computed){
        rebuild_chains();
    }
}

void MeshDrawing::rebuild_chains() {
    build_chains();
    compute_chains();
}

void MeshDrawing::reset() {
    contours.clear();
    chains.clear();
    singularities.clear();
    compute_lines();
    compute_chains();
}

//void MeshDrawing::create_svg_file_colored_chains(string path_to_svg) {
//    std::cout << "Creating SVG..." << std::endl;        
//    svg::Document svg = SVGCreator::Create_SVG(path_to_svg, *props);
//    
//    if(props->show_axis)
//        SVGCreator::ShowFrame(svg, camera);
//    
//    // if saving from viewer, use the random colors generated previously and stored in each chain
//    if(props->chaining){
//        if(props->extract_hidden){
//            for(auto c: chains){
//                if(not c->isVisible())
//                    SVGCreator::Add_Line(svg, c, props->color_hidden, props->pen_size);
//            }
//        }
//        for(auto c: chains){
//            if(c->isVisible())
//                SVGCreator::Add_Line(svg, c, c->color, props->pen_size);
//        }        
//    }
//    else{
//        add_contours_to_svg(svg);
//    }
//
//    svg.save();
//}

void MeshDrawing::create_svg_file(string path_to_svg) {
    std::cout << "Creating SVG..." << std::endl;        
    svg::Document svg = SVGCreator::Create_SVG(path_to_svg, props -> viewport, props -> svg_bg);
    
    if(props->show_axis)
        SVGCreator::Show_Frame(svg, camera);
    
    if(props->chaining){
        add_chains_to_svg(svg);       
    }
    else{
        add_contours_to_svg(svg);
    }
    if(props->show_singularities)
        add_singularities_to_svg(svg);

    svg.save();
}

void MeshDrawing::add_singularities_to_svg(svg::Document& svg) {
    for(auto s: singularities){
        SVGCreator::Draw_Circle(svg, s);
    }
}


void MeshDrawing::update_viewer_mesh(igl::opengl::glfw::Viewer& v) {
    v.data().set_mesh(mesh.V(), mesh.F());  
    v.core().align_camera_center(mesh.V());    
}

void MeshDrawing::update_viewer_content(igl::opengl::glfw::Viewer& v) {
    v.data().clear();
    update_viewer_mesh(v);
    
    if(props->chaining){
        v.data().add_edges(ch_lines.P1, ch_lines.P2, ch_lines.color);
    }
    else{
        if(props->boundaries)
            v.data().add_edges(b_lines.P1, b_lines.P2, b_lines.color);        

        if(props->occlusive)
            v.data().add_edges(o_lines.P1, o_lines.P2, o_lines.color);

        if(props->suggestive)
            v.data().add_edges(s_lines.P1, s_lines.P2, s_lines.color);

        if(props->sharp)
            v.data().add_edges(sh_lines.P1, sh_lines.P2, sh_lines.color);

        if(props->ridges)
            v.data().add_edges(r_lines.P1, r_lines.P2, r_lines.color);

        if(props->valleys)
            v.data().add_edges(v_lines.P1, v_lines.P2, v_lines.color);

        if(props->demarcating)
            v.data().add_edges(d_lines.P1, d_lines.P2, d_lines.color); 
    }
    
    camera.show(v);
}

Vector3d random_color(){
    return Vector3d( static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX), 
                        static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX), 
                        static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX));
}

void MeshDrawing::compute_chains() {
    ch_lines = Lines();

    for(auto c: chains){
        Vector3d color;
        if(c->isVisible()){
            if(props->show_chains){
                color = random_color();                
            }
            else{
                switch(c->get_type()){
                    case ContourType::BOUNDARY:
                        color = props->color_boundaries;
                        break;
                    case ContourType::OCCLUSION:
                        color = props->color_occlusive;
                        break;
                    case ContourType::SHARP:
                        color = props->color_sharp;
                        break;
                    case ContourType::SUGGESTIVE:
                        color = props->color_suggestive;
                        break;
                    case ContourType::DEMARCATING:
                        color = props->color_demarcating;
                        break;
                    case ContourType::RIDGES:
                        color = props->color_ridges;
                        break;
                    case ContourType::VALLEYS:
                        color = props->color_valleys;
                        break;
                }                      
            }
            c->color = color;        
            c->Draw(ch_lines, color);            
        } 
        else if(props->extract_hidden){
            c->color = props->color_hidden;
            c->Draw(ch_lines, props->color_hidden);
        }
    }
}

void MeshDrawing::show_singularities(igl::opengl::glfw::Viewer& viewer) {  
    for(auto v: singularities){
        if(v->type() == VertexType::CUSP){
            v->Draw(viewer, RowVector3d(1.0f, 0.0f, 0.0f));
        }else if(v->type() == VertexType::Y_JUNCTION){
            v->Draw(viewer, RowVector3d(0.0f, 1.0f, 0.0f));
        }else if(v->type() == VertexType::X_JUNCTION){
            v->Draw(viewer, RowVector3d(1.0f, 0.1f, 0.9f));            
        }else if(v->type() == VertexType::T_JUNCTION){
            v->Draw(viewer, RowVector3d(0.0f, 0.0f, 1.0f));            
        }else if(v->type() == VertexType::BOUNDARY_CUSP){
            v->Draw(viewer, RowVector3d(1.0f, 0.6f, 0.3f));
        }
    }     
}


void MeshDrawing::update_camera_from_viewer(igl::opengl::glfw::Viewer& v, bool camera_changed) {
    camera.setFromViewer(v);
}

void MeshDrawing::update_props_from_viewer(igl::opengl::glfw::Viewer& v) {
    camera.setFromViewer(v);    
    camera.save_to_props(props);
}

void MeshDrawing::update_viewer_from_properties(igl::opengl::glfw::Viewer& v) {
    camera.setFromProperties(*props);
    
    v.core().camera_base_zoom = 1.0;
    v.core().camera_base_translation.Identity();
    
    v.core().camera_eye = camera.get_position();
    v.core().camera_center = camera.get_target();
    v.core().camera_up = camera.get_up();
    
    v.core().orthographic = camera.get_projType() == ProjectionType::ORTHOGRAPHIC;
    v.core().camera_dnear = camera.get_near();
    v.core().camera_dfar = camera.get_far();
    
    v.core().camera_view_angle = 2.0 * atan((camera.get_top()-camera.get_bottom())/(2.0*camera.get_near()));
}

void MeshDrawing::show_gaussian_curvature(igl::opengl::glfw::Viewer& v) {
    v.data().set_colors(compute_colors(mesh.gaussian_curvature()));
}

void MeshDrawing::show_principal_curvature_min(igl::opengl::glfw::Viewer& v) {
    v.data().set_colors(compute_colors(mesh.principal_curvature_min()));
}

void MeshDrawing::show_principal_curvature_max(igl::opengl::glfw::Viewer& v) {
    v.data().set_colors(compute_colors(mesh.principal_curvature_max()));
}

void MeshDrawing::show_radial_curvature(igl::opengl::glfw::Viewer& v, bool camera_changed) {
    if(camera_changed or not mesh.is_radial_computed()){
        update_camera_from_viewer(v, true);
        mesh.compute_radial_curvature(camera);
    }
    v.data().set_colors(compute_colors(mesh.radial_curvature()));
}

void MeshDrawing::show_maximality(igl::opengl::glfw::Viewer& v) {
    v.data().set_colors(compute_colors(mesh.maxima()));
}

void MeshDrawing::show_minimality(igl::opengl::glfw::Viewer& v) {
    v.data().set_colors(compute_colors(mesh.minima()));
}

void MeshDrawing::reset_mesh(bool rad, bool pri, bool ext, bool gau, bool der, int sr, double sf, double gr) {
    if(rad)
        mesh.reset_radial();
    if(pri)
        mesh.reset_principal();
    if(ext)
        mesh.reset_extremas();
    if(gau)
        mesh.reset_gaussian();
    if(der)
        mesh.reset_derivative();
    
    mesh.setProperties(sr, sf, gr);
}



void MeshDrawing::compute_lines(){
    o_lines = Lines();
    s_lines = Lines();
    d_lines = Lines();
    r_lines = Lines();
    v_lines = Lines();
    b_lines = Lines();
    sh_lines = Lines();
    
    for(auto c: contours){
        if(c->isType(ContourType::OCCLUSION)){
            if(c->isVisible())
                c->Draw(o_lines, props->color_occlusive);
            else if(props->extract_hidden)
                c->Draw(o_lines, props->color_hidden);
        }
        else if(c->isType(ContourType::BOUNDARY)){
            if(c->isVisible())
                c->Draw(b_lines, props->color_boundaries);
            else if(props->extract_hidden)
                c->Draw(b_lines, props->color_hidden);                
        }            
        else if(c->isType(ContourType::SHARP)){
            if(c->isVisible())
                c->Draw(sh_lines, props->color_sharp);
            else if(props->extract_hidden)
                c->Draw(sh_lines, props->color_hidden);                
        }
        else if(c->isType(ContourType::SUGGESTIVE)){
            if(c->isVisible())
                c->Draw(s_lines, props->color_suggestive);
            else if(props->extract_hidden)
                c->Draw(s_lines, props->color_hidden);
        }  
        else if(c->isType(ContourType::RIDGES)){
            if(c->isVisible())
                c->Draw(r_lines, props->color_ridges);
            else if(props->extract_hidden)
                c->Draw(r_lines, props->color_hidden);
        }   
        else if(c->isType(ContourType::VALLEYS)){
            if(c->isVisible())
                c->Draw(v_lines, props->color_valleys);
            else if(props->extract_hidden)
                c->Draw(v_lines, props->color_hidden);
        }  
        else if(c->isType(ContourType::DEMARCATING)){
            if(c->isVisible())
                c->Draw(d_lines, props->color_demarcating);
            else if(props->extract_hidden)
                c->Draw(d_lines, props->color_hidden);
        }             
    }
}


void MeshDrawing::add_chains_to_svg(svg::Document& svg){
    if(props->extract_hidden){    
        for(auto c: chains){
            if(not c->isVisible()){
                switch(c->get_type()){
                case ContourType::BOUNDARY:
                    SVGCreator::Draw_Line(svg, c, c->color, props->width_boundaries, props->opacity_boundaries);
                    break;
                case ContourType::OCCLUSION:
                    SVGCreator::Draw_Line(svg, c, c->color, props->width_occlusive, props->opacity_occlusive);
                    break;
                case ContourType::SHARP:
                    SVGCreator::Draw_Line(svg, c, c->color, props->width_sharp, props->opacity_sharp);
                    break;
                case ContourType::SUGGESTIVE:
                    SVGCreator::Draw_Line(svg, c, c->color, props->width_suggestive, props->opacity_suggestive);
                    break;
                case ContourType::DEMARCATING:
                    SVGCreator::Draw_Line(svg, c, c->color, props->width_demarcating, props->opacity_demarcating);
                    break;
                case ContourType::RIDGES:
                    SVGCreator::Draw_Line(svg, c, c->color, props->width_ridges, props->opacity_ridges);
                    break;
                case ContourType::VALLEYS:
                    SVGCreator::Draw_Line(svg, c, c->color, props->width_valleys, props->opacity_valleys);                    
                    break;
                }                 
            }
        }
    }
    for(auto c: chains){
        if(c->isVisible()){
            switch(c->get_type()){
            case ContourType::BOUNDARY:
                SVGCreator::Draw_Line(svg, c, c->color, props->width_boundaries, props->opacity_boundaries);
                break;
            case ContourType::OCCLUSION:
                SVGCreator::Draw_Line(svg, c, c->color, props->width_occlusive, props->opacity_occlusive);
                break;
            case ContourType::SHARP:
                SVGCreator::Draw_Line(svg, c, c->color, props->width_sharp, props->opacity_sharp);
                break;
            case ContourType::SUGGESTIVE:
                SVGCreator::Draw_Line(svg, c, c->color, props->width_suggestive, props->opacity_suggestive);
                break;
            case ContourType::DEMARCATING:
                SVGCreator::Draw_Line(svg, c, c->color, props->width_demarcating, props->opacity_demarcating);
                break;
            case ContourType::RIDGES:
                SVGCreator::Draw_Line(svg, c, c->color, props->width_ridges, props->opacity_ridges);
                break;
            case ContourType::VALLEYS:
                SVGCreator::Draw_Line(svg, c, c->color, props->width_valleys, props->opacity_valleys);
                break;
            }                
        }
    }      
}

void MeshDrawing::add_contours_to_svg(svg::Document& svg){
    if(props->extract_hidden){
        for(auto c: contours){
            if(not c->isVisible())
                switch(c->get_type()){
                case ContourType::BOUNDARY:
                    SVGCreator::Draw_Line(svg, c, props->color_hidden, props->width_boundaries, props->opacity_boundaries);
                    break;
                case ContourType::OCCLUSION:
                    SVGCreator::Draw_Line(svg, c, props->color_hidden, props->width_occlusive, props->opacity_occlusive);
                    break;
                case ContourType::SHARP:
                    SVGCreator::Draw_Line(svg, c, props->color_hidden, props->width_sharp, props->opacity_sharp);
                    break;
                case ContourType::SUGGESTIVE:
                    SVGCreator::Draw_Line(svg, c, props->color_hidden, props->width_suggestive, props->opacity_suggestive);
                    break;
                case ContourType::DEMARCATING:
                    SVGCreator::Draw_Line(svg, c, props->color_hidden, props->width_demarcating, props->opacity_demarcating);
                    break;
                case ContourType::RIDGES:
                    SVGCreator::Draw_Line(svg, c, props->color_hidden, props->width_ridges, props->opacity_ridges);
                    break;
                case ContourType::VALLEYS:
                    SVGCreator::Draw_Line(svg, c, props->color_hidden, props->width_valleys, props->opacity_valleys);                    
                    break;
                }
        }
    }
    for(auto c: contours){
        if(c->isVisible()){
            switch(c->get_type()){
            case ContourType::BOUNDARY:
                SVGCreator::Draw_Line(svg, c, props->color_boundaries, props->width_boundaries, props->opacity_boundaries);
                break;
            case ContourType::OCCLUSION:
                SVGCreator::Draw_Line(svg, c, props->color_occlusive, props->width_occlusive, props->opacity_occlusive);
                break;
            case ContourType::SHARP:
                SVGCreator::Draw_Line(svg, c, props->color_sharp, props->width_sharp, props->opacity_sharp);
                break;
            case ContourType::SUGGESTIVE:
                SVGCreator::Draw_Line(svg, c, props->color_suggestive, props->width_suggestive, props->opacity_suggestive);
                break;
            case ContourType::DEMARCATING:
                SVGCreator::Draw_Line(svg, c, props->color_demarcating, props->width_demarcating, props->opacity_demarcating);
                break;
            case ContourType::RIDGES:
                SVGCreator::Draw_Line(svg, c, props->color_ridges, props->width_ridges, props->opacity_ridges);
                break;
            case ContourType::VALLEYS:
                SVGCreator::Draw_Line(svg, c, props->color_valleys, props->width_valleys, props->opacity_valleys);                    
                break;
            }
        }
    }    
}

MatrixXd MeshDrawing::compute_colors (const VectorXd &values) const {
    const unsigned int num_values = values.rows();
    MatrixXd verticesColor{num_values, 3};
    double minValue, maxValue;

    estimateRangeOfValues(values, minValue, maxValue);

    const double denominator = 1.0 / (maxValue - minValue);
#pragma omp parallel for
    for (unsigned int i = 0; i < num_values; ++i) {
        const double normalizedValue = (values(i) - minValue) * denominator;
        igl::colormap(igl::COLOR_MAP_TYPE_PARULA, normalizedValue, verticesColor(i, 0), verticesColor(i, 1), verticesColor(i, 2));
    }

     return verticesColor;
}


void MeshDrawing::estimateRangeOfValues (const VectorXd &values, double &minValue, double &maxValue) const {
    const unsigned int numValues = values.rows();
    VectorXd sortedValues{values};
    sort(sortedValues.data(), sortedValues.data() + numValues);

    const unsigned int index_secondDecile = round(0.2 * (numValues + 1.0));
    const unsigned int index_eighthDecile = round(0.8 * (numValues + 1.0));

    minValue = sortedValues(index_secondDecile);
    maxValue = sortedValues(index_eighthDecile);
}
