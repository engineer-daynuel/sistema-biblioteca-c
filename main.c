#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h> 
#include <ctype.h> 

// DEFINICOES DE MACRO E CONSTANTES 
#ifdef _WIN32
    #define limpar_tela() system("cls")
#else
    #define limpar_tela() system("clear")
#endif

#define MAX_LIVROS 100
#define MAX_USUARIOS 100
#define MAX_EMPRESTIMOS 100

// ESTRUTURAS
struct cadastro_do_livro {
    int  codigo;
    char titulo[101];
    char autor[81];
    char editora[61];
    char ano[11]; 
    int  exemplares;
    int  status; // 1 = disponivel, 2 = emprestado
};

struct cadastro_de_usuarios {
    int matricula;
    char nomeCompleto[101];
    char curso[51];
    char telefone[16]; 
    char dataCadastro[11];
};

struct cadastro_de_emprestimo {
    int codigoEmprestimo;
    int matriculaUsuario;
    int codigolivro;
    char dataEmprestimo[11];
    char dataDevolucao[11];
    int status; // 1 = ativo, 2 = devolvido
};

struct Contagem {
    int codigo_livro;
    int vezes_emprestado;
};

// PROTOTIPOS
int ler_inteiro_seguro();
int ler_apenas_numeros(char *destino, int tamanho_max);
void formatar_data_atual(char *buffer, size_t tamanho);
void to_lowercase_copy(char *dest, const char *src, size_t size); 

int codigo_livro_existe(struct cadastro_do_livro lista_livros[], int total_livros, int codigo_a_verificar);
int buscar_livro_por_codigo(struct cadastro_do_livro lista[], int total_livros, int codigo_buscado);
int buscar_usuario_por_matricula(struct cadastro_de_usuarios lista[], int total_usuarios, int matricula_buscada);
int buscar_usuario_por_nome(struct cadastro_de_usuarios lista[], int total_usuarios, const char *nome_buscado);
int data_para_int(const char *data_str);

void salvar_dados_livros(struct cadastro_do_livro lista[], int total_livros);
void salvar_dados_usuarios(struct cadastro_de_usuarios lista[], int total_usuarios);
void salvar_dados_emprestimos(struct cadastro_de_emprestimo lista[], int total_emprestimos);
void carregar_dados_livros(struct cadastro_do_livro lista[], int *total_livros);
void carregar_dados_usuarios(struct cadastro_de_usuarios lista[], int *total_usuarios);
void carregar_dados_emprestimos(struct cadastro_de_emprestimo lista[], int *total_emprestimos);

void gerenciar_cadastro_livro(struct cadastro_do_livro lista[], int *total_de_livros);
void gerenciar_cadastro_usuario(struct cadastro_de_usuarios lista[], int *total_de_usuarios);
void realizar_emprestimo(struct cadastro_de_emprestimo lista_emprestimo[], int *total_emprestimos, struct cadastro_do_livro lista_livros[], int total_livros, struct cadastro_de_usuarios lista_usuarios[], int total_usuarios);
void realizar_devolucao(struct cadastro_de_emprestimo lista_emprestimo[], int total_emprestimos, struct cadastro_do_livro lista_livros[], int total_livros);
void listar_emprestimos_ativos(struct cadastro_de_emprestimo lista_emprestimo[], int total_emprestimos);
void pesquisar_livro(struct cadastro_do_livro lista[], int total_livros);
void pesquisar_usuario(struct cadastro_de_usuarios lista[], int total_usuarios);

void ordenar_ranking(struct Contagem ranking[], int tamanho);
void relatorio_mais_emprestados(struct cadastro_do_livro lista_livros[], int total_livros, struct cadastro_de_emprestimo lista_emprestimo[], int total_emprestimos);
void relatorio_atraso(struct cadastro_de_emprestimo lista_emprestimo[], int total_emprestimos, struct cadastro_de_usuarios lista_usuarios[], int total_usuarios);


// IMPLEMENTACAO DAS FUNCOES UTILITARIAS

// Funcao robusta para evitar travamentos de buffer (substitui scanf("%d"))
int ler_inteiro_seguro() {
    char buffer[256];
    int valor;
    while (1) {
        if (fgets(buffer, sizeof(buffer), stdin) != NULL) {
            buffer[strcspn(buffer, "\n")] = '\0';
            
            if (strlen(buffer) == 0) continue; 

            int valido = 1;
            int inicio = (buffer[0] == '-') ? 1 : 0; 
            
            for (int i = inicio; buffer[i] != '\0'; i++) {
                if (!isdigit((unsigned char)buffer[i])) {
                    valido = 0;
                    break;
                }
            }

            if (valido && sscanf(buffer, "%d", &valor) == 1) {
                return valor;
            } else {
                printf("Entrada invalida. Digite apenas numeros: ");
            }
        }
    }
}

void to_lowercase_copy(char *dest, const char *src, size_t size) {
    size_t i;
    for (i = 0; i < size - 1 && src[i] != '\0'; i++) {
        dest[i] = tolower((unsigned char)src[i]); 
    }
    dest[i] = '\0';
}

int ler_apenas_numeros(char *destino, int tamanho_max) {
    char linha[256];
    char numeros[256] = ""; 
    int len_numeros = 0;
    
    if (fgets(linha, sizeof(linha), stdin) == NULL) {
        destino[0] = '\0';
        return 0; 
    }
    
    for (int i = 0; linha[i] != '\0'; i++) {
        if (isdigit((unsigned char)linha[i]) && len_numeros < (tamanho_max - 1)) {
            numeros[len_numeros++] = linha[i];
        }
    }
    numeros[len_numeros] = '\0'; 
    
    if (len_numeros >= 10) { 
        if (len_numeros >= 11) { 
             snprintf(destino, tamanho_max, "(%c%c) %c%c%c%c%c-%c%c%c%c", 
                     numeros[0], numeros[1], numeros[2], numeros[3], numeros[4], numeros[5], numeros[6], numeros[7], numeros[8], numeros[9], numeros[10]);
        } else { 
             snprintf(destino, tamanho_max, "(%c%c) %c%c%c%c-%c%c%c%c", 
                     numeros[0], numeros[1], numeros[2], numeros[3], numeros[4], numeros[5], numeros[6], numeros[7], numeros[8], numeros[9]);
        }
    } else {
        strcpy(destino, numeros);
    }

    return len_numeros; 
}

void formatar_data_atual(char *buffer, size_t tamanho) {
    time_t t = time(NULL); 
    struct tm *agora = localtime(&t); 
    strftime(buffer, tamanho, "%d/%m/%Y", agora);
}

int codigo_livro_existe(struct cadastro_do_livro lista_livros[], int total_livros, int codigo_a_verificar) {
    for (int i = 0; i < total_livros; i++) {
        if (lista_livros[i].codigo == codigo_a_verificar) return 1; 
    }
    return 0; 
}

int buscar_livro_por_codigo(struct cadastro_do_livro lista[], int total_livros, int codigo_buscado) {
    for (int i = 0; i < total_livros; i++) {
        if (lista[i].codigo == codigo_buscado) return i; 
    }
    return -1; 
}

int buscar_usuario_por_matricula(struct cadastro_de_usuarios lista[], int total_usuarios, int matricula_buscada) {
    for (int i = 0; i < total_usuarios; i++) {
        if (lista[i].matricula == matricula_buscada) return i; 
    }
    return -1; 
}

int buscar_usuario_por_nome(struct cadastro_de_usuarios lista[], int total_usuarios, const char *nome_buscado) {
    char nome_buscado_lower[101], nome_usuario_lower[101];
    to_lowercase_copy(nome_buscado_lower, nome_buscado, 101);

    for (int i = 0; i < total_usuarios; i++) {
        to_lowercase_copy(nome_usuario_lower, lista[i].nomeCompleto, 101); 
        if (strcmp(nome_usuario_lower, nome_buscado_lower) == 0) return i; 
    }
    return -1; 
}

void ordenar_ranking(struct Contagem ranking[], int tamanho) {
    int i, j, max_idx;
    struct Contagem temp;
    for (i = 0; i < tamanho - 1; i++) {
        max_idx = i;
        for (j = i + 1; j < tamanho; j++) {
            if (ranking[j].vezes_emprestado > ranking[max_idx].vezes_emprestado) max_idx = j;
        }
        temp = ranking[max_idx];
        ranking[max_idx] = ranking[i];
        ranking[i] = temp;
    }
}

int data_para_int(const char *data_str) {
    int dia, mes, ano;
    if (sscanf(data_str, "%d/%d/%d", &dia, &mes, &ano) == 3) {
        return ano * 10000 + mes * 100 + dia;
    } 
    if (strlen(data_str) >= 8) {
        char s_dia[3], s_mes[3], s_ano[5];
        strncpy(s_dia, data_str, 2); s_dia[2] = '\0';
        strncpy(s_mes, data_str + 2, 2); s_mes[2] = '\0';
        strncpy(s_ano, data_str + 4, 4); s_ano[4] = '\0';
        dia = atoi(s_dia); mes = atoi(s_mes); ano = atoi(s_ano);
        if (dia > 0 && mes > 0 && ano > 0) return ano * 10000 + mes * 100 + dia;
    }
    return 0; 
}

// PERSISTENCIA
void salvar_dados_livros(struct cadastro_do_livro lista[], int total_livros) {
    FILE *arquivo = fopen("livros.txt", "w"); 
    if (arquivo == NULL) return;
    fprintf(arquivo, "%d\n", total_livros); 
    for (int i = 0; i < total_livros; i++) {
        fprintf(arquivo, "%d\n%s\n%s\n%s\n%s\n%d\n%d\n", lista[i].codigo, lista[i].titulo, lista[i].autor, lista[i].editora, lista[i].ano, lista[i].exemplares, lista[i].status);
    }
    fclose(arquivo);
}

void salvar_dados_usuarios(struct cadastro_de_usuarios lista[], int total_usuarios) {
    FILE *arquivo = fopen("usuarios.txt", "w"); 
    if (arquivo == NULL) return;
    fprintf(arquivo, "%d\n", total_usuarios); 
    for (int i = 0; i < total_usuarios; i++) {
        fprintf(arquivo, "%d\n%s\n%s\n%s\n%s\n", lista[i].matricula, lista[i].nomeCompleto, lista[i].curso, lista[i].telefone, lista[i].dataCadastro);
    }
    fclose(arquivo);
}

void salvar_dados_emprestimos(struct cadastro_de_emprestimo lista[], int total_emprestimos) {
    FILE *arquivo = fopen("emprestimos.txt", "w"); 
    if (arquivo == NULL) return;
    fprintf(arquivo, "%d\n", total_emprestimos); 
    for (int i = 0; i < total_emprestimos; i++) {
        fprintf(arquivo, "%d\n%d\n%d\n%s\n%s\n%d\n", lista[i].codigoEmprestimo, lista[i].matriculaUsuario, lista[i].codigolivro, lista[i].dataEmprestimo, lista[i].dataDevolucao, lista[i].status);
    }
    fclose(arquivo);
}

void carregar_dados_livros(struct cadastro_do_livro lista[], int *total_livros) {
    FILE *arquivo = fopen("livros.txt", "r"); 
    if (arquivo == NULL || fscanf(arquivo, "%d\n", total_livros) != 1) { *total_livros = 0; if(arquivo) fclose(arquivo); return; }
    for (int i = 0; i < *total_livros; i++) {
        if (fscanf(arquivo, "%d\n", &lista[i].codigo) != 1 || fscanf(arquivo, "%100[^\n]%*c", lista[i].titulo) != 1 ||
            fscanf(arquivo, "%80[^\n]%*c", lista[i].autor) != 1 || fscanf(arquivo, "%60[^\n]%*c", lista[i].editora) != 1 ||
            fscanf(arquivo, "%10[^\n]%*c", lista[i].ano) != 1 || fscanf(arquivo, "%d\n", &lista[i].exemplares) != 1 ||
            fscanf(arquivo, "%d\n", &lista[i].status) != 1) { *total_livros = i; break; }
    }
    fclose(arquivo);
}

void carregar_dados_usuarios(struct cadastro_de_usuarios lista[], int *total_usuarios) {
    FILE *arquivo = fopen("usuarios.txt", "r"); 
    if (arquivo == NULL || fscanf(arquivo, "%d\n", total_usuarios) != 1) { *total_usuarios = 0; if(arquivo) fclose(arquivo); return; }
    for (int i = 0; i < *total_usuarios; i++) {
        if (fscanf(arquivo, "%d\n", &lista[i].matricula) != 1 || fscanf(arquivo, "%100[^\n]%*c", lista[i].nomeCompleto) != 1 ||
            fscanf(arquivo, "%50[^\n]%*c", lista[i].curso) != 1 || fscanf(arquivo, "%15[^\n]%*c", lista[i].telefone) != 1 ||
            fscanf(arquivo, "%10[^\n]%*c", lista[i].dataCadastro) != 1) { *total_usuarios = i; break; }
    }
    fclose(arquivo);
}

void carregar_dados_emprestimos(struct cadastro_de_emprestimo lista[], int *total_emprestimos) {
    FILE *arquivo = fopen("emprestimos.txt", "r"); 
    if (arquivo == NULL || fscanf(arquivo, "%d\n", total_emprestimos) != 1) { *total_emprestimos = 0; if(arquivo) fclose(arquivo); return; }
    for (int i = 0; i < *total_emprestimos; i++) {
        if (fscanf(arquivo, "%d\n", &lista[i].codigoEmprestimo) != 1 || fscanf(arquivo, "%d\n", &lista[i].matriculaUsuario) != 1 ||
            fscanf(arquivo, "%d\n", &lista[i].codigolivro) != 1 || fscanf(arquivo, "%10[^\n]%*c", lista[i].dataEmprestimo) != 1 ||
            fscanf(arquivo, "%10[^\n]%*c", lista[i].dataDevolucao) != 1 || fscanf(arquivo, "%d\n", &lista[i].status) != 1) { *total_emprestimos = i; break; }
    }
    fclose(arquivo);
}

// GERENCIAMENTO
void gerenciar_cadastro_livro(struct cadastro_do_livro lista[], int *total_de_livros) {
    if (*total_de_livros >= MAX_LIVROS) {
        printf("\nERRO: Limite maximo de livros cadastrados atingido (%d)!\n", MAX_LIVROS); return; 
    }
    int codigo; char titulo[101], autor[81], editora[61], ano[11]; int exemplares;

    do {
        printf("\n--- Cadastro de Novo Livro ---\n");
        printf("Digite o Codigo do Livro (deve ser unico, ou 0 para VOLTAR): ");
        codigo = ler_inteiro_seguro();
        
        if (codigo == 0) { printf("Operacao cancelada. Voltando ao Menu.\n"); return; }
        if (codigo < 1) { printf("ERRO: O codigo deve ser positivo.\n"); continue; }
        if (codigo_livro_existe(lista, *total_de_livros, codigo)) {
            printf("\nERRO: Codigo %d ja esta em uso.\n", codigo);
        } else break; 
    } while (1); 

    printf("Digite o Titulo: ");
    fgets(titulo, 101, stdin); titulo[strcspn(titulo, "\n")] = 0; 
    printf("Digite o Autor: ");
    fgets(autor, 81, stdin); autor[strcspn(autor, "\n")] = 0;
    printf("Digite a Editora: ");
    fgets(editora, 61, stdin); editora[strcspn(editora, "\n")] = 0;
    printf("Digite o Ano de Publicacao (AAAA): ");
    fgets(ano, 11, stdin); ano[strcspn(ano, "\n")] = 0;
    
    printf("Digite a Quantidade de Exemplares: ");
    exemplares = ler_inteiro_seguro();

    int indice = *total_de_livros;
    lista[indice].codigo = codigo; 
    strcpy(lista[indice].titulo, titulo); strcpy(lista[indice].autor, autor);
    strcpy(lista[indice].editora, editora); strcpy(lista[indice].ano, ano);
    lista[indice].exemplares = exemplares; lista[indice].status = 1; 

    (*total_de_livros)++; 
    salvar_dados_livros(lista, *total_de_livros);
    printf("\nLivro '%s' cadastrado com sucesso!\n", titulo);
}
    
void gerenciar_cadastro_usuario(struct cadastro_de_usuarios lista[], int *total_de_usuarios) {
    if (*total_de_usuarios >= MAX_USUARIOS) {
        printf("\nERRO: Limite maximo atingido (%d)!\n", MAX_USUARIOS); return; 
    }
    int matricula; char nomeCompleto[101], curso[51], telefone_formatado[16]; 
    
    do {
        printf("\n--- Cadastro de Novo Usuario ---\n");
        printf("Digite a Matricula (unica, ou 0 para VOLTAR): ");
        matricula = ler_inteiro_seguro();
        
        if (matricula == 0) { printf("Operacao cancelada.\n"); return; }
        if (matricula < 1) { printf("ERRO: Matricula positiva.\n"); continue; }
        if (buscar_usuario_por_matricula(lista, *total_de_usuarios, matricula) != -1) {
            printf("\nERRO: Matricula %d ja esta em uso.\n", matricula);
        } else break; 
    } while (1);
    
    printf("Digite o Nome Completo: ");
    fgets(nomeCompleto, 101, stdin); nomeCompleto[strcspn(nomeCompleto, "\n")] = 0;
    printf("Digite o Curso: ");
    fgets(curso, 51, stdin); curso[strcspn(curso, "\n")] = 0;
    
    int num_digitos = 0;
    do {
        printf("Digite o Telefone (ex: (11) 987654321): ");
        num_digitos = ler_apenas_numeros(telefone_formatado, sizeof(telefone_formatado));
        if (num_digitos < 8) printf("\nERRO: E necessario pelo menos 8 digitos.\n");
    } while (num_digitos < 8);
    
    char dataCadastro[11];
    formatar_data_atual(dataCadastro, sizeof(dataCadastro));

    int indice = *total_de_usuarios;
    lista[indice].matricula = matricula;
    strcpy(lista[indice].nomeCompleto, nomeCompleto);
    strcpy(lista[indice].curso, curso);
    strcpy(lista[indice].telefone, telefone_formatado); 
    strcpy(lista[indice].dataCadastro, dataCadastro); 

    (*total_de_usuarios)++; 
    salvar_dados_usuarios(lista, *total_de_usuarios);
    printf("\nUsuario '%s' cadastrado com sucesso!\n", nomeCompleto);
}

void realizar_emprestimo(struct cadastro_de_emprestimo lista_emprestimo[], int *total_emprestimos, struct cadastro_do_livro lista_livros[], int total_livros, struct cadastro_de_usuarios lista_usuarios[], int total_usuarios) {
    if (*total_emprestimos >= MAX_EMPRESTIMOS) {
        printf("\nERRO: Limite de emprestimos atingido!\n"); return;
    }
    int matricula_user = 0, codigo_livro, indice_usuario = -1; 
    const int PRAZO_DIAS = 15; 
    
    printf("\n--- Realizar Emprestimo ---\n");
    printf("\nIdentificar usuario por:\n 1-Matricula\n 2-Nome Completo\n 0-Voltar\n Digite: ");
    int opc_busca_user = ler_inteiro_seguro();
    
    if (opc_busca_user == 0) { return; } 
    else if (opc_busca_user == 1) {
        printf("Digite a Matricula: ");
        matricula_user = ler_inteiro_seguro();
        indice_usuario = buscar_usuario_por_matricula(lista_usuarios, total_usuarios, matricula_user);
    } else if (opc_busca_user == 2) {
        char nome_busca[101];
        printf("Digite o Nome Completo do Usuario: ");
        fgets(nome_busca, 101, stdin); nome_busca[strcspn(nome_busca, "\n")] = 0; 
        indice_usuario = buscar_usuario_por_nome(lista_usuarios, total_usuarios, nome_busca);
        if (indice_usuario != -1) matricula_user = lista_usuarios[indice_usuario].matricula; 
    } else {
        printf("Opcao invalida.\n"); return;
    }
    
    if (indice_usuario == -1) { printf("\nERRO: Usuario nao encontrado.\n"); return; }
    
    printf("Digite o Codigo do Livro (ou 0 para VOLTAR): ");
    codigo_livro = ler_inteiro_seguro();
    if (codigo_livro == 0) return; 
    
    int indice_livro = buscar_livro_por_codigo(lista_livros, total_livros, codigo_livro);
    if (indice_livro == -1) { printf("\nERRO: Livro nao encontrado.\n"); return; }
    if (lista_livros[indice_livro].exemplares < 1) { printf("\nERRO: Livro indisponivel.\n"); return; }
    
    int indice = *total_emprestimos;
    time_t t = time(NULL); struct tm *agora = localtime(&t); 
    strftime(lista_emprestimo[indice].dataEmprestimo, 11, "%d/%m/%Y", agora);
    agora->tm_mday += PRAZO_DIAS; mktime(agora); 
    strftime(lista_emprestimo[indice].dataDevolucao, 11, "%d/%m/%Y", agora);
    
    lista_emprestimo[indice].codigoEmprestimo = indice + 1; 
    lista_emprestimo[indice].matriculaUsuario = matricula_user;
    lista_emprestimo[indice].codigolivro = codigo_livro;
    lista_emprestimo[indice].status = 1; 

    lista_livros[indice_livro].exemplares--;
    (*total_emprestimos)++;

    salvar_dados_livros(lista_livros, total_livros); 
    salvar_dados_emprestimos(lista_emprestimo, *total_emprestimos);
    
    printf("\nEmprestimo #%d realizado! Livro: '%s' | Usuario: '%s'\n", lista_emprestimo[indice].codigoEmprestimo, lista_livros[indice_livro].titulo, lista_usuarios[indice_usuario].nomeCompleto);
}

void realizar_devolucao(struct cadastro_de_emprestimo lista_emprestimo[], int total_emprestimos, struct cadastro_do_livro lista_livros[], int total_livros) {
    printf("\n--- Devolucao de Livro ---\n");
    printf("Digite o Codigo do Emprestimo (ou 0 para VOLTAR): ");
    int cod_emprestimo_busca = ler_inteiro_seguro();
    if (cod_emprestimo_busca == 0) return; 

    int indice = -1;
    for (int i = 0; i < total_emprestimos; i++) {
        if (lista_emprestimo[i].codigoEmprestimo == cod_emprestimo_busca && lista_emprestimo[i].status == 1) {
            indice = i; break;
        }
    }

    if (indice != -1) {
        int cod_livro = lista_emprestimo[indice].codigolivro;
        int ind_livro = buscar_livro_por_codigo(lista_livros, total_livros, cod_livro);
        
        lista_emprestimo[indice].status = 2; 
        if (ind_livro != -1) lista_livros[ind_livro].exemplares++;
        salvar_dados_livros(lista_livros, total_livros); 
        salvar_dados_emprestimos(lista_emprestimo, total_emprestimos);
        printf("\nDevolucao realizada com sucesso!\n");
    } else {
        printf("\nERRO: Emprestimo ativo nao encontrado.\n");
    }
}

void listar_emprestimos_ativos(struct cadastro_de_emprestimo lista[], int total) {
    int encontrou = 0;
    printf("\n--- Emprestimos Ativos ---\n");
    for (int i = 0; i < total; i++) {
        if (lista[i].status == 1) {
            printf("ID: %d | Matr: %d | Livro: %d | Devolucao: %s\n", lista[i].codigoEmprestimo, lista[i].matriculaUsuario, lista[i].codigolivro, lista[i].dataDevolucao);
            encontrou = 1;
        }
    }
    if (!encontrou) printf("Nao ha emprestimos ativos.\n");
}

void pesquisar_livro(struct cadastro_do_livro lista[], int total_livros) {
    if (total_livros == 0) { printf("\nNenhum livro cadastrado.\n"); return; }
    
    printf("\n--- Pesquisar Livro ---\n");
    printf("Buscar por:\n 1-Codigo\n 2-Titulo\n 3-Autor\n 0-Voltar\n Digite: ");
    int criterio = ler_inteiro_seguro();
    if (criterio == 0) return; 
    
    if (criterio == 1) {
        printf("Digite o Codigo: ");
        int codigo = ler_inteiro_seguro();
        int ind = buscar_livro_por_codigo(lista, total_livros, codigo);
        if (ind != -1) printf("\nCodigo: %d | Titulo: %s | Autor: %s | Ex: %d\n", lista[ind].codigo, lista[ind].titulo, lista[ind].autor, lista[ind].exemplares);
        else printf("\nNao encontrado.\n");
        return;
    }

    if (criterio == 2 || criterio == 3) {
        char termo[101], termo_low[101];
        printf("Digite o termo de busca: ");
        fgets(termo, 101, stdin); termo[strcspn(termo, "\n")] = 0;
        to_lowercase_copy(termo_low, termo, 101); 

        int achou = 0;
        for (int i = 0; i < total_livros; i++) {
            char alvo_low[101];
            to_lowercase_copy(alvo_low, (criterio == 2) ? lista[i].titulo : lista[i].autor, 101); 
            if (strstr(alvo_low, termo_low) != NULL) {
                 printf("Codigo: %d | Titulo: %s | Autor: %s | Ex: %d\n", lista[i].codigo, lista[i].titulo, lista[i].autor, lista[i].exemplares);
                 achou = 1;
            }
        }
        if (!achou) printf("Nenhum resultado.\n");
    } else printf("Opcao invalida.\n");
}

void pesquisar_usuario(struct cadastro_de_usuarios lista[], int total_usuarios) {
    if (total_usuarios == 0) { printf("\nNenhum usuario cadastrado.\n"); return; }
    
    printf("\n--- Pesquisar Usuario ---\n");
    printf("Buscar por:\n 1-Matricula\n 2-Nome Completo\n 0-Voltar\n Digite: ");
    int criterio = ler_inteiro_seguro();
    if (criterio == 0) return; 
    
    if (criterio == 1) {
        printf("Digite a Matricula: ");
        int mat = ler_inteiro_seguro();
        int ind = buscar_usuario_por_matricula(lista, total_usuarios, mat);
        if (ind != -1) printf("\nMatricula: %d | Nome: %s | Tel: %s\n", lista[ind].matricula, lista[ind].nomeCompleto, lista[ind].telefone);
        else printf("\nNao encontrado.\n");
        return;
    }

    if (criterio == 2) {
        char termo[101], termo_low[101];
        printf("Digite o Nome: ");
        fgets(termo, 101, stdin); termo[strcspn(termo, "\n")] = 0;
        to_lowercase_copy(termo_low, termo, 101); 
        
        int achou = 0;
        for (int i = 0; i < total_usuarios; i++) {
            char nome_low[101];
            to_lowercase_copy(nome_low, lista[i].nomeCompleto, 101);
            if (strstr(nome_low, termo_low) != NULL) {
                 printf("Matricula: %d | Nome: %s | Tel: %s\n", lista[i].matricula, lista[i].nomeCompleto, lista[i].telefone);
                 achou = 1;
            }
        }
        if (!achou) printf("Nenhum resultado.\n");
    } else printf("Opcao invalida.\n");
}

void relatorio_mais_emprestados(struct cadastro_do_livro lista_livros[], int total_livros, struct cadastro_de_emprestimo lista_emprestimo[], int total_emprestimos) {
    if (total_livros == 0 || total_emprestimos == 0) { printf("\nDados insuficientes.\n"); return; }
    struct Contagem ranking[MAX_LIVROS];
    for (int i = 0; i < total_livros; i++) { ranking[i].codigo_livro = lista_livros[i].codigo; ranking[i].vezes_emprestado = 0; }
    for (int i = 0; i < total_emprestimos; i++) {
        for (int j = 0; j < total_livros; j++) {
            if (ranking[j].codigo_livro == lista_emprestimo[i].codigolivro) { ranking[j].vezes_emprestado++; break; }
        }
    }
    ordenar_ranking(ranking, total_livros);
    printf("\n--- Livros Mais Emprestados ---\n");
    int limite = (total_livros < 5) ? total_livros : 5; 
    for (int i = 0; i < limite; i++) {
        int ind = buscar_livro_por_codigo(lista_livros, total_livros, ranking[i].codigo_livro);
        if (ind != -1 && ranking[i].vezes_emprestado > 0) printf("%dº | Titulo: %s | Emprestimos: %d\n", i + 1, lista_livros[ind].titulo, ranking[i].vezes_emprestado);
    }
}

void relatorio_atraso(struct cadastro_de_emprestimo lista_emprestimo[], int total_emprestimos, struct cadastro_de_usuarios lista_usuarios[], int total_usuarios) {
    if (total_emprestimos == 0) { printf("\nNao ha emprestimos.\n"); return; }
    char data_hoje_str[15]; 
    printf("\n--- Emprestimos em Atraso ---\nDigite a data de hoje (DD/MM/AAAA): ");
    
    fgets(data_hoje_str, sizeof(data_hoje_str), stdin);
    data_hoje_str[strcspn(data_hoje_str, "\n")] = 0;
    
    int data_hoje_int = data_para_int(data_hoje_str);
    if (data_hoje_int == 0) { printf("ERRO: Data invalida.\n"); return; }
    
    int atraso = 0;
    for (int i = 0; i < total_emprestimos; i++) {
        if (lista_emprestimo[i].status == 1) { 
            if (data_hoje_int > data_para_int(lista_emprestimo[i].dataDevolucao)) {
                atraso = 1;
                int ind = buscar_usuario_por_matricula(lista_usuarios, total_usuarios, lista_emprestimo[i].matriculaUsuario);
                printf("ID Emprestimo: %d | Devolucao Prevista: %s | Usuario: %s\n", lista_emprestimo[i].codigoEmprestimo, lista_emprestimo[i].dataDevolucao, (ind != -1) ? lista_usuarios[ind].nomeCompleto : "Desconhecido");
            }
        }
    }
    if (!atraso) printf("Nenhum atraso encontrado.\n");
}

int main() {
    struct cadastro_do_livro lista_de_livros[MAX_LIVROS];
    struct cadastro_de_usuarios lista_de_usuarios[MAX_USUARIOS];
    struct cadastro_de_emprestimo lista_de_emprestimos[MAX_EMPRESTIMOS];
    int total_livros = 0, total_usuarios = 0, total_emprestimos = 0;

    carregar_dados_livros(lista_de_livros, &total_livros);
    carregar_dados_usuarios(lista_de_usuarios, &total_usuarios);
    carregar_dados_emprestimos(lista_de_emprestimos, &total_emprestimos);

    int opc = 0, subOpcoes;
  
    while (opc != 5) {
        printf("\n--- BIBLIOTECA ---\n1-Cadastrar livros\n2-Cadastrar usuarios\n3-Realizar emprestimo\n4-Outras Opcoes\n5-Sair\nDigite: ");
        opc = ler_inteiro_seguro();
        
        if (opc != 5) limpar_tela();

        switch (opc) {
            case 1: gerenciar_cadastro_livro(lista_de_livros, &total_livros); break;
            case 2: gerenciar_cadastro_usuario(lista_de_usuarios, &total_usuarios); break;
            case 3: realizar_emprestimo(lista_de_emprestimos, &total_emprestimos, lista_de_livros, total_livros, lista_de_usuarios, total_usuarios); break;
            case 4:
                printf("\n--- OUTRAS OPCOES ---\n1-Devolucao\n2-Pesquisar livro\n3-Pesquisar usuarios\n4-Emprestimos ativos\n5-Relatorios\n6-Voltar\nDigite: ");
                subOpcoes = ler_inteiro_seguro();
                if (subOpcoes != 6) limpar_tela();
                
                if (subOpcoes == 1) realizar_devolucao(lista_de_emprestimos, total_emprestimos, lista_de_livros, total_livros);
                else if (subOpcoes == 2) pesquisar_livro(lista_de_livros, total_livros);
                else if (subOpcoes == 3) pesquisar_usuario(lista_de_usuarios, total_usuarios);
                else if (subOpcoes == 4) listar_emprestimos_ativos(lista_de_emprestimos, total_emprestimos);
                else if (subOpcoes == 5) {
                    printf("\n--- RELATORIOS ---\n1-Livros Mais Emprestados\n2-Emprestimos em Atraso\n0-Voltar\nDigite: ");
                    int opc_relatorio = ler_inteiro_seguro();
                    if (opc_relatorio == 1) relatorio_mais_emprestados(lista_de_livros, total_livros, lista_de_emprestimos, total_emprestimos);
                    else if (opc_relatorio == 2) relatorio_atraso(lista_de_emprestimos, total_emprestimos, lista_de_usuarios, total_usuarios);
                }
                break; 
            case 5: break;
            default: printf("Opcao invalida.\n"); break;
        }
    }
    salvar_dados_livros(lista_de_livros, total_livros);
    salvar_dados_usuarios(lista_de_usuarios, total_usuarios);
    salvar_dados_emprestimos(lista_de_emprestimos, total_emprestimos);
    return 0;
}
