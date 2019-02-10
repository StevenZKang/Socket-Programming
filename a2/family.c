#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "family.h"

/* Number of word pointers allocated for a new family.
   This is also the number of word pointers added to a family
   using realloc, when the family is full.
*/
static int family_increment = 0;
	

/* Set family_increment to size, and initialize random number generator.
   The random number generator is used to select a random word from a family.
   This function should be called exactly once, on startup.
*/
void init_family(int size) {
    family_increment = size;
    srand(time(0));
}


/* Given a pointer to the head of a linked list of Family nodes,
   print each family's signature and words.

   Do not modify this function. It will be used for marking.
*/
void print_families(Family* fam_list) {
    int i;
    Family *fam = fam_list;
    
    while (fam) {
        printf("***Family signature: %s Num words: %d\n",
               fam->signature, fam->num_words);
        for(i = 0; i < fam->num_words; i++) {
            printf("     %s\n", fam->word_ptrs[i]);
        }
        printf("\n");
        fam = fam->next;
    }
}


/* Return a pointer to a new family whose signature is 
   a copy of str. Initialize word_ptrs to point to 
   family_increment+1 pointers, numwords to 0, 
   maxwords to family_increment, and next to NULL.
*/
Family *new_family(char *str) {
	Family *new_fam = malloc(sizeof(Family));
	(*new_fam).signature = malloc(sizeof(char) * (strlen(str)+1));
	strcpy((*new_fam).signature, str);
	char ** new_word_ptrs = malloc(sizeof(char*) * (family_increment+1));
	if (new_word_ptrs == NULL){
		printf("word_ptrs malloc error");
		exit(1);
	}
	(*new_fam).word_ptrs = new_word_ptrs;
	(*new_fam).num_words = 0;
	(*new_fam).max_words = family_increment; 
	(*new_fam).next = NULL; 
	return new_fam;
}


/* Add word to the next free slot fam->word_ptrs.
   If fam->word_ptrs is full, first use realloc to allocate family_increment
   more pointers and then add the new pointer.
*/
void add_word_to_family(Family *fam, char *word) {
    //Check if word_ptrs is full
    if ( fam->num_words >= fam->max_words ){
    	fam->word_ptrs = realloc(fam->word_ptrs, (sizeof(char*) * (family_increment + fam->num_words))); 
    	if (new_word_ptrs == NULL){
		printf("word_ptrs realloc error");
		exit(1);
	}
    	fam->max_words += family_increment;
	}
	
	(*fam).word_ptrs[fam->num_words] = word;
	(*fam).num_words++;
}


/* Return a pointer to the family whose signature is sig;
   if there is no such family, return NULL.
   fam_list is a pointer to the head of a list of Family nodes.
*/
Family *find_family(Family *fam_list, char *sig) {
	//Iterate through the linked list of fams until one with sig is found
	Family *curr_fam = fam_list; 
	while(curr_fam != NULL){
		//If the strings are equal
		if(strcmp(curr_fam->signature, sig) == 0){
			return curr_fam; 
		}
		curr_fam = curr_fam->next;
	}
    return NULL;
}


/* Return a pointer to the family in the list with the most words;
   if the list is empty, return NULL. If multiple families have the most words,
   return a pointer to any of them.
   fam_list is a pointer to the head of a list of Family nodes.
*/
Family *find_biggest_family(Family *fam_list) {
    Family *largest_fam = fam_list;
    Family *curr_fam = fam_list; 
    //Will return the first largest family
	while(curr_fam != NULL){
		if (curr_fam->num_words > largest_fam->num_words){
			largest_fam = curr_fam; 
		}
		curr_fam = curr_fam->next;
	}
	return largest_fam;
}


/* Deallocate all memory rooted in the List pointed to by fam_list. */
void deallocate_families(Family *fam_list) {
    Family *curr_fam = fam_list;
    Family *temp_fam; 
    while(curr_fam != NULL){
		temp_fam = curr_fam->next;
    	free(curr_fam->word_ptrs);
    	free(curr_fam);    
    	curr_fam = temp_fam; 
    	
	}
}

/*Updates word_sig into the signature of a word given the guessed letter

  Helper function for generate_families
*/
void generate_sig(char *word, char *word_sig, char letter){
	int i = 0;
	while(word[i]){
		if(word[i] == letter){
			word_sig[i] = letter;
		}
		else{
			word_sig[i] = '-'; 
		}
		i++;
	}
	word_sig[i] = '\0';
}


/* Generate and return a linked list of all families using words pointed to
   by word_list, using letter to partition the words.

   Implementation tips: To decide the family in which each word belongs, you
   will need to generate the signature of each word. Create only the families
   that have at least one word from the current word_list.
*/
Family *generate_families(char **word_list, char letter) {
	
	int index = 0;
	_Bool match;
	Family *fam_list = NULL; 
	Family *fam_end = fam_list; 
	
	char word_sig[strlen(word_list[0])+1]; 
	//Generate signature for word 
	while(word_list[index]){
		
		generate_sig(word_list[index], word_sig, letter);
		match = 0; 
		Family *curr_fam = fam_list;
		
		//Compare signature with current family signatures
		while(curr_fam != NULL){
			//If signature matches a family signatures, append word to fam
			if (strcmp(curr_fam->signature, word_sig) == 0){
				match = 1;
				add_word_to_family(curr_fam, word_list[index]);
			}
			
		}
		//If no matches then create a new fam with that signature and append it to linked list
		if(!match){
			//Check if this is first family 
			if (fam_list == NULL){
				fam_list = new_family(word_sig);
				fam_end = fam_list; 
			}
			else{
				fam_end->next = new_family(word_sig);
				fam_end = fam_end->next;
			}
			add_word_to_family(fam_end, word_list[index]);
		}
		
		index++; 
	}
	
    return fam_list;
}


/* Return the signature of the family pointed to by fam. */
char *get_family_signature(Family *fam) {
    return fam->signature;
}


/* Return a pointer to word pointers, each of which
   points to a word in fam. These pointers should not be the same
   as those used by fam->word_ptrs (i.e. they should be independently malloc'd),
   because fam->word_ptrs can move during a realloc.
   As with fam->word_ptrs, the final pointer should be NULL.
*/
char **get_new_word_list(Family *fam) {
	int count = fam->num_words; 
    char **ptr = malloc(sizeof(char*) *count);
    for(int i = 0; i < count; i++){
    	ptr[i] = fam->word_ptrs[i]; 
	}
	return ptr; 
}


/* Return a pointer to a random word from fam. 
   Use rand (man 3 rand) to generate random integers.
*/
char *get_random_word_from_family(Family *fam) {
	int random = rand() % fam->num_words;
    return fam->word_ptrs[random];
}
