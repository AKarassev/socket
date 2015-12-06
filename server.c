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
#include <stdlib.h>         /*pour le random*/
#include <unistd.h>


#define TAILLE_MAX_NOM      256
#define NB_CLIENTS_MAX      42


typedef struct sockaddr sockaddr;
typedef struct sockaddr_in sockaddr_in;
typedef struct hostent hostent;
typedef struct servent servent;

//Strucutre qui définit le type utilisé pour le jeu
typedef struct
{
    char question[200];     /*Question*/
    char reponse[200];      /*Réponse*/
}quest_rep;

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


int in_game;                                /*Booléen pour savoir si le jeu est lancé*/
Client * maitre_jeu;                        /*Mémoire de celui qui a démarré le jeu*/






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
            if((write(arrClient[i].sock,answer,strlen(answer)+1)) < 0){
                perror("erreur : impossible d'ecrire le message destine au serveur.");
                exit(1);
            } 
        }     
    }    
}




//Envoie un message à tous les clients du serveur de la part du serveur
void message_serv(char buffer[]){
    char *answer = malloc (sizeof (*answer) * 256);
    strcat(answer,buffer);
    printf("%s\n", answer);
    int i;
    for (i=0;i<nb_client;i++){
            write(arrClient[i].sock,answer,strlen(answer)+1); 
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
    Client * client = (Client *) c;
    quest_rep questionnaire[10];
    quest_rep Question;
    int random;
    char * rep; 
    //Faire l'affichage réponse : /a:réponse
    message_serv("Pour répondre entrez /a:[reponse]");
    random=rand()%(10-0) +0;
    strcpy(Question.question,"2+3");
    strcpy(Question.reponse,"5");
    questionnaire[0]=Question;
    strcpy(Question.question,"Capital de la France");
    strcpy(Question.reponse,"Paris");
    questionnaire[1]=Question;
    strcpy(Question.question,"Qui joue L'empereur dans Gladiator");
    strcpy(Question.reponse,"Joaquim Phoenix");
    questionnaire[2]=Question;
    strcpy(Question.question,"Quel est la nationalité de Charles Darwin");
    strcpy(Question.reponse,"Anglaise");
    questionnaire[3]=Question;
    strcpy(Question.question,"Le prénom d'Alzeihmer");
    strcpy(Question.reponse,"Alois");
    questionnaire[4]=Question;
    strcpy(Question.question,"10*5-32");
    strcpy(Question.reponse,"18");
    questionnaire[5]=Question;
    strcpy(Question.question,"Quel est la Nationalité de Mario");
    strcpy(Question.reponse,"Italien");
    questionnaire[6]=Question;
    strcpy(Question.question,"Qui a écrit Le Petit Prince");
    strcpy(Question.reponse,"Antoine de Saint-Exupéry");
    questionnaire[7]=Question;
    strcpy(Question.question,"Quel est la nationalité du Baron Rouge");
    strcpy(Question.reponse,"Allemand");
    questionnaire[8]=Question;
    strcpy(Question.question,"Qui a tué Achille pendant le siège de Troie");
    strcpy(Question.reponse,"Pâris");
    questionnaire[9]=Question;
    //Afficher la première question
    message_serv(questionnaire[random].question);
    while(1){
        char buffer[256];
        char *answer = malloc (sizeof (*answer) * 256);
        int longueur;
        int in_game=0;
        longueur = read((*client).sock, buffer, sizeof(buffer));

        //Permet de vider le buffer des ancinnes données
        buffer[longueur]='\0';      // explicit null termination: updated based on comments

        sleep(3);
        rep = malloc (256*sizeof(char));
        strcpy(rep,"/a:");
        strcat(rep,questionnaire[random].reponse);
        if(longueur > 0){
            envoyer_message(client, buffer);
            if  (strcmp(buffer,rep)==0){
                //Afficher Bonne réponse
                message_serv("Bonne réponse");
                random=rand()%(10-0) +0;
                //Afficher la question
                message_serv(questionnaire[random].question);
            }
        }
    }
}

//Liste tous les clients connectés dans une chaîne de caractère
char* listeClient(){
    char* res = malloc(nb_client*50*sizeof(char));
    strcpy(res,"Liste des clients connectés:");
    int i = 0;
    for(i; i<nb_client; i++){
        strcat(res, "\n - ");
        strcat(res, arrClient[i].pseudo);
    }
    printf("\n");

    return res;
}



    /*______________________________________________________________________________________*/
    /*______________________________________________________________________________________*/
    /*--------------------------------------------------------------------------------------*/ 








    /*______________________________________________________________________________________*/
    /*                                                                                      */
    /*--------------------------METHODES ADMIN ---------------------------------------------*/
    /*______________________________________________________________________________________*/



//Vérifie le statut de la connection des clients connectés
char* checkSocketStatus(){
    int error = 0;
    socklen_t len = sizeof (error);
    int retval;
    char* str = malloc(nb_client*256*sizeof(char));
    char* tmp = malloc(256*sizeof(char));
    strcpy(str, "Analyse de l'état des connexions\n\n");

    int i = 0;
    for(i; i<nb_client; i++){
        sprintf( tmp, "\nUtilisateur n°%i\n", i);
        strcat(str, tmp);
        sprintf(tmp, "Pseudo: %s\n", arrClient[i].pseudo);
        strcat(str, tmp);
        retval = getsockopt (arrClient[i].sock, SOL_SOCKET, SO_ERROR, &error, &len);
        if (retval != 0) {
        /* there was a problem getting the error code */
            fprintf(stderr, "Erreur sur le code erreur du socket: %s\n", strerror(retval));
        }
        if (error != 0) {
            /* socket has a non zero error status */
            sprintf(tmp, "Erreur de socket: %s\n\n", strerror(error));
            strcat(str, tmp);
        }
        else{
            sprintf(tmp, "La connection est OK.\n\n");
            strcat(str, tmp);
        }       
    }
    return str;
}



//Affiche les informations relatives au clients connectés
void listerInfo(){
    int i = 0;
    for(i; i<nb_client; i++){
        printf("\nUtilisateur %i\n", i);
        printf("Pseudo: %s\n", arrClient[i].pseudo);
        printf("Socket: %i\n", arrClient[i].sock);
        printf("Connecté: %i\n\n", arrClient[i].connected);
    }
}



//Supprime un clients= de la liste des clients connectés
/*BUG : après supression, l'utilisateur suivant peut toujours recevoir les messages,
        mais ne peut plus en émettre. On perd églament la main sur le serveur*/
void supprimerUtilisateur(Client *client_supprime){
    Client copyArray[NB_CLIENTS_MAX];
    int i,j = 0;
    int tmp_nb_client = nb_client;
    for(i; i<tmp_nb_client; i++){
        if(arrClient[i].sock != (*client_supprime).sock){
            //printf("On ajoute %s\n", arrClient[i].pseudo);
            copyArray[j] = arrClient[i];
            j++;
        }
        else{
            //printf("On supprime %s\n", arrClient[i].pseudo);
            close(arrClient[i].sock);
            nb_client--;
        }
    }

    memcpy(arrClient, copyArray, sizeof(arrClient));
}
    /*______________________________________________________________________________________*/
    /*______________________________________________________________________________________*/
    /*--------------------------------------------------------------------------------------*/ 







    /*______________________________________________________________________________________*/
    /*                                                                                      */
    /*--------------------------TRAITEMENT DES COMMANDES------------------------------------*/
    /*______________________________________________________________________________________*/

//Traite les commandes reçues par le client
static void * commande (void * c){
    Client * client = (Client *) c;
	char buffer[256];
	char *answer = malloc (sizeof (*answer) * 256);
	int longueur;
    pthread_t thread_game;                /* thread du jeu*/

    in_game=0;

    //Si le client n'a pas de pseudo
    while(strlen((*client).pseudo)<=1){
        longueur = read((*client).sock, buffer, sizeof(buffer));
        sleep(3);
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
    		printf("%s a entré la commande /l\n", (*client).pseudo);
    		strcpy(answer, listeClient());
    		coloriser(answer, 'v');
    		write((*client).sock,answer,strlen(answer)+1); 
    	}
        else if(strcmp(buffer,"/h")==0){
            printf("%s a entré la commande /h\n", (*client).pseudo);
            strcpy(answer, "__________________________\n                          \n/q          - Quitter le serveur\n/l          - Lister les utilisateurs connectés\n/game       - Lancer le jeu\n/endgame    - Arrêter le jeur\n/h          - Afficher les commandes\n__________________________\n\n");
            coloriser(answer, 'v');
            write((*client).sock,answer,strlen(answer)+1);  
        }
        //lancement du jeu
        else if (strcmp(buffer,"/game")==0){
            if (in_game==0){
                pthread_create(&thread_game, NULL, game, NULL);
                in_game=1;}
        }
        else if (strcmp(buffer,"/endgame")==0) {
            if (in_game==1){
                //suppression du thread
                in_game=0;}
        }          
        //Cas d'un message normale
        else if(longueur > 0){
            envoyer_message(client, buffer);	
    	}



    }


}




//Un thread qui surveille les commandes faites sur le serveur
static void * commandeServeur (void * socket_serveur){
    int* socket = (int *) socket_serveur;
    char* cmd = malloc(16*sizeof(char));
    char* mesg = malloc(2048*sizeof(char));
    char* help = malloc(1024*sizeof(char));
    strcpy(help, "__________________________\n");
    strcat(help, "                          \n");
    strcat(help, "/q          - Arrêter le serveur\n");
    strcat(help, "/cs         - Vérifier le statut des connexions\n");
    strcat(help, "/l          - Lister les informations des cliens\n");
    strcat(help, "/h          - Lister les commandes\n");

    while(1){
        fgets(cmd, sizeof(cmd), stdin);
        cmd[strcspn(cmd, "\n")] = '\0'; //enlève le caractère de saut de ligne 

        if(strcmp(cmd,"/q")==0){
            printf("/q entrée\n");
            strcpy(mesg, "Arrêt du serveur, connection interrompue.");
            coloriser(mesg, 'r');
            int i = 0;

            for(i; i<nb_client; i++){
                if((write(arrClient[i].sock,mesg,strlen(mesg)+1)) < 0){
                        perror("erreur : impossible d'ecrire le message destine au serveur.");
                        exit(1);
                }
                close(arrClient[i].sock); //fermeture du socket du client
            } 
            sleep(3);
            close(*socket) ; //fermeture du socket serveur
            printf("Arrêt du serveur\n");
            exit(0);          
        }
        else if(strcmp(cmd,"/cs")==0){
            printf("/cs entrée\n");
            strcpy(mesg, checkSocketStatus());
            printf("%s\n", mesg);
        }
        else if(strcmp(cmd,"/l")==0){
            printf("/l entrée\n");
            strcpy(mesg, listeClient());
            coloriser(mesg, 'j');
            printf("%s\n", mesg);
        }
        else if(strcmp(cmd,"/h")==0){
            printf("/h entrée\n");
            coloriser(help, 'j');
            printf("%s\n", help);
        }
    }

}



    /*______________________________________________________________________________________*/
    /*______________________________________________________________________________________*/
    /*--------------------------------------------------------------------------------------*/ 


/*------------------------------------------------------*/





    /*______________________________________________________________________________________*/
    /*                                                                                      */
    /*------------------------------------MAIN----------------------------------------------*/
    /*______________________________________________________________________________________*/
    /*______________________________________________________________________________________*/
    /*______________________________________________________________________________________*/
    /*--------------------------------------------------------------------------------------*/ 


int main(int argc, char **argv) {
  
    int             socket_descriptor, 			/* descripteur de socket */
	               	nouv_socket_descriptor, 	/* [nouveau] descripteur de socket */
			     	longueur_adresse_courante; 	/* longueur d'adresse courante d'un client */
    sockaddr_in 	adresse_locale, 			/* structure d'adresse locale*/
			        adresse_client_courant; 	/* adresse client courant */
    hostent*		ptr_hote; 					/* les infos recuperees sur la machine hote */
    servent*		ptr_service; 				/* les infos recuperees sur le service de la machine */
    char 			machine[TAILLE_MAX_NOM+1]; 	/* nom de la machine locale */

    pthread_t       thread_cmd;                 /* thread des commandes du serveur */


    
    


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

    //Thread permettant de lancer des commandes dans le temrinal
    pthread_create(&thread_cmd, NULL, commandeServeur, &socket_descriptor);

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

    return 0;  
}

    /*______________________________________________________________________________________*/
    /*______________________________________________________________________________________*/
    /*--------------------------------------------------------------------------------------*/ 
    /*______________________________________________________________________________________*/
    /*______________________________________________________________________________________*/
    /*--------------------------------------------------------------------------------------*/ 
