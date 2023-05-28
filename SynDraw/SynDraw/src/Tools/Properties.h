#ifndef PROPERTIES_H
#define PROPERTIES_H

#include <string>
#include <sstream>
#include <fstream>
#include <vector>
#include <iostream>
#include <stdexcept> 

#include "../Tools/ProjectionType.h"
#include "PropertiesFileReader.h"

using namespace std;
using namespace Eigen;

/**\brief Properties data container and accessor
 * \author Bastien Wailly <bastien.wailly@inria.fr>, Inria */
class Properties{
public:    
    string mesh_in;         /*!<@brief Path to input 3D mesh */ 
    string svg_out;         /*!<@brief Path to output SVG */ 
    bool svg_bg;            /*!<@brief Set to true to add white background to SVG */ 
    bool show_axis;         /*!<@brief Set to true to show axis helper in SVG */ 
    bool show_chains;       /*!<@brief Set to true to assign a random color to each chains */
    bool show_singularities;/*!<@brief Set to true to show singularities in SVG */
    
    Vector3f camera_target;     /*!<@brief Camera look at target (world space) */ 
    Vector3f camera_position;   /*!<@brief Camera position (world space) */ 
    Vector3f camera_up;         /*!<@brief Camera up vector (world space) */ 
    
    ProjectionType p_type;  /*!<@brief Orthographic or Perspective */
    double p_left;          /*!<@brief Camera view frustum left value */
    double p_right;         /*!<@brief Camera view frustum right value */
    double p_bottom;        /*!<@brief Camera view frustum bottom value */
    double p_top;           /*!<@brief Camera view frustum top value */
    double p_near;          /*!<@brief Camera view frustum near value */
    double p_far;           /*!<@brief Camera view frustum far value */
    
    Vector4f viewport;      /*!<@brief Camera and SVG viewport frame*/
    
    bool occlusive;         /*!<@brief Set to true to enable occlusive contours */
    bool boundaries;        /*!<@brief Set to true to enable boundary contours */
    bool suggestive;        /*!<@brief Set to true to enable suggestive contours */
    bool ridges;            /*!<@brief Set to true to enable ridges*/
    bool valleys;           /*!<@brief Set to true to enable valleys */
    bool sharp;             /*!<@brief Set to true to enable sharp creases */
    bool demarcating;       /*!<@brief Set to true to enable demarcating curves*/
    
    
    bool chaining;          /*!<@brief Set to true to enable chaining */
    double chain_angle;     /*!<@brief Maximum chain angle between two lines (deg)*/
    double voting_factor;   /*!<@brief Percentage of lines per stroke used to compute visibility of said stroke */

    double sharp_angle;     /*!<@brief Maximum dihedral angle for sharp creases */
    
    bool extract_hidden;    /*!<@brief Set to true to enable hidden lines */
    bool frustum_culling;   /*!<@brief Set to true to enable frustum culling */
    bool use_smooth;        /*!<@brief Set to true to enable interpolated occlusion contours */
    
    double gradient_radius; /*!<@brief Radius used to compute gradient at a vertex */
    int ring_size;          /*!<@brief Size of ring for curvature computations */
    double curvature_scale; /*!<@brief Multiplying factor for radial curvature */
    
    double intersections_tol;   /*!<@brief Numerical tolerance for intersections (min distance between two vertices) */
    double raycast_tol;         /*!<@brief Numerical tolerance for raycasts and visibility (min distance between hitpoint and line)*/
    
    bool suggestive_hysteresis;         /*!<@brief Set to true to enable hysteresis thresholding for suggestive contours */
    double suggestive_threshold;        /*!<@brief Hysteresis threshold for suggestive contours */
    double suggestive_tolerance;        /*!<@brief Hysteresis tolerance for suggestive contours */
    bool suggestive_filtering;          /*!<@brief Set to true to enable view-dependent filtering for suggestive contours */
    double suggestive_max_visibility;   /*!<@brief Max ndotv for suggestive contours */
    double suggestive_min_visibility;   /*!<@brief Min ndotv for suggestive contours  */
    double suggestive_speed;            /*!<@brief Suggestive contours speed */
        
    bool ridges_hysteresis;         /*!<@brief Set to true to enable hysteresis thresholding for ridges */
    bool ridges_filtering;          /*!<@brief Set to true to enable view-dependent filtering for ridges */
    double ridges_threshold;        /*!<@brief Hysteresis threshold for ridges */
    double ridges_tolerance;        /*!<@brief Hysteresis tolerance for ridges */
    double ridges_max_visibility;   /*!<@brief Max ndotv for ridges */
    double ridges_min_visibility;   /*!<@brief Min ndotv for ridges */
    
    bool valleys_hysteresis;        /*!<@brief Set to true to enable hysteresis thresholding for valleys */
    bool valleys_filtering;         /*!<@brief Set to true to enable view-dependent filtering for valleys */
    double valleys_threshold;       /*!<@brief Hysteresis threshold for valleys */
    double valleys_tolerance;       /*!<@brief Hysteresis tolerance for valleys */
    double valleys_max_visibility;  /*!<@brief Max ndotv for valleys */
    double valleys_min_visibility;  /*!<@brief Min ndotv for valleys */ 
    
    bool demarcating_hysteresis;        /*!<@brief Set to true to enable hysteresis thresholding for demarcating curves */
    bool demarcating_filtering;         /*!<@brief Set to true to enable view-dependent filtering for demarcating curves */
    double demarcating_threshold;       /*!<@brief Hysteresis threshold for demarcating */
    double demarcating_tolerance;       /*!<@brief Hysteresis tolerance for demarcating */
    double demarcating_max_visibility;  /*!<@brief Max ndotv for demarcating */
    double demarcating_min_visibility;  /*!<@brief Min ndotv for demarcating */
    
    Vector3d color_occlusive;       /*!<@brief Line color of occlusive contours */
    Vector3d color_boundaries;      /*!<@brief Line color of boundary contours */
    Vector3d color_suggestive;      /*!<@brief Line color of suggestive contours */
    Vector3d color_demarcating;     /*!<@brief Line color of demarcating */
    Vector3d color_ridges;          /*!<@brief Line color of ridges */
    Vector3d color_valleys;         /*!<@brief Line color of valleys */
    Vector3d color_hidden;          /*!<@brief Line color of hidden lines */
    Vector3d color_sharp;           /*!<@brief Line color of sharp creases */
    
    double width_occlusive;     /*!<@brief Line width of occlusive contours */
    double width_sharp;         /*!<@brief Line width of sharp creases */
    double width_boundaries;    /*!<@brief Line width of boundary contours */
    double width_valleys;       /*!<@brief Line width of valleys */
    double width_ridges;        /*!<@brief Line width of ridges */
    double width_demarcating;   /*!<@brief Line width of demarcating */
    double width_suggestive;    /*!<@brief Line width of suggestive contours */
    
    double opacity_occlusive;     /*!<@brief Line opacity of occlusive contours */
    double opacity_sharp;         /*!<@brief Line opacity of sharp creases */
    double opacity_boundaries;    /*!<@brief Line opacity of boundary contours */
    double opacity_valleys;       /*!<@brief Line opacity of valleys */
    double opacity_ridges;        /*!<@brief Line opacity of ridges */
    double opacity_demarcating;   /*!<@brief Line opacity of demarcating */
    double opacity_suggestive;    /*!<@brief Line opacity of suggestive contours */    
    

    /**\brief create a new Properties and set it to default*/
    Properties(){
        default_values();
    };
    
    /**\brief load properties from file
     * \param f path to properties file*/
    void load_from_file(string f){
        if(not P.Read(f))
            throw("Error: cannot read properties file.");
        load();
    }
    
    /**\brief reload properties from previously used file*/    
    void reload_from_file(){
        if(not P.reload())
            throw("Error: cannot reload properties file.");
        load();
    }
    
    /**\brief save properties to file
     * \param fname path to desired properties file*/    
    void save_to_file(string fname){
        ofstream file;
        file.open(fname);

        file << boolalpha;
        
        file << "#### I/O" << endl;
        write(file, "mesh_in", mesh_in);
        write(file, "svg_out", svg_out);
                
        file << "#### SVG" << endl;        
        write(file, "svg_bg", svg_bg);
        write(file, "show_axis", show_axis);        
        write(file, "show_chains", show_chains);
        write(file, "show_singularities", show_singularities);
        
        file << "#### CAMERA" << endl;        
        write(file, "camera_target", camera_target.format(fmt));
        write(file, "camera_position", camera_position.format(fmt));
        write(file, "camera_up", camera_up.format(fmt));
        if(p_type == ProjectionType::ORTHOGRAPHIC)
            write(file, "p_type", "ORTHOGRAPHIC");
        else
            write(file, "p_type", "PERSPECTIVE");
        write(file, "p_left", p_left);
        write(file, "p_right", p_right);
        write(file, "p_bottom", p_bottom);
        write(file, "p_top", p_top);
        write(file, "p_near", p_near);
        write(file, "p_far", p_far);
        write(file, "viewport", viewport.format(fmt));
        
        file << "#### COMPUTATIONAL" << endl;
        write(file, "extract_hidden", extract_hidden);
        write(file, "frustum_culling", frustum_culling);
        write(file, "ring_size", ring_size);
        write(file, "curvature_scale", curvature_scale);
        write(file, "gradient_radius", gradient_radius);
        file << scientific << setprecision(1);
        write(file, "intersections_tol", intersections_tol);
        write(file, "raycast_tol", raycast_tol);
        file << defaultfloat;
        
        file << "#### CONTOUR TYPES" << endl;
        write(file, "occlusive", occlusive);
        write(file, "use_smooth", use_smooth);
        write(file, "boundaries", boundaries);
        write(file, "sharp", sharp);
        write(file, "suggestive", suggestive);
        write(file, "ridges", ridges);
        write(file, "valleys", valleys);
        write(file, "demarcating", demarcating);
        
        file << "#### CHAINING" << endl; 
        write(file, "chaining", chaining);
        write(file, "chain_angle", chain_angle);
        write(file, "voting_factor", voting_factor);
        
        file << "#### FILTERING" << endl;
        write(file, "sharp_angle", sharp_angle);
        write(file, "suggestive_hysteresis", suggestive_hysteresis);
        write(file, "suggestive_filtering", suggestive_filtering);
        write(file, "suggestive_speed", suggestive_speed);
        write(file, "suggestive_threshold", suggestive_threshold);
        write(file, "suggestive_tolerance", suggestive_tolerance);
        write(file, "suggestive_max_visibility", suggestive_max_visibility);
        write(file, "suggestive_min_visibility", suggestive_min_visibility);
        write(file, "demarcating_filtering", demarcating_filtering);
        write(file, "demarcating_hysteresis", demarcating_hysteresis);
        write(file, "demarcating_threshold", demarcating_threshold);
        write(file, "demarcating_tolerance", demarcating_tolerance);
        write(file, "demarcating_max_visibility", demarcating_max_visibility);
        write(file, "demarcating_min_visibility", demarcating_min_visibility);
        write(file, "ridges_hysteresis", ridges_hysteresis);
        write(file, "ridges_filtering", ridges_filtering);
        write(file, "ridges_threshold", ridges_threshold);
        write(file, "ridges_tolerance", ridges_tolerance);
        write(file, "ridges_max_visibility", ridges_max_visibility);
        write(file, "ridges_min_visibility", ridges_min_visibility);
        write(file, "valleys_hysteresis", valleys_hysteresis);
        write(file, "valleys_filtering", valleys_filtering);
        write(file, "valleys_threshold", valleys_threshold);
        write(file, "valleys_tolerance", valleys_tolerance);
        write(file, "valleys_max_visibility", valleys_max_visibility);
        write(file, "valleys_min_visibility", valleys_min_visibility);
        
        file << "#### COLORS" << endl;
        write(file, "color_occlusive", color_occlusive.format(fmt));
        write(file, "color_boundaries", color_boundaries.format(fmt));
        write(file, "color_sharp", color_sharp.format(fmt));
        write(file, "color_suggestive", color_suggestive.format(fmt));
        write(file, "color_ridges", color_ridges.format(fmt));
        write(file, "color_valleys", color_valleys.format(fmt));
        write(file, "color_demarcating", color_demarcating.format(fmt));
        write(file, "color_hidden", color_hidden.format(fmt));
        
        file << "#### LINE WIDTHS" << endl;
        write(file, "width_occlusive", width_occlusive);
        write(file, "width_valleys", width_valleys);
        write(file, "width_ridges", width_ridges);
        write(file, "width_suggestive", width_suggestive);
        write(file, "width_demarcating", width_demarcating);
        write(file, "width_boundaries", width_boundaries);
        write(file, "width_sharp", width_sharp);
        
        file << "#### LINE ALPHAS" << endl;
        write(file, "opacity_occlusive", opacity_occlusive);
        write(file, "opacity_valleys", opacity_valleys);
        write(file, "opacity_ridges", opacity_ridges);
        write(file, "opacity_suggestive", opacity_suggestive);
        write(file, "opacity_demarcating", opacity_demarcating);
        write(file, "opacity_boundaries", opacity_boundaries);
        write(file, "opacity_sharp", opacity_sharp);        
        
        file.close();
    }
    
    /**\brief set every property to its default value*/
    void default_values(){
        mesh_in="./in.obj";
        show_axis=false;
        svg_out="./out.svg";
        svg_bg=true;
        extract_hidden=false;
        camera_target << 0, 0, 0;
        camera_position << 0, 0, -5;
        camera_up << 0, 1, 0;
        p_type=ProjectionType::PERSPECTIVE;
        p_near=0.1;
        p_far=100;
        p_left=-5;
        p_right=5;
        p_top=5;
        p_bottom=-5;
        viewport << 0, 0, 256, 256;
        chaining=true;
        chain_angle=90;
        voting_factor=0.8;
        extract_hidden=false;
        frustum_culling=false;
        ring_size=4;
        curvature_scale=20;
        gradient_radius=1;
        intersections_tol=1e-4;
        raycast_tol=1e-3;
        sharp_angle=30;
        suggestive=false;
        demarcating=false;
        ridges=false;
        occlusive=true;
        boundaries=true;
        valleys=true;
        use_smooth=false;
        sharp=false;
        suggestive_filtering=false;
        suggestive_hysteresis=false;
        suggestive_speed=0.20;
        suggestive_threshold=0.2;
        suggestive_tolerance=0.8;
        suggestive_max_visibility=0.1;
        suggestive_min_visibility=-1;
        ridges_filtering=false;
        ridges_hysteresis=false;
        ridges_threshold=3.6;
        ridges_tolerance=6.5;
        ridges_max_visibility=-1;
        ridges_min_visibility=-1;        
        valleys_filtering=false;
        valleys_hysteresis=false;
        valleys_threshold=1;
        valleys_tolerance=2;
        valleys_max_visibility=0.1;
        valleys_min_visibility=-1;
        demarcating_filtering=false;
        demarcating_hysteresis=false;
        demarcating_threshold=0.05;
        demarcating_tolerance=0.3;
        demarcating_max_visibility=-0.34;
        demarcating_min_visibility=-1;        
        color_occlusive << 0,0,0;
        color_boundaries << 0,0,0;
        color_sharp << 1,0,0;
        color_suggestive << 61.0/255.0, 176.0/255.0, 61.0/255.0;
        color_demarcating << 103.0/255.0, 203.0/255.0, 255.0/255.0;
        color_ridges << 240.0/255.0, 85.0/255.0, 240.0/255.0;
        color_valleys << 236.0/255.0, 171.0/255.0, 27.0/255.0;
        color_hidden << 0.9,0.9,0.9;
        width_occlusive = 3.0f;
        width_sharp = 1.0f;
        width_boundaries = 3.0f;
        width_valleys = 1.0f;
        width_ridges = 2.0f;
        width_demarcating = 1.5f;
        width_suggestive = 2.0f;    
        opacity_occlusive = 1.0f;
        opacity_sharp = 1.0f;
        opacity_boundaries = 1.0f;
        opacity_valleys = 1.0f;
        opacity_ridges = 1.0f;
        opacity_demarcating = 1.0f;
        opacity_suggestive = 1.0f;         
        show_chains = false;
        show_singularities = false;
    }
 
    
private:  
    PropertiesFileReader P;
    Eigen::IOFormat fmt = Eigen::IOFormat(FullPrecision, DontAlignCols, "", ", ", "", "", "", "");

    /**\brief read a bool property 
     * \param b out bool
     * \param s in string property name
     * \details b will be true if property named s is "true" or "0" */
    void readBool(bool& b, string s){
        try{
            istringstream(P.V(s)) >> std::boolalpha >> b;
        }
        catch(const std::exception & e ){
            std::cout << "Error reading bool : " << s << std::endl;
            throw;
        }        
    }
    
    /**\brief read a double property
     * \param d out double
     * \param s in string property name*/    
    void readDouble(double& d, string s){
        try{
            d = stod(P.V(s));
        }
        catch(const std::exception & e ){
            std::cout << "Error reading double : " << s << std::endl;
            throw;
        }
    }
    
    /**\brief read an int property
     * \param d out int
     * \param s in string property name*/     
    void readInt(int& i, string s){
        try{
            i = stoi(P.V(s));
        }
        catch(const std::exception & e ){
            std::cout << "Error reading int : " << s << std::endl;
            throw;
        }
    }
    
    /**\brief read a float vector property
     * \param s in string property name
     * \return float vector value of s*/     
    VectorXf readVector_f(string s){
        try{
            int i=0;
            auto cords = split(P.V(s), ',');
            VectorXf v(cords.size());
            for(auto c: cords){
                v(i++) = stod(c);
            }
            return v;
        }
        catch(const std::exception & e ){
            std::cout <<  "Error reading vector : " << s << std::endl;
            throw;
        }
    }
    
    /**\brief read a double vector property
     * \param s in string property name
     * \return double vector value of s*/     
    VectorXd readVector_d(string s){
        try{
            int i=0;
            auto cords = split(P.V(s), ',');
            VectorXd v(cords.size());
            for(auto c: cords){
                v(i++) = stod(c);
            }
            return v;
        }
        catch(const std::exception & e ){
            std::cout <<  "Error reading vector : " << s << std::endl;
            throw;
        }
    }  
    
    /**\brief write property label and value inside a file
     * \param file opened file stream
     * \param label name of the property 
     * \param value template value of the property
     * \tparam T writable value (must implement <<operator)*/
    template<class T>
    void write(ofstream& file, string label, T& value){
        file << label << "=" << value << endl;
    }
    
    
    /**\brief split string with a given character
     * \param s the string to split
     * \param delimiter the character to split with
     * \return list of result string (excluding delimiter)*/
    std::vector<std::string> split(std::string s, char delimiter)
    {
       std::vector<std::string> tokens;
       std::string token;
       std::istringstream tokenStream(s);
       while (std::getline(tokenStream, token, delimiter))
       {
          tokens.push_back(token);
       }
       return tokens;
    }
    
    /**\brief load all values from file*/
    void load(){
        mesh_in = P.V("mesh_in");
        svg_out = P.V("svg_out");
        
        camera_target = readVector_f("camera_target");
        camera_position = readVector_f("camera_position");
        camera_up = readVector_f("camera_up");
        viewport = readVector_f("viewport");
        
        color_occlusive = readVector_d("color_occlusive");
        color_boundaries = readVector_d("color_boundaries");
        color_sharp = readVector_d("color_sharp");
        color_suggestive = readVector_d("color_suggestive");
        color_ridges = readVector_d("color_ridges");
        color_valleys = readVector_d("color_valleys");
        color_demarcating = readVector_d("color_demarcating");
        color_hidden = readVector_d("color_hidden");
        
        readInt(ring_size, "ring_size");
        
        readBool(svg_bg, "svg_bg");
        readBool(extract_hidden, "svg_hidden");
        readBool(show_axis, "show_axis");
        readBool(chaining, "chaining");
        readBool(extract_hidden, "extract_hidden");
        readBool(use_smooth, "use_smooth");
        readBool(boundaries, "boundaries");
        readBool(occlusive, "occlusive");
        readBool(suggestive, "suggestive");
        readBool(sharp, "sharp");
        readBool(ridges, "ridges");
        readBool(valleys, "valleys");   
        readBool(demarcating, "demarcating");
        readBool(suggestive_hysteresis, "suggestive_hysteresis");
        readBool(suggestive_filtering, "suggestive_filtering");
        readBool(ridges_hysteresis, "ridges_hysteresis");
        readBool(valleys_hysteresis, "valleys_hysteresis");
        readBool(ridges_filtering, "ridges_filtering");
        readBool(valleys_filtering, "valleys_filtering");
        readBool(demarcating_filtering, "demarcating_filtering");
        readBool(demarcating_hysteresis, "demarcating_hysteresis");
        readBool(frustum_culling, "frustum_culling");
        readBool(show_chains, "show_chains");
        readBool(show_singularities, "show_singularities");
        
        readDouble(p_left, "p_left");
        readDouble(p_right, "p_right");
        readDouble(p_bottom, "p_bottom");
        readDouble(p_top, "p_top");
        readDouble(p_near, "p_near");
        readDouble(p_far, "p_far");
        readDouble(chain_angle, "chain_angle");
        readDouble(voting_factor, "voting_factor");
        readDouble(curvature_scale, "curvature_scale");
        readDouble(suggestive_threshold, "suggestive_threshold");
        readDouble(suggestive_tolerance, "suggestive_tolerance");
        readDouble(suggestive_max_visibility, "suggestive_max_visibility");
        readDouble(suggestive_min_visibility, "suggestive_min_visibility");
        readDouble(suggestive_speed, "suggestive_speed");
        readDouble(intersections_tol, "intersections_tol");
        readDouble(raycast_tol, "raycast_tol");
        readDouble(gradient_radius, "gradient_radius");
        readDouble(sharp_angle, "sharp_angle");
        readDouble(ridges_threshold, "ridges_threshold");
        readDouble(ridges_tolerance, "ridges_tolerance");
        readDouble(valleys_threshold, "valleys_threshold");
        readDouble(valleys_tolerance, "valleys_tolerance");
        readDouble(ridges_max_visibility, "ridges_max_visibility");
        readDouble(ridges_min_visibility, "ridges_min_visibility");
        readDouble(valleys_max_visibility, "valleys_max_visibility");
        readDouble(valleys_min_visibility, "valleys_min_visibility");
        readDouble(demarcating_threshold, "demarcating_threshold");
        readDouble(demarcating_tolerance, "demarcating_tolerance");
        readDouble(demarcating_max_visibility, "demarcating_max_visibility");
        readDouble(demarcating_min_visibility, "demarcating_min_visibility");
        readDouble(width_boundaries, "width_boundaries");
        readDouble(width_sharp, "width_sharp");
        readDouble(width_valleys, "width_valleys");
        readDouble(width_demarcating, "width_demarcating");
        readDouble(width_suggestive, "width_suggestive");
        readDouble(width_ridges, "width_ridges");
        readDouble(width_occlusive, "width_occlusive");
        readDouble(opacity_boundaries, "opacity_boundaries");
        readDouble(opacity_sharp, "opacity_sharp");
        readDouble(opacity_valleys, "opacity_valleys");
        readDouble(opacity_demarcating, "opacity_demarcating");
        readDouble(opacity_suggestive, "opacity_suggestive");
        readDouble(opacity_ridges, "opacity_ridges");
        readDouble(opacity_occlusive, "opacity_occlusive");        

        if(P.V("p_type") == "ORTHOGRAPHIC")
            p_type = ProjectionType::ORTHOGRAPHIC;
        else if(P.V("p_type") == "PERSPECTIVE")
            p_type = ProjectionType::PERSPECTIVE;
        else{
            std::cout << "Does not name a projection type : " << P.V("p_type") << std::endl;
            throw;
        }        
    }    
}; 


#endif /* PARAMETERS_H */

