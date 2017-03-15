#include <iostream>
#include <random>
using namespace std;
 
int main() {
	// your code goes here
 
 
 
std::random_device rd;
std::default_random_engine generator(rd()); // rd() provides a random seed
std::uniform_real_distribution<double> distribution(0.1,10);
 
double number = distribution(generator);
 
	cout << number << endl;
 
	return 0;
}