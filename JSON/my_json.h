#ifndef MY_JSON_H
#define MY_JSON_H
 typedef struct
 {
 	char *name;
    int status;
 	int value;
    int len;
 	
 }Json; 

void JSON_LORA(char * a,Json * tag);
#endif
