///////////////////////////////////////////////////////////////////////////////
// You need to
//    1. Read the programming assignment in homework #6.
//    2. Implement function GetStudentName.
//    3. Implement function MinProcessingTime
//    4. Compile your code as explained in the PDF file.
//    5. Run the executable on small and large unit tests.
//    6. Test and debug your code. Make sure that your program does not have
//       any memory leaks.
//    7. Remove all commented out code. Double check that your program does not
//       print any debug information on the screen.
//    8. Submit your code ("student_code_6.h") via Canvas.
///////////////////////////////////////////////////////////////////////////////

//required libraries
#include <string>
#include <vector>
#include <algorithm>
#include <limits>
#include <queue>
#include <cmath>

//you can include standard C++ libraries here

// This function should return your name.
// The name should match your name in Canvas

void GetStudentName(std::string& your_name)
{
   //replace the placeholders "Firstname" and "Lastname"
   //with you first name and last name
   your_name.assign("Jahleel Murray");
}

struct Profile
{
   int time {0};
   int energy {0};
};

int mptRecHelper(const std::vector<std::vector<Profile>> p, int mE, int i, /* std::vector<std::vector<int>>*/ int** OPT) {
    int overflow_separator = 100000000;
    int inf_num = std::numeric_limits<int>::max() - overflow_separator;
    if (i == 0) {
        return 0;
    }
    if (OPT[i][mE] !=  -1)  {
        return OPT[i][mE];
    }
    std::priority_queue <int, std::vector<int>, std::greater<int>> arrOfPaths;
    for (int k = 0; k < 8; k++) {
        int shell;
        if (mE - p[i - 1][k].energy >= 0) {
            shell = p[i - 1][k].time + mptRecHelper(p, mE - p[i - 1][k].energy, i - 1, OPT);
            if (shell < inf_num) {
                arrOfPaths.push(shell);
            }
        }
    }
    if (arrOfPaths.size() > 0) {
        int OPT_val = arrOfPaths.top();
        OPT[i][mE] = OPT_val;
        return OPT[i][mE];
    }
    else {
        return inf_num;
    }
}

int MinProcessingTime(const std::vector<std::vector<Profile>>& profiles,
    int maxEnergy)
{
    // The number of cores is fixed. You can assume that it is always equal to 8.   
    const int nCores = 8;
    int** resultTable;
    resultTable = new int* [profiles.size()+1];
    for (int i = 0; i <= profiles.size(); i++) {
        resultTable[i] = new int[maxEnergy + 1];
    }
    
    
    for (int i = 0; i <= profiles.size(); i++)
        for (int j = 0; j < maxEnergy + 1; j++)
            resultTable[i][j] = -1;
           
    //std::vector<std::vector<int>> resultTable(profiles.size()+1, std::vector<int>(maxEnergy + 1, -1));    
    int answer = mptRecHelper(profiles, maxEnergy, profiles.size(), resultTable);
    return answer;

}
