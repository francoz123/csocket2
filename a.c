#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <ctype.h> 
int strong_password(const char *pass);
int main()
{
    int n;
    char password[26] = "pass2#Pword";
    printf("%s", password);
    
    
    char sender[256], msg[256];
	sscanf(password, "%s %n", sender, &n);
	printf("\n%s", sender);
	
	printf("%d", strong_password(password));
}

int strong_password(const char *p) {
    int upper, lower, digit, special;
    upper = lower = digit = special = 0;
    char c;

    while (*p != '\0') {
        c = *p; 
        if (isupper(c)) upper++;
        else if (islower(c)) lower++;
        else if (isdigit(c)) digit++;
        else if (ispunct(c)) special++;
        else continue;
        p++;
    }
    //printf("%d\n%d\n%d\n%d\n", upper,lower, digit, special);
    return (upper && lower && digit && special);
}