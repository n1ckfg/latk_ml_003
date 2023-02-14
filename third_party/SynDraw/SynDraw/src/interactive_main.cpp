/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   interactive_main.c
 * Author: bwailly
 *
 * Created on June 12, 2019, 1:51 PM
 */

#include <Eigen/Core>
#include <iostream>
#include <string>
#include <chrono>

#include "LineViewer.h"

int main(int argc, char **argv) {
    LineViewer viewer;
    viewer.launch();

    return 0;
}