#include "argumentos.h"
#include "tipos.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/*
La función recorre argv y va validando los argumentos por este orden de precedencia m->f->archivos
-h necesita estar como unico argumento, sino devuelve error.
Ninguno es obligatorio, todos tienen valores por default
Si se invoca un flag (-m, -f), el siguiente argumento se toma como su opcion asignada.

Si no encuentra un argumento, inmediatamente chequea por el siguiente hasta alcanzar el final de argv.

./simpletron -f bin -m 20, detecta -f y espera encontrar archivos. Busca -m y 20 como archivos.
./simpletron dadd -f txt, busca dadd, -f y txt como archivos
./simpletron -m 20 b:hola , cumple el orden de precedencia. Almacena cant_mem = 20 y busca b:hola como archivo binario
./simpletron -f bin -   cumple el orden. OK
./simpletron b:stdin     ,intenta tomar como archivo de entrada a stdin en formato binario, por ahora lo tomé como error.
./simpletron t:stdin ó ./simpletron -           toman a stdin como archivo de entrada en formato de texto
*/

status_t validacion_cla(int argc, char** argv, params_t *param) {
    size_t i = 1, j, cant_archivos;
    bool_t stdin_flag = FALSE;
    long temp;
    archivo_t * aux = NULL;
    char * pc = NULL;
    /*---------------------------------VALIDACIONES---------------------------------*/
    /*Valido que los nombres no apunten a NULL para usar funciones get()*/
    if (argv == NULL || param == NULL)
        return ST_ERROR_PTR_NULO;
    for(j = 1; j < argc; j++){
        if( argv[j] == NULL)
            return ST_ERROR_PTR_NULO;
    }
    /*-----------------------------DEFAULT-----------------------------*/
    param->cant_memoria = DEFAULT_MEMORIA;
    param->cant_archivos = DEFAULT_CANT_ARCHIVOS;
    if ((param->archivo_salida = (archivo_t*) malloc(sizeof (archivo_t))) == NULL)
        return ST_ERROR_MEMORIA;
    param->archivo_salida->formato = FMT_TXT;
    param->archivo_salida->nombre = DEFAULT_ARCHIVO_SALIDA;
    param->archivo_salida->stream = NULL;
    if ((param->archivo_entrada = (archivo_t*) malloc(sizeof (archivo_t))) == NULL){
        destruir_params(param);
        return ST_ERROR_MEMORIA;
    }
    param->archivo_entrada->formato = FMT_TXT;
    param->archivo_entrada->nombre = DEFAULT_ARCHIVO_ENTRADA;
    param->archivo_entrada->stream = NULL;

    /*Comienzo a recorrer argv[] hasta llegar a argv[argc]*/
    if(i == argc)
        return ST_OK;
    /*---------------------------------AYUDA---------------------------------*/
    /*Forma de ejecutar: ./simpletron -h o --help */
    if (argc == 2 && (!strcmp(argv[i], FLAG_CLA_AYUDA_CORTO) || !strcmp(argv[i], FLAG_CLA_AYUDA_LARGO))){
        destruir_params(param);
        return ST_AYUDA;
    }
    /*---------------------------------MEMORIA---------------------------------*/
    if (!strcmp(argv[i], FLAG_CLA_MEMORIA_CORTO) || !strcmp(argv[i], FLAG_CLA_MEMORIA_LARGO)) {
        if(++i == argc){
            destruir_params(param);
            return ST_ERROR_MEMORIA_INGRESADA_INVALIDA;
        }
        temp = strtol(argv[i], &pc, 10);
        /*En el caso de que el usuario ingrese caracter alfabetico o pida memoria con decimales*/
        if (temp <= 0 || pc == NULL || (*pc != '\n' && *pc != '\0')){
            destruir_params(param);
            return ST_ERROR_MEMORIA_INGRESADA_INVALIDA;
        }
        param->cant_memoria = temp;
        
        if(++i == argc)
            return ST_OK;
    }

    /*---------------------------------FORMATO SALIDA---------------------------------*/
    /*Indica el formato de la salida. Si FMT es txt , el formato debe ser texto. Si*/
    /*FMT es bin, el formato debe ser binario. Por omisión, el formato es texto.*/
    /*Busco el flag de formato*/
    if (!strcmp(argv[i], FLAG_CLA_FORMATO_CORTO) || !strcmp(argv[i], FLAG_CLA_FORMATO_LARGO)) {
        if(++i == argc){
            destruir_params(param);
            return ST_ERROR_FORMATO_ARCHIVO_INVALIDO;
        }
        if(!strcmp(argv[i], FLAG_CLA_FORMATO_OPCION_BIN)){
            param->archivo_salida->formato = FMT_BIN;
        }
        else if(!strcmp(argv[i], FLAG_CLA_FORMATO_OPCION_TXT)){
            param->archivo_salida->formato = FMT_TXT;
        }
        else{
            destruir_params(param);
            return ST_ERROR_FORMATO_ARCHIVO_INVALIDO;
        }
        if(++i == argc)
            return ST_OK;
    }

    /*---------------------------------ARCHIVOS---------------------------------*/
    for (cant_archivos = 0; i < argc; i++){
        if ((aux = (archivo_t*) realloc(param->archivo_entrada, sizeof (archivo_t)*(++cant_archivos))) == NULL){
            destruir_params(param);
            return ST_ERROR_MEMORIA;
        }
        param->archivo_entrada = aux;
        param->archivo_entrada[cant_archivos - 1].nombre = get_name_lmsfile(argv[i]);
        param->archivo_entrada[cant_archivos - 1].formato = get_fmt_lmsfile(argv[i]);
        
        if(!stdin_flag && param->archivo_entrada[cant_archivos-1].nombre == NULL)
            stdin_flag = TRUE;
    }

    if (stdin_flag && (cant_archivos != 1 || param->archivo_entrada->formato == FMT_BIN)){
        destruir_params(param);
        return ST_ERROR_STDIN_INVALIDO;
    }
    param->cant_archivos = cant_archivos;
    
    return ST_OK;
}
char * get_name_lmsfile(char* name){
    if(!strncmp(name,FMT_BIN_DELIMITER,2) || !strncmp(name,FMT_TXT_DELIMITER,2))
        return name + 2;
    if(!strcmp(name,FLAG_CLA_STDIN_CORTO))
        return NULL;
    return name;
}
formato_t get_fmt_lmsfile(char* name){
    return strncmp(name,FMT_BIN_DELIMITER,2) ? FMT_TXT : FMT_BIN;
}
void destruir_params(params_t * param){
    if(param == NULL)
        return;
    free(param->archivo_entrada);
    free(param->archivo_salida);
    param->archivo_entrada = NULL;
    param->archivo_salida = NULL;
}
