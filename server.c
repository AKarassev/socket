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
#define TAILLE_MAX_NOM 256
#define NB_CLIENTS_MAX 42

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


Client arrClient[NB_CLIENTS_MAX];           /*Tableau contenant tous les clients*/
int nb_client = 0;                          /*Nombre de clients connectés au serveur*/



/*------------------------------------------------------*/
//Renvoie le message reçu par le client
void renvoi (int sock) {

    char buffer[256];
    char *answer = malloc (sizeof (*answer) * 256);
    int longueur;
   
    if ((longueur = read(sock, buffer, sizeof(buffer))) <= 0) 
    	return;
    
    printf("message lu : %s \n", buffer);
    
    answer[0] = 'R';
    answer[1] = 'E';
    answer[2] = '#';
    answer = strcat(answer, buffer);
    answer[longueur+4] ='\0';
    
    printf("message apres traitement : %s \n", answer);
    
    printf("renvoi du message traite.\n");

    /* mise en attente du programme pour simuler un delai de transmission */
    sleep(3);
    
    write(sock,answer,strlen(answer)+1);
    free(answer);
    printf("message envoye. \n");

    return;
    
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
    printf("Test taille pseudo %i\n", strlen((*client).pseudo)==0);
    printf("el pseudo %s\n", (*client).pseudo);
    while(strlen((*client).pseudo)<=1){
        longueur = read((*client).sock, buffer, sizeof(buffer));
        buffer[longueur]='\0'; 
        printf("Le pseudo %s\n", buffer);
        strcpy((*client).pseudo, buffer);
        write(1,buffer,longueur);
    }
	
	
    while(1){

    	longueur = read((*client).sock, buffer, sizeof(buffer));

    	//Permet de vider le buffer des ancinnes données
        buffer[longueur]='\0';    	// explicit null termination: updated based on comments
        // printf("%s\n",buffer); 		// print the current receive buffer with a newline
        // fflush(stdout);         	// make sure everything makes it to the output
        // buffer[0]='\0';

    	printf("message lu : %s \n", buffer);

    	sleep(3);
    	//Si le buffer contient la commande de sortie
    	if(strcmp(buffer,"/q")==0){
            printf("Le client quitte le chat.\n");
            answer = "Au revoir!\n";
            write((*client).sock,answer,strlen(answer)+1);
            pthread_exit(NULL);
    	}
        //lancement du jeu
    	/*else if (strcmp(buffer,"/game")==0{
            //jeux();
        }*/
        //Cas du message normale
        else if(longueur > 0){
            printf("renvoi\n");
            strcpy(answer, (*client).pseudo);
            strcat(answer,": ");
            strcat(answer,buffer);
            strcat(answer,"\n");
            int i;
            for (i=0;i<nb_client;i++){
               write(arrClient[i].sock,answer,strlen(answer)+1); 
            }	
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

    /* 2 facons de definir le service que l'on va utiliser a distance */
    /* (commenter l'une ou l'autre des solutions) */
    
    /*-----------------------------------------------------------*/
    /* SOLUTION 1 : utiliser un service existant, par ex. "irc" */
    /*
    if ((ptr_service = getservbyname("irc","tcp")) == NULL) {
		perror("erreur : impossible de recuperer le numero de port du service desire.");
		exit(1);
    }
    adresse_locale.sin_port = htons(ptr_service->s_port);
    */
    /*-----------------------------------------------------------*/
    /* SOLUTION 2 : utiliser un nouveau numero de port */
    adresse_locale.sin_port = htons(5000);
    /*-----------------------------------------------------------*/
    
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
