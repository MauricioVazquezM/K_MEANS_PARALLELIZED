# Documentation

## Functions 

### Load_Csv Function

- This function reads a CSV file containing two-dimensional points into a two-dimensional array. It's tailored for files where each line represents a point with its x and y coordinates separated by a delimiter. The function provides basic error handling for file opening issues but assumes the input file is well-formatted and that the points array has been appropriately allocated before calling the function.

```cpp
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
```