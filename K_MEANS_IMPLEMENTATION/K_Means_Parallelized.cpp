#include <omp.h>
#include <iostream>
#include <string>
#include <fstream>
#include <cmath>

using namespace std;

/* 
    DEFINING SUPPORT FUNCTIONS
*/


/*
    Reading data from a CSV file
*/
void load_CSV(string file_name, float** points, long long int size) {

    // Creating a input file stream in and opening the specified file
    ifstream in(file_name);

    // Checking if the file strem in failed to open 
    if (!in) {

        // Printing error message: filoe could not be read
        cerr << "Couldn't read file: " << file_name << "\n";

    }

    // Initializing point_number counter auxiliar variable
    long long int point_number = 0; 

    // Settiung a while loop that will continue as along as the end of the file has been reached
    while (!in.eof() && (point_number < size)) {

        // Allocating memory. This will be used to read each line
        char* line = new char[12];

        // Defining row size axuliar variable 
        streamsize row_size = 12;

        // Reading and storing the characters
        in.read(line, row_size);

        // Construction of a string "row" from the character array "line"
        string row = line;

        // Checking that the read line is shorter that 12 characters
        if (row.size() < 12) {

            // Skiping the rest of the loop iteration, if the condition is true
            continue;

        }

        // Extracting substring of the fisrt 5 characters from row
        points[point_number][0] = stof(row.substr(0, 5));

        // Extracting substring of the from the 6th character, with a length of 5 characters
        points[point_number][1] = stof(row.substr(6, 5));

        // Increments our point_number counter auxiliar variable
        point_number++;

    }
    
}

