//AUTOR: Ángela Sanz Miguel

#include <iostream> //Necesario para cout
#include <cstring> //Necesario para la función strcpy
#include <cstdlib> //Necesario para la función exit
#include <unistd.h> //Proporciona acceso a las llamadas al sistema através de API POSIX
#include <sys/wait.h> //Necesario para la función wait
#include <fcntl.h>  // Necesario para llamadas sistema archivos: open, close...
using namespace std;

constexpr int MAX_CARACTERES = 255; //Constante que usaremos a lo largo de nuestro código para limitar la longitud máxima de una cadena

#define BLUE "\033[38;5;153m" //Definimos colores que usaremos para mostrar el logo de nuestra shell
#define YELLOW "\033[38;5;187m"
#define RESET "\033[0m" //Configuración de color por defecto

const char *homeDir = getenv("HOME"); //Dirección de nuestro directorio

/* CABECERA DE LAS FUNCIONES*/
void read(char [], int);
    //lee de la entrada estandar, una cadena de caracteres de la longitud indicada como segundo parametro (sin contar el \0)
void mostrar_trozos(char* []);
    //Permite imprimir un array de caracteres; mostrarlo como si fuese un "string"

int trocear_linea(char *, const char *, char ** &);
    //Divide la cadena que pasamos como primer parametro, por los caracteres pasados como segundo parametro, y lo devuelve en el tercer parametro
char *strtrim(char *);
    //elimina espacios en blanco y tabuladores, al comienzo y al final de la cadena

void getredir(char *std, char *&in, char *&out, char *&err, bool &modificador);
    //str ~ cadena que llega a la funcion (p.ej "cat<aa>bb")
    /*esultado del programa:
        std ~ mandato a ejecutarse (cat)
        in ~ puerto stdin (aa)
        out ~ puerto stdout (bb)
        err ~ puerto stderr 

    Versión modificada donde tenemos en cuenta si hay >> 
    Para ello el booleano modificador
        · modificador == FALSE -> es el caso '>' donde en el caso de existir el archivo de salida vaciaremos su contenido
        · modificador == TRUE -> es el caso '>>' donde en el caso de existir el archivo de salida, añadiremos la nueva información al final
    */
void ejecutar(char[]);  
    //Ejecuta un mandato simple, sin ninguna tubería
void ejecutar_pipe(char[], char[]); 
    /*Ejecuta una tubería, los dos parametros se corresponden con: 
        Primer parametro: primer mandato de la tubería, el que escribe en ella
        Segundo parametro: segundo mandato de la tubería, el que lee de ella
    Nota: los mandatos pueden tener su entrada y salida redirigida
    */
void comprobar_pipes(char []);
    /*Comprobamos si el mandato que vamos a ejecutar tiene o no tubería '|'
        · Si no tiene tubería, llamamos a la función ejecutar()
        · Si tiene tubería, llamamos a la función ejecutar_pipe();
        · Si hay más de un pipe, dará error, no contemplamos esa opción
    */
void ejecutar_multiples(char []);
    //Permite Ejecutar varios comandos escritos en al misma linea, separados por ";"

void redireccionarES(char *, char *, char*, bool);
    //Redireccionamos la entrada, la salida y la salida de error, en caso de que se haya indicado en el mandato

void mostrarPrompt();
    //Imprime por pantalla el prompt, en el inclulle la ruta del directorio actual
void logo(); 
    //Muestra el mensaje inicial de nuestra shell; lo mostraremos cuando se ejecute su versión interactiva:



void read(char str[], int tam){ //tam es el tamaño sin contar el \0
//Lee una cadena de caracteres de la longitud dada y evita los problemas de lectura
    if(cin.fail()){
        cin.clear();
        cin.ignore(256, '\n');
    }
    cin.getline(str, tam+1);
}

void mostrar_trozos(char *str[]){
    //Permite imprimir un array de caracteres
    for(int i=0; str[i]!=NULL; i++) { // El último trozo del array es siempre NULL
        cout << str[i] << endl;
    }
}

int trocear_linea(char *str, const char *delim, char **&parts) {
    //Divide la cadena que pasamos como primer parametro (str) por los caracteres pasados como segundo parametro (delim), y lo devuelve en el tercer parametro (parts)
    parts = (char **)malloc(sizeof(char *));
    parts[0] = NULL;

    if (str == NULL){
        return 0; // caso que no recibamos str
    }

    int numParts = 0;

    int i = 0;
    while (str[i] != '\0'){
        int j = i;
        // recorrer str hasta encontrar un delimitador o el final
        while (str[j] != '\0' && strchr(delim, str[j]) == NULL){
        j++;
        }

        if (str[j] != '\0') { // Si no se ha llegado al final
            str[j] = '\0'; // terminar la subcadena, sustituir el delimitador por un '\0'
            j++;      // Avanzar al siguiente caracter de la cadena str
        }

        if (str[i] != '\0'){ // Si la subcadena tiene caracteres
            // actualizar trozos
            numParts++;
            parts = (char **)realloc(parts, (numParts + 1) * sizeof(char *));
            parts[numParts - 1] = &str[i];
            parts[numParts] = NULL; // necesario para usar después con execvp
        }

        i = j; // Para continuar con el siguiente trozo
    }

    return numParts;
}

char *strtrim(char *str){
    //elimina espacios en blanco y tabuladores, al comienzo y al final de la cadena
    if (str == NULL){
        return NULL;
    }

    // Avanza por la izquierda
    int i = 0;
    while (str[i] == ' ' || str[i] == '\t'){
        i++;
    }

    // Avanza por la derecha
    int f = strlen(str) - 1;
    while (f > i && (str[f] == ' ' || str[f] == '\t')) {
        f--;
    }

    str[f + 1] = '\0'; // Poner el carácter de fin de cadena

    // Devuelve la dirección del nuevo string
    return &str[i];
}

void getredir(char *std, char *&in, char *&out, char *&err, bool &modificador){
    //str ~ cadena que llega a la funcion (p.ej "cat<aa>bb")
    /*Resultado del programa:
        std ~ mandato a ejecutarse (cat)
        in ~ puerto stdin (aa)
        out ~ puerto stdout (bb)
        err ~ puerto stderr 
    */

    /*Versión modificada donde tenemos en cuenta si hay >> 
    Para ello el booleano modificador
        · modificador == FALSE -> es el caso '>' donde en el caso de existir el archivo de salida vaciaremos su contenido
        · modificador == TRUE -> es el caso '>>' donde en el caso de existir el archivo de salida, añadiremos la nueva información al final

    */

    in = NULL; //Damos un valor por defecto
    out = NULL; 
    err = NULL;
    modificador=false;

    int i=0;
    while(std!=NULL && std[i]!='\0'){
        switch(std[i]){
            case '<':{
                in = &std[i+1];
                std[i] = '\0';
                break;
            }
            case '>':{
                std[i] = '\0'; //Haya dos o uno, se pondrá el fin de cadena en el primer > (o en el único que hay)
                //Comprobamos si hay ">" o ">>"
                if(std[i+1]=='>'){ //si el siguiente caracter también es >, entonces hay ">>"
                    i++; //cambiamos el valor de la i, para que el out comience en el siguiente elemento de la cadena después del segundo ">"
                    modificador=true; 
                }else{
                    modificador=false;
                }
                out = &std[i+1];
                break;
            }
            case '2':{
                if(std[i+1]=='>'){ //Tendrían que estar seguidos 2> para que indicase la salida de error
                    err = &std[i+2]; //contamos a partir del i+2, porque son dos caracteres ("2>")
                    std[i] = '\0'; //es en el primer caracter en el que "cerramos" la cadena anterior
                    i++; //Notar que tenemos que incrementar dos veces la i, por como hemos estructurado el codigo, en cada repeticion del while
                    //se incrementa una vez, por lo tanto, necesitamos incrementarlo otra
                    break;
                }
            }
        }
        i++;
    }
    in = strtrim(in); //eliminarmos los posibles espacios y tabulaciones que pueda tener al comienzo y al final
    if(in!=NULL && *in == '\0'){//en caso de que la cadena in este vacía (solo tenga un \0)
        in = NULL; //hacemos que apunte a NULL
    }
    out = strtrim(out); //eliminarmos los posibles espacios y tabulaciones que pueda tener al comienzo y al final
    if(out!=NULL && *out == '\0'){ //en caso de que la cadena in este vacía (solo tenga un \0)
        out = NULL; //hacemos que apunte a NULL
    }
    err = strtrim(err); //eliminarmos los posibles espacios y tabulaciones que pueda tener al comienzo y al final
    if(err!=NULL && *err == '\0'){ //en caso de que la cadena in este vacía (solo tenga un \0)
        err = NULL; //hacemos que apunte a NULL
    }
}

void ejecutar_multiples(char comando[]){ //Permite Ejecutar varios comandos escritos en al misma linea, separados por ";"
    //Separamos los posibles mandatos que pudiera haber en una linea (separados por ";")

    // Aprovechamos esta funcion para hacer el tratamiento de los comentarios
    /* Como mucho puede haber un comentario por linea, porque todo lo que se situe a la derecha de '#' (nuestro delimitador de comentarios)
       estará comentado y por lo tanto, no será ejecutado
    */

    for(int i=0; comando[i]!='\0'; i++){ 
        /*Lo tratamos directamente con un for, recorriendo la cadena, porque usar trocear_linea(), nos generaría problemas con el caso donde
          el '#' este colocado al comiendo de la linea*/
        if(comando[i]=='#'){
            comando[i]='\0';
            break;
        }
    }

    //Puntero doble a char = array de cadenas
    char **comandos_separados; 

    //Seprarmos los posibles mandatos que pudiera haber en la linea, dentro de la parte ejecutable (sin_comentarios[0]), no del comentario
    trocear_linea(comando, ";", comandos_separados); 

    for(int i=0; comandos_separados[i]!=nullptr; i++){ 
        //mientras que haya todavia mandatos en comandos_separados (comandos_separados[i]!=nullptr), los ejecutamos
        comprobar_pipes(comandos_separados[i]); //ejecuta cada uno de los mandatos
        //mandamos los comandos separados a comprobar_pipes(), que hará la distinción de tratamiento en función de si tienen tuberías o no
    }
}

void comprobar_pipes(char comando[]){ //Comprobamos si el mandato que vamos ha ejecutar tiene o no pipe
    /*  Si no tiene pipe, llamamos a la función ejecutar()
        Si si tiene pipe, llamamos a la funcion ejecutar_pipe()
        Si hay más de una pipe, dará un error, porque no contemplamos esa opción*/

    char **comandos_pipe;

    int numParts = trocear_linea(comando, "|", comandos_pipe);

    if(numParts>2){
        perror("Se ha introducido más de una tubería. Situación no contemplada");
    }else if(numParts==2){
        ejecutar_pipe(comandos_pipe[0], comandos_pipe[1]);
    }else{
        ejecutar(comandos_pipe[0]);
    }
}

void ejecutar(char comando[]){ //Función que nos permite ejecutar un mandato suelto (no esta conectado a otro por ninguna tubería)
    //PARTE 1. Construcción del mandato en trozos_cmd
    char *in, *out, *err; 
    bool modificador;
    getredir(comando, in, out, err, modificador); //Comprobamos si ha habido una redirección de la entrada o de la salida
    //si no se ha redireccionado la entrada o la salida, in y out apuntarán respectivamente a NULL

    char **trozos_cmd; //Puntero doble a char = array de cadenas
    trocear_linea(comando, " \t", trozos_cmd); //dividimos el array de caracteres que nos pasan (que contiene el comando) por sus espacios y tabuladores

    //PARTE 2. Ejecución del mandato en un nuevo proceso
    if(trozos_cmd[0]!=nullptr && strcmp(trozos_cmd[0], "cd")==0){ //COMANDO INTERNO
        if(trozos_cmd[1]==nullptr || strcmp(trozos_cmd[1], "~")==0){ //Si no se ha indicado ninguna ruta, o se ha puesto "~", se cambiará al directorio home
            if(homeDir!=nullptr){
                chdir(homeDir);
            }else{
                perror("Error. No se pudo obtener la dirección del directorio home del usuario");
            }
        }else{
            chdir(trozos_cmd[1]);
        }
    }
    else{ //RESTO DE COMANDOS
        pid_t pid = fork(); //Se crea un nuevo proceso
        switch(pid) {
            case -1: //ERROR
                perror("Error. No se ha podido crear un proceso con fork\n");
                break;

            case 0:{ //Proceso Hijo
                redireccionarES(in, out, err, modificador);
                execvp(trozos_cmd[0], trozos_cmd); //Cambiamos el código del proceso hijo
                perror("Error al ejecutar la función execvp");
                exit(2);  // llega aquí si hay error
                break;
            }

            default:{ //Proceso Padre
                wait(NULL); // espera a que termine el proceso hijo
                free(trozos_cmd); //Una vez terminado, liberar memoria reservada
                break;
            }
        }
    }
}

void ejecutar_pipe(char comando1[], char comando2[]){ //Función que nos permite ejecutar dos comandos unidos por una tubería
    //Comprobamos las redirecciones de entrada O de la salida
    char *in1, *out1, *err1; 
    char *in2, *out2, *err2; 
    bool modificador1, modificador2;

    getredir(comando1, in1, out1, err1, modificador1);
    getredir(comando2, in2, out2, err2, modificador2);
    //si no se ha redireccionado la entrada o la salida, in y out apuntarán respectivamente a NULL

    //tratamos los mandatos que tengan el formato adecuado
    //eliminamos los espacios y tabulaciones al comienzo y al final; y lo troceamos en partes
    char **trozos_cmd1;
    char **trozos_cmd2;

    trocear_linea(comando1, " \t", trozos_cmd1); //dividimos el array de caracteres que nos pasan (que contiene el comando) por sus espacios y tabuladores
    trocear_linea(comando2, " \t", trozos_cmd2); 
    
    //creamos un nuevo proceso
    pid_t pidP = fork(); //para no trabajar sobre la shell y en caso de que haya un error no se interrumpa
    switch(pidP){
        case -1: //ERROR
            perror("Error. No se ha podido crear un proceso con fork\n");
            break;
        case 0:{ //Proceso HIJO
            //corresponde con el proceso Padre que crea la tubería para compartir con sus hijos

            int tuberia[2]; //creamos la tubería
            if(pipe(tuberia)== -1){
                perror("Error. No se ha podido crear la tubería");
                exit(6);
            }

            //Creamos el Primer Proceso Hijo
            pid_t pid1 = fork();
            switch(pid1){
                case -1: // Error
                    perror("Error. No se ha podido crear un proceso con fork\n");
                    break;
                case 0:{ // PRIMER HIJO: ejecutará el primer mandato y escribirá en la tubería
                    close(tuberia[0]); //cerramos su extremo de letura de la tubería

                    //redireccionamos el extremo de escritura de la tubería a stdout
                    if(dup2(tuberia[1], 1)== -1){
                        perror("Error. No se pudo redireccionar stdout");
                        exit(4);
                    }

                    //stdout ya redireccionado, cerramos el descriptor out tubería 
                    close(tuberia[1]);

                    //Redireccionamos la entrada y salida, si se ha indicado en el mandato
                    redireccionarES(in1, out1, err1, modificador1);
                    execvp(trozos_cmd1[0], trozos_cmd1); //cambiamos el código del proceso hijo, al del primer comando
                    perror("Error al ejecutar la función execvp (primer mandato de una tubería)");
                    exit(2); //llega aqui si hay error
                    break;
                }
            }

            //Se crea el Segundo Proceso Hijo
            pid_t pid2 = fork();
            switch(pid2){
                case -1: // Error
                    perror("Error. No se ha podido crear un proceso con fork\n");
                    break;
                case 0:{ // SEGUNDO HIJO: ejecutará el segundo mandato y leerá de la tubería
                    close(tuberia[1]); //cerramos su extremo de escritura de la tuberia

                    //redireccionamos stdin al extremo de lectura de la tubería
                    if(dup2(tuberia[0], 0)== -1){
                        perror("Error. No se pudo redireccionar stdin");
                        exit(3);
                    }

                    //stdin ya redireccionado, cierra descriptor in tubería
                    close(tuberia[0]);

                    //Redireccionamos la entrada y salida, si se ha indicado en el mandato
                    redireccionarES(in2, out2, err2, modificador2);

                    execvp(trozos_cmd2[0], trozos_cmd2); //cambiamos el código del proceso hijo, al del segundo comando
                    perror("Error al ejecutar la función execvp (segundo mandato de una tubería)");
                    exit(2); //llega aqui si hay error
                    break;
                }
            }

            //el proceso padre cierra extreos de la tubería ya que ya no los usa
            close(tuberia[0]);
            close(tuberia[1]);

            //y espera a que terminen los dos hijos
            wait(NULL);
            wait(NULL);
            exit(0);
            break;
        }
        default:{ //Corresponde con el proceso padre (en este caso la shell)
            wait(NULL); //espera al proceso hijo
            //Si no esperaba al proceso hijo, se nos mostraba el prompt antes de que apareciese el resultado del mandato anterior
            free(trozos_cmd1); //Una vez terminado, liberar memoria reservada
            free(trozos_cmd2);
            break;
        }
    }

}

void redireccionarES(char *in, char *out, char *err, bool modificador){
    //Redireccionamos la entrada, la salida y la salida de error, en caso de que se haya indicado en el mandato
    int fd;
    if(in!=NULL){ //REDIRECCIONAR LA ENTRADA
        close(0); //cerramos la entrada estandar
        fd = open(in, O_RDONLY); // abre el archivo como sólo lectura
        if(fd == -1){
            perror("Error. No se pudo abrir el archivo para la lectura para redireccionar la entrada\n");
            exit(3);
        }

    }
    if(out!=NULL){ //REDIRECCIONAR LA SALIDA
        close(1); //cerramos la salida estandar
        
        if(modificador==true){
            fd = open(out,  O_WRONLY|O_CREAT|O_APPEND, 0644); //abrimos la salida indicada
            //Se abrirá como solo escritura (O_WRDONLY), si el archivo no existe se creará (O_CREAT)
        }
        else{
            fd = open(out,  O_WRONLY|O_CREAT|O_TRUNC, 0644); //abrimos la salida indicada
            //Se abrirá como solo escritura (O_WRDONLY), si el archivo no existe se creará (O_CREAT)
            //Y si el archivo ya existia, es un archivo regular y el modo de apertura permite escribir, entonces el archivo se vaciará (O_TRUNC)
        }
        if(fd == -1){
            perror("Error. No se pudo redireccionar la salida\n");
            exit(4);
        }
    }
    if(err!=NULL){ //REDIRECCIONAR LA SALIDA DE ERRORES
        close(2); //cerramos la salida de error estandar
        fd = open(err,  O_WRONLY|O_CREAT|O_TRUNC, 0644);//abrimos la salida indicada
        //Se abrirá como solo escritura (O_WRDONLY), si el archivo no existe se creará (O_CREAT)
        //Y si el archivo ya existia, es un archivo regular y el modo de apertura permite escribir, entonces el archivo se vaciará (O_TRUNC)
        if(fd == -1){
            perror("No se pudo redireccionar la salida de error\n");
            exit(5);
        }
    }

}

void mostrarPrompt(){
    char directorioTrabajo[MAX_CARACTERES]; //Buffer para almacenar la ruta de nuestro directorio de trabajo

    getcwd(directorioTrabajo, sizeof(directorioTrabajo));
    /*usamos la función char *getcwd(char *buf, size_t size);
        · buf es un puntero a buffer donde se almancenará la ruta del directorio actual
        · size es el tamaño del buffer en bytes
    */

    if(directorioTrabajo==NULL){ //Caso donde la funcion getcwd falle 
    //por overflow, buffer demasiado pequeño) o por incompatibilidad, esta disponible en la mayoría de sistemas UNIX y compatibles con POSIX)
        strcpy(directorioTrabajo, "");
    }

    if(homeDir!=nullptr && strcmp(directorioTrabajo, homeDir)==0){
        strcpy(directorioTrabajo, "~");
    }

    cout << directorioTrabajo  << "$ ";
}



void logo(){ //Muestra el mensaje inicial de nuestra shell; lo mostraremos cuando se ejecute su versión interactiva:
    const char *logo ="\n                                                                                              ..;===+.\n"
                        "                                                                                        .:=iiiiii=+=  \n"
                        "                                                                                     .=i))=;::+)i=+,  \n"
                        "                                                                                  ,=i);)I)))I):=i=;   \n"
                        "                                                                               .=i==))))ii)))I:i++    \n"
                        "                                                                             +)+))iiiiiiii))I=i+:'    \n"
                        "                                                        .,:;;++++++;:,.       )iii+:::;iii))+i='      \n"
                        "                                                     .:;++=iiiiiiiiii=++;.    =::,,,:::=i));=+'       \n"
                        "                                                   ,;+==ii)))))))))))ii==+;,      ,,,:=i))+=:         \n"
                        "                                                 ,;+=ii))))))IIIIII))))ii===;.    ,,:=i)=i+           \n"
                        "                                                ;+=ii)))IIIIITIIIIII))))iiii=+,   ,:=));=,            \n"
                        "                                              ,+=i))IIIIIITTTTTITIIIIII)))I)i=+,,:+i)=i+              \n"
                        "                                             ,+i))IIIIIITTTTTTTTTTTTI))IIII))i=::i))i='               \n"
                        "                                            ,=i))IIIIITLLTTTTTTTTTTIITTTTIII)+;+i)+i`                 \n"
                        "                                            =i))IIITTLTLTTTTTTTTTIITTLLTTTII+:i)ii:'                  \n"
                        "                                           +i))IITTTLLLTTTTTTTTTTTTLLLTTTT+:i)))=,                    \n"
                        "                                           =))ITTTTTTTTTTTLTTTTTTLLLLLLTi:=)IIiii;                    \n"
                        "                                          .i)IIITTTTTTTTLTTTITLLLLLLLT);=)I)))))i;                    \n"
                        "                                          :))IIITTTTTLTTTTTTLLHLLLLL);=)II)IIIIi=:                    \n"
                        "                                          :i)IIITTTTTTTTTLLLHLLHLL)+=)II)ITTTI)i=                     \n"
                        "                                          .i)IIITTTTITTLLLHHLLLL);=)II)ITTTTII)i+                     \n"
                        "                                          =i)IIIIIITTLLLLLLHLL=:i)II)TTTTTTIII)i'                     \n"
                        "                                        +i)i)))IITTLLLLLLLLT=:i)II)TTTTLTTIII)i;                      \n"
                        "                                      +ii)i:)IITTLLTLLLLT=;+i)I)ITTTTLTTTII))i;                       \n"
                        "                                     =;)i=:,=)ITTTTLTTI=:i))I)TTTLLLTTTTTII)i;                        \n"
                        "                                   +i)ii::,  +)IIITI+:+i)I))TTTTLLTTTTTII))=,                         \n"
                        "                                 :=;)i=:,,    ,i++::i))I)ITTTTTTTTTTIIII)=+'                          \n"
                        "                               .+ii)i=::,,   ,,::=i)))iIITTTTTTTTIIIII)=+                             \n"
                        "                              ,==)ii=;:,,,,:::=ii)i)iIIIITIIITIIII))i+:'                              \n"
                        "                             +=:))i==;:::;=iii)+)=  `:i)))IIIII)ii+'                                  \n"
                        "                           .+=:))iiiiiiii)))+ii;                                                      \n"
                        "                          .+=;))iiiiii)));ii+                                                         \n"
                        "                         .+=i:)))))))=+ii+                                                            \n"
                        "                        .;==i+::::=)i=;                                                               \n"
                        "                        ,+==iiiiii+,                                                                  \n"
                        "                        `+=+++;`                                                                      \n";

    const char *nombre = "         _                   _             _    _        _         _            _       _    _            _             _                          \n"
                        "        / /\\                /\\ \\     _    /\\ \\ /\\ \\     /\\_\\      / /\\         / /\\    / /\\ /\\ \\         _\\ \\          _\\ \\       \n"
                        "       / /  \\              /  \\ \\   /\\_\\ /  \\ \\\\ \\ \\   / / /     / /  \\       / / /   / / //  \\ \\       /\\__ \\        /\\__ \\       \n"
                        "      / / /\\ \\            / /\\ \\ \\_/ / // /\\ \\_\\\\ \\ \\_/ / /     / / /\\ \\__   / /_/   / / // /\\ \\ \\     / /_ \\_\\      / /_ \\_\\   \n"
                        "     / / /\\ \\ \\          / / /\\ \\___/ // / /\\/_/ \\ \\___/ /     / / /\\ \\___\\ / /\\ \\__/ / // / /\\ \\_\\   / / /\\/_/     / / /\\/_/     \n"
                        "    / / /  \\ \\ \\        / / /  \\/____// / / ______\\ \\ \\_/      \\ \\ \\ \\/___// /\\ \\___\\/ // /_/_ \\_\\/  / / /         / / /            \n"
                        "   / / /___/ /\\ \\      / / /    / / // / / /\\_____\\\\ \\ \\        \\ \\ \\     / / /\\/___/ // /____/\\    / / /         / / /                 \n"
                        "  / / /_____/ /\\ \\    / / /    / / // / /  \\/____ / \\ \\ \\   _    \\ \\ \\   / / /   / / // /\\____\\/   / / / ____    / / / ____              \n"
                        " / /_________/\\ \\ \\  / / /    / / // / /_____/ / /   \\ \\ \\ /_/\\__/ / /  / / /   / / // / /______  / /_/_/ ___/\\ / /_/_/ ___/\\              \n"
                        "/ / /_       __\\ \\_\\/ / /    / / // / /______\\/ /     \\ \\_\\\\ \\/___/ /  / / /   / / // / /_______\\/_______/\\__\\/_______/\\__\\/          \n"
                        "\\_\\___\\     /____/_/\\/_/     \\/_/ \\/___________/       \\/_/ \\_____\\/   \\/_/    \\/_/ \\/__________/\\_______\\/    \\_______\\/           \n";

    cout << YELLOW << logo << RESET << endl;
    cout << BLUE << nombre << RESET << endl;
}