#include <stdio.h>

int main()
{
   int column = 0, tabspaces = 0, counter = 0;
   int c;
   c = getchar();
   while (c != EOF){
     if (c == '\t'){
       tabspaces = (8 - (column % 8));
       for (counter = 0; counter < tabspaces; counter++){
         putchar(' ');
       }
       column += tabspaces;
     }

     else if (c == '\b'){
       if (column > 0){
         putchar('\b');
         column--;
       }
       else{
         putchar('\b');
       }
     }
     else if (c == '\n' || c == '\r'){
       putchar(c);
       column = 0;
     }

     else {
       putchar(c);
       column++;
     }
     c = getchar();
   }
   return 0;
}   
