/* Data wrangler
 *
 * Skeleton program written by Ben Rubinstein, April 2014
 *
 * Modifications by Tessa(Hyeri) Song 597952, May 2014
 *
 * This program is desinged to investigate simple data which
 * consists of integers.
 * This allows users to expand or print the data, select some specific 
 * rows or columns and get the statistical answers
 * to specific columns like average, standard deviation and so on.
 *
 * Algorithms are fun
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <math.h>

#define COLS 100	/* max number of columns of data */
#define ROWS 1000	/* max number of rows of data */
#define LINELEN	1000	/* max input line length */
#define MAXPROJECT 100	/* max number of columns to project onto */
#define NUMINTSINS 2	/* required number of integers in select command */
#define MAXINTSING 2	/* max number of integers in summary command */
#define MAXINDEX 1000	/* max number of indexes pointing at the first char
			 * of each command in the whole piped command
			 */
#define MAXCPRSTR 2	/* max length of string to contain comparison char
 			 * and null character */
 			 
#define DATADELIM ","	/* column delimeter for record input/output */

#define ERROR (-1)	/* error return value from some functions */

#define ADD		'a'	/* command to add a record to data */
#define PRINT		'?'	/* command to print the data */
#define PROJECT		'p'	/* command to project data */
#define SELECT		's'	/* command to select rows of data */
#define SUMMARY		'g'	/* command to summarize the data */
#define ALLCOMMANDS	"a?psg"	/* list of all commands */
#define PSG		"psg"   /* list of pipe-able commands */

#define AVERAGE		'a'	/* command to solve out average of a column */
#define SDEVIATION	's'	/* command to solve out standard deviation of 
				 * a column
				 */
#define MEDIAN		'm'	/* command to solve out median of a column */
#define CORRELATION	'c'	/* command to solve out correlation of two
				 * columns
				 */
#define ALLSUBCOMMANDS	"asmc"	/* list of all sub-commands of summary */

#define BIGGER 		'>'	
#define EQUAL		'='	
#define SMALLER		'<'	
#define ALLCOMPARISONS	">=<"	/*list of all comparisons used in select*/

#define COMPOSE		'|'	/* character to indicate 'compose' */

typedef int record_t[COLS];	/* one element of data */
typedef record_t data_t[ROWS];	/* a dataset of records */
typedef int (*aptr_t)[COLS];	/* pointer to point at int array 
				 * whose length is COLS 
				 */

/****************************************************************/

/* function prototypes */

void print_prompt(void);
int read_line(char *line, int maxlen);
void investigate_line(data_t data, char* line, int* rows, int* cols
    , int* is_first_add);
void process_line(data_t data, data_t temp_data, int *rows, int *cols
    , char *line, int* is_first_add, int is_last_token
    , int* was_warning_printed);
int parse_add(char *str, char *delim, int results[], int max_results
    , int is_first_add);
int parse_select(char *str, int results[], char* cprsn);
int extract_ints(char* str, char *delim, int results[], int max_results);
void copy_record(record_t dest, record_t src, int len);
void copy_data(data_t dest, data_t src, int rows, int cols);
void print_record(record_t record, int record_len);
void print_data(data_t data, int rows, int cols);
int is_empty(data_t data, int rows, int cols);
void do_add(record_t record, data_t data, int *rows, int cols);
void do_project(data_t src, data_t dest, int rows, int cols
    , int target[], int target_len);
int do_select(data_t src, data_t dest, int rows, int cols
    , int s_nums[], char *cprsn);
int* extract_col(data_t data, int rows, int col);
double average(int A[], int n);
double deviation(int A[], int n);
double median(int A[], int n);
double correlation(int A[], int B[], int n);
void selection_sort(int A[], int n);
int biggest_idx(int A[], int n);
void swap(int* ptr01, int* ptr02);

/****************************************************************/

/* orchestrate the entire program
 */
int
main(int argc, char *argv[]) {
	
    char line[LINELEN+1];
    data_t data;
    int cols = 0;
    int rows = 0;
    int is_first_add = 1;
	
    /* prompt for commands by-the-line:
     * read the lines, investigate them; repeat.
     * pass is_first_add to check whether it is the first time to add something
     * to data or not to decide the length of a row.
     */
	 
    print_prompt();

    while (read_line(line, LINELEN)) {
        investigate_line(data, line, &rows, &cols, &is_first_add);
	print_prompt();
    }

    /* all done */
    printf("\n");
    return 0;
    
}

/****************************************************************/

/* prompt user to enter for more input
 */
void
print_prompt(void) {
	
    printf("> ");

}

/****************************************************************/

/* read in a line of input, strip space
 */
int
read_line(char *line, int maxlen) {
   
    int n = 0;
    int oversize = 0;
    int c;
    
    while (((c=getchar())!=EOF) && (c!='\n')) {
    	if (n < maxlen) {
    	    if (!isspace(c)) {
		line[n++] = c;
	    }
	}
	else {
	    oversize++;
	}
    }
    line[n] = '\0';
    if (oversize > 0) {
	printf("Warning! %d over limit. Line truncated.\n", oversize);
    }
    return ((n>0) || (c!=EOF));
    
}

/****************************************************************/

/* investigate the line passed from main function.
 * This function deals with the line in two ways
 * based on whether the line consists of piped commands or not.
 * If the line is piped, the two temporary data_t arrays declared are used,
 * the two pointers to point at them keeping swapped after each command.
 * After deciding if line is piped and running some tests
 * to see if it is valid,
 * process_line is called to process each command in detail.
 */
void 
investigate_line(data_t data, char* line, int* rows, int* cols
    , int* is_first_add){
    
    int i;
    int is_last_token = 0;
    int temp_cols;
    int temp_rows;
    data_t temp_data01;
    data_t temp_data02;
    aptr_t ptr01;
    aptr_t ptr02;
    aptr_t temp_ptr;
    int line_len;
    int com_index[MAXINDEX];
    int num_index;
    int was_warning_printed;
	
    if (strchr(line, COMPOSE)) {
	temp_rows = *rows;
	temp_cols = *cols;
	ptr01 = temp_data01;
	ptr02 = temp_data02;
	was_warning_printed = 0;
	
	com_index[0] = 0;
	line_len = strlen(line);
	
	copy_data(temp_data01, data, *rows, *cols);
	
	/* find out all the indexes of each command in line and
 	 * store them in an array.
 	 * replace all the compose characters with null charater.
	 */
	for (i = 1, num_index = 1; i<line_len; i++) {
	    if (line[i] == COMPOSE) {
	        com_index[num_index++] = i+1;
		line[i] = '\0';
	    }
	}
	
	for (i = 0; i<num_index; i++) {
	    if (strchr(PSG, line[com_index[i]]) == NULL) {
	        printf("Invalid command.\n");
		return;
	    }			
	    if (line[com_index[i]] == SUMMARY) {
	        if (i != num_index-1) {
		    printf("Summary Command should be at the end.\n");
		    return;
		}
	    }
	    
	    /* use the flag of is_last_token to make it possible
	     * to print out only the result by the last command. 
	     */
	    if (i == num_index-1) {
	        is_last_token = 1;
	    } 
	    else {
		is_last_token = 0;
	    }
	    
	    process_line(ptr01, ptr02, &temp_rows, &temp_cols
	        , line+com_index[i], is_first_add, is_last_token
	        , &was_warning_printed);
	    
	    /* if any warning message has been printed out
	     * before reaching the last command,
	     * the whole process with the line will stop. 
	     */
	    if (was_warning_printed) {
	        return;
	    }
	    
	    temp_ptr = ptr01;
	    ptr01 = ptr02;
	    ptr02 = temp_ptr;		
	}		
    } else {
	is_last_token = 1;
	process_line(data, temp_data01, rows, cols, line
            , is_first_add, is_last_token, &was_warning_printed);
    }

}

/****************************************************************/

/* parse a line of input into the components of a command;
 * call appropriate function to execute command
 */
void
process_line(data_t data, data_t temp_data, int *rows, int *cols, char *line
    , int *is_first_add, int is_last_token, int* was_warning_printed) {

    record_t record;
    int columns[MAXPROJECT];
    int s_nums[NUMINTSINS];
    int g_nums[MAXINTSING];
    int nrows;
    double avrg, devi, medn, corln;
    int* acol01;
    int* acol02;
    int comtype, sum_comtype, i, len;
    char cprsn[MAXCPRSTR];

    /* do nothing on a NULL or empty line */
    if (!line || strlen(line) == 0) {
    	return;
    }
	
    /* the command type is given by the first char
     * make sure it is valid
     */
    comtype = line[0];
    if (strchr(ALLCOMMANDS, comtype) == NULL) {
  	printf("Unknown command \'%c\'\n", comtype);
	return;
    }

    /* check if the data is empty */
    if (comtype != ADD && (*rows<=0 || *cols<=0)){
    	printf("Empty data.\n");
	*was_warning_printed = 1;
	return;
    }
    
    /* drill down to command specifics: specific parsing, execution */
    if (comtype == ADD) {
	len = parse_add(line+1, DATADELIM, record, *cols, *is_first_add);
	if (len == ERROR) {
	    printf("Invalid record not added to data.\n");
    	    return;
	}
	
	/* check if the record is added for the first time */
	if (*is_first_add) {
	    if (len == 0 || len > COLS) {
	        printf("num_col must be between %d and %d.\n", 1, COLS);
		return;
	    }
	    *cols = len;
	    *is_first_add = 0;
	}
	
	if (len < *cols) {
	    printf("Record's %d columns too few. Not added.\n", len);
	    return;
	}
	if (len > *cols) {
	    printf("Record truncated from %d to %d columns.\n", len, *cols);
	    len = *cols;
	}
	do_add(record, data, rows, *cols);
    } else if (comtype == PRINT) {
	print_data(data, *rows, *cols);
    } else if (comtype == PROJECT) {		
	len = extract_ints(line+1, DATADELIM, columns, MAXPROJECT);
	if (len == ERROR) {
	    *was_warning_printed = 1;
  	    printf("Invalid target columns specified.\n");
	    return;
	}
	for (i=0; (i<len) && (i<MAXPROJECT); i++) {
	    if (columns[i] < 1 || columns[i] > *cols) {
	        *was_warning_printed = 1;
		printf("Invalid column %d.\n", columns[i]);
		return;
		}
	}
	if (len > MAXPROJECT) {
	    printf("Projecting onto %d of %d cols specified\n"
	        ,MAXPROJECT, len);
	    len = MAXPROJECT;
	}
	do_project(data, temp_data, *rows, *cols, columns, len);
	
	/* if the command is not the last one in the line,
	 * update the temporary cols.
	 */
	if (!is_last_token) {
	    *cols = len;
	}	
	
	/* if the command is the last one, print out the result */
	if (is_last_token) {
	    print_data(temp_data, *rows, len);
	}
	
    } else if (comtype == SELECT) {
    	
    	/* put the numbers in the command in s_nums array */
	len = parse_select(line+1, s_nums, cprsn);
	if (len == ERROR || len != 2) {
	    *was_warning_printed = 1;
	    printf("Invalid command.\n");
	    return;
	}

	/* the first num of s_nums indicates a specific column */	
	if (s_nums[0] < 1 || s_nums[0] > *cols) {
	    *was_warning_printed = 1;
	    printf("Invalid column %d\n", s_nums[0]);
  	    return;
	}
	
	nrows = do_select(data, temp_data, *rows, *cols
	    , s_nums, cprsn);
	if (!is_last_token) {
	    *rows = nrows;
	}		
	if (is_last_token) {
	    print_data(temp_data, nrows, *cols);
	}	
    } else if (comtype == SUMMARY) {
    	
    	/* the second char in the line should be one of the
    	 * sub-summary command types.
    	 */
	sum_comtype = line[1];
	if (strchr(ALLSUBCOMMANDS, sum_comtype) == NULL) {
	    printf("Unknown summary command \'%c\'\n", sum_comtype);
	    return;
	}

	/* put the nums in the command in g_nums array */	
	len = extract_ints(line+2, DATADELIM, g_nums, MAXINTSING);

	if (len == ERROR || len == 0 || len > 2) {
	    printf("Invalid command.\n");
	    return;
	}
	if ((len == 1 && sum_comtype == CORRELATION)
	    || (len == 2 && sum_comtype != CORRELATION)){
	    printf("Invalid command.\n");
	    return;   
	}
	for (i = 0; i<len; i++) {
	    if (g_nums[i] < 1 || g_nums[i] > *cols) {
	        printf("Invalid column %d.\n", g_nums[i]);
	        return;
	    }
	}
	
	/* extract all the numbers in the specific col and get the address
	 * of it.
	 */
	acol01 = extract_col(data, *rows, g_nums[0]);
	if(i == 2){
	    acol02 = extract_col(data, *rows, g_nums[1]);
	}
	
	if (sum_comtype == AVERAGE) {
	    avrg = average(acol01, *rows);
	    printf("%.1f\n", avrg);
	} else if (sum_comtype == SDEVIATION) {
	    devi = deviation(acol01, *rows);
	    printf("%.6f\n", devi);
	} else if (sum_comtype == MEDIAN) {
	    medn = median(acol01, *rows);
	    printf("%.1f\n", medn);
	} else {	
	    corln = correlation(acol01, acol02, *rows);
	    printf("%.1f\n", corln);
	}
    } 
    return;
    
}

/****************************************************************/

/* parse add command and store the integers in it in results array.
 * check if is_first_add is true to decide the maximum number.
 * call extract_ints to extract integers.
 */
int
parse_add(char *str, char *delim, int results[], int max_results
    , int is_first_add) {

    int num_results = 0;
    
    if (is_first_add) {
        max_results = COLS;
    }
    num_results = extract_ints(str, delim, results, max_results);
    return num_results;

}

/****************************************************************/

/* parse selet command to find out the specific comparison char
 * and to store all the integers in results array.
 */
int 
parse_select(char *str, int results[], char* cprsn){
	
    int num_results = 0;
    int i=0;

    while (strchr(ALLCOMPARISONS, str[i]) == NULL && str[i] != '\0') {
	i++;
    }

    if (str[i]=='\0') {
	return ERROR;
    }
    else {
	cprsn[0] = str[i];
    }
    
    cprsn[1] = '\0';
    num_results = extract_ints(str, cprsn, results, NUMINTSINS);
	
    return num_results;
    
}

/****************************************************************/

/* parse string for a delimited-list of positive integers.
 * Returns number of ints parsed or -1 if a delimited
 * token is not a valid int. If more than max_results
 * ints are parsed, the excess will not be written to results;
 * it is recommended that the calling function notify the
 * user that some data was discarded. Note! str will be
 * modified as a side-effect of running this function: delims
 * replaced by \0
 */
int 
extract_ints(char* str, char *delim, int results[], int max_results){
	
    int num_results = 0;
    int num;
    char* token = strtok(str, delim);
	
    while (token != NULL) {	
	if ((num=atoi(token)) == 0) {
	    return ERROR;
	}
	if (num_results < max_results) {
	    results[num_results] = num;
	}
	num_results++;
	token = strtok(NULL, delim);
    }

    return num_results;
    
}

/****************************************************************/

/* copy contents of one record to another
 */
void
copy_record(record_t dest, record_t src, int len) {
	
    int i;
    
    for (i=0; i<len; i++) {
	dest[i] = src[i];
    }
    
}

/****************************************************************/

/* copy contents of one data to another
 */
void
copy_data(data_t dest, data_t src, int rows, int cols){
	
    int i = 0;
    
    if (rows <= 0 || cols <= 0) {
	return;
    }
    for (i = 0;i<rows;i++) {
	copy_record(dest[i], src[i], cols);
    }
    
}

/****************************************************************/

/* print a record
 */
void
print_record(record_t record, int record_len) {
	
    int i = 0;
    
    if (record_len == 0) {
	return;
    }
    printf("%d", record[i++]);
    while (i<record_len) {
	printf("%s %d", DATADELIM, record[i++]);
    }
    printf("\n");
    
}

/****************************************************************/

/* print an entire dataset
 */
void
print_data(data_t data, int rows, int cols) {
	
    int i;
    
    if (rows <= 0 || cols <= 0) {
	printf("Empty data.\n");
	return;
    }
    for (i=0; i<rows; i++) {
	print_record(data[i], cols);
    }
    
}

/****************************************************************/

/* copy over a given record to the end of data
 */
void
do_add(record_t record, data_t data, int *rows, int cols) {
	
    if (*rows >= ROWS) {
	printf("Cannot add another record, already at limit\n");
	return;
    }
    copy_record(data[(*rows)++], record, cols);
    printf("Added record: ");
    print_record(record, cols);
    
}

/****************************************************************/

/* project the data onto the specified columns.
 * assumes the target columns specified are all valid.
 * column indexing begins at 1.
 */
void
do_project(data_t src, data_t dest, int rows, int cols
    , int target[], int target_len) {

    int i, j, k;
    
    for (i=0; i<rows; i++) {
	for (j=0, k=0; j<target_len; j++, k++) {
	    dest[i][k] = src[i][target[j] - 1];
	}
    }
    
}

/****************************************************************/
/* select the required rows by the given condition. 
 * return the number of the rows.
 * row indexing begins at 1.
 */
int 
do_select(data_t src, data_t dest, int rows, int cols
    , int s_nums[], char* cprsn) {

    int i;
    int nrows=0;
	

    for (i = 0; i<rows; i++) {
        if (*cprsn == BIGGER) {
      	    if (src[i][s_nums[0]-1] > s_nums[1]) {
		copy_record(dest[nrows++], src[i], cols);
	    } 
	} else if (*cprsn == EQUAL) {
	    if (src[i][s_nums[0]-1] == s_nums[1]) {
		copy_record(dest[nrows++], src[i], cols);
	    }
	} else {
	    if (src[i][s_nums[0]-1] < s_nums[1]) {
		copy_record(dest[nrows++], src[i], cols);
	    }
	}
    }	
    return nrows;
    
}


/****************************************************************/

/* extract the numbers of the specific col, store them
 * using malloc and return the address.
 */
int* 
extract_col(data_t data, int rows, int col){
    int i;
    int* acol = (int *)malloc(rows*sizeof(int));
    
    for(i = 0; i<rows; i++){
        acol[i] = data[i][col-1];
    }
    
    return acol;
}

/****************************************************************/

/* return the average of array
 */
double 
average(int A[], int n){
	
    int i, sum=0;
    double avrg;
	
    for (i = 0; i<n; i++) {
	sum+=A[i];
    }
			
    avrg = (double)sum/n;
    return avrg;
    
}

/****************************************************************/

/* return the standard deviation of array
 */
double 
deviation(int A[], int n){
	
    int i;
    double avrge = average(A, n);
    double varnc=0.0;
    double devi;

    for (i = 0; i<n; i++) {
	varnc += (A[i]-avrge)*(A[i]-avrge);
    }
	
    varnc = varnc/n;
	
    devi = sqrt(varnc);
	
    return devi;
	
}

/****************************************************************/

/* return the median of array
 */
double 
median(int A[], int n){

    double medn;

    selection_sort(A, n);
	
    if (n%2){
	medn = A[n/2];	
    } else {
	medn = (A[n/2]+A[n/2-1])/2.0;
    }
	
    return medn;
	
}

/****************************************************************/

/* return the correlation of two arrays.
 */
double 
correlation(int A[], int B[], int n){
	
    double avrg1, avrg2;
    double devi1, devi2;
    double corln=0.0;
    int i;
	
    avrg1 = average(A, n);
    avrg2 = average(B, n);
	
    devi1 = deviation(A, n);
    devi2 = deviation(B, n);
	
    for (i = 0; i<n; i++) {
	corln += (A[i]-avrg1)*(B[i]-avrg2);
    }
	
    corln = corln/n/(devi1*devi2);
	
    return corln;
	
}

/****************************************************************/

/* sort an array keep placing the biggest number to the end 
 */
void 
selection_sort(int A[], int n){
	
    int i;
    int max_idx;
    
    for (i = n; i>1 ; i--) {
        max_idx = biggest_idx(A, i);
        swap(&A[i-1], &A[max_idx]);	
    	    
    }
    
}

/****************************************************************/

/* find out the index where the biggest number is located in array
 */
int 
biggest_idx(int A[], int n){
	
    int i;
    int max_idx = 0;
    
    for (i = 1; i<n ;i++) {
        if (A[max_idx]<A[i]) {
            max_idx = i;
        }
    }
    return max_idx;
    
}


/****************************************************************/

/* swap the two numbers using call-by-reference
 */
void 
swap(int* ptr01, int* ptr02){
	
    int temp;
    
    temp = *ptr01;
    *ptr01 = *ptr02;
    *ptr02 = temp;
    
}








