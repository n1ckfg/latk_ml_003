#include "ContourExtractor.h"


ContourExtractor::~ContourExtractor() {
//    for(int i=0; i<contours.size(); i++){
//        delete contours[i];
//    }
//    contours.clear();
}

ContourExtractor::ContourExtractor(MeshObject& obj):mesh(obj){
//    mesh = obj;
}