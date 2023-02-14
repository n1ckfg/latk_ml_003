#include <iostream>
#include "Tools/InputParser.h"
#include "Common/MeshDrawing.h"
#include "Tools/Properties.h"

#include <cstdlib>
#include <string>

using namespace Eigen;
using namespace std;

Vector3f random_unit_vector(){
    return Vector3f( static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX), 
                        static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX), 
                        static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX));
}

int main(int argc, char **argv){
    InputParser input(argc, argv);
    if(!input.cmdOptionExists("-p"))
        throw("Error: you must specify a properties file with -p");
    
    // read properties for initialization
    shared_ptr<Properties> props = make_shared<Properties>();
    std::cout << "Reading properties..." << std::endl; 
    props->load_from_file(input.getCmdOption("-p"));

    // create drawing object
    MeshDrawing drawing(props);
    drawing.load_mesh();
    drawing.update_camera_from_props();
    
    for(int i=0; i< 10; i++){
        // edit properties attributes directly
        props -> camera_position = random_unit_vector();
        
        // compute and save SVG
        if(drawing.compute(true)) 
            drawing.create_svg_file(std::to_string(i).append(".svg"));
    }
    
    return 0;
}
