#include <gtk/gtk.h>
#include <stdlib.h>
#include <stdbool.h>

#define MAX_LINE_LENGTH 256
#define DB_FILE "usuarios.txt"

GtkBuilder *builder;
GtkWidget *window;
GtkStack *stack;

void mensagem (char texto_primario[100], char texto_secundario[100], char nome_icone[100])
{
    GtkMessageDialog *messageDialog = GTK_MESSAGE_DIALOG(gtk_builder_get_object(builder, "mensagem_dialog"));

    g_object_set(messageDialog, "text", texto_primario, NULL);
    g_object_set(messageDialog, "secondary_text", texto_secundario, NULL);
    g_object_set(messageDialog, "icon_name", nome_icone, NULL);

    gtk_widget_show_all(GTK_WIDGET(messageDialog));
    gtk_dialog_run(GTK_DIALOG(messageDialog));
    gtk_widget_hide(GTK_WIDGET(messageDialog));
}

int validar_usuario(const char *usuario, const char *senha) {
    FILE *file = fopen(DB_FILE, "r");
    char linha[MAX_LINE_LENGTH];

    if (!file) {
        perror("Erro ao abrir arquivo");
        return 0;
    }

    while (fgets(linha, sizeof(linha), file)) {
        char line_usuario[128];
        char line_senha[128];

        sscanf(line, "%127s %127s", line_usuario, line_senha);

        if (strcmp(line_usuario, usuario) == 0 && strcmp(line_senha, senha) == 0) {
            fclose(file);
            return 1;
        }
    }

    fclose(file);
    return 0;
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

void login(char *usuario, char *senha)
{
    if(validar_usuario(usuario, senha))
    {
        mensagem("Bem Vindo", "Usuario Logado com sucesso!","emblem-default");
        gtk_stack_set_visible_child_name(stack, "")
    }
    else
    {
//		TODO: fazer a chamada para orientar como cadastrar o usuario
//		if (resposta == 'y' || resposta == 'Y') {
//      	inserir_usuario(usuario, senha);
//          mensagem("Cadastro", "Usuario cadastrado com sucesso!", "emblem-default");
//      } else {
//      	mensagem("Login incorreto", "Email ou senha incorretos!", "dialog-error");
//      }
        mensagem("Login incorreto", "Email ou senha incorretos!","dialog-error");
    }
}

void on_button_login_clicked(GtkWidget *widget, gpointer data)
{
    char *usuario = gtk_entry_get_text(GTK_ENTRY(gtk_builder_get_object(builder, "usuario")));
    char *senha = gtk_entry_get_text(GTK_ENTRY(gtk_builder_get_object(builder, "senha")));
    login(usuario,senha);
}

void on_button_cadastro_clicked(GtkWidget *widget, gpointer data)
{
	if(validar_usuario(usuario, senha))
    {
        mensagem("Erro Cadastro", "Usuario já cadastrado","emblem-default");
    }
    else
    {

    }
}

void on_main_login_destroy(GtkWidget *widget, gpointer data)
{
    gtk_main_quit();
}

void on_butto_sair_industria_clicked(GtkWidget *widget, gpointer data)
{
    gtk_main_quit();

}void on_button_cadastrar_industria_clicked(GtkWidget *widget, gpointer data)
{
    gtk_main_quit();
}

void on_button_listar_industrias_clicked(GtkWidget *widget, gpointer data)
{
    gtk_main_quit();
}

void on_btn_cancelar_clicked(GtkWidget *widget, gpointer data)
{
    gtk_main_quit();
}

void on_btn_salvar_clicked(GtkWidget *widget, gpointer data)
{
    gtk_main_quit();
}

int main(int argc, char *argv[])
{
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
    "on_btn_cancelar_clicked",                  G_CALLBACK(on_btn_cancelar_clicked),
    "on_btn_salvar_clicked",                    G_CALLBACK(on_btn_salvar_clicked),
     NULL);
     gtk_builder_connect_signals(builder, NULL);

     stack = GTK_STACK(gtk_builder_get_object(builder, "stack"));
     window = GTK_WIDGET(gtk_builder_get_object(builder, "main_login"));

     gtk_widget_show_all(window);
     gtk_main();
     return 0;
}
