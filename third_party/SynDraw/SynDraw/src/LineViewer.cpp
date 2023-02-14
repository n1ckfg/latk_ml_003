#include <cmath>
#include <Eigen/Core>

#include "LineViewer.h"
#include <igl/file_dialog_open.h>
#include <igl/file_dialog_save.h>
#include <igl/opengl/glfw/imgui/ImGuiHelpers.h>
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <libgen.h>

#include <string>
#include <iostream>

//filedialog.mm : for Apple file dialog
extern std::vector<std::string> openFileDialog(char const * const aTitle ,
     char const * const aDefaultPathAndFile ,
     const std::vector<std::string> & filters);

extern std::vector<std::string> saveFileDialog(char const * const aTitle ,
     char const * const aDefaultPathAndFile ,
     const std::vector<std::string> & filters);

LineViewer::LineViewer(){
    viewer.core().background_color << 1.0, 1.0, 1.0, 0.0;
    viewer.core().lighting_factor = 0.5;
    viewer.data().point_size = 10;
    viewer.data().show_lines = false;
    viewer.data().line_width = 2.0;
    viewer.data().set_face_based(true);
    
    props = std::make_shared<Properties>();
    
    default_properties();
    createMenu();
    
    drawing = MeshDrawing(props);
    default_colors();    
    
    mesh_loaded=false;
    camera_changed=false;
    to_recompute_all=true;
    to_recompute_chains=true;
}

void LineViewer::launch() {
    viewer.callback_mouse_move =
    [&](igl::opengl::glfw::Viewer & viewer,int mouse_x,int mouse_y)->bool{
        if(viewer.down)
            camera_changed = true;
        return false;
    };
    viewer.launch();
}

void LineViewer::button_load_mesh() {
    string fname = load_window({"off", "obj"});
    if(fname == "")
        return;
    viewer.data().clear();
    props->mesh_in = fname;      
    drawing.reset();    
    drawing.load_mesh();
    drawing.update_viewer_mesh(viewer);
    mesh_loaded = true;
    camera_changed = false;
    viewer.data().show_faces = true;
}

void LineViewer::button_compute() {
    if(not mesh_loaded)
        return;
    drawing.update_camera_from_viewer(viewer, camera_changed);
    drawing.compute();
//    drawing.compute_lines();
//    if(props->chaining)
//        drawing.compute_chains();    
    redraw();
    viewer.data().show_faces = false; 
}

void LineViewer::button_save_props() {
    string fname = igl::file_dialog_save();
    drawing.update_props_from_viewer(viewer);
    props->save_to_file(fname);
}

void LineViewer::button_save_svg() {
    string fname = igl::file_dialog_save();
    drawing.update_props_from_viewer(viewer);
    drawing.create_svg_file(fname);
}

//void LineViewer::button_load_props() {
//    string fname = load_window({"properties"});
//    if(fname == "")
//        return;
//    props->load_from_file(fname);
//    viewer.data().clear();
//    drawing.reset();
//    drawing.load_mesh();
//    drawing.update_viewer_mesh(viewer);
//    drawing.update_viewer_from_properties(viewer);
//    mesh_loaded = true;
//    camera_changed = false;
//    viewer.data().show_faces = true;
//    drawing.compute();
//    redraw();
//    viewer.data().show_faces = false;     
//}

string LineViewer::load_window(std::vector<std::string> filters) {
    std::string fname = "";
#ifdef __APPLE__           
    filters = openFileDialog("Select a file", "/", filters);
    if(filters.size() > 0)
        fname = filters[0];
#else
    fname = igl::file_dialog_open();
#endif
    return fname;
}

string LineViewer::save_window(std::vector<std::string> filters) {
    std::string fname = "";
#ifdef __APPLE__           
    filters = openFileDialog("Select a file", "/", filters);
    if(filters.size() > 0)
        fname = filters[0];
#else
    fname = igl::file_dialog_open();
#endif
    return fname;
}


void LineViewer::draw_axis(){
    viewer.data().add_edges(RowVector3d(1.0f, 0.0f, 0.0f), RowVector3d(0.0f, 0.0f, 0.0f), RowVector3d(1.0f, 0.0f, 0.0f));
    viewer.data().add_edges(RowVector3d(0.0f, 1.0f, 0.0f), RowVector3d(0.0f, 0.0f, 0.0f), RowVector3d(0.0f, 1.0f, 0.0f));
    viewer.data().add_edges(RowVector3d(0.0f, 0.0f, 1.0f), RowVector3d(0.0f, 0.0f, 0.0f), RowVector3d(0.0f, 0.0f, 1.0f));     
}

void LineViewer::redraw(bool recompute_lines){
    if(mesh_loaded){
        if(recompute_lines){
            if(props->chaining)
                drawing.compute_chains();
            else
                drawing.compute_lines();                
        }
        drawing.update_viewer_content(viewer);
        
        if(props->show_singularities){
            drawing.show_singularities(viewer);
        }
    }
    if(props->show_axis){
        draw_axis();
    }
}

void LineViewer::vectof(Vector3d v, float val[]){
    val[0] = v(0);
    val[1] = v(1);
    val[2] = v(2);
}


void LineViewer::default_properties() {
    props->default_values();
}

void LineViewer::default_colors() {
    vectof(props->color_occlusive, occ_color);
    vectof(props->color_boundaries, bou_color);
    vectof(props->color_ridges, rid_color);
    vectof(props->color_suggestive, sug_color);
    vectof(props->color_valleys, val_color);
    vectof(props->color_sharp, sha_color);
    vectof(props->color_hidden, hid_color);
    vectof(props->color_demarcating, dem_color);
}

void LineViewer::createMenu() {
    viewer.plugins.push_back(&menu);
    
    menu.callback_draw_viewer_menu = [&]() {
        const ImVec2 buttonSize{-1, 0};
        
        
        // IO buttons
        if(ImGui::Button("Load mesh", buttonSize)){
            button_load_mesh();
        }
        
//        if(ImGui::Button("Load properties", buttonSize)){
//            button_load_props();
//        }
        
        if(ImGui::Button("Save properties", buttonSize)){
            button_save_props();
        }
        
        if(ImGui::Button("Save SVG", buttonSize)){
            button_save_svg();
        } 
        
        ImGui::PushStyleColor(ImGuiCol_Button, (ImU32)ImColor(60, 227, 30));        
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImU32)ImColor(227, 60, 30));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImU32)ImColor(227, 0, 0));
        ImGui::Separator();
        
        if(ImGui::Button("Compute", buttonSize)){
            button_compute();
        }
        ImGui::PopStyleColor(3);
        ImGui::Separator();   

        ImGui::SetNextTreeNodeOpen(false, ImGuiCond_Once);         
        if (ImGui::CollapsingHeader("Visualization", ImGuiTreeNodeFlags_DefaultOpen)) {
            
            static double pen_size = 1;
            if (ImGui::InputDouble("Line width", &(pen_size), 0, 0, "%.1f")) {
                viewer.data().line_width = pen_size;
            } 
            
            // Show Axis
            if (ImGui::Checkbox("Show axis", &(props->show_axis))) {
                if(props->show_axis){
                    draw_axis();
                }
                else{
                    redraw();
                }
            }
            
            if (ImGui::Checkbox("Show singularities", &(props->show_singularities))) {
                if(mesh_loaded){
                    if(props->show_singularities)
                        drawing.show_singularities(viewer);
                    else{
                        redraw();               
                    }
                }
            }        

            if(ImGui::Button("Show radial curv.", buttonSize)){
                if(mesh_loaded){
                    drawing.show_radial_curvature(viewer, camera_changed);
                    viewer.data().show_faces = true;
                }
            }

            if(ImGui::Button("Show gaussian curv.", buttonSize)){
                if(mesh_loaded){
                    drawing.show_gaussian_curvature(viewer);
                    viewer.data().show_faces = true;                
                }
            }        

            if(ImGui::Button("Show min principal curv.", buttonSize)){
                if(mesh_loaded){
                    drawing.show_principal_curvature_min(viewer);
                    viewer.data().show_faces = true; 
                }
            }

            if(ImGui::Button("Show max principal curv.", buttonSize)){
                if(mesh_loaded){
                    drawing.show_principal_curvature_max(viewer);
                    viewer.data().show_faces = true; 
                }
            }

            if(ImGui::Button("Show minimalities", buttonSize)){
                if(mesh_loaded){
                    drawing.show_minimality(viewer);
                    viewer.data().show_faces = true; 
                }
            }        

            if(ImGui::Button("Show maximalities", buttonSize)){
                if(mesh_loaded){
                    drawing.show_maximality(viewer);
                    viewer.data().show_faces = true; 
                }
            }         
        }
        ImGui::Separator();
        
        // Computation group
        ImGui::SetNextTreeNodeOpen(false, ImGuiCond_Once);         
        if (ImGui::CollapsingHeader("Computation", ImGuiTreeNodeFlags_DefaultOpen)) {
            
            ImGui::Checkbox("Extract hidden", &(props->extract_hidden));
            
            ImGui::Checkbox("Frustum culling", &(props->frustum_culling));
            
            // ring size
            if(ImGui::InputInt("Ring size", &(props->ring_size), 1, 1)){
                if(props->ring_size < 2)
                    props->ring_size = 2;
                drawing.reset_mesh(true, true, true, false, true, props->ring_size, props->curvature_scale, props->gradient_radius);
            }
            
//            // curvature scale
//            if(ImGui::InputDouble("Curvature scale", &(props->curvature_scale), 0, 0, "%.1f")) {
//                if(props->curvature_scale <= 0)
//                    props->curvature_scale = 0.1;
//                drawing.reset_mesh(true, false, false, false, false, props->ring_size, props->curvature_scale, props->gradient_radius);                
//            }             
//            
//            // gradient radius
//            if(ImGui::InputDouble("Gradient radius", &(props->gradient_radius), 0, 0, "%.1f")) {
//                if(props->gradient_radius <= 0)
//                    props->gradient_radius = 0.1;
//                drawing.reset_mesh(true, false, true, false, false, props->ring_size, props->curvature_scale, props->gradient_radius);                
//            }
            
            // tolerances
            static int intersection_tol_exponent = -4;
            if(ImGui::InputInt("Intersections tol 10e", &intersection_tol_exponent, 1, 1)){
                props->intersections_tol = pow(10, intersection_tol_exponent);
            }
            
            static int raycast_tol_exponent = -4;
            if(ImGui::InputInt("Raycast tol 10e", &raycast_tol_exponent, 1, 1)){
                props->raycast_tol = pow(10, raycast_tol_exponent);
            }            
        }
  
        ImGui::Separator();        
        
        // Chaining group   
        ImGui::SetNextTreeNodeOpen(false, ImGuiCond_Once);                 
        if (ImGui::CollapsingHeader("Chaining", ImGuiTreeNodeFlags_DefaultOpen)) {
            if(ImGui::Checkbox("Chain contours", &(props->chaining))){
                if(mesh_loaded){
                    if(props->chaining)
                        drawing.ensure_chains();
                    redraw();
                }
            }        
            
            ImGui::Checkbox("Show chains", &(props->show_chains));         

            if(not props->chaining){
                ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
                ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);             
            }            
            const double amin = 0.0; const double amax = 180.0;
            if(ImGui::SliderScalar("Chain angle", ImGuiDataType_Double, &(props->chain_angle), &amin, &amax, "%.1f deg")){
                if(mesh_loaded){
                    drawing.rebuild_chains();
                    redraw();
                }
            }

            const double fmin = 0.0; const double fmax = 1.0;
            if(ImGui::SliderScalar("Voting factor", ImGuiDataType_Double, &(props->voting_factor), &fmin, &fmax, "%.2f")){
                if(mesh_loaded){
                    drawing.rebuild_chains();
                    redraw();
                }
            }  
            
            if(not props->chaining){
                ImGui::PopItemFlag();
                ImGui::PopStyleVar();                   
            }               
        }
               
        ImGui::Separator();
        
        // edge contours group
        ImGui::SetNextTreeNodeOpen(false, ImGuiCond_Once);         
        if (ImGui::CollapsingHeader("Edge contours", ImGuiTreeNodeFlags_DefaultOpen)) {
        
            ImGui::PushStyleColor(ImGuiCol_CheckMark, (ImU32)ImColor(227, 60, 30));        
            ImGui::Checkbox("Boundary contours", &(props->boundaries));
            ImGui::Checkbox("Occluding contours", &(props->occlusive));
            ImGui::PopStyleColor();
            
            if(not props->occlusive){
                ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
                ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);             
            }
            ImGui::Checkbox("Use interpolated", &(props->use_smooth));
            if(not props->occlusive){
                ImGui::PopItemFlag();
                ImGui::PopStyleVar();                   
            }         
            
            ImGui::PushStyleColor(ImGuiCol_CheckMark, (ImU32)ImColor(227, 60, 30));        
            ImGui::Checkbox("Creases", &(props->sharp));
            ImGui::PopStyleColor();            
            
            if(not props->sharp){
                ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
                ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);             
            }
            const double amin2 = 0.0; const double amax2 = 180.0;
            ImGui::SliderScalar("Dihedral angle", ImGuiDataType_Double, &(props->sharp_angle), &amin2, &amax2, "%.1f deg");
            if(not props->sharp){
                ImGui::PopItemFlag();
                ImGui::PopStyleVar();                   
            }             
        }
        
        const double vis_min = -1; const double vis_max = 1;           
        
        ImGui::Separator();
        
        ImGui::SetNextTreeNodeOpen(false, ImGuiCond_Once);         
        if (ImGui::CollapsingHeader("Face contours", ImGuiTreeNodeFlags_DefaultOpen)) {

            ImGui::PushID("Suggestive");
            ImGui::PushStyleColor(ImGuiCol_CheckMark, (ImU32)ImColor(227, 60, 30));        
            ImGui::Checkbox("Suggestive contours", &(props->suggestive));
            ImGui::PopStyleColor();        
            ImGui::SameLine();
    //        ImGui::SetNextTreeNodeOpen(props->suggestive); 
            if(not props->suggestive)
                ImGui::SetNextTreeNodeOpen(false); 
            if (ImGui::CollapsingHeader("+", ImGuiTreeNodeFlags_DefaultOpen)) {      
                const double speedmin = 0.0; const double speedmax = 1.0;
                ImGui::SliderScalar("Speed", ImGuiDataType_Double, &(props->suggestive_speed), &speedmin, &speedmax, "%.2f");

                ImGui::Checkbox("Hysteresis filtering", &(props->suggestive_hysteresis));
                if(not props->suggestive_hysteresis){
                    ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
                    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);             
                }
                    const double sugg_thr_min = -1; const double sugg_thr_max = 1;   
                    const double sugg_tol_min = 0; const double sugg_tol_max = 2;
                    ImGui::SliderScalar("Thr", ImGuiDataType_Double, &(props->suggestive_threshold), &sugg_thr_min, &sugg_thr_max, "%.2f");
                    ImGui::SliderScalar("Tol", ImGuiDataType_Double, &(props->suggestive_tolerance), &sugg_tol_min, &sugg_tol_max, "%.2f");
                if(not props->suggestive_hysteresis){
                    ImGui::PopItemFlag();
                    ImGui::PopStyleVar();                   
                }            

                ImGui::Checkbox("Visibility filtering", &(props->suggestive_filtering));
                if(not props->suggestive_filtering){
                    ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
                    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);             
                }            
                    ImGui::SliderScalar("max", ImGuiDataType_Double, &(props->suggestive_max_visibility), &vis_min, &vis_max, "%.2f");
                    ImGui::SliderScalar("min", ImGuiDataType_Double, &(props->suggestive_min_visibility), &vis_min, &vis_max, "%.2f");                    
                if(not props->suggestive_filtering){
                    ImGui::PopItemFlag();
                    ImGui::PopStyleVar();                   
                }                  
            }
            ImGui::PopID();

            ImGui::Separator();        
            ImGui::PushID("Demarcating");
            ImGui::PushStyleColor(ImGuiCol_CheckMark, (ImU32)ImColor(227, 60, 30));        
            ImGui::Checkbox("Demarcating Curves", &(props->demarcating));
            ImGui::PopStyleColor();
            ImGui::SameLine();
    //        ImGui::SetNextTreeNodeOpen(props->demarcating);            
            if(not props->demarcating)
                ImGui::SetNextTreeNodeOpen(false); 
            if (ImGui::CollapsingHeader("+", ImGuiTreeNodeFlags_DefaultOpen)) {

                ImGui::Checkbox("Hysteresis filtering", &(props->demarcating_hysteresis));
                if(not props->demarcating_hysteresis){
                    ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
                    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);             
                }
                const double demarcating_thr_min = -1; const double demarcating_thr_max = 1;   
                const double demarcating_tol_min = 0; const double demarcating_tol_max = 2;                           
                ImGui::SliderScalar("Thr", ImGuiDataType_Double, &(props->suggestive_threshold), &demarcating_thr_min, &demarcating_thr_max, "%.2f");
                ImGui::SliderScalar("Tol", ImGuiDataType_Double, &(props->suggestive_tolerance), &demarcating_tol_min, &demarcating_tol_max, "%.2f");
                if(not props->demarcating_hysteresis){
                    ImGui::PopItemFlag();
                    ImGui::PopStyleVar();                   
                }            

                ImGui::Checkbox("Visibility filtering", &(props->demarcating_filtering));
                if(not props->demarcating_filtering){
                    ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
                    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);             
                }            
                ImGui::SliderScalar("max", ImGuiDataType_Double, &(props->demarcating_max_visibility), &vis_min, &vis_max, "%.2f");
                ImGui::SliderScalar("min", ImGuiDataType_Double, &(props->demarcating_min_visibility), &vis_min, &vis_max, "%.2f");                
                if(not props->demarcating_filtering){
                    ImGui::PopItemFlag();
                    ImGui::PopStyleVar();                   
                }             
            }
            ImGui::PopID();   

            ImGui::Separator();        
            ImGui::PushID("ridges");
            ImGui::PushStyleColor(ImGuiCol_CheckMark, (ImU32)ImColor(227, 60, 30));        
            ImGui::Checkbox("Ridges", &(props->ridges));
            ImGui::PopStyleColor();
            ImGui::SameLine();
    //        ImGui::SetNextTreeNodeOpen(props->ridges);   
            if(not props->ridges)
                ImGui::SetNextTreeNodeOpen(false);         
            if (ImGui::CollapsingHeader("+", ImGuiTreeNodeFlags_DefaultOpen)) {

                ImGui::Checkbox("Hysteresis filtering", &(props->ridges_hysteresis));
                if(not props->ridges_hysteresis){
                    ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
                    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);             
                }
                const double ridges_thr_min = -10; const double ridges_thr_max = 100;   
                const double ridges_tol_min = 0; const double ridges_tol_max = 20;                           
                ImGui::SliderScalar("Thr", ImGuiDataType_Double, &(props->ridges_threshold), &ridges_thr_min, &ridges_thr_max, "%.2f");
                ImGui::SliderScalar("Tol", ImGuiDataType_Double, &(props->ridges_tolerance), &ridges_tol_min, &ridges_tol_max, "%.2f");
                if(not props->ridges or not props->ridges_hysteresis){
                    ImGui::PopItemFlag();
                    ImGui::PopStyleVar();                   
                }            

                ImGui::Checkbox("Visibility filtering", &(props->ridges_filtering));
                if(not props->ridges_filtering){
                    ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
                    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);             
                }            
                ImGui::SliderScalar("max", ImGuiDataType_Double, &(props->ridges_max_visibility), &vis_min, &vis_max, "%.2f");
                ImGui::SliderScalar("min", ImGuiDataType_Double, &(props->ridges_min_visibility), &vis_min, &vis_max, "%.2f");                
                if(not props->ridges_filtering){
                    ImGui::PopItemFlag();
                    ImGui::PopStyleVar();                   
                }                  
            }
            ImGui::PopID();

            ImGui::Separator();
            ImGui::PushID("valleys");
            ImGui::PushStyleColor(ImGuiCol_CheckMark, (ImU32)ImColor(227, 60, 30));        
            ImGui::Checkbox("Valleys", &(props->valleys));
            ImGui::PopStyleColor();
            ImGui::SameLine();
    //        ImGui::SetNextTreeNodeOpen(props->valleys);      
            if(not props->valleys)
                ImGui::SetNextTreeNodeOpen(false);         
            if (ImGui::CollapsingHeader("+", ImGuiTreeNodeFlags_DefaultOpen)) {

                ImGui::Checkbox("Hysteresis filtering", &(props->valleys_hysteresis));
                if(not props->valleys_hysteresis){
                    ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
                    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);             
                }
                const double valleys_thr_min = -10; const double valleys_thr_max = 100;
                const double valleys_tol_min = 0; const double valleys_tol_max = 20;           
                ImGui::SliderScalar("Thr", ImGuiDataType_Double, &(props->valleys_threshold), &valleys_thr_min, &valleys_thr_max, "%.2f");
                ImGui::SliderScalar("Tol", ImGuiDataType_Double, &(props->valleys_tolerance), &valleys_tol_min, &valleys_tol_max, "%.2f");
                if(not props->valleys_hysteresis){
                    ImGui::PopItemFlag();
                    ImGui::PopStyleVar();                   
                }            

                ImGui::Checkbox("Visibility filtering", &(props->valleys_filtering));
                if(not props->valleys_filtering){
                    ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
                    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);             
                }            
                ImGui::SliderScalar("max", ImGuiDataType_Double, &(props->valleys_max_visibility), &vis_min, &vis_max, "%.2f");
                ImGui::SliderScalar("min", ImGuiDataType_Double, &(props->valleys_min_visibility), &vis_min, &vis_max, "%.2f");                
                if(not props->valleys_filtering){
                    ImGui::PopItemFlag();
                    ImGui::PopStyleVar();                   
                }                 
            }
            ImGui::PopID();       
        }
        
        
        static bool op = false;
        static float menu_width = 300.0f;
        int width_window, height_window;
        glfwGetWindowSize(viewer.window, &width_window, &height_window);    
        ImGui::SetNextWindowPos(ImVec2(width_window - menu_width, 0.0f), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(0.0f, 0.0f), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSizeConstraints(ImVec2(menu_width, -1.0f), ImVec2(menu_width, -1.0f)); 
        ImGui::SetNextWindowCollapsed(true, ImGuiCond_Once);                 
        ImGui::Begin("Strokes", &op, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize);
//        if (ImGui::CollapsingHeader("Colors", ImGuiTreeNodeFlags_DefaultOpen)) { 
        ImGui::PushItemWidth(menu_width/2.0);
            if(props->boundaries){
                ImGui::InputDouble("Boundaries width", &(props->width_boundaries), 0, 0, "%.1f");
                ImGui::InputDouble("Boundaries opacity", &(props->opacity_boundaries), 0, 0, "%.1f");
                if(ImGui::ColorPicker3("Boundaries color", bou_color)){
                    props->color_boundaries = Vector3d(bou_color[0], bou_color[1], bou_color[2]);
                    redraw(true);
                }
            }        
            if(props->occlusive){
                ImGui::Separator();
                ImGui::InputDouble("Occluding width", &(props->width_occlusive), 0, 0, "%.1f");
                ImGui::InputDouble("Occluding opacity", &(props->opacity_boundaries), 0, 0, "%.1f");
                if(ImGui::ColorPicker3("Occluding color", occ_color)){
                    props->color_occlusive = Vector3d(occ_color[0], occ_color[1], occ_color[2]);
                    redraw(true);
                }
            }
            if(props->sharp){
                ImGui::Separator();
                ImGui::InputDouble("Crease width", &(props->width_sharp), 0, 0, "%.1f");
                ImGui::InputDouble("Crease opacity", &(props->opacity_sharp), 0, 0, "%.1f");
               if(ImGui::ColorPicker3("Crease color", sha_color)){
                    props->color_sharp = Vector3d(sha_color[0], sha_color[1], sha_color[2]);
                    redraw(true);
                }                
            }            
            if(props->suggestive){
                ImGui::Separator();
                ImGui::InputDouble("Suggestive width", &(props->width_suggestive), 0, 0, "%.1f");
                ImGui::InputDouble("Suggestive opacity", &(props->opacity_suggestive), 0, 0, "%.1f");
                if(ImGui::ColorPicker3("Suggestive color", sug_color)){
                    props->color_suggestive = Vector3d(sug_color[0], sug_color[1], sug_color[2]);
                    redraw(true);
                }
            }
            if(props->demarcating){
                ImGui::Separator();
                ImGui::InputDouble("Demarcating width", &(props->width_demarcating), 0, 0, "%.1f");
                ImGui::InputDouble("Demarcating opacity", &(props->opacity_demarcating), 0, 0, "%.1f");
                if(ImGui::ColorPicker3("Demarcating color", dem_color)){
                    props->color_demarcating = Vector3d(dem_color[0], dem_color[1], dem_color[2]);
                    redraw(true);
                }
            }            
            if(props->ridges){
                ImGui::Separator();
                ImGui::InputDouble("Ridges width", &(props->width_ridges), 0, 0, "%.1f");
                ImGui::InputDouble("Ridges opacity", &(props->opacity_ridges), 0, 0, "%.1f");
                if(ImGui::ColorPicker3("Ridges color", rid_color)){
                    props->color_ridges = Vector3d(rid_color[0], rid_color[1], rid_color[2]);
                    redraw(true);
                }
            }
            if(props->valleys){
                ImGui::Separator();
                ImGui::InputDouble("Valleys width", &(props->width_valleys), 0, 0, "%.1f");
                ImGui::InputDouble("Valleys opacity", &(props->opacity_valleys), 0, 0, "%.1f");
                if(ImGui::ColorPicker3("Valleys color", val_color)){
                    props->color_valleys = Vector3d(val_color[0], val_color[1], val_color[2]);
                    redraw(true);
                }
            }
            ImGui::Separator();
            if(ImGui::ColorPicker3("Hidden lines color", hid_color)){
                    props->color_hidden = Vector3d(hid_color[0], hid_color[1], hid_color[2]);
                    redraw(true);
             }                
//        }  
        ImGui::PopItemWidth();
        ImGui::End();

    };
}
