#include <iostream>
#include <vector>
#include <cmath>
#include <limits>
#include <cstdlib>
#include <omp.h>
#include <stdio.h>
#include <chrono>
#include <fstream>
#include <string>
#include <sstream>
#include <algorithm>
#include <iostream>
#include <random>

using namespace std;
using namespace std::chrono;



/* 
    DEFINING Load and Save FUNCTIONS
*/


/*
    Reading data from a CSV file
*/
void load_CSV(string file_name, float** points, long long int size) {

    // Opening the CSV file
    ifstream in(file_name);

    // Checking if the file was sucessfully opened
    if (!in) {

        // Priting message of unsucessful open file
        cerr << "Couldn't read file: " << file_name << "\n";

        // Exit the function
        return;

    }

    // Initializing point_numer variable to keep track of the number of points read
    long long int point_number = 0;

    // Declaring line variable to store each line read from the file
    string line;

    // While loop for processing each line
    while (getline(in, line) && point_number < size) {

        // Using istringstream to parse the line string
        istringstream iss(line);

        // Declaring coord variable to store the floating-point values representing coordinates of a point
        float coord;

        // Declaring delimiter variable to temporarily hold the character that separates the coordinates in the file
        char delimiter;

        // While loop that is for extracting data from the iss variable
        while (iss >> coord >> delimiter) {

            // Assigning the value of coord to the fisrt coordinate (x-coordinate)
            points[point_number][0] = coord;

            // This line attempt to read the next value into coord again
            iss >> coord >> delimiter;

            // Assigning the value of coord to the second coordinate (y-coordinate)
            points[point_number][1] = coord;
        }

        // Incrementing the point number variable after processing each variable
        point_number++;
    }

    // Closing the file after reading all necessary data
    in.close();

}

/*
    Writing data to a CSV file
*/
void save_to_CSV(string file_name, float** points, long long int size) {
    ofstream fout(file_name);

    // Checking if the file was successfully opened for writing
    if (!fout) {

        // Priting message of unsucessful open file
        cerr << "Couldn't write to file: " << file_name << "\n";

        // Exit the function
        return;

    }

    // For loop to iterate over each in the points array
    for (long long int i = 0; i < size; i++) {

        // Writing the x-coordindate, the y-coordinate and the cluster id
        fout << points[i][0] << "," << points[i][1] << "," << points[i][2] << "\n";
    }

    // Closing the file
    fout.close();

}

/* 
    IMPLEMENTING K_MEANS 
*/

/** K-Means function
 *  Performs the k-means algorithm in parallel to cluster data points into groups.
 *  Array of data points where each row represents a point with "x", "y" coordinates and its cluster assignment
 *  @param points  
 *  Number of desired clusters
 *  @param num_clusters 
 *  Data set size (number of points).    
 *  @param size
 *  Maximum number of iterations allowed for the algorithm           
 *  @param max_iterations 
 */

void kmeans_paralelo(float** points, int num_clusters, long long int size, int max_iterations) {

    // Generating a uniformly-distributed integer random number
    std::random_device rd;

    // Generating a randon number based on the Mersenne Twister algorithm
    std::mt19937 gen(rd());
    
    // Generating a random integer within a specified range, here between o and cluster number -1
    std::uniform_int_distribution<int> distribution(0, num_clusters - 1);

    // OpenMP Directive:  instructs the compiler to parallelize the loop that follows. OpenMP automatically 
    // divides the loop's iterations among the available threads, allowing the loop to execute much faster on multicore 
    // processors
    #pragma omp parallel for
    //For loop to iterate over all data points
    for (long long int i = 0; i < size; i++) {

        // Assigning each points clustr assignment is determined by generating a random number
        points[i][2] = distribution(gen); 
                                          
    }

    // Converge auxiliar variable that acts as a flag indicating whether the algorithm has converged - meaning that further do
    // not siginifcatly change the outcome, usually defined as the cenroid of clusters no longer moving 
    bool converge = false; 

    // Integer auxuliar variable that counts the number if iterations the algorithm has performed.
    int cuenta = 0;

    // While loop that perfomr the iterative process of the algorithm       
    while (!converge && cuenta < max_iterations) {

        // Setting auxiliar converge variable on true 
        converge = true;

        // Incrementing to track the number of iterations the algorithm has performed
        cuenta++;

        // Dynamically allocating memory for centroids array, representing the coordinates of the centroids
        float** centroids = new float*[num_clusters]; 

        // Dynamically allocating memory for counts array, this is to keep track of the number of points assigned to each cluster
        int* counts = new int[num_clusters];          

        // Declaring a vector to store the indices of data points chosen as initial centroids
        std::vector<int> centroid_indices;

        // While loop to iterate that is going to continue as lonsg as the number of slected indices is less than the deisreed number of clusters. This
        // conditiion ensures that you select a unique index for each cluster
        while (centroid_indices.size() < num_clusters) {

            // Generating a random index using a uniform integer distributtion
            int index = distribution(gen); 

            // Checcking uniqueness
            if (std::find(centroid_indices.begin(), centroid_indices.end(), index) == centroid_indices.end()) {

                // Adding the unique index
                centroid_indices.push_back(index);

            }

        }

        // For loop to iterate over the numbr of clusters
        for (int i = 0; i < num_clusters; i++) {

            // For each cluster, this line isdynamically allocates memory for an array of two float values, which represent the x and y coordinates of the centroid
            centroids[i] = new float[2]{points[centroid_indices[i]][0], points[centroid_indices[i]][1]};

            // Initializing the count of data associated with the ith cluster to 0
            counts[i] = 0;

        }

        // OpenMP Directive:  instructs the compiler to parallelize the loop that follows. OpenMP automatically 
        // divides the loop's iterations among the available threads, allowing the loop to execute much faster on multicore 
        // processors
        #pragma omp parallel for
        // For loop iterating all over the points in the dataset
        for (long long int i = 0; i < size; i++) {
            
            // For each point, retrieves the cluster assignment from the third position of its array
            int cluster = (int)points[i][2];

            // OpenMP Directive: ensures that these additions are done atomically, preventing race conditions when multiple threads try to update the coordinates of the 
            // same centroid simultaneously. Atomic operations are critical in parallel computing to ensure data integrity when shared data is being updated.
            #pragma omp atomic
            // Updating centroid coordindates (x-coordinate)
            centroids[cluster][0] += points[i][0];  
            // OpenMP Directive: ensures that these additions are done atomically, preventing race conditions when multiple threads try to update the coordinates of the 
            // same centroid simultaneously. Atomic operations are critical in parallel computing to ensure data integrity when shared data is being updated. 
            #pragma omp atomic
            // Updating centroid coordindates (x-coordinate)
            centroids[cluster][1] += points[i][1];
            // OpenMP Directive: ensures that these additions are done atomically, preventing race conditions when multiple threads try to update the coordinates of the 
            // same centroid simultaneously. Atomic operations are critical in parallel computing to ensure data integrity when shared data is being updated.   
            #pragma omp atomic
            // Incrementing the count of points assigned to the cluster  
            counts[cluster]++;

        }

        // For loop to iterate over each cluster
        for (int i = 0; i < num_clusters; i++) {

            // If the current cluster has one or more assigned points (counts[i] > 0), the coordinates of its centroid are updated
            if (counts[i] > 0) {

                // Updating centroid coordindates (x-coordinate)
                centroids[i][0] /= counts[i];

                // Updating centroid coordindates (y-coordinate) 
                centroids[i][1] /= counts[i];

            }

        }

        // OpenMP Directive:  instructs the compiler to parallelize the loop that follows. OpenMP automatically 
        // divides the loop's iterations among the available threads, allowing the loop to execute much faster on multicore 
        // processors. The reduction(&& : converge) clause is used for the converge variable, a boolean flag indicating whether the 
        // algorithm has converged (i.e., if there were no changes in cluster assignments in this iteration)
        #pragma omp parallel for reduction(&& : converge)
        // For loop to iterate all over the points
        for (long long int i = 0; i < size; i++) {

            // Initializing min_distance with infity to ensure thata any actual distance calculated will be smaller, helping to find the
            // minimum distance to a centroid
            float min_distancia = INFINITY;

            // Initializing min_cluster -1, indicating that no cluster has been assigned yet
            int min_cluster = -1;

            // For loop iterating through the clusters
            for (int j = 0; j < num_clusters; j++) {

                // Calculating the difference in the x-coordinates between the ith and the jth centroid
                float distancia_x = points[i][0] - centroids[j][0];

                // Calculating the difference in the y-coordinates between the ith and the jth centroid
                float distancia_y = points[i][1] - centroids[j][1];

                // Calculating the euclidean distance
                float distancia = sqrt(std::pow(distancia_x, 2) + std::pow(distancia_y, 2));

                // Checking if the distance (distancia) from the current data point to the centroid being considered in this iteration is 
                // less than the smallest distance found so far (min_distancia)
                if (distancia < min_distancia) {

                    // Updating min_distance variable
                    min_distancia = distancia;

                    // Updating min_cluster to the index of this closer centroid
                    min_cluster = j;

                }

            }

            // Checking if the nearest cluster (min_cluster) identified for the i-th data point is different from the data point's current cluster 
            // assignment (points[i][2])
            if (min_cluster != points[i][2]) {

                // Updating cluster assignment
                points[i][2] = min_cluster;

                // Indicating non-convergence
                converge = false;

            }

        }

        // For loop to iterate over eacj¡h centroid
        for (int i = 0; i < num_clusters; i++) {

            // Deallocating memory for each array of centroids coordinates
            delete[] centroids[i];

        }

        // Deallocating the memory assigned fot the centroids array
        delete[] centroids;

        // Deallocating the memory assigned for the counts array
        delete[] counts;

    }

}

int main(int argc, char** argv) {
    if (argc < 4) {
        cerr << "Usage: " << argv[0] << " <data_file.csv> <num_clusters> <output_file.csv>\n";
        return 1;
    }

    const long long int size = atoll(argv[1]);
    const int num_clusters = atoi(argv[2]);
    const string input_file_name = argv[1];
    const string output_file_name_paralelo = argv[3];
    const int max_iterations = 20;

    int num_threads = omp_get_max_threads();
    if (argc >= 5) {
        num_threads = atoi(argv[4]);
    }
    omp_set_num_threads(num_threads);


    float** paralelo = new float*[size];
    for (long long int i = 0; i < size; i++) {
        paralelo[i] = new float[3]{0.0, 0.0, -1}; 
                                                  
    }


    load_CSV(input_file_name, paralelo, size);

    double start_paralelo = omp_get_wtime();
    kmeans_paralelo(paralelo, num_clusters, size, max_iterations);
    double tiempo_ejecucion_paralelo = omp_get_wtime() - start_paralelo;
    cout << "Tiempo de ejecución en paralelo: " << tiempo_ejecucion_paralelo << "\n";
    
    save_to_CSV(output_file_name_paralelo, paralelo, size);

    for (long long int i = 0; i < size; i++) {
        delete[] paralelo[i];
    }
    delete[] paralelo;

    return 0;
}