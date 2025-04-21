#include <gtk/gtk.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <openssl/evp.h>

// Função para hash da senha usando EVP
char *hash_password(const char *password) {
    EVP_MD_CTX *mdctx;
    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned int hash_len;

    mdctx = EVP_MD_CTX_new();
    EVP_DigestInit_ex(mdctx, EVP_sha256(), NULL);
    EVP_DigestUpdate(mdctx, password, strlen(password));
    EVP_DigestFinal_ex(mdctx, hash, &hash_len);
    EVP_MD_CTX_free(mdctx);

    char *output = malloc(hash_len * 2 + 1);
    for (unsigned int i = 0; i < hash_len; i++) {
        sprintf(output + (i * 2), "%02x", hash[i]);
    }
    output[hash_len * 2] = 0;

    return output;
}

void save_user_data(const char *username, const char *hashed_password) {
    FILE *file = fopen("users.txt", "a");
    if (file != NULL) {
        fprintf(file, "%s,%s\n", username, hashed_password);
        fclose(file);
    } else {
        perror("Não foi possível abrir o arquivo users.txt");
    }
}

void on_login_button_clicked(GtkWidget *widget, gpointer data) {
    // Lógica de login (não implementada aqui)
}

void on_register_button_clicked(GtkWidget *widget, gpointer data) {
     // Converte data para GtkWidget**
     GtkWidget **entries = (GtkWidget **)data; 
    
     // Checa se entries e seus componentes são válidos
     if (entries == NULL) {
         g_printerr("Erro: Entradas não foram passadas corretamente (entries NULL).\n");
         return;
     }
     
     // Verifica se há widgets em entries
     for (int i = 0; i < 2; i++) {
         if (entries[i] == NULL) {
             g_printerr("Erro: entries[%d] é NULL.\n", i);
         } else if (!GTK_IS_ENTRY(entries[i])) {
             g_printerr("Erro: entries[%d] não é uma GtkEntry válida.\n", i);
         }
     }
 
     GtkWidget *username_entry = entries[0]; 
     GtkWidget *password_entry = entries[1]; 
 
     // Verifique se são entradas válidas
     if (GTK_IS_ENTRY(username_entry) && GTK_IS_ENTRY(password_entry)) {
         const char *username = gtk_entry_get_text(GTK_ENTRY(username_entry));
         const char *password = gtk_entry_get_text(GTK_ENTRY(password_entry));
 
         // Verifique se username e password não são vazios
         if (username == NULL || strlen(username) == 0 || password == NULL || strlen(password) == 0) {
             g_printerr("Erro: Valores de entrada são NULL ou vazios.\n");
             return;
         } else {
             // Printando para depuração
             g_print("Username: %s, Password: %s\n", username, password);
             
             // Hash e salvar dados
             char *hashed_password = hash_password(password);
             save_user_data(username, hashed_password);
             free(hashed_password);
         }
     } else {
         g_printerr("Erro: Um ou mais widgets não são entradas válidas.\n");
     }
}

void activate(GtkApplication *app, gpointer user_data) {
    GtkWidget *window;
    GtkWidget *grid;
    GtkWidget *username_entry;
    GtkWidget *password_entry;
    GtkWidget *login_button;
    GtkWidget *register_button;

    window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "Sistema de Gestão");
    gtk_container_set_border_width(GTK_CONTAINER(window), 10);

    grid = gtk_grid_new();
    gtk_container_add(GTK_CONTAINER(window), grid);

    username_entry = gtk_entry_new();
    password_entry = gtk_entry_new();
    gtk_entry_set_visibility(GTK_ENTRY(password_entry), FALSE);

    GtkWidget *form_entries[2] = {username_entry, password_entry};
    g_print("Username Entry: %p, Password Entry: %p\n", username_entry, password_entry);

    login_button = gtk_button_new_with_label("Entrar");
    g_signal_connect(login_button, "clicked", G_CALLBACK(on_login_button_clicked), form_entries);

    g_print("Conectando sinais...\n");
    register_button = gtk_button_new_with_label("Cadastrar");
    g_signal_connect(register_button, "clicked", G_CALLBACK(on_register_button_clicked), form_entries);

    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Usuário:"), 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), username_entry, 1, 0, 2, 1);
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Senha:"), 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), password_entry, 1, 1, 2, 1);
    gtk_grid_attach(GTK_GRID(grid), login_button, 0, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), register_button, 1, 2, 1, 1);

    gtk_widget_show_all(window);
}

int main(int argc, char **argv) {
    g_print("Iniciando a aplicação...\n");
    GtkApplication *app;
    int status;

    app = gtk_application_new("br.exemplo.gtk", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    g_print("Sinal 'activate' conectado.\n");

    status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);

    g_print("Aplicação finalizada.\n");
    return status;
}