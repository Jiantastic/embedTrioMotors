#include <iostream>
#include <string>
#include <sstream>
#include "slre.h"

using namespace std;

// SLRE is buggy with brackets, refer to github issues

// int main(){
// 	const char *request = " GET /index.html HTTP/1.0\r\n\r\n";
// 	struct slre_cap caps[4];

// 	if (slre_match("^\\s*(\\S+)\\s+(\\S+)\\s+HTTP/(\\d)\\.(\\d)",
// 				request, strlen(request), caps, 4, 0) > 0) {
// 	printf("Method: [%.*s], URI: [%.*s]\n",
// 			caps[0].len, caps[0].ptr,
// 			caps[1].len, caps[1].ptr);
// 	} else {
// 	printf("Error parsing [%s]\n", request);
// 	}
// 	return 0;
// }

// ^(R-?[0-9][0-9]?[0-9]?(\\.[0-9][0-9]?)?)?(V[0-9][0-9]?[0-9]?(\\.[0-9][0-9]?[0-9]?)?)?$

int main(){
	// stringstream ss;
	// string inputString = "R-3V1";
	// string rString = "";
	// string vString = "";
	// int placeholderForVStringLocation = 0;
	// // split into R and V commands - had to do this because this regex libray doesn't work with R-V optional for some reason
	// for(int i=0;i<inputString.length();i++){
	// 	if(inputString[i] == 'V'){
	// 		placeholderForVStringLocation = i;
	// 	}
	// }

	// for(int i=0;i<inputString.length();i++){
	// 	if(i < placeholderForVStringLocation){
	// 		ss << inputString
	// 	}
	// 	ss << inputString[i];

	// }


	// IMPORTANT : strip whitespace before processing input!
	const char *request = "R-322.12";
	// music has 16 capturing groups <- test if this works!
	struct slre_cap caps[17];


	// R command standalone - fully functional
	// int x = slre_match("^R(-?[0-9][0-9]?[0-9]?(\\.[0-9][0-9]?)?)$",
	// 		request, strlen(request), caps, 10, 0);

	
	int x = slre_match("^R(-?[0-9][0-9]?[0-9]?(\\.[0-9][0-9]?)?)$",
		request, strlen(request), caps, 10, 0);

	if(x > 0){
		const char *test = caps[0].ptr;
		float f1;
		f1 = atof(test);
		cout << "value of float f1 " << f1 << endl;
		// string s = test;
		// cout << "value of s " << s << endl;
		printf("Method: [%.*s], URI: [%.*s]\n",
         caps[0].len, caps[0].ptr,
         caps[1].len, caps[1].ptr);
	}

	// V command standalone - fully functional
	// int x = slre_match("^V([0-9][0-9]?[0-9]?(\\.[0-9][0-9]?[0-9]?)?)$",
	// 		request, strlen(request), caps, 10, 0);


	// R and V command combined - tested for R-V commands (surprisingly works wtf?), also works for R commands <- buggy af but doesn't matter in our use case
	// int x = slre_match("^R(-?[0-9][0-9]?[0-9]?(\\.[0-9][0-9]?)?V[0-9][0-9]?[0-9]?(\\.[0-9][0-9]?[0-9]?)?)$",
	// 		request, strlen(request), caps, 10, 0);


	

	// int x = slre_match("^\\d\\d?$",
	// 	request, strlen(request), caps, 10, 0);

			


	// int x = slre_match("^(R-?[0-9][0-9]?[0-9]?(\\.[0-9][0-9]?)?)?$",
	// 			request, strlen(request), caps, 10, 0);
	
	// int x = slre_match("^(V[0-9][0-9]?[0-9]?(\\.[0-9][0-9]?[0-9]?)?)?$",
	// 			request, strlen(request), caps, 10, 0);
 
	// match music - I think it works fairly well, needs more testing 
	// int x = slre_match("^T([A-G][#\\^]?[1-8])([A-G][#\\^]?[1-8])?([A-G][#\\^]?[1-8])?([A-G][#\\^]?[1-8])?([A-G][#\\^]?[1-8])?([A-G][#\\^]?[1-8])?([A-G][#\\^]?[1-8])?([A-G][#\\^]?[1-8])?([A-G][#\\^]?[1-8])?([A-G][#\\^]?[1-8])?([A-G][#\\^]?[1-8])?([A-G][#\\^]?[1-8])?([A-G][#\\^]?[1-8])?([A-G][#\\^]?[1-8])?([A-G][#\\^]?[1-8])?([A-G][#\\^]?[1-8])?$",
	// 		request, strlen(request), caps, 16, 0);

	// printf("value : %d",x);

	// if (slre_match("^(R-?[0-9][0-9]?[0-9]?(\\.[0-9][0-9]?)?)?(V[0-9][0-9]?[0-9]?(\\.[0-9][0-9]?[0-9]?)?)?$",
	// 			request, strlen(request), caps, 10, 0) > 0) {
	// printf("one: [%.*s], two: [%.*s], three: [%.*s], four: [%.*s] \n",
	// 		caps[0].len, caps[0].ptr,
	// 		caps[1].len, caps[1].ptr,
	// 		caps[2].len, caps[2].ptr,
	// 		caps[3].len, caps[3].ptr);
	// } else {
	// printf("Error parsing [%s]\n", request);
	// }
	return 0;
}

// match dot

//(R-?\d{1,3}(\.\d{1,2})?)?(V\d{1,3}(\.\d{1,3})?)?


// test this
// ^(R-?[0-9][0-9]?[0-9]?(\\.[0-9][0-9]?)?)?(V[0-9][0-9]?[0-9]?(\\.[0-9][0-9]?[0-9]?)?)?$
// int main(){
// 	const char *request = "R10";
// 	struct slre_cap caps[4];
// // \\. for .   ------ [0-9]? for \d, make sure you have ^ and $
// 	if (slre_match("^(R-?[0-9][0-9]?[0-9]?(\\.[0-9][0-9]?)?)?$",
// 				request, strlen(request), caps, 4, 0) > 0) {
// 	printf("oeee");
// 	} else {
// 	printf("Error parsing [%s]\n", request);
// 	}
// 	return 0;
// }