///////////////////////////////////////////////////////////////////////////////
// You need to
//    1. Read the programming assignment in homework #5.
//    2. Implement function GetStudentName.
//    3. Implement function MaxTour
//    4. Compile your code as explained in the PDF file.
//    5. Run the executable on small and large unit tests.
//    6. Test and debug your code. Make sure that your program does not have
//       any memory leaks.
//    7. Remove all commented out code. Double check that your program does not
//       print any debug information on the screen.
//    8. Submit your code ("student_code_5.h") via Canvas.
///////////////////////////////////////////////////////////////////////////////

//required libraries
#include <string>
#include <vector>
#include <stdio.h>      /* printf */
#include <math.h>       /* pow */
#include <limits>

//you can include standard C++ libraries here

// This function should return your name.
// The name should match your name in Canvas

void GetStudentName(std::string& your_name)
{
    //replace the placeholders "Firstname" and "Lastname"
    //with you first name and last name
    your_name.assign("Jahleel Murray");
}

struct Point
{
    double x{ 0.0 };
    double y{ 0.0 };
};

/*
double manhat(int p1, int p2, const std::vector<Point>& point_arr) 
{
    double distance = sqrt(pow((point_arr[p1].x - point_arr[p2].x), 2.0) +
        pow((point_arr[p1].y - point_arr[p2].y), 2.0));
    return distance;
}
*/
double manhat(int p1, int p2, const std::vector<Point>& point_arr)
{
    // Calculating distance
    return sqrt(pow((point_arr[p1].x - point_arr[p2].x), 2) +
        pow((point_arr[p1].y - point_arr[p2].y), 2) * 1.0);
}

double manhat0(int p1, const std::vector<Point>& point_arr)
{
    // Calculating distance
    return sqrt(pow((point_arr[p1].x), 2) +
        pow((point_arr[p1].y), 2) * 1.0);
}

/*
double manhat0(int p1, const std::vector<Point>& point_arr)
{
    double distance = sqrt(pow((point_arr[p1].x - 0), 2.0) +
        pow((point_arr[p1].y - 0), 2.0));
    return distance;
}
*/

void swapd(double* a, double* b)
{
    double t = *a;
    *a = *b;
    *b = t;
}

int partition(std::vector<double>& dc, int low, int high)
{
    double pivot = dc[high]; // pivot 
    int i = (low - 1); // Index of smaller element and indicates the right position of pivot found so far

    for (int j = low; j <= high - 1; j++)
    {
        // If current element is smaller than the pivot 
        if (dc[j] < pivot)
        {
            i++; // increment index of smaller element 
            swapd(&dc[i], &dc[j]);
        }
    }
    swapd(&dc[long(i) + 1], &dc[high]);
    return (i + 1);
}


/* The main function that implements QuickSort
arr[] --> Array to be sorted,
low --> Starting index,
high --> Ending index */
void quickSort(std::vector<double>& dc, int low, int high)
{
    if (low < high)
    {
        /* pi is partitioning index, arr[p] is now
        at right place */
        int pi = partition(dc, low, high);
        // Separately sort elements before 
        // partition and after partition 
        quickSort(dc, low, pi - 1);
        quickSort(dc, pi + 1, high);
    }
}
    
int MaxTour(const std::vector<Point>& points, double maxDistance)
{ 
   std::vector<std::vector<double>> distMatrix(points.size(), std::vector<double>(points.size())); 
   int answer = 0;
   for (int j = 0; j < points.size(); j++) {
       if (manhat0(j, points) * 2.0 > maxDistance) {
           distMatrix[0][j] = 2000000000.0;
       }
       else {
           distMatrix[0][j] = manhat0(j, points) * 2.0;
           answer = 1;
       }
   }

   std::vector<std::vector<double>> distBtwn(points.size(), std::vector<double>(points.size()));
   for (int i = 0; i < points.size(); i++) {
       for (int j = 0; j < points.size(); j++) {
           distBtwn[i][j] = manhat(i, j, points);
       }
   }

   for (int i = 1; i < points.size(); i++) {
       for (int j = 0; j < points.size(); j++) {
           if (points.size() - j > i) {
               std::vector<double> d_copy;
               for (int k = j + 1; k < points.size(); k++){
                   double the_distance = distMatrix[long(i) - 1][k];
                   d_copy.push_back(the_distance - manhat0(k, points) + distBtwn[j][k]);
               }
               //use quicksort()
               quickSort(d_copy, 0, d_copy.size() - 1);
               double min_dist = d_copy[0];
               if (distMatrix[0][j]/2 + min_dist > maxDistance) {
                   distMatrix[i][j] = 2000000000.0;
               }
               else {
                   distMatrix[i][j] = distMatrix[0][j]/2 + min_dist;
                   if (i+1 > answer) {
                       answer = i+1;
                   }
               }
           }
           else {
               distMatrix[i][j] = 2000000000.0;
           }
       }
   }
   return answer;
}
