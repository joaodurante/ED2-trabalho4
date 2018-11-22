/***************************************************************************************************
* @title Trabalho Aula 10 ED2
* @version 1.0
* @author Guilherme Krambeck
* @author João Pedro Durante
 ***************************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <conio.h>

/********************************* CONSTANTS *********************************/

#define BIBLIOTECA_FILENAME "resources/biblioteca.bin"
#define BUSCAR_FILENAME "resources/buscar.bin"
#define REMOVER_FILENAME "resources/remover.bin"
#define REGISTERS_FILENAME "registers.bin"
#define HASH_FILENAME "hash.bin"
#define WRITE_MODE "r+b"
#define CREATE_WRITE_MODE "w+b"
#define READ_MODE "rb"
#define VECTOR_SIZE 100
#define HASH_TABLE_SIZE 31
#define NO_KEY "@@"
#define DELETED_KEY "**"
#define ISBN_LENGTH 3
#define BIBLIOTECA_TEST_INDEX 1
#define BUSCAR_TEST_INDEX 2
#define REMOVER_TEST_INDEX 3
#define NIL -1

/********************************* REGISTERS *********************************/
struct book {
    char isbn[ISBN_LENGTH];
    char titulo[50];
    char autor[50];
    char ano[5];
};
typedef struct book Book;

struct testIndex {
	int biblioteca;
	int buscar;
	int remover;
};
typedef struct testIndex TestIndex;
#define HEADER_SIZE sizeof(TestIndex)

struct hash {
    char code[ISBN_LENGTH];
	int rrn;
};
typedef struct hash Hash;
#define HASH_SIZE sizeof(Hash)

/********************************* PROTOTYPES *********************************/
/**
 * Responsável pela exibicao e distribuicao das operacoes
 */
void menu();

/**
 * Aguarda o usuário pressionar qualquer tecla para
 * continuar a execução do programa
 */
void waitForUserResponse();

/**
 * Efetua a limpeza da tela
 */
void clearScreen();

/**
 * Efetua a abertua do arquivo no modo de abertura informado
 *
 * @param file ponteiro que receberá o arquivo aberto
 * @param filename nome do arquivo a ser aberto
 * @param openMode modo de abertura do arquivo
 * @param createIfNotExists flag para criar o arquivo caso o mesmo não esteja criado ainda
 * @return sucesso ou falha na abertura do arquivo
 */
bool openFile(FILE** file, const char* filename, bool writeMode, bool createIfNotExists);

/**
 * Carrega os arquivos
 *
 * @return verdadeiro se todos os arquivos foram carregados ou falso se falhou para carregar algum arquivo

 */
bool loadFilesToMemory();

/**
 * Inserir novos livros no sistema
 */
void insert();

/**
 * Incrementa o indice de registros no arquivo principal
 * @param file arquivo principal de registros
 * @param index índice atual dos registros
 * @param testIndexControl arquivo que será lido o indice
 */
void incrementNumberOfRegisters(FILE *file, int index, int testIndexControl);

/**
 * Funcao que pega a quantidade de registros escritos no arquivo principal
 * @param file arquivo principal com os registros
 * @param testIndexControl arquivo que será lido o indice
 * @return inteiro com a quantidade de registros  (indice)
 */
int getIndex(FILE* file, int testIndexControl);

/**
 * Funcao de insercao do ISBN na tabela de hash
 *
 * @param hashFile arquivo de indices hash
 * @param isbnCode ISBN que sera inserido
 * @param rrn rrn do registro no arquivo principal
 * @return verdadeiro se ocorreu com sucesso ou falso caso tenha
 * tido algum erro durante a insercao na tabela de hash
 */
bool insertToIndex(FILE *hashFile, const char *isbnCode, int rrn);

/**
 * Funcao hash
 *
 * @param key chave para ser transformada em hashCode
 * @return (hashCode) código de enderecamento da key na tabela de hash
 */
int hashCode(const char *key);

/**
 * Abre o arquivo de hash table e caso nao tenha sido criado, cria
 * e inicializa todas as HASH_SIZE posicoes com NO_KEY
 *
 * @param hashFile ponteiro que recebera o arquivo aberto
 */
void openHashFile(FILE **hashFile);

/**
 * Produra o isbnCode na tabela de hash e retorna o rrn armazenado
 *
 * @param hashFile ponteiro do arquivo de hash
 * @param isbnCode isbnCode a ser buscada na tabela de hash
 * @return rrn do registro no arquivo principal ou NIL caso nao seja encontrado
 */
int findInHashTable(FILE *hashFile, const char *isbnCode);

/**
 * Funcao que printa o registro de acordo com o offset recebido
 *
 * @param file arquivo a ser buscado com o offset
 * @param offset do registro no arquivo
 */
void printBook(FILE *file, int offset);

/**
 * Busca de ISBN no arquivo de registros utilizando indice Hash
 */
void searchByISBN();

/**
 * Funcao de remocao no arquivo hash
 */
void remove();
/********************************* GLOBAL VARIABLES *********************************/
Book biblioteca[VECTOR_SIZE];
char buscar[ISBN_LENGTH][VECTOR_SIZE];
char remover[ISBN_LENGTH][VECTOR_SIZE];

/**
 * Init function
 */
int main() {
	menu();
}

void menu(){
	bool allFilesLoaded = false;
    int resp;
    do {
    	clearScreen();
		printf("Menu");
		printf("\n1.Inserir");
		printf("\n2.Remover");
		printf("\n3.Buscar");
		printf("\n4.Carrega Arquivos");
		printf("\n0.Sair");
		printf("\nEscolha: ");
		scanf("%d", &resp);
		fflush(stdin);
        if(resp != 4 && resp != 0 && !allFilesLoaded){
            printf("\nE preciso carregar todos os arquivos de testes primeiro");
        }else{
            switch(resp){
                case 1:
                    insert();
                    break;
                case 2:
                    remove();
                    break;
                case 3:
                    searchByISBN();
                    break;
                case 4:
                    allFilesLoaded = loadFilesToMemory();
                    break;
                case 0:
                    return;
                default: printf("\nEscolha Invalida");
            }
        }
	    waitForUserResponse();
    } while(resp != 0);
}

void insert(){
	FILE *registersFile;
	if (!openFile(&registersFile, REGISTERS_FILENAME, true, true)){
		return;
	}
	int index = getIndex(registersFile, BIBLIOTECA_TEST_INDEX);
	Book book = biblioteca[index];
	if(strlen(book.isbn) == 0){
		printf("\nNao ha mais registros para inserir!");
		fclose(registersFile);
		return;
	}
	printf("\n%s", book.isbn);
	fseek(registersFile, 0, SEEK_END);
	int rrn = (ftell(registersFile)-HEADER_SIZE)/sizeof(Book);

	FILE *hashFile;
	openHashFile(&hashFile);

	if (insertToIndex(hashFile, book.isbn, rrn)){
		fwrite(&book, sizeof(Book), 1, registersFile);
		printf("\nChave %s inserida com sucesso", book.isbn);
	}
	incrementNumberOfRegisters(registersFile, index, BIBLIOTECA_TEST_INDEX);
	fclose(hashFile);
	fclose(registersFile);
}

bool insertToIndex(FILE *hashFile, const char *isbnCode, int rrn){
	int initialHashCode = hashCode(isbnCode);
	printf("\nEndereco %d", initialHashCode);
	for(int i=0;i<=HASH_TABLE_SIZE;i++){
		int hashCode = initialHashCode+i > HASH_TABLE_SIZE ? 0 : initialHashCode+i;
		if (i > 0){
			printf("\nColisao");
			printf("\nTentativa %d", i);
		}

		Hash hash;
		fseek(hashFile, hashCode * HASH_SIZE, 0);
		fread(&hash, HASH_SIZE, 1, hashFile);
		if (strcmp(hash.code, isbnCode) == 0){
			printf("\nErro: Chave %s duplicada\n", isbnCode);
			return false;
		}
		if (strcmp(hash.code, NO_KEY) == 0 || strcmp(hash.code, DELETED_KEY) == 0){
			fseek(hashFile, hashCode*HASH_SIZE, 0);
			strcpy(hash.code, isbnCode);
			hash.rrn = rrn;
			fwrite(&hash, HASH_SIZE, 1, hashFile);
			return true;
		}
	}
	printf("Todas as posicoes da tabela estao ocupadas, nao e possivel inserir mais registros!");
	return false;
}

void searchByISBN(){
	FILE *registersFile;
	if (!openFile(&registersFile, REGISTERS_FILENAME, true, true)){
		return;
	}

	int index = getIndex(registersFile, BUSCAR_TEST_INDEX);
	char isbn[ISBN_LENGTH];
	strcpy(isbn, buscar[index]);
	if(strlen(isbn) == 0){
		printf("\nNao ha mais registros para buscar!");
		fclose(registersFile);
		return;
	}
	printf("\n%s", isbn);

	FILE *hashFile;
	openHashFile(&hashFile);
	int rrn = findInHashTable(hashFile, isbn);
	if (rrn != NIL){
		int offset = (rrn * sizeof(Book)) + HEADER_SIZE;
		printBook(registersFile, offset);
	}
	incrementNumberOfRegisters(registersFile, index, BUSCAR_TEST_INDEX);
	fclose(hashFile);
	fclose(registersFile);
}

void printBook(FILE *file, int offset){
	Book book;
	fseek(file, offset, SEEK_SET);
	if(fread(&book, 1, sizeof(Book), file) == sizeof(Book)){
		if(strlen(book.isbn) > 0){
			printf("\n\nISBN: %s\n", book.isbn);
			printf("TITULO: %s\n", book.titulo);
			printf("AUTOR: %s\n", book.autor);
			printf("ANO: %s", book.ano);
		}
	}else{
		printf("\nErro ao obter as informacoes do arquivo de registros!");
	}
}

void openHashFile(FILE **hashFile){
	*hashFile = fopen(HASH_FILENAME, WRITE_MODE);
	if (*hashFile == NULL){
		if (!openFile(hashFile, HASH_FILENAME, true, true)){
			return;
		}else{
			Hash hash;
			strcpy(hash.code, NO_KEY);
			hash.rrn = NIL;
			for(int i=0;i<HASH_TABLE_SIZE;i++){
				fwrite(&hash, HASH_SIZE,1, *hashFile);
			}
		}
	}
}

int findInHashTable(FILE *hashFile, const char *isbnCode){
	int initialHashCode = hashCode(isbnCode);
	for(int i=0;i<=HASH_TABLE_SIZE;i++){
		int hashCode = initialHashCode+i > HASH_TABLE_SIZE ? 0 : initialHashCode+i;
		Hash hash;
		fseek(hashFile, hashCode * HASH_SIZE, 0);
		fread(&hash, HASH_SIZE, 1, hashFile);
		if (strcmp(hash.code, isbnCode) == 0){
			printf("\nChave %s encontrada, endereco %d, %d acessos", isbnCode, hashCode, i+1);
			return hash.rrn;
		}
	}
	printf("\nChave %s nao encontrada", isbnCode);
	return NIL;
}

int hashCode(const char *key){
	return atoi(key) % HASH_TABLE_SIZE;
}

int getIndex(FILE* file, int testIndexControl){
	TestIndex testIndex;
	rewind(file);
	if(!fread(&testIndex, sizeof(TestIndex), 1, file)){
		rewind(file);
		testIndex.biblioteca = 0;
		testIndex.buscar = 0;
		testIndex.remover = 0;
		fwrite(&testIndex, sizeof(TestIndex), 1, file);
	}
	switch (testIndexControl){
		case BIBLIOTECA_TEST_INDEX:
			return testIndex.biblioteca;
			break;
		case BUSCAR_TEST_INDEX:
			return testIndex.buscar;
			break;
		case REMOVER_TEST_INDEX:
			return testIndex.remover;
			break;
	}
	return NIL;
}

void incrementNumberOfRegisters(FILE* file, int index, int testIndexControl){
    rewind(file);
	TestIndex testIndex;
	index++;
	fread(&testIndex, sizeof(TestIndex), 1, file);
	switch (testIndexControl){
		case BIBLIOTECA_TEST_INDEX:
			testIndex.biblioteca = index;
			break;
		case BUSCAR_TEST_INDEX:
			testIndex.buscar = index;
			break;
		case REMOVER_TEST_INDEX:
			testIndex.remover = index;
			break;
	}
	rewind(file);
    fwrite(&testIndex, sizeof(TestIndex), 1, file);
}

bool loadFilesToMemory(){
	bool allFilesLoaded = true;
	FILE *bibliotecaFile, *buscarFile, *removerFile;
	int i=0;
	if (openFile(&bibliotecaFile, BIBLIOTECA_FILENAME, false, false)){
		i=0;
		while(fread(&biblioteca[i], sizeof(Book), 1, bibliotecaFile)){
			i++;
		}
		fclose(bibliotecaFile);
	}else{
		allFilesLoaded = false;
	}

	if (openFile(&buscarFile, BUSCAR_FILENAME, false, false)){
		i=0;
		while(fread(&buscar[i], sizeof(char), ISBN_LENGTH, buscarFile)){
			i++;
		}
		fclose(buscarFile);
	}else{
		allFilesLoaded = false;
	}

	if (openFile(&removerFile, REMOVER_FILENAME, false, false)){
		i=0;
		while(fread(&remover[i], sizeof(char), ISBN_LENGTH, removerFile)){
			i++;
		}
		fclose(removerFile);
	}else{
		allFilesLoaded = false;
	}
	if (allFilesLoaded){
        printf("\nTodos os arquivos foram carregados com sucesso!");
	}
	return allFilesLoaded;
}

bool openFile(FILE** file, const char* filename, bool writeMode, bool createIfNotExists){
	*file = fopen(filename, writeMode ? WRITE_MODE : READ_MODE);

	if (*file == NULL && createIfNotExists){
		*file = fopen(filename, CREATE_WRITE_MODE);
	}
	if (*file == NULL){
		printf("\nNao foi possivel abrir o arquivo %s", filename);
		return false;
	}
	return true;
}

void waitForUserResponse(){
    printf("\n\nPressione qualquer tecla para continuar...");
	getchar();
	//getchar();
}

void clearScreen(){
   system("cls");
   //system("clear");
}

void remove(){
	FILE *registersFile;
	if(!openFile(&registersFile, REGISTERS_FILENAME, true, false))
		return;
	int removeIndex = getIndex(registersFile, REMOVER_TEST_INDEX);
	char key[ISBN_LENGTH];
	strcpy(key, remover[removeIndex]);
	if(strlen(key) == 0){
		printf("\nNao ha mais livros a serem removidos!\n");
		fclose(registersFile);
		return;
	}
	FILE *hashFile;
	openHashFile(&hashFile);

	int hashPosition = findInHashTable(hashFile, key);
	if(hashPosition != NIL){
		int offset = ftell(hashFile) - HASH_SIZE;
		fseek(hashFile, offset, SEEK_SET);
		Hash hash;
		strcpy(hash.code, DELETED_KEY); 					  //sobrescreve com os digitos "**"
		hash.rrn = NIL; 									  //sobrescreve com -1
		fwrite(&hash, 1, HASH_SIZE, hashFile);
		printf("\nA chave %s foi excluida\n", key);
	}

	incrementNumberOfRegisters(registersFile, removeIndex, REMOVER_TEST_INDEX);
	fclose(hashFile);
	fclose(registersFile);
}







