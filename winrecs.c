#include <stdio.h>
#include <assert.h>

int main(){
   char c;
   char winner = '0';
   while ((c = getchar()) != EOF){
	if ( c == '!'){
	    c = getchar();
	    if (c == 'W' || c == 'B' || c == 'D'){
	        winner = c;
	        assert(getchar() == '\n');
	        continue;
	    }
	}
	printf("%c ", winner);
	while (c != '\n' && c != EOF){
	    putchar(c);
	    c = getchar();
	}
	putchar(c);
   }
}
