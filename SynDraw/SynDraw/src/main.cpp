#include <iostream>
#include "Tools/InputParser.h"
#include "Common/MeshDrawing.h"
#include "Tools/Properties.h"

using namespace std;

int main(int argc, char **argv){
    InputParser input(argc, argv);
    if(!input.cmdOptionExists("-p"))
        throw("Error: you must specify a properties file with -p");
    
    shared_ptr<Properties> props = make_shared<Properties>();
    std::cout << "Reading properties..." << std::endl; 
    props->load_from_file(input.getCmdOption("-p"));

    MeshDrawing drawing(props);
    drawing.load_mesh();
    drawing.update_camera_from_props();
    if(drawing.compute())
        drawing.create_svg_file(props->svg_out);
    
    return 0;
}
