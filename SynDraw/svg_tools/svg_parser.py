#!/usr/bin/env python
import sys
import re
import numpy as np
from PIL import Image
import svgwrite
import random 
import argparse
import uuid
from math import pi, cos, sin
from cairosvg import svg2png
from xml.dom import minidom
import json

def getPolylineData(p):
    poly = {}

    points = p.getAttribute('points').split()
    points = map(lambda p: tuple(map(float, p.split(','))), points)
    point_list = [[x[0],x[1]] for x in points]
    poly['points'] = point_list

    center = [0,0]
    for x in point_list: center=np.sum([center,x], axis=0)
    poly['center'] = np.array(center)/len(point_list)

    return poly


def getPathData(p):
    path = {}
    mx,my = 0,0
    d_string = p.getAttribute('d').replace(',', ' ').split()

    d = []
    center_points = []
    while(d_string):
        if(d_string[0].isalpha()):
            c = d_string.pop(0)
            if c.upper() == 'A':
                print " path command 'A' is not supported"
                del d_string[:7]
            elif c.upper() == 'H' or c.upper() == 'V':
                d.append('L')
            else:
                d.append(c.upper())

        # cubic bezier curves                
        if c.upper() == 'C':
            cx1 = float(d_string.pop(0))
            cy1 = float(d_string.pop(0))
            cx2 = float(d_string.pop(0))
            cy2 = float(d_string.pop(0))                
            x = float(d_string.pop(0))
            y = float(d_string.pop(0))

            if c.islower():
                x += mx
                y += my
                cx1 += mx
                cy1 += my
                cx2 += mx
                cy2 += my

            mx, my = x, y
            prev_cc = [cx2, cy2]
            center_points.append([x,y])
            center_points.append([cx1,cy1])
            center_points.append([cx2,cy2])
            d.append([cx1,cy1])
            d.append([cx2,cy2])
            d.append([x,y])

        # cubic bezier curves with inferred control point
        elif c.upper() == 'S':
            cx2 = float(d_string.pop(0))
            cy2 = float(d_string.pop(0))              
            x = float(d_string.pop(0))
            y = float(d_string.pop(0))

            if c.islower():
                cx2 += mx
                cy2 += my
                x += mx
                y += my                

            mx, my = x, y
            cc = [2*x + prev_cc[0], 2*y + prev_cc[1]]
            prev_cc = cc
            center_points.append([cx2,cy2])         
            center_points.append([x,y])
            center_points.append(cc)
            d.append([cx2, cy2])
            d.append([x,y])        

        # quadratic bezier curves
        elif c.upper() == 'Q':
            cx = float(d_string.pop(0))
            cy = float(d_string.pop(0))
            x = float(d_string.pop(0))
            y = float(d_string.pop(0))

            if c.islower():
                x += mx
                y += my
                cx += mx
                cy += my

            mx, my = x, y
            prev_qc = [cx, cy]
            center_points.append([x,y])
            center_points.append([cx,cy])
            d.append([cx,cy])
            d.append([x,y])

        # quadratic bezier curves with inferred control point
        elif c.upper() == 'T':
            x = float(d_string.pop(0))
            y = float(d_string.pop(0))

            if c.islower():
                x += mx
                y += my                

            mx, my = x, y
            qc = [2*x + prev_qc[0], 2*y + prev_qc[1]]
            prev_qc = qc
            center_points.append([x,y])
            center_points.append(qc)
            d.append([x,y])

        # line to / move to
        elif c.upper() == 'L' or c.upper() == 'M':
            x = float(d_string.pop(0))
            y = float(d_string.pop(0))

            if c.islower():
                x += mx
                y += my                

            mx, my = x, y
            center_points.append([x,y])
            d.append([x,y])

        # horizontal line to
        elif c.upper() == 'H':
            x = float(d_string.pop(0))
            y = 0

            if c.islower():
                x += mx
                y += my

            mx = x
            center_points.append([x,my])
            d.append([x,y])

        # vertical line to
        elif c.upper() == 'V':
            x = 0
            y = float(d_string.pop(0))

            if c.islower():
                x += mx
                y += my

            my = y
            center_points.append([mx,y])
            d.append([x,y])               

    path['center'] = np.mean(np.array(center_points), axis=0)
    path['d'] = d

    return path
    

def to_rgb(im):
    print im.shape
    w, h = im.shape
    ret = np.empty((w, h, 3), dtype=np.uint8)
    ret[:, :, 0] = im
    ret[:, :, 1] = ret[:, :, 2] = ret[:, :, 0]
    return ret


def m_rot(a):
    return np.matrix([[cos(a), -sin(a), 0], 
                      [sin(a), cos(a),  0],
                      [0,      0,       1]])

def m_trans(x, y):
    return np.matrix([[1, 0, x], 
                      [0, 1, y],
                      [0, 0, 1]])

def m_scale(sx, sy):
    return np.matrix([[sx, 0, 0], 
                      [0, sy, 0],
                      [0, 0,  1]])

def disturbPath(path, noise=0):
    d = []
    for v in path['d']:
        # do stuff
        d.append(v)
    return d

def disturbPoly(poly, noise=0):
    points = []
    for v in poly['points']:
        # do stuff
        points.append(v)
    return points

def flatten(l):
    return [item for sublist in l for item in sublist]

def parseAndDisturb(in_node, out_parent, out_svg, noise=0):
    if in_node.nodeType != 1: return

    out_node = out_svg.createElement(in_node.nodeName)

    if in_node.nodeName == 'path':
        data = getPathData(in_node)
        disturbed_data = disturbPath(data, noise)
        for a in in_node.attributes.keys():
            if a == 'd':
                out_node.setAttribute(a, ''.join(str(x)+' ' for x in flatten(disturbed_data)))
            else:
                out_node.setAttribute(a, in_node.getAttribute(a))
    elif in_node.nodeName == 'polyline':
        data = getPolylineData(in_node)
        disturbed_data = disturbPoly(data, noise)

        for a in in_node.attributes.keys():
            if a == 'points':
                out_node.setAttribute(a, ''.join(str(x)+' ' for x in flatten(disturbed_data)))
            else:
                out_node.setAttribute(a, in_node.getAttribute(a))        
    else:
        for a in in_node.attributes.keys():
            out_node.setAttribute(a, in_node.getAttribute(a))

    for child in in_node.childNodes:
        parseAndDisturb(child, out_node, out_svg, noise=0)

    out_parent.appendChild(out_node)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("svg_in", help="svg input file")
    parser.add_argument("svg_out", help="svg output file")    

    a = parser.parse_args()
    svg_in = minidom.parse(a.svg_in)
    svg_out = minidom.Document()
    parseAndDisturb(svg_in.documentElement, svg_out, svg_out)

    file = open(a.svg_out, 'w')
    file.write(svg_out.toprettyxml())
    file.close()

if __name__ == '__main__':
    main()


