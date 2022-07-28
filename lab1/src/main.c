#include <stdio.h>

__asm hash(char *str,int *hashes)
{
	PUSH {r4,r5}
	MOV r4,#4
	MOV r3, #0 // r3 stores the result and is initialized as zero
loop
	LDRB  r2, [r0]	// Load byte into r2 from memory pointed to by r0 (str pointer)
	MOV   r5,r1
	CMP   r2, #'0' // compare it with the character before '1', skip if '0'
	BLS   skip    // If byte is lower or same, then skip this byte
	CMP   r2, #'9'    // Compare it with the '9' character
	BHI   letter // If it is higher, then check for letter (upper case)
  SUBS r2,#'1'-1
	SUBS r3,r2 //subtract the corresponding arithmetic
  B skip

letter
	CMP   r2, #'A'-1  // compare it with the character before 'a'
	BLS   skip    // If byte is lower or same, then skip this byte
	CMP   r2, #'Z' 	// Compare it with the 'z' character
	BHI   skip    // If it is higher, then skip this byte
	
  SUBS r2,#'A' //make r2 into hash array index. !its ok to change the value!
	MUL  r2,r4
  ADDS r5,r2 // now r2 points to the hash of the corresponding letter
	LDRB r2,[r5]
	ADDS r3,r2  // add the hash to the result
  LDRB  r2, [r0]  // fix r2 register again, by loading byte pointed by r0 again
skip    
	ADDS  r0, r0, #1 // Increment str pointer
  CMP   r2, #0      // Was the byte 0? 
	BNE   loop    // If not, repeat the loop
  MOV r0,r3 //store result into return register r0
	POP {r4,r5}
	BX    lr          // Else return from subroutine
}

int hash();

int main(void)
{
int hash_array[26]= {10,42,12,21,7,5,67,48,69,2,36,3,19,1,14,51,71,8,26,54,75,15,6,59,13,35}; //static initialize the given hash array

char hash1[]="CHRIS";
char hash2[]="NICK";
char hash3[]=" / % F3";	
volatile int result1,result2,result3;
	result1=hash(hash1 , hash_array);
	result2=hash(hash2, hash_array);
  result3=hash(hash3, hash_array);
printf("%s is equal to a hash of %d\n",hash1,result1);	
printf("%s is equal to a hash of %d\n",hash2, result2);
printf("%s is equal to a hash of %d\n",hash3, result3);	
}
