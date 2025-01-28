//AUTOR: Ángela Sanz Miguel

//VERSIÓN PERMITIENDO VARIOS FICHEROS DE COMANDOS
#include "Funciones.cpp"
#include <fstream> // Necesario para ifstream 

int main(int argc, char *argv[]){
   
    char comando[MAX_CARACTERES+1];
    //La constante MAX_CARACTERES esta definida al comienzo del fichero funciones.cpp

    switch(argc){ //Nº de argumentos
        case 1:{ //VERSIÓN INTERACTIVA
            logo();
            while(true){
                mostrarPrompt(); //Mostramos el mensaje del prompt
                read(comando, MAX_CARACTERES);
                if(strcmp(comando, "salir")==0){
                    break;
                }
                ejecutar_multiples(comando);
                //ejecutar_multiples() -> permite ejecutar varios comandos escritos en una misma linea, separados por ";"
            }
            break;
        }

        default:{ //VERSIÓN VARIOS FICHEROS DE COMANDOS 
            ifstream entrada; //define un flujo de entrada
            for(int i=1; argv[i]!=nullptr; i++){ 
                //recorremos el array argv, que contendrá el nombre de los ficheros, hasta que se acabe, sus elementos apunten a nullptr
                entrada.open(argv[i]); //abre el fichero y lo asocia a entrada
                if(entrada.fail()){
                    cerr << "Error. No puede abrirse el archivo " << argv[i] << endl;
                    exit(1);
                } else{
                    //lee una línea de comando del fichero
                    entrada.getline(comando, MAX_CARACTERES+1, '\n');
                    while(!entrada.eof() && strcmp(comando, "salir")){ //mientras no se acabe el fichero, ni el comando sea salir
                        ejecutar_multiples(comando); //ejecuta la linea de comando 
                        //ejecutar_multiples() -> permite ejecutar varios comandos escritos en una misma linea, separados por ";"
                        entrada.getline(comando, MAX_CARACTERES+1, '\n'); //lee otra linea del fichero
                    }
                    entrada.close();
                }
            }
            break;
        }
    }
    return 0;
}