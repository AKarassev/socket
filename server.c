/*----------------------------------------------
Serveur à lancer avant le client
------------------------------------------------*/
#include <stdlib.h>
#include <stdio.h>
#include <linux/types.h> 	/* pour les sockets */
#include <sys/socket.h>
#include <netdb.h> 		/* pour hostent, servent */
#include <string.h> 		/* pour bcopy, ... */  
#include <pthread.h> 

#define TAILLE_MAX_NOM      256
#define NB_CLIENTS_MAX      42


typedef struct sockaddr sockaddr;
typedef struct sockaddr_in sockaddr_in;
typedef struct hostent hostent;
typedef struct servent servent;

//Structure qui définit un client du serveur
typedef struct{
    int sock;               /*socket du client*/
    char pseudo[50];        /*pseudo du client*/
    pthread_t thread;       /*thread du serveur auquel est affecté le client*/
    int connected;         /*booléen indiquant si le client est connecté ou non*/
} Client;


//Variables globales pour pouvoir être récupérées depuis les threads
Client arrClient[NB_CLIENTS_MAX];           /*Tableau contenant tous les clients*/
int nb_client = 0;                          /*Nombre de clients connectés au serveur*/


    /*______________________________________________________________________________________*/
    /*                                                                                      */
    /*--------------------------METHODES DU SERVEUR-----------------------------------------*/
    /*______________________________________________________________________________________*/

//Envoie le message du client à tous les clients du serveur
void envoyer_message(Client * client, char buffer[]){
    char *answer = malloc (sizeof (*answer) * 256);

    strcpy(answer, (*client).pseudo);
    strcat(answer,": ");
    strcat(answer,buffer);
    printf("%s\n", answer);
    int i;
    for (i=0;i<nb_client;i++){
        if(strcmp((*client).pseudo,arrClient[i].pseudo)!=0){
            write(arrClient[i].sock,answer,strlen(answer)+1); 
        }     
    }    
}

//Change la couleur d'une chaîne de caractere
void coloriser(char* answer, char choix){

    char originale[10]  = "\x1B[0m" ;
    char rouge[10]      = "\x1B[31m";
    char vert[10]       = "\x1B[32m";
    char jaune[10]      = "\x1B[33m";
    char bleu[10]       = "\x1B[34m";
    char magenta[10]    = "\x1B[35m";
    char cyan[10] 		= "\x1B[36m";

    char couleur[10];

    switch(choix) {
        case 'o' :
            strcpy(couleur, originale);
            break;
        case 'r' :
            strcpy(couleur, rouge);
            break;
        case 'v' :
            strcpy(couleur, vert);
            break;
        case 'j' :
            strcpy(couleur, jaune);
            break;
        case 'b' :
            strcpy(couleur, bleu);
            break;
        case 'm' :
            strcpy(couleur, magenta);
            break;
        case 'c' :
            strcpy(couleur, cyan);
            break;                                                      
        default :
            printf("Choix de couleur invalide\n" );
    }


    char *res = malloc (sizeof (*res) * 256);
    strcpy(res, couleur);
    strcat(res, answer);
    strcat(res, originale);

    strcpy(answer, res);
}


static void * game (void * c){

	return;
}

//Supprime un clients= de la liste des clients connectés
void supprimerUtilisateur(Client *client_supprime){
	Client copyArray[NB_CLIENTS_MAX];
	int i,j = 0;
	int tmp_nb_client = nb_client;

	for(i; i<tmp_nb_client; i++){
		if(arrClient[i].sock != (*client_supprime).sock){
			printf("On ajoute %s\n", arrClient[i].pseudo);
			copyArray[j] = arrClient[i];
			j++;
		}
		else{
			printf("On supprime %s\n", arrClient[i].pseudo);
			close(arrClient[i].sock);
			nb_client--;
		}
	}
	// printf("test supprimerUtilisateur copyArray %i\n", copyArray[0].sock);

	memcpy(arrClient, copyArray, sizeof(arrClient));

	// printf("test supprimerUtilisateur arrClient %i\n", arrClient[0].sock);
}

//Liste tous les clients connectés dans une chaîne de caractère
char* listeClient(){
	char* res = malloc(nb_client*50*sizeof(char));
	strcpy(res,"Liste des clients connectés:");
	int i = 0;
	for(i; i<nb_client; i++){
		strcat(res, "\n-");
		strcat(res, arrClient[i].pseudo);
	}

	return res;
}



    /*______________________________________________________________________________________*/
    /*                                                                                      */
    /*--------------------------TRAITEMENT DES COMMANDES------------------------------------*/
    /*______________________________________________________________________________________*/

//Traite la commande reçue par le client
static void * commande (void * c){
    Client * client = (Client *) c;
	char buffer[256];
	char *answer = malloc (sizeof (*answer) * 256);
	int longueur;

    //Si le client n'a pas de pseudo
    while(strlen((*client).pseudo)<=1){
        longueur = read((*client).sock, buffer, sizeof(buffer));
        buffer[longueur]='\0'; 
        strcpy((*client).pseudo, buffer);
        write(1,buffer,longueur);
    }
	
	
    while(1){

    	longueur = read((*client).sock, buffer, sizeof(buffer));

    	//Permet de vider le buffer des ancinnes données
        buffer[longueur]='\0';    	// explicit null termination: updated based on comments

    	sleep(3);
    	// Quitter le serveur
    	if(strcmp(buffer,"/q")==0){
    		printf("%s a entré la commande /q\n", (*client).pseudo);
            strcpy(answer, (*client).pseudo);
            strcat(answer," a quitté le serveur.\n");
           	coloriser(answer, 'm');
           	envoyer_message(client, answer);
           	supprimerUtilisateur(client);
           	// free(client);  ERROR: free(): invalid pointer: 0x0000000000603180 ***
            pthread_exit(NULL);
    	}
    	// Lister les utilisateurs connectés
    	else if(strcmp(buffer,"/l")==0){
    		printf("%s a entre la commande /l\n", (*client).pseudo);
    		strcpy(answer, listeClient());
    		coloriser(answer, 'm');
    		write((*client).sock,answer,strlen(answer)+1); 
    	}
        //lancement du jeu
    	/*else if (strcmp(buffer,"/game")==0{
            //jeux();
        }*/
        //Cas d'un message normale
        else if(longueur > 0){
            envoyer_message(client, buffer);	
    	}



    }

    return;

}




    /*______________________________________________________________________________________*/
    /*______________________________________________________________________________________*/
    /*--------------------------------------------------------------------------------------*/ 


/*------------------------------------------------------*/






/*------------------------------------------------------*/
main(int argc, char **argv) {
  
    int             socket_descriptor, 			/* descripteur de socket */
	               	nouv_socket_descriptor, 	/* [nouveau] descripteur de socket */
			     	longueur_adresse_courante; 	/* longueur d'adresse courante d'un client */
    sockaddr_in 	adresse_locale, 			/* structure d'adresse locale*/
			        adresse_client_courant; 	/* adresse client courant */
    hostent*		ptr_hote; 					/* les infos recuperees sur la machine hote */
    servent*		ptr_service; 				/* les infos recuperees sur le service de la machine */
    char 			machine[TAILLE_MAX_NOM+1]; 	/* nom de la machine locale */

    pthread_t       thread_game;                /* thread du jeu*/


    
    


    gethostname(machine,TAILLE_MAX_NOM);		/* recuperation du nom de la machine */
    
    /* recuperation de la structure d'adresse en utilisant le nom */
    if ((ptr_hote = gethostbyname(machine)) == NULL) {
		perror("erreur : impossible de trouver le serveur a partir de son nom.");
		exit(1);
    }
    
    /* initialisation de la structure adresse_locale avec les infos recuperees */			
    
    /* copie de ptr_hote vers adresse_locale */
    bcopy((char*)ptr_hote->h_addr, (char*)&adresse_locale.sin_addr, ptr_hote->h_length);
    adresse_locale.sin_family		= ptr_hote->h_addrtype; 	/* ou AF_INET */
    adresse_locale.sin_addr.s_addr	= INADDR_ANY; 			/* ou AF_INET */


    /*utiliser un nouveau numero de port */
    adresse_locale.sin_port = htons(5000);
    /*-----------------------------------*/
    
    printf("numero de port pour la connexion au serveur : %d \n", 
		   ntohs(adresse_locale.sin_port) /*ntohs(ptr_service->s_port)*/);
    
    /* creation de la socket */
    if ((socket_descriptor = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("erreur : impossible de creer la socket de connexion avec le client.");
		exit(1);
    }

    /* association du socket socket_descriptor à la structure d'adresse adresse_locale */
    if ((bind(socket_descriptor, (sockaddr*)(&adresse_locale), sizeof(adresse_locale))) < 0) {
		perror("erreur : impossible de lier la socket a l'adresse de connexion.");
		exit(1);
    }
    







    /*______________________________________________________________________________________*/
    /*                                                                                      */
    /*---------------------------------SERVEUR EN ECOUTE------------------------------------*/
    /*______________________________________________________________________________________*/


    /* initialisation de la file d'ecoute */
    listen(socket_descriptor,5);

    /* attente des connexions et traitement des donnees recues */
    while(1) {
    
		longueur_adresse_courante = sizeof(adresse_client_courant);
		
		/* adresse_client_courant sera renseigné par accept via les infos du connect */
        /*on check s'il y a assez de place sur le serveur*/
		if (nb_client >= NB_CLIENTS_MAX) {
					perror("erreur : le serveur est saturé");
					exit(1);
		}
        else{
            //si le client n'est pas encore connecté
            if(arrClient[nb_client].connected == 0){
                if ((nouv_socket_descriptor = accept(socket_descriptor,(sockaddr*)(&adresse_client_courant),&longueur_adresse_courante))< 0) {
                    perror("erreur : impossible d'accepter la connexion avec le client.");
                    exit(1);
                }
                //création d'un thread pour le client
                else{
                    arrClient[nb_client].connected = 1;
                    arrClient[nb_client].pseudo[0] = '\0';
                    arrClient[nb_client].sock = nouv_socket_descriptor;
                    pthread_create(&arrClient[nb_client].thread, NULL, commande, &arrClient[nb_client]);
                    nb_client++;    //on incrémente le nombre de clients connectés sur le serveur
                }                
            }
        }



    }

    /*______________________________________________________________________________________*/
    /*______________________________________________________________________________________*/
    /*--------------------------------------------------------------------------------------*/ 

    
}
