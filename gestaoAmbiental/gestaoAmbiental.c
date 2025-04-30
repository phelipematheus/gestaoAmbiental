#include <gtk/gtk.h>
#include <stdlib.h>
#include <stdbool.h>
#include <openssl/aes.h>
#include <openssl/rand.h>

#define MAX_LINE_LENGTH 256
#define DB_FILE "usuarios.txt"
#define DADOS_FILE "dados.txt"
#define DADOS_RESIDUOS_FILE "dados_residuos.txt"

GtkBuilder *builder;
GtkWidget *window;
GtkStack *stack;
gchar *text_to_save = NULL;

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

void carregar_dados_residuos(GtkListStore *liststore, FILE *arquivo) {
	char linha[MAX_LINE_LENGTH];
    while (fgets(linha, sizeof(linha), arquivo)) {
        char cnpj_empresa[15], mes_residuo_ind[10], ano_residuo_ind[5];
        char qtd_residuos_ind[1000], custo_residuo_ind[1000];

        int ret = sscanf(linha, "%14[^,],%9[^,],%4[^,],%999[^,],%999[^,\n]",
			cnpj_empresa, mes_residuo_ind, ano_residuo_ind, qtd_residuos_ind, custo_residuo_ind);

		if (ret == 5) {
            GtkTreeIter iter;
            gtk_list_store_append(liststore, &iter);
            gtk_list_store_set(liststore, &iter,
                               0, cnpj_empresa,
                               1, mes_residuo_ind,
                               2, ano_residuo_ind,
                               3, qtd_residuos_ind,
                               4, custo_residuo_ind,
                               -1);
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
                  const char *email_empresa, const char *telefone_empresa, const char *endereco_empresa,
                  const char *data_abertura_empresa){
    FILE *file = fopen(DADOS_FILE, "a");

    if (!file) {
        perror("Erro ao abrir o arquivo");
        return;
    }

    fprintf(file, "%s,%s,%s,%s,%s,%s,%s,%s,%s,%s\n",
            nome_responsavel,
            cpf_responsavel,
            nome_empresa,
            cnpj_empresa,
            razao_social_empresa,
            nome_fantasia_empresa,
            email_empresa,
            telefone_empresa,
            endereco_empresa,
            data_abertura_empresa);

    fclose(file);
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
}

void on_button_cadastrar_industria_clicked(GtkWidget *widget, gpointer data) {
    gtk_stack_set_visible_child_name(stack, "view_cadastrar_industria");
}

void on_button_listar_industrias_clicked(GtkWidget *widget, gpointer data)
{
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
    char *cpf_responsavel = gtk_entry_get_text(GTK_ENTRY(gtk_builder_get_object(builder, "input_cpf_responsavel")));// numerico
    char *nome_empresa = gtk_entry_get_text(GTK_ENTRY(gtk_builder_get_object(builder, "input_nome_empresa")));
    char *cnpj_empresa = gtk_entry_get_text(GTK_ENTRY(gtk_builder_get_object(builder, "input_cnpj")));//numerico
    char *razao_social_empresa = gtk_entry_get_text(GTK_ENTRY(gtk_builder_get_object(builder, "input_razao_social")));
    char *nome_fantasia_empresa = gtk_entry_get_text(GTK_ENTRY(gtk_builder_get_object(builder, "input_nome_fantasia")));
    char *email_empresa = gtk_entry_get_text(GTK_ENTRY(gtk_builder_get_object(builder, "input_emai")));
    char *telefone_empresa = gtk_entry_get_text(GTK_ENTRY(gtk_builder_get_object(builder, "input_telefone")));
    char *endereco_empresa = gtk_entry_get_text(GTK_ENTRY(gtk_builder_get_object(builder, "input_endereco")));
	GtkCalendar *data_abertura_calendar = GTK_CALENDAR(gtk_builder_get_object(builder, "input_data_abertura"));
    char data_abertura_empresa[11];
	guint ano, mes, dia;
    gtk_calendar_get_date(data_abertura_calendar, &ano, &mes, &dia);
    mes += 1;
    sprintf(data_abertura_empresa, "%02d/%02d/%04d", dia, mes, ano);

	printf(data_abertura_empresa);

    salvar_dados(nome_responsavel, cpf_responsavel, nome_empresa,
                  cnpj_empresa, razao_social_empresa, nome_fantasia_empresa,
                  email_empresa, telefone_empresa, endereco_empresa, data_abertura_empresa);

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
	g_free(text_to_save);

	GtkListStore *liststore = GTK_LIST_STORE(gtk_builder_get_object(builder, "lista_rel_ind"));
	gtk_list_store_clear(liststore);
	FILE *arquivo = fopen(DADOS_RESIDUOS_FILE, "r");
	if (!arquivo) {
        perror("Erro ao abrir o arquivo");
        return;
    } else {
		carregar_dados_residuos(liststore, arquivo);
    }
}

void on_btn_voltar_ind_clicked(GtkWidget *widget, gpointer data) {
	gtk_stack_set_visible_child_name(stack, "view_industria");
}

void on_btn_relatorio_csv_ind_clicked(GtkWidget *widget, gpointer data) {
	gtk_main_quit();
}

void on_btn_relatorio_txt_ind_clicked(GtkWidget *widget, gpointer data) {
	gtk_main_quit();
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
    "on_button_listar_industrias_clicked",      G_CALLBACK(on_button_listar_industrias_clicked),
    "on_btn_cancelar_clicked",       			G_CALLBACK(on_btn_cancelar_clicked),
    "on_btn_salvar_responsavel_clicked",        G_CALLBACK(on_btn_salvar_responsavel_clicked),
	"on_btn_salva_ind_clicked",					G_CALLBACK(on_btn_salva_ind_clicked),
	"on_btn_voltar_ind_clicked",				G_CALLBACK(on_btn_voltar_ind_clicked),
	"on_btn_relatorio_csv_ind_clicked",			G_CALLBACK(on_btn_relatorio_csv_ind_clicked),
	"on_btn_relatorio_txt_ind_clicked",			G_CALLBACK(on_btn_relatorio_txt_ind_clicked),
	"on_btn_add_residuos_clicked",				G_CALLBACK(on_btn_add_residuos_clicked),
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
