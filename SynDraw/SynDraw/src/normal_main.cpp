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
    
    int mode;
    float bg;
    
    if(!input.cmdOptionExists("-p"))
        throw("Error: you must specify a properties file with -p");
    
    if(!input.cmdOptionExists("-o"))
        throw("Error: you must specify an output png image with -o");  
    
    if(!input.cmdOptionExists("-m"))
        mode = 1;
    else
        mode = stoi(input.getCmdOption("-m"));
    
    if(!input.cmdOptionExists("-b"))
        bg = 1;
    else
        bg = stod(input.getCmdOption("-b"));
    
    // IO
    shared_ptr<Properties> props = make_shared<Properties>();
    std::cout << "Reading properties..." << std::endl; 
    props->load_from_file(input.getCmdOption("-p"));
    const std::string &path_to_img = input.getCmdOption("-o");  

    // viewer
    MyViewer viewer;  
    MatrixXd N, V;    
    MatrixXi F;
    
    // camera
    ViewCamera camera;
    camera.setFromProperties(*props);
    camera.invert_proj();
    
    viewer.launch_init(true, false);    
    
    // set viewer camera
    igl::read_triangle_mesh(props->mesh_in, V, F); 
    viewer.data().set_mesh(V, F);  
    viewer.data().show_lines = false;    
    viewer.core().view = camera.get_view();
    viewer.core().proj = camera.get_proj();
    viewer.core().viewport = camera.get_viewport(); 
    viewer.core().background_color << 0.5, 0.5, 0.5, bg;  
    
    if(camera.get_projType() == ProjectionType::ORTHOGRAPHIC)
        viewer.core().orthographic = true;
    else
        viewer.core().orthographic = false;
    
    // normals
    if(mode == 1){
        igl::per_vertex_normals(V,F,N);
        viewer.data().set_normals(N);
    }
    else if(mode == 2){
        igl::per_face_normals(V,F,N);
        viewer.data().set_normals(N);
    }     
    
    // set shader
    viewer.data().updateGL(viewer.data(),viewer.data().invert_normals,viewer.data().meshgl);    
    igl::opengl::destroy_shader_program(viewer.data().meshgl.shader_mesh);

    {
        std::string mesh_vertex_shader_string =
        R"(#version 150
        uniform mat4 view;
        uniform mat4 proj;
        in vec3 position;
        in vec3 normal;
                
        out vec3 normal_eye;

        void main()
        {
            gl_Position = proj * view * vec4 (position, 1.0);
                
            normal_eye = vec3 (view * vec4 (normal, 0.0));
            normal_eye = normalize(normal_eye);                
        })";

        std::string mesh_fragment_shader_string =
        R"(#version 150
        in vec3 normal_eye;
        out vec4 outColor;

        void main()
        {
            outColor = vec4(normal_eye/2.0+0.5, 1.0);;
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

