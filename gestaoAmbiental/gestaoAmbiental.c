#include <gtk/gtk.h>
#include <stdlib.h>
#include <stdbool.h>
#include <openssl/aes.h>
#include <openssl/rand.h>

#define MAX_EMPRESAS 100
#define MAX_PATH 260
#define MAX_MESES 12
#define MAX_ANOS 100
#define MAX_LINE_LENGTH 256
#define DB_FILE "usuarios.txt"
#define DADOS_FILE "dados.txt"
#define DADOS_RESIDUOS_FILE "dados_residuos.txt"

GtkBuilder *builder;
GtkWidget *window;
GtkStack *stack;
gchar *text_to_save = NULL;

typedef struct {
    char cnpj[20];
    char empresa[100];
    double custo_total;
    int quantidade_residuos;
} Empresa;

typedef struct {
    int ano;
    int semestralTotais[2];
    int custoMensal[MAX_MESES];
} AnoRelatorio;

int get_mes_index(const char *mes) {
    const char *meses[] = {
        "Janeiro", "Fevereiro", "Março", "Abril", "Maio", "Junho",
        "Julho", "Agosto", "Setembro", "Outubro", "Novembro", "Dezembro"
    };
    for (int i = 0; i < MAX_MESES; i++) {
        if (strcmp(mes, meses[i]) == 0) {
            return i;
        }
    }
    return -1;
}

const char* obter_regiao(const char* estado) {
    if (strcmp(estado, "Acre (AC)") == 0 || strcmp(estado, "Amapá (AP)") == 0 || strcmp(estado, "Amazonas (AM)") == 0 ||
        strcmp(estado, "Maranhão (MA)") == 0 || strcmp(estado, "Pará (PA)") == 0 || strcmp(estado, "Rondônia (RO)") == 0 ||
        strcmp(estado, "Roraima (RR)") == 0 || strcmp(estado, "Tocantins (TO)") == 0) {
        return "Região Norte";
    } else if (strcmp(estado, "Alagoas (AL)") == 0 || strcmp(estado, "Bahia (BA)") == 0 || strcmp(estado, "Ceará (CE)") == 0 ||
               strcmp(estado, "Maranhão (MA)") == 0 || strcmp(estado, "Paraíba (PB)") == 0 || strcmp(estado, "Pernambuco (PE)") == 0 ||
               strcmp(estado, "Piauí (PI)") == 0 || strcmp(estado, "Rio Grande do Norte (RN)") == 0 || strcmp(estado, "Sergipe (SE)") == 0) {
        return "Região Nordeste";
    } else if (strcmp(estado, "Goiás (GO)") == 0 || strcmp(estado, "Mato Grosso (MT)") == 0 || strcmp(estado, "Mato Grosso do Sul (MS)") == 0 ||
               strcmp(estado, "Distrito Federal (DF)") == 0) {
        return "Região Centro-Oeste";
    } else if (strcmp(estado, "Espírito Santo (ES)") == 0 || strcmp(estado, "Minas Gerais (MG)") == 0 || strcmp(estado, "Rio de Janeiro (RJ)") == 0 ||
               strcmp(estado, "São Paulo (SP)") == 0) {
        return "Região Sudeste";
    } else if (strcmp(estado, "Paraná (PR)") == 0 || strcmp(estado, "Rio Grande do Sul (RS)") == 0 || strcmp(estado, "Santa Catarina (SC)") == 0) {
        return "Região Sul";
    }
    return "Região Desconhecida";
}

int validar_usuario(const char *usuario, const char *senha) {
    FILE *file = fopen(DB_FILE, "r");
    char linha[MAX_LINE_LENGTH];

    if (!file) {
        perror("Erro ao abrir arquivo");
        return 0;
    }

    while (fgets(linha, sizeof(linha), file)) {
        char linha_usuario[128];
        char linha_senha[128];

        sscanf(linha, "%127s %127s", linha_usuario, linha_senha);

        if (strcmp(linha_usuario, usuario) == 0 && strcmp(linha_senha, senha) == 0) {
            fclose(file);
            return 1;
        }
    }

    fclose(file);
    return 0;
}

FILE *open_file_with_incremental_name(char *base_path) {
    char filePath[MAX_PATH];
    int count = 0;
    do {
        if (count == 0) {
            snprintf(filePath, sizeof(filePath), "%s", base_path);
        } else {
            snprintf(filePath, sizeof(filePath), "%s(%d)", base_path, count);
        }
        FILE *file = fopen(filePath, "r");
        if (file) {
            fclose(file);
            count++;
        } else {
            return fopen(filePath, "w");
        }
    } while (count < 100);
    return NULL;
}

void extrair_dados(FILE *inputFile, AnoRelatorio *relatorios, int *anoCount, char *nome_cnpj) {
    char linha[256];

    for (int i = 0; i < MAX_ANOS; i++) {
        relatorios[i].ano = -1;
        relatorios[i].semestralTotais[0] = 0;
        relatorios[i].semestralTotais[1] = 0;
        for (int j = 0; j < MAX_MESES; j++) {
            relatorios[i].custoMensal[j] = 0;
        }
    }
	char cnpj[15];
    while (fgets(linha, sizeof(linha), inputFile)) {
        char mes[20];
        int ano, quantidade, custo;

        int numFields = sscanf(linha, "%[^,],%[^,],%d,%d,%d", cnpj, mes, &ano, &quantidade, &custo);

        if (numFields != 5) {
            printf("Formato inválido na linha: %s\n", linha);
            continue;
        }

        int monthIndex = get_mes_index(mes);
        if (monthIndex == -1) {
            continue;
        }

        int found = 0;
        for (int i = 0; i < *anoCount; i++) {
            if (relatorios[i].ano == ano) {
                found = 1;
                relatorios[i].custoMensal[monthIndex] += custo;
                if (monthIndex < 6) {
                    relatorios[i].semestralTotais[0] += quantidade;
                } else {
                    relatorios[i].semestralTotais[1] += quantidade;
                }
                break;
            }
        }

        if (!found) {
            if (*anoCount < MAX_ANOS) {
                relatorios[*anoCount].ano = ano;
                relatorios[*anoCount].custoMensal[monthIndex] += custo;
                if (monthIndex < 6) {
                    relatorios[*anoCount].semestralTotais[0] += quantidade;
                } else {
                    relatorios[*anoCount].semestralTotais[1] += quantidade;
                }
                (*anoCount)++;
            } else {
                printf("Máximo de anos atingido.\n");
            }
        }
    }

    strncpy(nome_cnpj, cnpj, 15);
}

void escrever_relatorio(FILE *outputFile, AnoRelatorio *relatorios, int anoCount) {
		for (int i = 0; i < anoCount; i++) {
        fprintf(outputFile, "Ano: %d\n", relatorios[i].ano);
        fprintf(outputFile, "Semestre,Total de Insumos\n");
        fprintf(outputFile, "Primeiro,%d\n", relatorios[i].semestralTotais[0]);
        fprintf(outputFile, "Segundo,%d\n", relatorios[i].semestralTotais[1]);

        fprintf(outputFile, "\nMês,Total de Gastos\n");
        const char *meses[] = {
            "Janeiro", "Fevereiro", "Março", "Abril", "Maio", "Junho",
            "Julho", "Agosto", "Setembro", "Outubro", "Novembro", "Dezembro"
        };
        for (int j = 0; j < MAX_MESES; j++) {
            fprintf(outputFile, "%s,%d\n", meses[j], relatorios[i].custoMensal[j]);
        }
        fprintf(outputFile, "\n");
    }
}

void gerar_relatorio(const char *caminho_saida, const char *formato, FILE *fp_saida, FILE *fp_dados, FILE *fp_residuos) {
    char linha[256];
    double menor_producao = __DBL_MAX__;
    char empresa_menor_producao[100];
    int total_residuos = 0;
    char estado_maior_volume[50];
    double maior_volume = 0;
    double aporte_financeiro = 0;

    int residuos_por_regiao[5] = {0}; // Índices: 0-Norte, 1-Nordeste, 2-Centro-Oeste, 3-Sudeste, 4-Sul

    Empresa empresas[MAX_EMPRESAS];
    int empresa_count = 0;

    fprintf(fp_saida, "Estado,Empresa,CNPJ,Quantidade de Resíduos,Total Cost\n");

    while (fgets(linha, sizeof(linha), fp_dados)) {
        char *responsavel, *cpf, *empresa, *cnpj, *razao_social, *nome_fantasia, *email, *telefone, *endereco, *estado, *data_abertura;
        responsavel = strtok(linha, ",");
        cpf = strtok(NULL, ",");
        empresa = strtok(NULL, ",");
        cnpj = strtok(NULL, ",");
        razao_social = strtok(NULL, ",");
        nome_fantasia = strtok(NULL, ",");
        email = strtok(NULL, ",");
        telefone = strtok(NULL, ",");
        endereco = strtok(NULL, ",");
        estado = strtok(NULL, ",");
        data_abertura = strtok(NULL, "\n");

        double custo_total_empresa = 0;
        int quantidade_residuos = 0;

        // Resetar o ponteiro do arquivo de resíduos
        fseek(fp_residuos, 0, SEEK_SET);
        while (fgets(linha, sizeof(linha), fp_residuos)) {
            char *cnpj_residuo = strtok(linha, ",");
            char *mes = strtok(NULL, ",");
            char *ano = strtok(NULL, ",");
            char *quantidade = strtok(NULL, ",");
            char *valor_estimado = strtok(NULL, "\n");

            if (strcmp(cnpj_residuo, cnpj) == 0) {
                int qnt_residuos = atoi(quantidade);
                double valor_custo = atof(valor_estimado);

                quantidade_residuos += qnt_residuos;
                custo_total_empresa += valor_custo;
            }
        }

        strcpy(empresas[empresa_count].cnpj, cnpj);
        strcpy(empresas[empresa_count].empresa, empresa);
        empresas[empresa_count].custo_total = custo_total_empresa;
        empresas[empresa_count].quantidade_residuos = quantidade_residuos;
        empresa_count++;

        fprintf(fp_saida, "%s,%s,%s,%d,%.2f\n", estado, empresa, cnpj, quantidade_residuos, custo_total_empresa);

        aporte_financeiro += custo_total_empresa;

        total_residuos += quantidade_residuos;
        if (quantidade_residuos > maior_volume) {
            maior_volume = quantidade_residuos;
            strcpy(estado_maior_volume, estado);
        }

        if (quantidade_residuos < menor_producao) {
            menor_producao = quantidade_residuos;
            strcpy(empresa_menor_producao, empresa);
        }

        const char *regiao = obter_regiao(estado);
        if (strcmp(regiao, "Região Norte") == 0) {
            residuos_por_regiao[0] += quantidade_residuos;
        } else if (strcmp(regiao, "Região Nordeste") == 0) {
            residuos_por_regiao[1] += quantidade_residuos;
        } else if (strcmp(regiao, "Região Centro-Oeste") == 0) {
            residuos_por_regiao[2] += quantidade_residuos;
        } else if (strcmp(regiao, "Região Sudeste") == 0) {
            residuos_por_regiao[3] += quantidade_residuos;
        } else if (strcmp(regiao, "Região Sul") == 0) {
            residuos_por_regiao[4] += quantidade_residuos;
        }
    }

    fprintf(fp_saida, "\nResumo,\n");
    fprintf(fp_saida, "Estado com maior volume de resíduos,%s,%d\n", estado_maior_volume, (int)maior_volume);
    fprintf(fp_saida, "Empresa com menor produção,%s,%d\n", empresa_menor_producao, (int)menor_producao);
    fprintf(fp_saida, "Total de Resíduos Tratados,%d\n", total_residuos);
    fprintf(fp_saida, "Aporte Financeiro Semestral,%.2f\n", aporte_financeiro);

    fprintf(fp_saida, "\nResíduos tratados por região,\n");
    fprintf(fp_saida, "Região,Quantidade de Resíduos\n");
    fprintf(fp_saida, "Região Norte,%d\n", residuos_por_regiao[0]);
    fprintf(fp_saida, "Região Nordeste,%d\n", residuos_por_regiao[1]);
    fprintf(fp_saida, "Região Centro-Oeste,%d\n", residuos_por_regiao[2]);
    fprintf(fp_saida, "Região Sudeste,%d\n", residuos_por_regiao[3]);
    fprintf(fp_saida, "Região Sul,%d\n", residuos_por_regiao[4]);
}

void deletar_registro_dados(const gchar *cnpj) {
    FILE *fp_dados, *fp_temp;
    char linha[256];
    int encontrado = 0;
	g_print("CNPJ capturado DENTRO: [%s]\n", cnpj);

    // Abre o arquivo original e um arquivo temporário
    fp_dados = fopen("dados.txt", "r");
    if (fp_dados == NULL) {
        printf("Erro ao abrir o arquivo de dados.\n");
        return;
    }

    // Criando um arquivo temporário para armazenar dados
    fp_temp = fopen("temp_dados.txt", "w");
    if (fp_temp == NULL) {
        fclose(fp_dados);
        printf("Erro ao criar arquivo temporário.\n");
        return;
    }

    // Lê cada linha do arquivo original
    while (fgets(linha, sizeof(linha), fp_dados)) {
        char cnpj_atual[20]; // Para armazenar o CNPJ atual

        // Use strtok para ler apenas o CNPJ da linha (quarto elemento)
        sscanf(linha, "%*[^,],%*[^,],%*[^,],%[^,]", cnpj_atual); // Utiliza sscanf para capturar o CNPJ

        if (strcmp(cnpj_atual, cnpj) != 0) {
            // Se não for o CNPJ que queremos deletar, escreva a linha no arquivo temporário
            fputs(linha, fp_temp);
        } else {
            // Registro encontrado; não escreve no arquivo temporário
            encontrado = 1;
        }
    }

    fclose(fp_dados);
    fclose(fp_temp);

    remove("dados.txt");
    rename("temp_dados.txt", "dados.txt");

    if (encontrado) {
        printf("Registro com CNPJ %s deletado com sucesso.\n", cnpj);
    } else {
        printf("Registro com CNPJ %s não encontrado.\n", cnpj);
        remove("temp_dados.txt");
    }
}

void carregar_dados_residuos(GtkListStore *liststore, FILE *arquivo) {
	char linha[MAX_LINE_LENGTH];
    while (fgets(linha, sizeof(linha), arquivo)) {
        char cnpj_empresa[15], mes_residuo_ind[10], ano_residuo_ind[5];
        char qtd_residuos_ind[1000], custo_residuo_ind[1000];

        int ret = sscanf(linha, "%14[^,],%9[^,],%4[^,],%999[^,],%999[^,\n]",
			cnpj_empresa, mes_residuo_ind, ano_residuo_ind, qtd_residuos_ind, custo_residuo_ind);

		if (ret == 5) {
			if(strcmp(cnpj_empresa, text_to_save) == 0){
				GtkTreeIter iter;
				gtk_list_store_append(liststore, &iter);
				gtk_list_store_set(liststore, &iter,
								   0, cnpj_empresa,
								   1, mes_residuo_ind,
								   2, ano_residuo_ind,
								   3, qtd_residuos_ind,
								   4, custo_residuo_ind,
								   -1);
			}
        } else {
            printf("Erro ao ler linha: %s\n", linha);
        }
    }

    fclose(arquivo);
}

void carregar_dados(GtkListStore *liststore, FILE *arquivo) {
    char linha[MAX_LINE_LENGTH];
    while (fgets(linha, sizeof(linha), arquivo)) {
        char nome_responsavel[128], cpf_responsavel[15], nome_empresa[128], cnpj_empresa[15];
        char razao_social_empresa[128], nome_fantasia_empresa[128], email_empresa[128];
        char telefone_empresa[15], endereco_empresa[128],data_abertura_empresa[11];

        int ret = sscanf(linha, "%127[^,],%14[^,],%127[^,],%14[^,],%127[^,],%127[^,],%127[^,],%14[^,],%127[^,\n],%10[^,\n]",
               nome_responsavel, cpf_responsavel, nome_empresa, cnpj_empresa, razao_social_empresa,
               nome_fantasia_empresa, email_empresa, telefone_empresa, endereco_empresa, data_abertura_empresa);

		if (ret == 10) {
            GtkTreeIter iter;
            gtk_list_store_append(liststore, &iter);
            gtk_list_store_set(liststore, &iter,
                               0, nome_responsavel,
                               1, cpf_responsavel,
                               2, nome_empresa,
                               3, cnpj_empresa,
                               4, razao_social_empresa,
                               5, nome_fantasia_empresa,
                               6, email_empresa,
                               7, telefone_empresa,
                               8, endereco_empresa,
                               9, data_abertura_empresa,
                               -1);
        } else {
            printf("Erro ao ler linha: %s\n", linha);
        }
    }

    fclose(arquivo);
}

void salvar_dados(const char *nome_responsavel, const char *cpf_responsavel, const char *nome_empresa,
                  const char *cnpj_empresa, const char *razao_social_empresa, const char *nome_fantasia_empresa,
                  const char *email_empresa, const char *telefone_empresa, const char *estado_empresa,
				  const char *endereco_empresa, const char *data_abertura_empresa) {
    FILE *file = fopen(DADOS_FILE, "a");

    if (!file) {
        perror("Erro ao abrir o arquivo");
        return;
    }

    fprintf(file, "%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s\n",
            nome_responsavel,
            cpf_responsavel,
            nome_empresa,
            cnpj_empresa,
            razao_social_empresa,
            nome_fantasia_empresa,
            email_empresa,
            telefone_empresa,
            estado_empresa,
            endereco_empresa,
            data_abertura_empresa);

    fclose(file);
}

void deletar_registro_residuos(const char *cnpj) {
    FILE *fp_residuos, *fp_temp;
    char linha[256];
    int encontrado = 0;
    g_print("CNPJ capturado DENTRO (Resíduos): [%s]\n", cnpj);

    fp_residuos = fopen("dados_residuos.txt", "r");
    if (fp_residuos == NULL) {
        printf("Erro ao abrir o arquivo de resíduos.\n");
        return;
    }

    fp_temp = fopen("temp_residuos.txt", "w");
    if (fp_temp == NULL) {
        fclose(fp_residuos);
        printf("Erro ao criar arquivo temporário.\n");
        return;
    }

    while (fgets(linha, sizeof(linha), fp_residuos)) {
        char cnpj_atual[20];
        int n = sscanf(linha, "%[^,]", cnpj_atual);

        if (n < 1) {
            continue;
        }
        cnpj_atual[strcspn(cnpj_atual, "\r\n")] = 0;

        if (strcmp(cnpj_atual, cnpj) != 0) {
            fputs(linha, fp_temp);
        } else {
            encontrado = 1;
        }
    }

    fclose(fp_residuos);
    fclose(fp_temp);

    remove("dados_residuos.txt");
    rename("temp_residuos.txt", "dados_residuos.txt");

    if (encontrado) {
        printf("Registro com CNPJ %s deletado com sucesso dos resíduos.\n", cnpj);
    } else {
        printf("Registro com CNPJ %s não encontrado nos resíduos.\n", cnpj);
        remove("temp_residuos.txt");
    }
}

void inserir_usuario(const char *usuario, const char *senha) {
    FILE *file = fopen(DB_FILE, "a");

    if (!file) {
        perror("Erro ao abrir arquivo para escrita");
        return;
    }

    fprintf(file, "%s %s\n", usuario, senha);
    fclose(file);
}

void mensagem (char texto_primario[100], char texto_secundario[100], char nome_icone[100]) {
    GtkMessageDialog *messageDialog = GTK_MESSAGE_DIALOG(gtk_builder_get_object(builder, "mensagem_dialog"));

    g_object_set(messageDialog, "text", texto_primario, NULL);
    g_object_set(messageDialog, "secondary_text", texto_secundario, NULL);
    g_object_set(messageDialog, "icon_name", nome_icone, NULL);

    gtk_widget_show_all(GTK_WIDGET(messageDialog));
    gtk_dialog_run(GTK_DIALOG(messageDialog));
    gtk_widget_hide(GTK_WIDGET(messageDialog));
}

// -------------------
// Funções dos botões
// -------------------

void on_main_login_destroy(GtkWidget *widget, gpointer data) {
    gtk_main_quit();
}

void on_button_login_clicked(GtkWidget *widget, gpointer data) {
    char *usuario = gtk_entry_get_text(GTK_ENTRY(gtk_builder_get_object(builder, "usuario")));
    char *senha = gtk_entry_get_text(GTK_ENTRY(gtk_builder_get_object(builder, "senha")));

    if(validar_usuario(usuario, senha)) {
        mensagem("Bem Vindo", "Usuario Logado com sucesso!","emblem-default");
        gtk_stack_set_visible_child_name(stack, "view_industria");

		GtkListStore *liststore = GTK_LIST_STORE(gtk_builder_get_object(builder, "lista_responsavel"));
		gtk_list_store_clear(liststore);
		FILE *arquivo = fopen(DADOS_FILE, "r");
		if (!arquivo) {
			perror("Erro ao abrir o arquivo");
			return;
		} else {
			carregar_dados(liststore, arquivo);
		}
    } else {
        mensagem("Login incorreto", "Email ou senha incorretos! \n Fa\u00E7a o cadastro PRIMEIRO!","dialog-error");
    }
}

void on_button_cadastro_clicked(GtkWidget *widget, gpointer data) {
    char *usuario = gtk_entry_get_text(GTK_ENTRY(gtk_builder_get_object(builder, "usuario")));
    char *senha = gtk_entry_get_text(GTK_ENTRY(gtk_builder_get_object(builder, "senha")));

   	if(validar_usuario(usuario, senha)) {
        mensagem("Erro Cadastro !", "Usuario j\u00E1 cadastrado","dialog-error");
    } else {
		inserir_usuario(usuario, senha);
		mensagem("Sucesso !", "Usuario cadastrado com sucesso!","emblem-default");
    }

}

void on_butto_sair_industria_clicked(GtkWidget *widget, gpointer data) {
	gtk_stack_set_visible_child_name(stack, "view_login");
	GtkListStore *liststore = GTK_LIST_STORE(gtk_builder_get_object(builder, "lista_responsavel"));
	gtk_list_store_clear(liststore);
	FILE *arquivo = fopen(DADOS_FILE, "r");
	if (!arquivo) {
        perror("Erro ao abrir o arquivo");
        return;
    } else {
		carregar_dados(liststore, arquivo);
    }
}

void on_button_cadastrar_industria_clicked(GtkWidget *widget, gpointer data) {
    gtk_stack_set_visible_child_name(stack, "view_cadastrar_industria");
}

void on_button_rel_global_csv_clicked(GtkWidget *widget, gpointer data) {
    FILE *fp_dados, *fp_residuos, *fp_saida;
    char downloadsPath[MAX_PATH];
#ifdef _WIN32
    snprintf(downloadsPath, MAX_PATH, "%s\\Downloads\\relatorio_global.csv", getenv("USERPROFILE"));
#else
    snprintf(downloadsPath, MAX_PATH, "%s\\Downloads\\relatorio_global.csv", getenv("HOME"));
#endif
    fp_saida = open_file_with_incremental_name(downloadsPath);
    if (fp_saida == NULL) {
        mensagem("Erro!", "Erro ao abrir o arquivo de sa\u00EDda.", "dialog-error");
        g_print("Erro ao abrir o arquivo de sa\u00EDda.\n");
        return;
    }

    fp_dados = fopen(DADOS_FILE, "r");
    fp_residuos = fopen(DADOS_RESIDUOS_FILE, "r");
    if (fp_dados == NULL || fp_residuos == NULL) {
        fclose(fp_saida);
        mensagem("Erro!", "Erro ao abrir arquivos de dados.", "dialog-error");
        g_print("Erro ao abrir arquivos de dados.\n");
        return;
    }

    gerar_relatorio(downloadsPath, "csv", fp_saida, fp_dados, fp_residuos);

    // Fechar os arquivos
    fclose(fp_dados);
    fclose(fp_residuos);
    fclose(fp_saida);

    printf("Relat\u00F3rio global gerado com sucesso em %s\n", downloadsPath);
}

void on_button_rel_global_txt_clicked(GtkWidget *widget, gpointer data) {
    FILE *fp_dados, *fp_residuos, *fp_saida;
    char downloadsPath[MAX_PATH];
#ifdef _WIN32
    snprintf(downloadsPath, MAX_PATH, "%s\\Downloads\\relatorio_global.txt", getenv("USERPROFILE"));
#else
    snprintf(downloadsPath, MAX_PATH, "%s\\Downloads\\relatorio_global.txt", getenv("HOME"));
#endif
    fp_saida = fopen(downloadsPath, "w");
    if (fp_saida == NULL) {
        mensagem("Erro!", "Erro ao abrir o arquivo de sa\u00EDda.", "dialog-error");
        g_print("Erro ao abrir o arquivo de sa\u00EDda.\n");
        return;
    }

    fp_dados = fopen(DADOS_FILE, "r");
    fp_residuos = fopen(DADOS_RESIDUOS_FILE, "r");
    if (fp_dados == NULL || fp_residuos == NULL) {
        fclose(fp_saida);
        mensagem("Erro!", "Erro ao abrir arquivos de dados.", "dialog-error");
        g_print("Erro ao abrir arquivos de dados.\n");
        return;
    }

    gerar_relatorio(downloadsPath, "txt", fp_saida, fp_dados, fp_residuos);

    fclose(fp_dados);
    fclose(fp_residuos);
    fclose(fp_saida);

    printf("Relat\u00F3rio global gerado com sucesso em %s\n", downloadsPath);
}

void on_button_delete_industria_clicked(GtkWidget *widget, gpointer data) {
	GtkTreeView *tree_view = GTK_TREE_VIEW(gtk_builder_get_object(builder, "id_listar_industrias"));
    GtkTreeSelection *selection = gtk_tree_view_get_selection(tree_view);
    GtkTreeModel *model;
    GtkTreeIter iter;

    if (selection && gtk_tree_selection_get_selected(selection, &model, &iter)) {
        gtk_tree_model_get(model, &iter, 3, &text_to_save, -1);
        deletar_registro_dados(text_to_save);
        deletar_registro_residuos(text_to_save);
        g_free(text_to_save);
    } else {
        printf("Nenhum registro selecionado.\n");
    }

	GtkListStore *liststore = GTK_LIST_STORE(gtk_builder_get_object(builder, "lista_responsavel"));
	gtk_list_store_clear(liststore);
	FILE *arquivo = fopen(DADOS_FILE, "r");
	if (!arquivo) {
		perror("Erro ao abrir o arquivo");
		return;
	} else {
		carregar_dados(liststore, arquivo);
	}
}

void on_btn_cancelar_clicked(GtkWidget *widget, gpointer data) {
	gtk_stack_set_visible_child_name(stack, "view_industria");
}

void on_btn_salvar_responsavel_clicked(GtkWidget *widget, gpointer data) {
	char *nome_responsavel = gtk_entry_get_text(GTK_ENTRY(gtk_builder_get_object(builder, "input_responsavel")));
    char *cpf_responsavel = gtk_entry_get_text(GTK_ENTRY(gtk_builder_get_object(builder, "input_cpf_responsavel")));
    char *nome_empresa = gtk_entry_get_text(GTK_ENTRY(gtk_builder_get_object(builder, "input_nome_empresa")));
    char *cnpj_empresa = gtk_entry_get_text(GTK_ENTRY(gtk_builder_get_object(builder, "input_cnpj")));
    char *razao_social_empresa = gtk_entry_get_text(GTK_ENTRY(gtk_builder_get_object(builder, "input_razao_social")));
    char *nome_fantasia_empresa = gtk_entry_get_text(GTK_ENTRY(gtk_builder_get_object(builder, "input_nome_fantasia")));
    char *email_empresa = gtk_entry_get_text(GTK_ENTRY(gtk_builder_get_object(builder, "input_emai")));
    char *telefone_empresa = gtk_entry_get_text(GTK_ENTRY(gtk_builder_get_object(builder, "input_telefone")));
    char *estado_empresa = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(gtk_builder_get_object(builder, "input_combo_estado")));
    char *endereco_empresa = gtk_entry_get_text(GTK_ENTRY(gtk_builder_get_object(builder, "input_endereco")));
    char data_abertura_empresa[11];

    guint ano, mes, dia;
	gtk_calendar_get_date(GTK_CALENDAR(gtk_builder_get_object(builder, "input_data_abertura")), &ano, &mes, &dia);
    mes += 1;
    sprintf(data_abertura_empresa, "%02d/%02d/%04d", dia, mes, ano);

    salvar_dados(nome_responsavel, cpf_responsavel, nome_empresa,
                  cnpj_empresa, razao_social_empresa, nome_fantasia_empresa,
                  email_empresa, telefone_empresa,estado_empresa , endereco_empresa, data_abertura_empresa);

    mensagem("Sucesso", "Dados salvos com sucesso!", "emblem-default");

	gtk_stack_set_visible_child_name(stack, "view_industria");
}

void on_btn_salva_ind_clicked(GtkWidget *widget, gpointer data) {
	char *mes_ind = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(gtk_builder_get_object(builder, "input_mes_ind")));
	char *ano_ind = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(gtk_builder_get_object(builder, "input_ano_ind")));
	char *qtd_residuos_ind = gtk_entry_get_text(GTK_ENTRY(gtk_builder_get_object(builder, "input_qtd_residuos_ind")));
	char *custo_ind = gtk_entry_get_text(GTK_ENTRY(gtk_builder_get_object(builder, "input_custos_residuos_ind")));

	FILE *file = fopen(DADOS_RESIDUOS_FILE, "a");

    if (!file) {
        perror("Erro ao abrir o arquivo");
        return;
    }

    fprintf(file, "%s,%s,%s,%s,%s\n",
            text_to_save,
            mes_ind,
            ano_ind,
            qtd_residuos_ind,
            custo_ind);

    fclose(file);


	GtkListStore *liststore = GTK_LIST_STORE(gtk_builder_get_object(builder, "lista_rel_ind"));
	gtk_list_store_clear(liststore);
	FILE *arquivo = fopen(DADOS_RESIDUOS_FILE, "r");
	if (!arquivo) {
        perror("Erro ao abrir o arquivo");
        return;
    } else {
		carregar_dados_residuos(liststore, arquivo);
    }
    g_free(text_to_save);
}

void on_btn_voltar_ind_clicked(GtkWidget *widget, gpointer data) {
	gtk_stack_set_visible_child_name(stack, "view_industria");
}

void on_btn_relatorio_csv_ind_clicked(GtkWidget *widget, gpointer data) {
	FILE *inputFile = fopen(DADOS_RESIDUOS_FILE, "r");
    if (inputFile == NULL) {
        mensagem("Erro!", "Erro ao abrir o arquivo de entrada.", "dialog-error");
        g_print("Erro ao abrir o arquivo de entrada.\n");
        return;
    }

	AnoRelatorio relatorios[MAX_ANOS];
    int anoCount = 0;
    char cnpj[15]= "";
	// Chama a função para extrair os dados
	extrair_dados(inputFile, relatorios, &anoCount, cnpj);
	fclose(inputFile);
	g_print("%s",cnpj);

	char downloadsPath[MAX_PATH];
#ifdef _WIN32
	snprintf(downloadsPath, MAX_PATH, "%s\\Downloads\\relatorio_%s.csv", getenv("USERPROFILE"), cnpj);
#else
	snprintf(downloadsPath, MAX_PATH, "%s\\Downloads\\relatorio_%s.csv", getenv("HOME"), cnpj);
#endif
    FILE *outputFile = open_file_with_incremental_name(downloadsPath);
    if (outputFile == NULL) {
        fclose(inputFile);
        mensagem("Erro!", "Erro ao abrir o arquivo de saída.", "dialog-error");
        g_print("Erro ao abrir o arquivo de saída.\n");
        return;
    }

	escrever_relatorio(outputFile, relatorios, anoCount);
    fclose(outputFile);

    mensagem("Sucesso!", "Relatório gerado com sucesso na pasta Downloads.", "dialog-info");
    g_print("Relatório gerado com sucesso.\n");
}

void on_btn_relatorio_txt_ind_clicked(GtkWidget *widget, gpointer data) {
		FILE *inputFile = fopen(DADOS_RESIDUOS_FILE, "r");
    if (inputFile == NULL) {
        mensagem("Erro!", "Erro ao abrir o arquivo de entrada.", "dialog-error");
        g_print("Erro ao abrir o arquivo de entrada.\n");
        return;
    }

	AnoRelatorio relatorios[MAX_ANOS];
    int anoCount = 0;
    char cnpj[15]= "";
	// Chama a função para extrair os dados
	extrair_dados(inputFile, relatorios, &anoCount, cnpj);
	fclose(inputFile);
	g_print("%s",cnpj);

	char downloadsPath[MAX_PATH];
#ifdef _WIN32
	snprintf(downloadsPath, MAX_PATH, "%s\\Downloads\\relatorio_%s.txt", getenv("USERPROFILE"), cnpj);
#else
	snprintf(downloadsPath, MAX_PATH, "%s\\Downloads\\relatorio_%s.txt", getenv("HOME"), cnpj);
#endif
    FILE *outputFile = open_file_with_incremental_name(downloadsPath);
    if (outputFile == NULL) {
        fclose(inputFile);
        mensagem("Erro!", "Erro ao abrir o arquivo de saída.", "dialog-error");
        g_print("Erro ao abrir o arquivo de saída.\n");
        return;
    }

	escrever_relatorio(outputFile, relatorios, anoCount);
    fclose(outputFile);

    mensagem("Sucesso!", "Relatório gerado com sucesso na pasta Downloads.", "dialog-info");
    g_print("Relatório gerado com sucesso.\n");
}

void on_btn_add_residuos_clicked(GtkWidget *widget, gpointer data) {
    GtkTreeView *tree_view = GTK_TREE_VIEW(gtk_builder_get_object(builder, "id_listar_industrias"));
    GtkTreeSelection *selection = gtk_tree_view_get_selection(tree_view);
    GtkTreeModel *model;
    GtkTreeIter iter;

	if (selection && gtk_tree_selection_get_selected(selection, &model, &iter)) {
		gtk_tree_model_get(model, &iter, 3, &text_to_save, -1);

		gtk_stack_set_visible_child_name(stack, "view_residuos");

		GtkListStore *liststore = GTK_LIST_STORE(gtk_builder_get_object(builder, "lista_rel_ind"));
		gtk_list_store_clear(liststore);
		FILE *arquivo = fopen(DADOS_RESIDUOS_FILE, "r");
		if (!arquivo) {
			perror("Erro ao abrir o arquivo");
			return;
		} else {
			carregar_dados_residuos(liststore, arquivo);
		}

    } else {
        mensagem("Erro ao adicionar residuos! ", "Selecione um item da lista para acessar essa tela","dialog-error");
    }
	g_free(text_to_save);
}

void on_btn_delete_ind_clicked (GtkWidget *widget, gpointer data) {
	GtkTreeView *tree_view = GTK_TREE_VIEW(gtk_builder_get_object(builder, "lista_rel_ind"));
    GtkTreeSelection *selection = gtk_tree_view_get_selection(tree_view);
    GtkTreeModel *model;
    GtkTreeIter iter;
    char cnpj[20];
    if (selection && gtk_tree_selection_get_selected(selection, &model, &iter)) {
        gtk_tree_model_get(model, &iter, 0, &cnpj, -1);
        deletar_registro_residuos(cnpj);
    } else {
        printf("Nenhum registro selecionado.\n");
    }
}

int main(int argc, char *argv[]) {
	gtk_init(&argc, &argv);

    builder = gtk_builder_new_from_file("login.glade");

    gtk_builder_add_callback_symbols
    (builder,
    "on_main_login_destroy",                    G_CALLBACK(on_main_login_destroy),
    "on_button_login_clicked",                  G_CALLBACK(on_button_login_clicked),
    "on_button_cadastro_clicked",               G_CALLBACK(on_button_cadastro_clicked),
    "on_butto_sair_industria_clicked",          G_CALLBACK(on_butto_sair_industria_clicked),
    "on_button_cadastrar_industria_clicked",    G_CALLBACK(on_button_cadastrar_industria_clicked),
	"on_button_rel_global_csv_clicked",			G_CALLBACK(on_button_rel_global_csv_clicked),
	"on_button_rel_global_txt_clicked",			G_CALLBACK(on_button_rel_global_txt_clicked),
	"on_button_delete_industria_clicked",		G_CALLBACK(on_button_delete_industria_clicked),
    "on_btn_cancelar_clicked",       			G_CALLBACK(on_btn_cancelar_clicked),
    "on_btn_salvar_responsavel_clicked",        G_CALLBACK(on_btn_salvar_responsavel_clicked),
	"on_btn_salva_ind_clicked",					G_CALLBACK(on_btn_salva_ind_clicked),
	"on_btn_voltar_ind_clicked",				G_CALLBACK(on_btn_voltar_ind_clicked),
	"on_btn_relatorio_csv_ind_clicked",			G_CALLBACK(on_btn_relatorio_csv_ind_clicked),
	"on_btn_relatorio_txt_ind_clicked",			G_CALLBACK(on_btn_relatorio_txt_ind_clicked),
	"on_btn_add_residuos_clicked",				G_CALLBACK(on_btn_add_residuos_clicked),
	"on_btn_delete_ind_clicked",				G_CALLBACK(on_btn_delete_ind_clicked),
	NULL);
    gtk_builder_connect_signals(builder, NULL);

    stack = GTK_STACK(gtk_builder_get_object(builder, "stack"));
    window = GTK_WIDGET(gtk_builder_get_object(builder, "main_login"));

    gtk_widget_show_all(window);
    gtk_main();

    GtkTreeView *tree_view = GTK_TREE_VIEW(gtk_builder_get_object(builder, "id_listar_industrias"));
    GtkTreeSelection *selection = gtk_tree_view_get_selection(tree_view);
	gtk_tree_selection_set_mode(selection, GTK_SELECTION_SINGLE);

    return 0;
}
