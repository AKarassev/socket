/*-----------------------------------------------------------
Client a lancer apres le serveur avec la commande :
client <adresse-serveur> <message-a-transmettre>
------------------------------------------------------------*/
#include <stdlib.h>
#include <stdio.h>
#include <linux/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>

typedef struct sockaddr 	sockaddr;
typedef struct sockaddr_in 	sockaddr_in;
typedef struct hostent 		hostent;
typedef struct servent 		servent;


/*________________________________________________________*/
/*                                                        */
/*-----------------METHODES DU CLIENT---------------------*/
/*________________________________________________________*/

//Le client est toujours en écoute des envois du serveur
static void * ecoute (void * socket_descriptor){
    int longueur;
    int * socket = (int *) socket_descriptor;
    char buffer[256];
    while(1){
        longueur = read(socket, buffer, (int)sizeof(buffer));
        buffer[longueur]='\0';
        printf("%s \n", buffer);
    }
}

/*________________________________________________________*/
/*--------------------------------------------------------*/



int main(int argc, char **argv) {
  
    int     		socket_descriptor, 	/* descripteur de socket */
		    		longueur; 		    /* longueur d'un buffer utilisé */
    sockaddr_in 	adresse_locale; 	/* adresse de socket local */
    hostent *		ptr_host; 		    /* info sur une machine hote */
    servent *		ptr_service; 		/* info sur service */
    char 			buffer[256];
    char *			prog; 			    /* nom du programme */
    char *			host; 			    /* nom de la machine distante */
    char  			mesg[256]; 			/* message envoyé */

    char			pseudo[50];
    pthread_t       thread_ecoute;      /*Thread sur lequel le client sera en écoute du serveur*/
    
    //renvoie une erreur s'il n'y a pas assez d'aguments
    /*if (argc != 3) {
		perror("usage : client <adresse-serveur> <message-a-transmettre>");
		exit(1);
    }*/
    if (argc != 2) {
		perror("usage : client <adresse-serveur>");
		exit(1);
    }
   
    prog = argv[0];
    host = argv[1];
    //mesg = argv[2];
    
    printf("nom de l'executable : %s \n", prog);
    printf("adresse du serveur  : %s \n", host);
    //printf("message envoye      : %s \n", mesg);
    
    //renvoie une erreur, si pas de correspondance pour l'hôte
    if ((ptr_host = gethostbyname(host)) == NULL) {
		perror("erreur : impossible de trouver le serveur a partir de son adresse.");
		exit(1);
    }
    
    /* copie caractere par caractere des infos de ptr_host vers adresse_locale */
    bcopy((char*)ptr_host->h_addr, (char*)&adresse_locale.sin_addr, ptr_host->h_length);
    adresse_locale.sin_family = AF_INET; /* ou ptr_host->h_addrtype; */
    
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
    
    /*-----------------------------------------------------------*/
 









    /*________________________________________________________*/
    /*                                                        */
    /*-----------------CONNEXION AU SERVEUR-------------------*/
    /*________________________________________________________*/

    /* SOLUTION 2 : utiliser un nouveau numero de port */
    adresse_locale.sin_port = htons(5000);
    /*-----------------------------------------------------------*/
    printf("numero de port pour la connexion au serveur : %d \n", ntohs(adresse_locale.sin_port)); 
    /* creation de la socket */
    if ((socket_descriptor = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("erreur : impossible de creer la socket de connexion avec le serveur.");
		exit(1);
    }   
    /* tentative de connexion au serveur dont les infos sont dans adresse_locale */
    if ((connect(socket_descriptor, (sockaddr*)(&adresse_locale), sizeof(adresse_locale))) < 0) {
		perror("erreur : impossible de se connecter au serveur.");
		exit(1);
    }   
    printf("connexion etablie avec le serveur. \n\n\n");


    /*________________________________________________________*/
    /*--------------------------------------------------------*/







    /*________________________________________________________*/
    /*                                                        */
    /*-----------------DIALOGUE DANS LE CHAT------------------*/
    /*________________________________________________________*/

    
	printf("Bienvenue dans le chat 		serveur: %s\n", host);
	printf("__________________________________________________\n\n");
    
    //Le client doit entrer son pseudo
    printf("Entrez votre pseudo: \n");
    fgets(pseudo, sizeof pseudo, stdin);
    pseudo[strcspn(pseudo, "\n")] = '\0'; //enlève le caractère de saut de ligne 
    // scanf("%s", pseudo);
    if ((write(socket_descriptor, pseudo, strlen(pseudo))) < 0) {
            perror("erreur : impossible d'ecrire le message destine au serveur.");
            exit(1);
    }
    printf("/q          - Quitter le serveur\n");
    printf("/game       - Lancer le jeu\n");
    printf("/endgame    - Arrêter le jeur\n");
    printf("/h          - Afficher les commandes\n");
    printf("__________________________\n\n");
    printf("Bien le bonjour %s !\n", pseudo);

    //Le client se met en écoute
    pthread_create(&thread_ecoute, NULL, ecoute, socket_descriptor);

    //Le client peut désormais envoyer des messages au serveur
    //Il pourra quitter en écrivant la commande "/q" 
	while(strcmp(mesg,"/q")!=0){
        
        fgets(mesg, sizeof(mesg), stdin);
        //avec fgets() le caractere de saut de ligne '\n' est rajouté,
        //il faut s'en débarrassé pour faire fonctionner strcmp
        mesg[strcspn(mesg, "\n")] = '\0'; //enlève le caractère de saut de ligne 

		//Le client envoie le message
		if ((write(socket_descriptor, mesg, strlen(mesg))) < 0) {
			perror("erreur : impossible d'ecrire le message destine au serveur.");
			exit(1);
    	}
	}
	printf("Vous quittez le chat.\n");
    printf("__________________________________________________\n\n");

    /*________________________________________________________*/
    /*--------------------------------------------------------*/
    







    printf("\nfin de la reception.\n");
    
    close(socket_descriptor);
    
    printf("connexion avec le serveur fermee, fin du programme.\n");
    

    exit(0);
    
}

