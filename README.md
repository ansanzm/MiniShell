
# **INSTRUCCIONES de la MINISHELL**

Trabajo de la asignatura Sistemas Operativos. Realizado el 1º semestre del curso 2024-2025.

Guia para la compilación y el uso de la minishell en sus modos interactivo y fichero de comandos.

## **Instrucciones de Construcción**

### **Requisitos Previos**

Para poder hacer uso del programa es necesario:

* Disponer de un compilador C++, como por ejemplo `g++`. Si no está instalado, se puede añadir ejecutando el siguiente mandato en la terminal:
    ```
    sudo apt install g++
    ```

### **Pasos para Construir el Programa**

1) Descarga en tu ordenador los ficheros `minishell.cpp` y `Funciones.cpp`. Asegúrate de guardar ambos dentro de la misma carpeta.

2) **Comando de Compilación.** Ejecuta el siguiente comando en la terminal para compilar el programa. 
    ```
    g++ minishell.cpp -o minishell
    ```
    Debes estar situado en el directorio donde se encuentran los archivos fuente, o en su defecto, sustituir el nombre del fichero, por su ruta absoluta o relativa. 
    
    También puedes modificar el nombre de tu futuro archivo ejecutable. Simplemente tienes que sustituir la palabra "minishell", por el nombre de tu preferencia.

    *Nota.* No es necesario compilar el archivo `Funciones.cpp`por separado, ya que su contenido está incluido en el archivo principal mediante `#include`, y por lo tanto el compilador procesa su contenido automáticamente. 

3) Como resultado del paso anterior, se ha generado un archivo ejecutable llamado `minishell` (si no especificaste otro nombre). Ya podríamos ejecutarlo.

<br>

## **Manual de Uso**

Nuestra shell dispone de dos modos de uso: el modo interactivo y modo de ficheros de comandos. A continuación explicaremos como hacer uso de ambos. 

### **Modo Interactivo**

1) **Inicio de la Minishell.**

    Ejecuta el archivo ejecutable que hemos generado en el apartado anterior, sin ningún argumento.

    ````
    ./minishell
    ````

2) **Interacción con la Minishell.** 

    En primer lugar, se mostrará un texto de bienvenida que incluye un logo y el nombre de la minishell. 

    Posteriormente aparecerá el prompt. Se trata de un texto que se muestra en la pantalla para indicar que la shell está lista para recibir comandos. 
    
    Generalmente, el prompt muestra información sobre el estado actual. En nuestro caso, está conformado por la ruta del directorio de trabajo actual seguida del símbolo `$`.

    ````
    /ruta/del/directorio$
    ````
    * `/ruta/del/directorio` : es la ruta absoluta del directorio donde te encuentras dentro del sistema de archivos.
    * `$` : indica que la shell está lista para recibir comandos

    Asimismo, si te encuentras en el directorio home del usuario, como prompt se mostrará:
    ```
    ~$
    ```
    * `~` : representa el directorio principal de usuario (equivalente a `/home/usuario`)

    <br>

    Escribe comandos directamente en el prompt, por ejemplo:
    ````
    ls -l
    ````
    ```` 
    cd /home
    ````

3) **Salida de la Minishell**

    Escribe `salir` en el prompt de la minishell para terminar su ejecución. 

<br>

### **Modo Fichero de Comandos**

1) **Inicio de la Minishell.** 

    Para iniciar la minishell en modo fichero de comandos, ejecuta el archivo ejecutable en la terminal proporcionando uno o más archivos que contengan los comandos a ejecutar, separados por espacios.

    ````
    ./minishell comandos1.sh comandos2.sh
    ````

    *Atención.* Si uno de esos archivos falla al abrirse, se interrumpirá la ejecución inmediatamente después de que se generé el problema. Es decir, los archivos posteriores al que produjo un error de apertura, no se ejecutarán.

2) **Ejecución**

    La minishell leerá cada archivo proporcionado, ejecutando los comandos línea por línea. 

    La ejecución de un fichero se detendrá automáticamente una vez se procesen todos los comandos de un archivo. Para posteriormente, pasar a ejecutar el siguiente fichero de mandatos en caso de que existiera. De modo que la ejecución finalizará completamente, cuando todos los ficheros hayan sido ejecutados. 

    Se puede interrumpir la ejecución de un archivo añadiendo `salir` como mandato dentro del guión. En ese caso finalizará la ejecución de dicho archivo, y pasará a ejecutarse el siguiente fichero, en caso de que lo hubiera. 


## **Funcionalidades Incorporadas**

* **Ejecución de Varios Mandatos en una Línea separados por `;`**

    La shell, permite tanto en el modo interactivo, como en el modo fichero de comandos, ejecutar varios mandatos situados en una misma línea, siempre y cuando estén separados por el caracter `;`. Por ejemplo:
    ````
        ls -l; cat fichero.txt
    ````

    <br>

* **Comentarios `#`**
    
    Nuestra shell, permite la introducción de comentarios precedidos por el caracter `#`. Una vez, se introduzca este caracter en una línea de mandatos, todo el contenido que se encuentre a su derecha, no será ejecutado. Por ejemplo:
    ````
    #echo hola
    ````
    La cadena "echo hola" es un comentario
    ````
    echo hola #saludos
    ````
    Solo la cadena "saludo" es un comentario

    <br>

* **Mandato interno `cd`**

    Se ha incluido el mandato interno `cd <path>` que permite modificar el directorio de trabajo actual, por el directorio de la ruta indicada en `<path>`.
    ````
    cd /home/usuario/documentos
    ````

    Asimismo, el comando puede utilizarse sin especificar `<path>`, o usando el símbolo especial `~` (`cd ~`). En ambos casos se cambiará al directorio home del usuario. 

    <br>

* **Redirección de la Entrada, la Salida y la Salida de Error Estándar**

    Se permite redirigir la entrada, la salida y la salida de error estándar, de los mandatos introducidos, utilizando la siguiente notación:

    * `< archivo` : utiliza archivo como entrada estándar abriéndolo para lectura

        ````
        sort < lista.txt
        ````
        Este comando toma el contenido de `lista.txt` como entrada para el comando `sort`

        <br>


    * `> archivo` : utiliza archivo como salida estándar. Si el archivo no existiera, se creará uno nuevo con el nombre indicado. En caso de que ya existiese, se eliminará el contenido previo del fichero. 
        ````
        echo "Hola" > salida.txt
        ````
        Escribe "Hola" en el archivo `salida.txt`. Si el archivo ya existe, su contenido se sobrescribirá. Sino, se creará un fichero con ese nombre que contenga el mensaje indicado.

    <br>

    * `>> archivo` : utiliza archivo como salida estándar. Si el archivo no existiera, se creará uno nuevo con el nombre archivo. En caso de que ya existiese, se añadirá la salida del mandato al final del mismo. 

        ````
        echo "Nuevo Texto" >> salida.txt
        ````
        Escribe "Nuevo Texto" en el archivo `salida.txt`. Si el archivo ya existe, su contenido se añadirá al final. Sino, se creará un fichero con ese nombre que contenga el mensaje indicado.
    
    <br>

    * `2> archivo` : utiliza archivo como salida de error estándar.Si el archivo no existiera, se creará uno nuevo con el nombre indicado. En caso de que ya existiese, se eliminará el contenido previo del fichero. 

    ````
    ls archivoInexistente 2> errores.txt
    ````

    Lista un archivo que no existe, lo que generará un error, que será guardado en el archivo `errores.txt`

    <br>

* **Implementación de Una Tubería**

    La shell admite dos comandos separados por el símbolo `|`, de manera que la salida estándar del primer comando se conectará mediante una tubería a la entrada estándar del segundo mandato. 

    Asimismo, es posible redirigir tanto la entrada, como la salida, como la salida de error de los comandos conectados. `

    Algunos ejemplos: 

    ````
    ls -l | grep documento
    ````
    Lista los archivos del directorio con `ls -l`, se lo pasa al mandato `grep` que filtra las líneas que contienen "documento", el resultado de `grep` será lo que se muestre por pantalla. 

    ```
    cat archivo.txt | sort > ordenado.txt
    ```
    Toma mediante `cat` el contenido de `archivo.txt`, lo ordena con `sort` y guarda el resultado en `ordenado.txt`

<br>

## **Códigos de Salida**

La shell devuelve un código de salida para indicar si el proceso terminó correctamente o ocurrió algún error. A continuación, se detallan los códigos de error que nuestra shell genera:

* **Error 1.** No puede abrirse el archivo

* **Error 2.** Error al ejecutar la función `execvp()`

* **Error 3.** No se pudo redireccionar la entrada estándar.

* **Error 4.** No se pudo redireccionar la salida estándar.

* **Error 5.** No se pudo redireccionar la salida de error estándar

* **Error 6.** No se pudo crear una tubería 
