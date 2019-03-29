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


void add_player(struct client **top, int fd, struct in_addr addr);
void remove_player(struct client **top, int fd);
void broadcast(struct game_state *game, char *outbuf);
void announce_turn(struct game_state *game);
void announce_winner(struct game_state *game, struct client *winner);
void advance_turn(struct game_state *game);

/*Attempt to write msg to client fd. 
 *If write falls then return client fd else return 0
*/
int write_to_client(int client_fd, char * msg){
	if (write(client_fd, msg, strlen(msg)) != strlen(msg)){
		return client_fd; 
	}
	return 0;
}

/* Send the message in outbuf to all clients */
void broadcast(struct game_state *game, char *outbuf, int exception_fd){
	for(p = game.head; p != NULL; p = p->next){
		if(p->fd != exception_fd){
			if (write_to_client(p->fd, outbuf) != 0){'
				char *remove_name = p->name; 
				remove_player(&game.head, p->fd);
				annouce_goodbye(game, remove_name);
			
			}
		}
	}
}

void annouce_goodbye(struct game_state *game, char *name){
	char goodbye[9 + len(name)];
	strcpy(goodbye, "Goodbye ");
	strcat(goodbye, name); 
	broadcast(game, goodbye, -1); 
}

void annouce_turn(struct game_state *game){
	for(p = game.head; p != NULL; p = p->next){
		if(p == game.has_next_turn){
			if (write_to_client(p->fd, YOUR_TURN) != 0){
				advance_turn(game);
				annouce_goodbye(game, p);
				remove_player(&game.head, p->fd); 
			} else{
				char turn_msg[14 + len(p->name)];
				strcpy(turn_msg, "It's ");
				strcat(turn_msg, p->name);
				strcat(turn_msg, "'s turn.");
				broadcast(game, turn_msg, p->fd); 
			}
		}
	}
}

/* Move the has_next_turn pointer to the next active client */
void advance_turn(struct game_state *game){
	game.has_next_turn = game.has_next_turn.next; 
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
                        
                        //If its the players turn handle their 
                        if(p == game.has_next_turn){
                        	//Loop until full line is read
                        	char buf[MAX_BUF] = {'\0'};
                        	int inbuf = 0;
                        	int nbytes; 
                        	int end; 
                        	while((nbytes = read(p->fd, buf, MAX_BUF)) > 0){
                        		inbuf += nbytes;  	
							 }
							 end = find_network_newline(buf, inbuf);
							 buf[end] = '\0';
                        	//Check if guess is valid
                        	if (strlen(buf) != 1 || buf[0] < 'a' || buf[0] > 'z'){
                        		//INVALID GUESS
							}else{
								//VALID GUESS
							}
							
                        //If it is not their turn, tell them its not their turn
						}else{
							if (write_to_client(p->fd, NOT_YOUR_TURN) != 0){
								//remove client
							}
							
						}
                        
                        
                        break;
                    }
                }
        
                // Check if any new players are entering their names
                for(p = new_players; p != NULL; p = p->next) {
                    if(cur_fd == p->fd) {
                        // TODO - handle input from an new client who has
                        // not entered an acceptable name.
                        char name_buf[MAX_NAME] = {'\0'};
                        int inbuf = 0;
                        int nbytes; 
                        int end; 
                        while((nbytes = read(p->fd, name_buf, MAX_NAME)) > 0){
                        	inbuf += nbytes;  	
						 }
						end = find_network_newline(name_buf, inbuf);
						name_buf[end] = '\0';
						strcpy(p->name, name_buf); 
						
                        //Add new player to head of in game list
                        //If current game list is empty 
                        if (game.head == NULL){
                        	game.has_next_turn = p; 
						}
                        p->next = game.head; 
                        game.head = p; 
                        //Need to remove p from new_players list!?!?!
                        break;
                    } 
                }
            }
        }
        annouce_turn(game);
    }
    return 0;
}

/*
 * Search the first n characters of buf for a network newline (\r\n).
 * Return one plus the index of the '\n' of the first network newline,
 * or -1 if no network newline is found.
 * Definitely do not use strchr or other string functions to search here. (Why not?)
 */
int find_network_newline(const char *buf, int n) {
    for (int i = 0; i < n; i++){
	if(buf[i] == '\r' && buf[i+1] == '\n'){
		return i; 
	}   
    }
    return -1;
}

