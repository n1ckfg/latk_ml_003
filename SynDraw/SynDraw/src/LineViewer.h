#ifndef LINEVIEWER_H
#define LINEVIEWER_H

#include <igl/opengl/glfw/Viewer.h>
#include <igl/opengl/glfw/imgui/ImGuiMenu.h>

#include "Common/MeshDrawing.h"
#include "Tools/Properties.h"

using namespace std;

/**
 * \brief Interactive viewer and drawing generator
 * \author Bastien Wailly <bastien.wailly@inria.fr>, Inria
 * \details Class that creates the libIGL viewer and its ImGui menu, and handles events.
 */
class LineViewer {
public:
    LineViewer();
    virtual ~LineViewer(){};

    /**\brief Launch interactive libIGL viewer.*/     
    void launch();
    
private:
    MeshDrawing drawing;
    std::shared_ptr<Properties> props;
    
    igl::opengl::glfw::imgui::ImGuiMenu menu;    
    igl::opengl::glfw::Viewer viewer;
       
    // flags
    bool camera_changed;
    bool mesh_loaded;
    bool to_recompute_all;
    bool to_recompute_chains;
    
    string save_path = "";
    string open_path = "";
    
    // lines colors
    float occ_color[3];
    float sug_color[3];
    float bou_color[3];
    float rid_color[3];
    float val_color[3];
    float dem_color[3];
    float sha_color[3];  
    float hid_color[3];
       
    /**\brief Reset all values to default.*/     
    void default_properties();
    
    /**\brief Create ImGui menu in viewer.
     * \details Callback function passed to viewer instance as callback_draw_viewer_menu.*/     
    void createMenu();
    
    /**\brief Work for "compute" button.*/      
    void button_compute();
    
    /**\brief Work for "load mesh" button.*/      
    void button_load_mesh();
    
    /**\brief Work for "save properties" button.*/      
    void button_save_props();
    
    /**\brief Work for "save svg" button.*/      
    void button_save_svg();
    
//    /**\brief Work for "load properties" button.
//     * \deprecated*/      
//    void button_load_props();
    
    /**\brief Redraw scene (geometry + lines).
     * \details This is the main drawing function of the viewer. It is called every time something changes.
     * \param recompute_lines flag to force lines recomputation before drawing.*/      
    void redraw(bool recompute_lines=false);
    
    /**\brief Draw axis helper in viewer.*/     
    void draw_axis();
    
    /**\brief Convert a vector to a float array.
     * \param v vector to convert.
     * \param val float array output.*/     
    void vectof(Vector3d v, float val[]);
    
    /**\brief Set all line colors to their default value.*/     
    void default_colors();
    
    /**\brief Opens a file dialog window and get a file open path.
     * \details Apple environment specific 
     * \param   filters     List of strings used to filter files in dialog window.
     * \return              Path to selected file*/     
    string load_window(std::vector<std::string> filters);   

    /**\brief Opens a file dialog window and get a file save path.
     * \details Apple environment specific 
     * \param   filters     List of strings used to filter files in dialog window.
     * \return              Path to file to save*/     
    string save_window(std::vector<std::string> filters);
};

#endif /* LINEVIEWER_H */

