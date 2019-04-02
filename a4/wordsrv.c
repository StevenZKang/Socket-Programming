#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <time.h>

#include "socket.h"
#include "gameplay.h"


#ifndef PORT
    #define PORT y
#endif
#define MAX_QUEUE 5

int write_to_client(int *client_fd, char * msg);
void add_player(struct client **top, int fd, struct in_addr addr);
void remove_player(struct client **top, int fd);
void activate_player(struct game_state *game, struct client **top, struct client *p);
void broadcast(struct game_state *game, char *outbuf, int exception_fd);
void announce_turn(struct game_state *game);
void announce_winner(struct game_state *game, struct client *winner);
void announce_join(struct game_state *game, struct client *p);
void announce_goodbye(struct game_state *game, char *name);
void advance_turn(struct game_state *game);
void clean_list(struct client **top);
int find_network_newline(const char *buf, int n);

/*Attempt to write msg to client fd. 
 *If write falls then set client_fd to -1 and return client fd, else return 0
*/
int write_to_client(int *client_fd, char * msg){
	char network_msg[strlen(msg) + 2];
	strcpy(network_msg,msg); 
	strcat(network_msg, "\r\n");

	if (write(*client_fd, network_msg, strlen(network_msg)) != strlen(network_msg)){
		*client_fd = -1; 
		return *client_fd; 
	}
	return 0;
}

/* Send the message in outbuf to all clients */
void broadcast(struct game_state *game, char *outbuf, int exception_fd){
	
	for(struct client *p = game->head; p != NULL; p = p->next){
		if(p->fd != exception_fd && p->fd != -1){
			if (write_to_client(&(p->fd), outbuf) != 0){
				announce_goodbye(game, p->name);
			}
		}
	}
}

void announce_join(struct game_state *game, struct client *p){
	//Broadcast new player has joined
	char join_msg[strlen(p->name) + 15];
	strcpy(join_msg, p->name);
	strcat(join_msg, " has joined.");
	broadcast(game, join_msg, -1); 
	//Write game status to new player 
	char status_msg[MAX_MSG];
	if (write_to_client(&(p->fd), status_message(status_msg, game)) != 0){
		announce_goodbye(game, p->name);
	}
	char turn_msg[16 + strlen(game->has_next_turn->name)];
	strcpy(turn_msg, "It's ");
	strcat(turn_msg, game->has_next_turn->name);
	strcat(turn_msg, "'s turn.");
	if (write_to_client(&(p->fd), turn_msg) != 0){
		announce_goodbye(game, p->name);
	}
}

void announce_goodbye(struct game_state *game, char *name){
	char goodbye[9 + strlen(name)];
	strcpy(goodbye, "Goodbye ");
	strcat(goodbye, name); 
	broadcast(game, goodbye, -1); 
}

void announce_turn(struct game_state *game){
	for(struct client *p = game->head; p != NULL; p = p->next){
		if(p == game->has_next_turn){
			if (write_to_client(&(p->fd), YOUR_TURN) != 0){
				advance_turn(game);
				announce_goodbye(game, p->name); 
			} else{
				char turn_msg[14 + strlen(p->name)];
				strcpy(turn_msg, "It's ");
				strcat(turn_msg, p->name);
				strcat(turn_msg, "'s turn.");
				broadcast(game, turn_msg, p->fd); 
			}
		}
	}
	//Prompt server
	printf("It's %s's turn", game->has_next_turn->name);
}

void announce_winner(struct game_state *game, struct client *p){
	
		if (write_to_client(&(p->fd), YOU_WIN) != 0){
			announce_goodbye(game, p->name);
		}
		char loser_msg[31 + strlen(p->name) + strlen(game->word)];
		strcpy(loser_msg, "The word was ");
		strcat(loser_msg, game->word); 
		strcat(loser_msg, ".\nGame Over! ");
		strcat(loser_msg, p->name);
		strcat(loser_msg, " wins!");
		broadcast(game, loser_msg, p->fd); 
}

/* Move the has_next_turn pointer to the next active client */
void advance_turn(struct game_state *game){
	
	if(game->has_next_turn->next == NULL){
		game->has_next_turn = game->head; 
	}else{
		game->has_next_turn = game->has_next_turn->next; 
	}
}


/* The set of socket descriptors for select to monitor.
 * This is a global variable because we need to remove socket descriptors
 * from allset when a write to a socket fails.
 */
fd_set allset;


/* Add a client to the head of the linked list
 */
void add_player(struct client **top, int fd, struct in_addr addr) {
    struct client *p = malloc(sizeof(struct client));

    if (!p) {
        perror("malloc");
        exit(1);
    }

    printf("Adding client %s\n", inet_ntoa(addr));

    p->fd = fd;
    p->ipaddr = addr;
    p->name[0] = '\0';
    p->in_ptr = p->inbuf;
    p->inbuf[0] = '\0';
    p->next = *top;
    *top = p;
}

/* Removes client from the linked list and closes its socket.
 * Also removes socket descriptor from allset 
 */
void remove_player(struct client **top, int fd) {
    struct client **p;

    for (p = top; *p && (*p)->fd != fd; p = &(*p)->next)
        ;
    // Now, p points to (1) top, or (2) a pointer to another client
    // This avoids a special case for removing the head of the list
    if (*p) {
        struct client *t = (*p)->next;
        printf("Removing client %d %s\n", fd, inet_ntoa((*p)->ipaddr));
        FD_CLR((*p)->fd, &allset);
        close((*p)->fd);
        free(*p);
        *p = t;
    } else {
        fprintf(stderr, "Trying to remove fd %d, but I don't know about it\n",
                 fd);
    }
}

/* Removes client from the new_players list and adds client to the
 * head of the active players list. 
 */
void activate_player(struct game_state *game, struct client **top, struct client *p){
	
	//Remove p from new_players list
	if (*top == p){
		*top = p->next; 
	}else{
		struct client *curr; 	
		for (curr = *top; curr->next != p ; curr = curr->next);
		curr->next = p->next; 
	}
	//Add p to head of active list
 	//If current game list is empty 
    if (game->head == NULL){
        game->has_next_turn = p; 
	}
    p->next = game->head; 
    game->head = p;	
}


void clean_list(struct client **top){
	
	for (struct client * p = *top; p != NULL; p = p->next){
		if(p->fd == -1){
			printf("Disconnect from %s\n", inet_ntoa(p->ipaddr)); 
			remove_player(top, p->fd); 
		}
	}
}

int main(int argc, char **argv) {
    int clientfd, maxfd, nready;
    struct client *p;
    struct sockaddr_in q;
    fd_set rset;
    
    if(argc != 2){
        fprintf(stderr,"Usage: %s <dictionary filename>\n", argv[0]);
        exit(1);
    }
    
    // Create and initialize the game state
    struct game_state game;

    srandom((unsigned int)time(NULL));
    // Set up the file pointer outside of init_game because we want to 
    // just rewind the file when we need to pick a new word
    game.dict.fp = NULL;
    game.dict.size = get_file_length(argv[1]);

    init_game(&game, argv[1]);
    
    // head and has_next_turn also don't change when a subsequent game is
    // started so we initialize them here.
    game.head = NULL;
    game.has_next_turn = NULL;
    
    /* A list of client who have not yet entered their name.  This list is
     * kept separate from the list of active players in the game, because
     * until the new playrs have entered a name, they should not have a turn
     * or receive broadcast messages.  In other words, they can't play until
     * they have a name.
     */
    struct client *new_players = NULL;
    
    struct sockaddr_in *server = init_server_addr(PORT);
    int listenfd = set_up_server_socket(server, MAX_QUEUE);
    
    // initialize allset and add listenfd to the
    // set of file descriptors passed into select
    FD_ZERO(&allset);
    FD_SET(listenfd, &allset);
    // maxfd identifies how far into the set to search
    maxfd = listenfd;
    
    //Input buffering variables
	char letter_guess[MAX_BUF];
    int inbuf = 0;
    int nbytes = 0;
   	int end;
   	
    while (1) {
        // make a copy of the set before we pass it into select
        rset = allset;
        nready = select(maxfd + 1, &rset, NULL, NULL, NULL);
        if (nready == -1) {
            perror("select");
            continue;
        }

        if (FD_ISSET(listenfd, &rset)){
            printf("A new client is connecting\n");
            clientfd = accept_connection(listenfd);

            FD_SET(clientfd, &allset);
            if (clientfd > maxfd) {
                maxfd = clientfd;
            }
            printf("Connection from %s\n", inet_ntoa(q.sin_addr));
            add_player(&new_players, clientfd, q.sin_addr);
            char *greeting = WELCOME_MSG;
            if(write(clientfd, greeting, strlen(greeting)) == -1) {
                fprintf(stderr, "Write to client %s failed\n", inet_ntoa(q.sin_addr));
                remove_player(&(game.head), p->fd);
            };
        }
        
        /* Check which other socket descriptors have something ready to read.
         * The reason we iterate over the rset descriptors at the top level and
         * search through the two lists of clients each time is that it is
         * possible that a client will be removed in the middle of one of the
         * operations. This is also why we call break after handling the input.
         * If a client has been removed the loop variables may not longer be 
         * valid.
         */
        int cur_fd;
        for(cur_fd = 0; cur_fd <= maxfd; cur_fd++) {
            if(FD_ISSET(cur_fd, &rset)) {
                // Check if this socket descriptor is an active player
                
                for(p = game.head; p != NULL; p = p->next) {
                    if(cur_fd == p->fd) {
                        //TODO - handle input from an active client
                        
                        //*******HANDLE PLAYER TURN*************
                        if(p == game.has_next_turn){
                        		//Read in from fd. 
                        		if( (nbytes = read(p->fd, &letter_guess[inbuf], MAX_BUF)) <= 0){
                        				p->fd = -1; 
										announce_goodbye(&game, p->name);
										advance_turn(&game);
										announce_turn(&game);
										break; 
										
								}
								printf("[%i] Read %i bytes", p->fd, nbytes);
								inbuf += nbytes; 		
								//Check if \r\n has been found then handle input. 
								if ((end = find_network_newline(letter_guess, inbuf)) != -1 && p->fd != 0){
									letter_guess[end] = '\0';
									printf("[%i] Found newline %s", p->fd, letter_guess);
									//Check if input is valid
		                        	if (strlen(letter_guess) != 1 || letter_guess[0] < 'a' || letter_guess[0] > 'z' || game.letters_guessed[letter_guess[0] - 'a'] == 1){
		                        		//INVALID GUESS
		                        		if(write_to_client(&(p->fd), INVALID_GUESS) != 0){
		                        			announce_goodbye(&game, p->name);
											advance_turn(&game);
											announce_turn(&game);
											break;  
										}
									}else{
										//VALID GUESS
										advance_turn(&game); 
										char * won; 
										if(player_guess(&game, letter_guess[0]) == 0){
											//Missed Guess
											printf("Letter %c is not in the word", letter_guess[0]);
											char miss_msg[21];
											strcpy(miss_msg, letter_guess);
											strcat(miss_msg, " is not in the word\r\n"); 
											if (write_to_client(&(p->fd), miss_msg) != 0){
												announce_goodbye(&game, p->name);
											}
										}else{
											//Correct Guess, check for winner
											won = strchr(game.guess, '-');
											if (won == NULL){
												broadcast(&game, game.word, -1); 
												announce_winner(&game, p);
												init_game(&game, argv[1]);
												break; 
											}
										}
										
										//Check if players have lost 
										if(game.guesses_left == 0){
											char answer_msg[31 + strlen(game.word)];
											strcpy(answer_msg, "No more guesses. The word was ");
											strcat(answer_msg, game.word);
											broadcast(&game, answer_msg, -1); 
											init_game(&game, argv[1]);
										}
										//Game is not over, Broadcast new status message 	
										else{
											char status_msg[MAX_MSG];
											broadcast(&game, status_message(status_msg, &game), -1); 
										}
										announce_turn(&game); 

									}
									
									//RESET THE BUFFER VARIABLE
								    inbuf = 0;
								}
                        		
	                        	
							
                        	
                        //If it is not their turn, clear their buffer tell them its not their turn
						}else{
							read(p->fd, p->inbuf, MAX_BUF);
							if(write_to_client(&(p->fd), NOT_YOUR_TURN) != 0){
								announce_goodbye(&game, p->name);
							}	
						}
                        
                        
                        break;
                    }
                }//End of active player handling block. 
        
                // Check if any new players are entering their names
                for(p = new_players; p != NULL; p = p->next) {
                    if(cur_fd == p->fd) {
                        // TODO - handle input from an new client who has
                        // not entered an acceptable name.
						int namebytes;
                        if( (namebytes = read(p->fd, p->in_ptr, MAX_BUF)) <= 0){
                        		remove_player(&new_players, p->fd); 
								break; 
						}
						printf("[%i] Read %i bytes", p->fd, namebytes);
						p->in_ptr += namebytes; 		
						//Check if \r\n has been found then handle input. 
						int name_end; 
						if ((name_end = find_network_newline(p->inbuf, p->in_ptr - p->inbuf)) != -1 && p->fd != 0){
								p->inbuf[name_end] = '\0';
								printf("[%i] Found newline %s", p->fd, p->inbuf); 
								//Check for valid name 
								if (strlen(p->inbuf) > 1 && unique_name(game.head, p->inbuf)){
									strcpy(p->name, p->inbuf); 
									activate_player(&game, &new_players, p);
									announce_join(&game, p); 
								}else{
									if (write_to_client(&(p->fd), INVALID_NAME) != 0){
										remove_player(&new_players, p->fd); 
										break;
									}  
								}
								p->in_ptr = p->inbuf; 
						}
                        
                        break;
                    } 
                }
           	 }
        }
        //Remove all clients with -1 fd.
        clean_list(&(game.head));
    }
    return 0;
}

/*
 * Search the first n characters of buf for a network newline (\r\n).
 * Return the index of the '\r' of the first network newline,
 * or -1 if no network newline is found.
 */
int find_network_newline(const char *buf, int n) {
    for (int i = 0; i < n; i++){
	if(buf[i] == '\r' && buf[i+1] == '\n'){
		//Annouce found newline to server
		return i; 
	}   
    }
    return -1;
}

