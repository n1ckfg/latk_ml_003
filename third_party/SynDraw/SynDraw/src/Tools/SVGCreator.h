#ifndef SVGCREATOR_H
#define SVGCREATOR_H

#include <string>

#include "../Common/ViewCamera.h"
#include "../Lib/simple_svg.hpp"

#include <iostream>

using namespace svg;

/**\brief Static methods for SVG manipulation
 * \author Bastien Wailly <bastien.wailly@inria.fr>, Inria */
class SVGCreator{
public:
    
    /**\brief Create SVG file at specified path and return document
     * \param filename path to SVG
     * \param viewport SVG viewport dimensions
     * \param add_bg set to true to add white background in SVG
     * \return SVG Document instance */
    static Document Create_SVG(const std::string filename, Vector4f viewport, bool add_bg){
        Dimensions dimensions(viewport(2), viewport(3));
        Layout layout = Layout(dimensions, Layout::BottomLeft);
        Document doc = Document(filename, layout);

        if(add_bg){
            Rectangle rect(Point(viewport(0), viewport(1)), viewport(2), viewport(3), Fill(Color::White));
            doc << rect;
        }

        return doc;
    }

    /**\brief Draw a Drawable element in an SVG document
     * \param doc instantiated SVG Document 
     * \param line element to draw
     * \param color SVG stroke color attribute
     * \param line_width SVG stroke width attribute
     * \tparam Line Drawable element */
    template <typename Line>
    static const void Draw_Line(Document& doc, Line& line, Vector3d& color, float line_width, float line_opacity){
        Color c = Color(255*color(0), 255*color(1), 255*color(2));
        Stroke s = Stroke(line_width, line_opacity, c);        
        line->Draw(doc, s);
    }
    
    /**\brief Draw a Drawable element in an SVG document
     * \param doc instantiated SVG Document 
     * \param sing element to draw
     * \tparam Singularity Drawable element */  
    template <typename Singularity>    
    static const void Draw_Circle(Document& doc, Singularity& sing){
        Color c(0,0,0);
        Stroke str(0, 0, c);        
        sing->Draw(doc, str);
    }

    /**\brief Draw 3D axes in SVG document using a camera
     * \param svg instantiated SVG document
     * \param cam camera to use for projection */
    static const void Show_Frame(Document& svg, ViewCamera& cam) {
        Vector2f o = cam.project_point(Vector3f(0.0, 0.0, 0.0));
        Vector2f x = cam.project_point(Vector3f(1.0, 0.0, 0.0));
        Vector2f y = cam.project_point(Vector3f(0.0, 1.0, 0.0));
        Vector2f z = cam.project_point(Vector3f(0.0, 0.0, 1.0));

        svg << Line(Point(o(0), o(1)), Point(x(0), x(1)), Stroke(1.0, Color::Red));
        svg << Line(Point(o(0), o(1)), Point(y(0), y(1)), Stroke(1.0, Color::Green));
        svg << Line(Point(o(0), o(1)), Point(z(0), z(1)), Stroke(1.0, Color::Blue));
    }
};

#endif
