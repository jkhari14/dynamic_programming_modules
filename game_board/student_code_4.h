///////////////////////////////////////////////////////////////////////////////
// You need to
//    1. Read the programming assignment in homework #4.
//    2. Implement function GetStudentName.
//    3. Implement function MinCost.
//    4. Compile your code as explained in the PDF file.
//    5. Run the executable on small and large unit tests.
//    6. Test and debug your code. Make sure that your program does not have
//       any memory leaks.
//    7. Remove all commented out code. Double check that your program does not
//       print any debug information on the screen.
//    8. Submit your code ("student_code_4.h") via Canvas.
///////////////////////////////////////////////////////////////////////////////

//required libraries
#include <string>
#include <vector>


//you can include standard C++ libraries here
#include <math.h>
#include <unordered_map>
#include "test_framework.h"
#include <iostream>
// This function should return your name.
// The name should match your name in Canvas

void GetStudentName(std::string& your_name)
{
   //replace the placeholders "Firstname" and "Lastname"
   //with you first name and last name
   your_name.assign("Enter Here");
}

int min(int a, int b)
{
	if (a < b) {
		return a;
	}
	else { // a >= b
		return b;
	}
}

int OPTF(int num, std::unordered_map<int, int> memo, std::unordered_map<int, int> pricevec, int f) {
	if (memo[num] == -1) {
		if (num >= 5) { 
			memo[num] = min((std::floor(pricevec[num] - (pricevec[num]/10)) + f) + OPTF(num - 1, memo, pricevec, f),
				f + pricevec[num] + pricevec[num - 1] + pricevec[num - 2] + pricevec[num - 3] + pricevec[num - 4] + OPTF(num - 5, memo, pricevec, f));
		
		}
		else {
			memo[num] = std::floor(pricevec[num] - (pricevec[num] / 10)) + f + OPTF(num - 1, memo, pricevec, f);
		}
	}

	return memo[num];
}
int MinCost(const std::vector<int>& prices, int fee)
{
	/*std::vector<int> OPT;
	for (int i = 0; i < prices.size()+1; i++) {
		OPT.push_back(-1);
	}
	OPT[0] = 0;
	*/
	std::unordered_map<int, int> OPT;
	OPT[0] = 0;
	for (int i = 1; i < prices.size()+1; i++) {
		OPT[i] = -1;
	}
	/*
	std::vector<int> prices_fitted;
	prices_fitted.push_back(0);
	for (int i = 0; i < prices.size(); i++) {
		prices_fitted.push_back(prices[i]);
	}
	*/
	std::unordered_map<int, int> price_map;
	price_map[0] = 0;
	for (int i = 1; i < prices.size() + 1; i++) {
		price_map[i] = prices[i-1];
	}

	int n = prices.size(); 
   /* implement your algorithm here */
	return OPTF(n, OPT, price_map, fee);

}
