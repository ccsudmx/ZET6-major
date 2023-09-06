#ifndef MY_JSON_H
#define MY_JSON_H
 typedef struct
 {
 	char *name;
    int status;
 	int value;
    int hours;
     int min;
     int sec;
     int ID;
     char *Num;
 	
 }Json; 


void JSON_LORA(char * a,Json * tag);
 void Json_time(char *a,Json *tag);
#endif
