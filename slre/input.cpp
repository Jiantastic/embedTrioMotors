#include <iostream>
#include "slre.h"

using namespace std;

int main(){

	// TODO : testing this in mbed, with interrupts + use main thread while(1) scan for input

	// IMPORTANT : strip whitespace before processing input!
	// const char *request = "R32.22V122.222";

	char test[200];
	test[0] = 'R';
	test[1] = '1';
	test[2] = '0';
	const char *request = (const char*)test;
	// music has 16 capturing groups <- test if this works!
	struct slre_cap caps[16];

	if(slre_match("^R(-?[0-9][0-9]?[0-9]?(\\.[0-9][0-9]?)?)[\r\n]+$",
		request, strlen(request), caps, 10, 0) > 0){

		printf("Group 1 - R command ONLY\n");

		printf("Method: [%.*s], URI: [%.*s]\n",
         caps[0].len, caps[0].ptr,
         caps[1].len, caps[1].ptr);

		// const char *test = caps[0].ptr;
		// float f1;
		// f1 = atof(test);
		// cout << "value of float f1 " << f1 << endl;
		// string s = test;
		// cout << "value of s " << s << endl;
		// printf("Method: [%.*s], URI: [%.*s]\n",
        //  caps[0].len, caps[0].ptr,
        //  caps[1].len, caps[1].ptr);
	}
	else if(slre_match("^V([0-9][0-9]?[0-9]?(\\.[0-9][0-9]?[0-9]?)?)[\r\n]+$",
			 request, strlen(request), caps, 10, 0) > 0){
				
		printf("Group 2 - V command ONLY\n");

		printf("CAP 1: [%.*s], CAP 2: [%.*s]\n",
         caps[0].len, caps[0].ptr,
         caps[1].len, caps[1].ptr);
	}
	else if(slre_match("^R(-?[0-9][0-9]?[0-9]?)V([0-9][0-9]?[0-9]?)$",
			request, strlen(request), caps, 10, 0) > 0){

		printf("Group 3 - RV command, NO decimals\n");

		printf("CAP 1: [%.*s], CAP 2: [%.*s]\n",
         caps[0].len, caps[0].ptr,
         caps[1].len, caps[1].ptr);
	}
	else if(slre_match("^R(-?[0-9][0-9]?[0-9]?\\.[0-9][0-9]?)V([0-9][0-9]?[0-9]?)$",
			request, strlen(request), caps, 10, 0) > 0){

		printf("Group 4 - RV command, decimal for R ONLY\n");

		printf("CAP 1: [%.*s], CAP 2: [%.*s]\n",
         caps[0].len, caps[0].ptr,
         caps[1].len, caps[1].ptr);

	}
	else if(slre_match("^R(-?[0-9][0-9]?[0-9]?)V([0-9][0-9]?[0-9]?\\.[0-9][0-9]?[0-9]?)$",
			request, strlen(request), caps, 10, 0) > 0){

		printf("Group 5 - RV command, decimal for V ONLY\n");

		printf("CAP 1: [%.*s], CAP 2: [%.*s]\n",
         caps[0].len, caps[0].ptr,
         caps[1].len, caps[1].ptr);

	}
	else if(slre_match("^R(-?[0-9][0-9]?[0-9]?\\.[0-9][0-9]?)V([0-9][0-9]?[0-9]?\\.[0-9][0-9]?[0-9]?)$",
			request, strlen(request), caps, 10, 0) > 0){

		
		printf("Group 6 - RV command, decimal for R AND V\n");

		printf("CAP 1: [%.*s], CAP 2: [%.*s]\n",
         caps[0].len, caps[0].ptr,
         caps[1].len, caps[1].ptr);

	}
	else if(slre_match("^T([A-G][#\\^]?[1-8])([A-G][#\\^]?[1-8])?([A-G][#\\^]?[1-8])?([A-G][#\\^]?[1-8])?([A-G][#\\^]?[1-8])?([A-G][#\\^]?[1-8])?([A-G][#\\^]?[1-8])?([A-G][#\\^]?[1-8])?([A-G][#\\^]?[1-8])?([A-G][#\\^]?[1-8])?([A-G][#\\^]?[1-8])?([A-G][#\\^]?[1-8])?([A-G][#\\^]?[1-8])?([A-G][#\\^]?[1-8])?([A-G][#\\^]?[1-8])?([A-G][#\\^]?[1-8])?$",
			request, strlen(request), caps, 16, 0) > 0){

		printf("Group 7 - Music!\n");

		printf("CAP 1: [%.*s], CAP 2: [%.*s], CAP 3: [%.*s], CAP 4: [%.*s], CAP 5: [%.*s], CAP 6: [%.*s], CAP 7: [%.*s], CAP 8: [%.*s], CAP 9: [%.*s], CAP 10: [%.*s], CAP 11: [%.*s], CAP 12: [%.*s], CAP 13: [%.*s], CAP 14: [%.*s], CAP 15: [%.*s], CAP 16: [%.*s]\n",
         caps[0].len, caps[0].ptr,
         caps[1].len, caps[1].ptr,
		 caps[2].len, caps[2].ptr,
		 caps[3].len, caps[3].ptr,
		 caps[4].len, caps[4].ptr,
		 caps[5].len, caps[5].ptr,
		 caps[6].len, caps[6].ptr,
		 caps[7].len, caps[7].ptr,
		 caps[8].len, caps[8].ptr,
		 caps[9].len, caps[9].ptr,
		 caps[10].len, caps[10].ptr,
		 caps[11].len, caps[11].ptr,
		 caps[12].len, caps[12].ptr,
		 caps[13].len, caps[13].ptr,
		 caps[14].len, caps[14].ptr,
		 caps[15].len, caps[15].ptr);
		
	}
	else{
		cout << "Error parsing. Please enter a valid command." << endl;
	}

	return 0;
}
