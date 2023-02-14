#ifndef DRAWABLE_H
#define DRAWABLE_H

#include "../Lib/simple_svg.hpp"
#include <igl/opengl/glfw/Viewer.h>

#include <Eigen/Core>

/**\file Drawable.h
 * \authour Bastien Wailly <bastien.wailly@inria.fr>, Inria
 * \brief Drawable structures + Drawable interface*/

/**\enum ContourType 
 * \brief All contour types used in this executable
 * \details For more details, please check ContourExtractor sub classes*/
enum class ContourType { 
    BOUNDARY,   /*!<Boundary contour, for non closed meshes */
    OCCLUSION,  /*!<Occlusion contours, where visibility changes */ 
    SHARP,      /*!<Sharp contours, given dihedral angle */
    SUGGESTIVE, /*!<Suggestive contours, using radial/gaussian curvature */
    RIDGES,     /*!<Ridges contours, using extremalities */
    VALLEYS,    /*!<Valley contours, using extremalities */
    DEMARCATING /*!<Demarcating curves, using curvature derivative */
};

/**\enum VertexType 
 *  \brief All types a vertex can have
 *  \details for more information, please check https://arxiv.org/pdf/1810.01175.pdf#section.4.3*/
enum class VertexType { 
    V_JUNCTION,     /*!<Regular joint vertex */    
    CUSP,           /*!<Curtain folds singularity */
    BOUNDARY_CUSP,  /*!<Boundary curtain folds singularity */ 
    Y_JUNCTION,     /*!<Surface intersection */
    T_JUNCTION,     /*!<Image space intersection singularity */
    X_JUNCTION      /*!<Bifurcation singularity */
};

/**\enum Visibility 
 *  \brief Visibility status*/
enum class Visibility { 
    VISIBLE,    /*!<Visible by camera */
    OCCLUDED,   /*!<Not visible by camera */
    UNKNOWN     /*!<Visibility to be determined */
} ;

/**\enum Side 
 *  \brief Binary side enum
 *  \details Used to disambiguate the two vertices of a contour segment, and their corresponding side*/
enum class Side { HEAD /*!<front side*/, TAIL /*!<back side*/};

/**\struct _lines
 * \brief Structure that holds data relative to lines to be drawn in the viewer
 * \details For a given line index i, (P1(i), P2(i)) is the segment to draw, color(i) is the color of the segment*/
struct _lines{
    
    Eigen::MatrixXd P1;     /*!<all first points of line segments*/   
    Eigen::MatrixXd P2;     /*!<all second points of line segments*/      
    Eigen::MatrixXd color;  /*!<all colors of line segments*/   
    
    _lines(){};
};

/**\typedef Lines*/
typedef _lines Lines;

/**\brief Drawable component behaviour
 * \author Bastien Wailly <bastien.wailly@inria.fr>, Inria
 * \details Abstract class for objects that are being drawn in an SVG or in the interactive Viewer.*/
class Drawable{
public:
    virtual ~Drawable(){};
    /**\brief Draw an element in an SVG document.
     * \param doc SVG document
     * \param str Stroke to use to draw the element*/ 
    virtual const void Draw(svg::Document& doc, svg::Stroke& str) = 0;
    
    /**\brief Prepare the lines to draw in the viewer
     * \param lines line matrices to append to
     * \param color color to draw the chain*/  
    virtual const void Draw(Lines& lines, Eigen::RowVector3d color) = 0; 
    
    /**\brief Work for "load mesh" button.*/ 
    virtual const bool isVisible() = 0;
    
    static const std::string typeToString(ContourType t){
        switch(t){
            case ContourType::BOUNDARY:
                return "boundary";
            case ContourType::DEMARCATING:
                return "demarcating";
            case ContourType::OCCLUSION:
                return "occlusion";
            case ContourType::RIDGES:
                return "ridge";
            case ContourType::VALLEYS:
                return "valley";
            case ContourType::SHARP:
                return "sharp";
            case ContourType::SUGGESTIVE:
                return "suggestive";
        }
    }
};

#endif
