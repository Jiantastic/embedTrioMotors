#include <iostream>
#include <string>
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
	const char *request = "R-3V1";
	struct slre_cap caps[16];

	// somewhat 
	int x = slre_match("^(R-?[0-9][0-9]?[0-9]?(\\.[0-9][0-9]?)?V[0-9][0-9]?[0-9]?(\\.[0-9][0-9]?[0-9]?)?)$",
				request, strlen(request), caps, 10, 0);

	// int x = slre_match("^(R-?[0-9][0-9]?[0-9]?(\\.[0-9][0-9]?)?)?$",
	// 			request, strlen(request), caps, 10, 0);
	
	// int x = slre_match("^(V[0-9][0-9]?[0-9]?(\\.[0-9][0-9]?[0-9]?)?)?$",
	// 			request, strlen(request), caps, 10, 0);

	// int x = slre_match("^T([A-G][#\\^]?[1-8])([A-G][#\\^]?[1-8])?([A-G][#\\^]?[1-8])?([A-G][#\\^]?[1-8])?([A-G][#\\^]?[1-8])?([A-G][#\\^]?[1-8])?([A-G][#\\^]?[1-8])?([A-G][#\\^]?[1-8])?([A-G][#\\^]?[1-8])?([A-G][#\\^]?[1-8])?([A-G][#\\^]?[1-8])?([A-G][#\\^]?[1-8])?([A-G][#\\^]?[1-8])?([A-G][#\\^]?[1-8])?([A-G][#\\^]?[1-8])?([A-G][#\\^]?[1-8])?$",
	// 		request, strlen(request), caps, 16, 0);

	printf("value : %d",x);

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