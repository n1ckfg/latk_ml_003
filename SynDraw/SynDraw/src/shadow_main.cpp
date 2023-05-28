#include <iostream>

#include <igl/read_triangle_mesh.h>
#include <igl/per_face_normals.h>
#include <igl/per_vertex_normals.h>
#include <igl/opengl/glfw/Viewer.h>
#include <igl/png/render_to_png.h>
#include <igl/igl_inline.h>
#include <igl/get_seconds.h>

#include "Tools/InputParser.h"
#include "Tools/Properties.h"
#include "Common/ViewCamera.h"

using namespace std;
using namespace Eigen;

// extending Viewer to : 
// - not recompute gl matrices but use the ones provided instead
// - save viewport to png and immediately quit
class MyViewer : public igl::opengl::glfw::Viewer
{
    public:
        void my_draw(){
            int width, height;
            glfwGetFramebufferSize(window, &width, &height);

            int width_window, height_window;
            glfwGetWindowSize(window, &width_window, &height_window);

            auto highdpi_tmp = (width_window == 0 ||  width == 0) ? highdpi : (width/width_window);

            if(fabs(highdpi_tmp-highdpi)>1e-8)
            {
              post_resize(width, height);
              highdpi=highdpi_tmp;
            }

            for (auto& core : core_list)
            {
              core.clear_framebuffers();
            }

            for (unsigned int i = 0; i<plugins.size(); ++i)
            {
              if (plugins[i]->pre_draw())
              {
                return;
              }
            }
            if (callback_pre_draw)
            {
              if (callback_pre_draw(*this))
              {
                return;
              }
            }

            for (auto& core : core_list)
            {
              for (auto& mesh : data_list)
              {
                if (mesh.is_visible & core.id)
                {
                  // only line that changed. don't update matrices please
                  core.draw(mesh, false);
                }
              }
            }
            for (unsigned int i = 0; i<plugins.size(); ++i)
            {
              if (plugins[i]->post_draw())
              {
                break;
              }
            }
            if (callback_post_draw)
            {
              if (callback_post_draw(*this))
              {
                return;
              }
            }    
        }
        
        bool my_launch_rendering(std::string path, float w, float h){
            // glfwMakeContextCurrent(window);
            // Rendering loop
            const int num_extra_frames = 5;
            int frame_counter = 0;
            while (!glfwWindowShouldClose(window))
            {
              double tic = igl::get_seconds();
              my_draw();
              glfwSwapBuffers(window);
              if(core().is_animating || frame_counter++ < num_extra_frames)
              {
                glfwPollEvents();
                // In microseconds
                double duration = 1000000.*(igl::get_seconds()-tic);
                const double min_duration = 1000000./core().animation_max_fps;
                if(duration<min_duration)
                {
                  std::this_thread::sleep_for(std::chrono::microseconds((int)(min_duration-duration)));
                }
              }
              else
              {
                glfwWaitEvents();
                frame_counter = 0;
              }
              // save to png and stop loop
              igl::png::render_to_png(path, w, h);
              
              return !glfwWindowShouldClose(window);

              #ifdef __APPLE__
                static bool first_time_hack  = true;
                if(first_time_hack) {
                  glfwHideWindow(window);
                  glfwShowWindow(window);
                  first_time_hack = false;
                }
              #endif
            }
            return EXIT_SUCCESS;    
        }
};


int main(int argc, char** argv) {
    InputParser input(argc, argv);
    
    if(!input.cmdOptionExists("-p"))
        throw("Error: you must specify a properties file with -p");
    
    if(!input.cmdOptionExists("-o"))
        throw("Error: you must specify an output png image with -o");  

    if(!input.cmdOptionExists("-a"))
        throw("Error: you must specify an alpha value with -a");        
    
    // IO
    shared_ptr<Properties> props = make_shared<Properties>();
    std::cout << "Reading properties..." << std::endl; 
    props->load_from_file(input.getCmdOption("-p"));
    const std::string &path_to_img = input.getCmdOption("-o");  
    float bg = stod(input.getCmdOption("-a"));   
    if(bg > 1) bg = 1;
    if(bg < 0) bg = 0; 

    // viewer
    MyViewer viewer;  
    MatrixXd N, V;    
    MatrixXi F;
    
    // camera
    ViewCamera camera;
    camera.setFromProperties(*props);
    camera.invert_proj();
    
    viewer.launch_init(true, false);    
    
    // compute shadow mesh (projection on ([0,0,0] [0,0,1]) plane)
    igl::read_triangle_mesh(props->mesh_in, V, F); 
    MatrixXd V_sh = V;
    for(int i = 0; i<V.rows(); i++){
        V_sh(i, 0) = V(i, 0);
        V_sh(i, 1) = V(i, 1);
        V_sh(i, 2) = 0;
        cout << V_sh.row(i) << endl;
    }
    
    MatrixXi F_sh = F;
    
    MatrixXd V2(V.rows()+V_sh.rows(), V.cols());
    V2 << V, V_sh;
    MatrixXi F2(F.rows()+F_sh.rows(), F.cols());
    F2 << F, (F.array()+V.rows());
    
    // compute colors
    Eigen::MatrixXd C(F2.rows(),3);
    if(V2.rows() == F2.rows()){
        C<< Eigen::RowVector3d(1.0,1.0,1.0).replicate(V.rows(),1),
            Eigen::RowVector3d(0.0,0.0,0.0).replicate(V_sh.rows(),1);        
    }
    else{
        C<< Eigen::RowVector3d(1.0,1.0,1.0).replicate(F.rows(),1),
            Eigen::RowVector3d(0.0,0.0,0.0).replicate(F_sh.rows(),1);        
    }
    
    // set camera
    viewer.data().show_lines = false;    
    viewer.core().view = camera.get_view();
    viewer.core().proj = camera.get_proj();
    viewer.core().viewport = camera.get_viewport(); 
    viewer.core().background_color << 1,1,1,bg;  
    
    if(camera.get_projType() == ProjectionType::ORTHOGRAPHIC)
        viewer.core().orthographic = true;
    else
        viewer.core().orthographic = false; 

    // set viewer content
    viewer.data().set_mesh(V2, F2);  
    viewer.data().set_colors(C);
    viewer.data().show_lines = false;    

    // set shader
    viewer.data().updateGL(viewer.data(),viewer.data().invert_normals,viewer.data().meshgl);    
    igl::opengl::destroy_shader_program(viewer.data().meshgl.shader_mesh);

    {
        std::string mesh_vertex_shader_string =
        R"(#version 150
        uniform mat4 view;
        uniform mat4 proj;
        in vec3 position;
        in vec4 Kd;
        out vec4 Kdi;

        void main()
        {
            gl_Position = proj * view * vec4 (position, 1.0);
            Kdi = Kd;
        })";

        std::string mesh_fragment_shader_string =
        R"(#version 150
        in vec4 Kdi;
        out vec4 outColor;

        void main()
        {
            outColor = Kdi;
        })";
        igl::opengl::create_shader_program(
            mesh_vertex_shader_string,
            mesh_fragment_shader_string,
            {},
            viewer.data().meshgl.shader_mesh
        );
    }
    // render
    viewer.my_launch_rendering(path_to_img, viewer.core().viewport(2), viewer.core().viewport(3));
    viewer.launch_shut();
        
    return (EXIT_SUCCESS);
}