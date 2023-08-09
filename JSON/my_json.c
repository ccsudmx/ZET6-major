#include "my_json.h"
#include "stdio.h"
void JSON_LORA(char * a,Json * tag)
{
 char *b=a;
 while(*b!=',')
 {
     b++;
    // printf("A0=%c",*b);
     
 }
 while(*b!=':')	
 {  
     b++;
 }

 b++;
 b++;	
 if(*b=='1')tag->status=1;
 else tag->status=0;      
 while(*b!=':')b++;
//  printf("A4=%c",*b);
// b++;
// printf("A5=%c",*b);
// b++;	
// printf("A6=%c",*b);
 b++;
 b++;
 if(*b=='0')tag->value=0;
 tag->value=(*b-48)*10;
 b++;
 tag->value+=(*b-48);
 if(tag->value==10&&(*++b=='0'))tag->value=100;


}

