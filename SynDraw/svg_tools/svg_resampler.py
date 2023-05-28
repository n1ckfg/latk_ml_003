import numpy as np 
import argparse
import sys
from xml.dom import minidom


def norm(p):
    return np.linalg.norm(p)

def resamplePolyline(data, p, a, b):
    prev = np.array(data[0])

    new_data = [prev.tolist()]

    i = 1
    while i < len(data):
        p1 = prev
        p2 = np.array(data[i])

        u = p2 - p1
        n = norm(u)
        if n > 0:
            u = u/n
        else:
            u = np.array([0,0,0])
        l = n/p

        # print l

        if l < a:
            while l < a and i < len(data) - 1:
                # print "  deleting next point"
                i = i + 1
                p2 = np.array(data[i])
                u = p2 - p1
                n = norm(u)
                if n > 0:
                    u = u/n
                else:
                    u = np.array([0,0,0])
                l = n/p
                # print "   new l : ", l

        if l > b:
            t = l//b
            d = n / (t+1)   
            pt = p1         
            # print "  need to add %s pts"% t
            while t > 0:
                # print "  adding point"
                pt = pt + d * u
                new_data.append([pt[0], pt[1]])
                t = t - 1

        new_data.append([p2[0], p2[1]])
        prev = p2

        # print
        i = i + 1

    return new_data

def getPolylineData(p):
    poly = {}

    points = p.getAttribute('points').split()
    points = map(lambda p: tuple(map(float, p.split(','))), points)
    point_list = [[x[0],x[1]] for x in points]

    return point_list    

def flatten(l):
    return [item for sublist in l for item in sublist]    

def parseAndResample(in_node, out_parent, out_svg, params):
    if in_node.nodeType != 1: return

    p, a, b = params

    # print in_node.nodeName, in_node.nodeType, in_node.nodeValue
    if in_node.nodeName == 'polyline':
        out_node = out_svg.createElement(in_node.nodeName)
        # debug_node = out_svg.createElement(in_node.nodeName)

        data = getPolylineData(in_node)
        new_data = resamplePolyline(data, p, a, b)

        for a in in_node.attributes.keys():
            if a == 'points':
                out_node.setAttribute(a, ''.join("%0.3f,%0.3f"%(x[0],x[1])+' ' for x in new_data))
                # debug_node.setAttribute(a, ''.join("%0.3f"%x+' ' for x in flatten(data)))  
            # elif a == 'stroke-width':
            #     out_node.setAttribute(a, '0.4')  
            #     debug_node.setAttribute(a, '0.5')  
            else:
                out_node.setAttribute(a, in_node.getAttribute(a))  
                # debug_node.setAttribute(a, in_node.getAttribute(a))  

        # debug_node.setAttribute('stroke', 'yellow')
        # out_node.setAttribute('stroke', 'red')
        # out_parent.appendChild(debug_node)

        out_parent.appendChild(out_node)
    else:
        out_node = out_svg.createElement(in_node.nodeName)
        for a in in_node.attributes.keys():
            out_node.setAttribute(a, in_node.getAttribute(a))
        out_parent.appendChild(out_node)

    for child in in_node.childNodes:
        parseAndResample(child, out_node, out_svg, params)

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("svg_in", help="svg input file")
    parser.add_argument("svg_out", help="png output file")
    parser.add_argument("p", help="interpoint distance parameter", type=float)
    parser.add_argument("min", help="min length of a line", type=float, default=0.75)
    parser.add_argument("max", help="max length of a line", type=float, default=1.25)

    args = parser.parse_args()
    
    svg_in = minidom.parse(args.svg_in)
    svg_out = minidom.Document()

    parseAndResample(svg_in.documentElement, svg_out, svg_out, [args.p, args.min, args.max])

    out_file = open(args.svg_out, 'w')
    out_file.write(svg_out.toprettyxml())
    out_file.close()
    

if __name__ == '__main__':
    main()

