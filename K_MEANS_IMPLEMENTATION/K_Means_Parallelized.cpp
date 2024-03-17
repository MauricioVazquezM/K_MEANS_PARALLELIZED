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

/** Función para el algoritmo de k-means
 *  Realiza el algoritmo de k-means de manera paralela para clusterizar puntos de datos en grupos.
 * 
 *  @param points         Matriz de puntos de datos donde cada fila representa un punto con coordenadas "x", "y" y su asignación de cluster.
 *  @param num_clusters     Número de clusters deseados.
 *  @param size           Tamaño del conjunto de datos (número de puntos).
 *  @param max_iterations Máximo número de iteraciones permitidas para el algoritmo.
 */
void kmeans_paralelo(float** points, int num_clusters, long long int size, int max_iterations) {
    // Generadores de numeros aleatorios
    std::random_device rd;
    std::mt19937 gen(rd());
    
    // Tomamos los numeros
    std::uniform_int_distribution<int> distribution(0, num_clusters - 1);

    // Distribución uniforme para la selección aleatoria de clusters iniciales
    // Aprovechamos para paralelizar el ciclo
    #pragma omp parallel for
    for (long long int i = 0; i < size; i++) {
        points[i][2] = distribution(gen); // En el indice 2 tenemos el cluster al
                                          // cual pertenece el punto.
    }

    // K-means termina de correr si se da una de las condiciones siguientes:
    bool converge = false; // Converge, i.e. los centroides ya no cambian de una iteración a otra
    int cuenta = 0;        // ó terminamos el número de iteraciones deseadas
    while (!converge && cuenta < max_iterations) {
        converge = true;
        cuenta++;

        // Inicializamos los centroides y los contadores de puntos
        float** centroids = new float*[num_clusters]; // Donde guardaremos los centroides iniciales
        int* counts = new int[num_clusters];          // Nos indica cuantos puntos hay en el cluster i
                                                      // con i = 0, 1,..., num_clusters

        // Selección aleatoria de clusters. Decidimos tomar los centroides
        // de manera aleatoria de puntos de datos, esto para que converja
        // más rápido y clusterice bien.
        std::vector<int> centroid_indices;
        while (centroid_indices.size() < num_clusters) {
            int index = distribution(gen); 
            if (std::find(centroid_indices.begin(), centroid_indices.end(), index) == centroid_indices.end()) {
                centroid_indices.push_back(index);
            }
        }

        // Uso de los puntos de datos seleccionados como centroides
        for (int i = 0; i < num_clusters; i++) {
            centroids[i] = new float[2]{points[centroid_indices[i]][0], points[centroid_indices[i]][1]};
            counts[i] = 0;
        }

        // Actualización de centroides y contadores basados en las asignaciones de clusters
        #pragma omp parallel for
        for (long long int i = 0; i < size; i++) {
            int cluster = (int)points[i][2];
            #pragma omp atomic
            centroids[cluster][0] += points[i][0];  
            #pragma omp atomic
            centroids[cluster][1] += points[i][1];  
            #pragma omp atomic
            counts[cluster]++;
        }

        // Cálculo de nuevos centroides, tomando el punto medio
        for (int i = 0; i < num_clusters; i++) {
            if (counts[i] > 0) {
                centroids[i][0] /= counts[i];
                centroids[i][1] /= counts[i];
            }
        }

        // Update cluster assignments based on new centroids
        #pragma omp parallel for reduction(&& : converge)
        for (long long int i = 0; i < size; i++) {
            float min_distancia = INFINITY;
            int min_cluster = -1;

            // Calculamos la distancia euclidiana de cada punto a cada centroide,
            // así asinamos cada punto al centroide más cercano.
            for (int j = 0; j < num_clusters; j++) {
                float distancia_x = points[i][0] - centroids[j][0];
                float distancia_y = points[i][1] - centroids[j][1];

                float distancia = sqrt(std::pow(distancia_x, 2) + std::pow(distancia_y, 2));

                if (distancia < min_distancia) {
                    min_distancia = distancia;
                    min_cluster = j;
                }
            }

            // Revisamos si los centroides cambiaron respecto a la iteración anterior,
            // Si cambiaron, actualizamos el cluster al que pertenece el punto.
            if (min_cluster != points[i][2]) {
                points[i][2] = min_cluster;
                converge = false;
            }
        }

        // Le damos flush a las variables temporales para iniciar la siguiente iteración
        // Liberamos memoria de centroides y los contadores correspondientes
        for (int i = 0; i < num_clusters; i++) {
            delete[] centroids[i];
        }
        delete[] centroids;
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

    // Establecer el número de hilos de OpenMP
    // Si como argumento no nos indican cuantos hilos usar, asignamos el máximo
    int num_threads = omp_get_max_threads();
    if (argc >= 5) {
        num_threads = atoi(argv[4]);
    }
    omp_set_num_threads(num_threads);

    // Asignar memoria para los puntos de datos
    // Memory allocations
    float** paralelo = new float*[size];
    for (long long int i = 0; i < size; i++) {
        paralelo[i] = new float[3]{0.0, 0.0, -1}; // un punto, modelado como arreglo, tiene:
                                                  // {coordenada x, coordenada y, cluster al que pertenece}
    }

    // Cargar datos desde el archivo CSV
    load_CSV(input_file_name, paralelo, size);

    // Realizar clustering k-means
    // Tomamos el tiempo de inicio y fin para calcular tiempo de ejecución
    double start_paralelo = omp_get_wtime();
    kmeans_paralelo(paralelo, num_clusters, size, max_iterations);
    double tiempo_ejecucion_paralelo = omp_get_wtime() - start_paralelo;
    cout << "Tiempo de ejecución en paralelo: " << tiempo_ejecucion_paralelo << "\n";

    // Guardamos los resultados en un nuevo archivo .csv
    save_to_CSV(output_file_name_paralelo, paralelo, size);

    // Liberamos la memoria que usamos
    for (long long int i = 0; i < size; i++) {
        delete[] paralelo[i];
    }
    delete[] paralelo;

    return 0;
}