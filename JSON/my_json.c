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

void Json_time(char *a,Json *tag)
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
tag->hours=(*b-48)*10;
 b++;
 tag->hours+=(*b-48);
 
  while(*b!=':')	
 {  
     b++;
 }

 b++;
 b++;
tag->min=(*b-48)*10;
 b++;
 tag->min+=(*b-48);
 
 
  while(*b!=':')	
 {  
     b++;
 }

 b++;
 b++;
tag->sec=(*b-48)*10;
 b++;
 tag->sec+=(*b-48);
 
    
    


}


