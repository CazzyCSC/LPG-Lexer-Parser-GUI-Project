//CSC 4101 Project: Lexer, Parser, and GUI (LPG)
//Zachary Sheridan
//Anas Mahmoud
//2 May 2025

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <gtk/gtk.h>

#define MAX_TOKEN_LEN 100
#define MAX_TOKENS 1000

//Token Definitions
typedef enum
{
    T_PROGRAM, T_END_PROGRAM, T_IF, T_LOOP,
    T_END_IF, T_END_LOOP,
    T_ASSIGN, T_PLUS, T_MINUS, T_MUL, T_DIV, T_MOD,
    T_LPAREN, T_RPAREN, T_SEMI, T_COLON,
    T_EQ, T_NEQ, T_GT, T_LT, T_GTE, T_LTE,
    T_AND, T_OR,
    T_ID, T_NUM,
    T_EOF, T_INVALID
} TokenType;

typedef struct 
{
    TokenType type;
    char lexeme[MAX_TOKEN_LEN];
} Token;

//Token Storage
Token tokens[MAX_TOKENS];
int tokenIndex = 0, current = 0;

//Error Check:Use in Parser
int errorOccurred = 0;
char errorLog[4096];
int errorCount = 0;

//-------------------------------
//Lexer:Oh Yeah, It's Lexing Time
//-------------------------------

//Check for keywords
int isKeyword(const char* str) 
{
    return strcmp(str, "program") == 0 || strcmp(str, "end_program") == 0 ||
           strcmp(str, "if") == 0 || strcmp(str, "end_if") == 0 ||
           strcmp(str, "loop") == 0 || strcmp(str, "end_loop") == 0;
}

//Assign keywords a Token
TokenType keywordType(const char* str) 
{
    if (strcmp(str, "program") == 0) return T_PROGRAM;
    if (strcmp(str, "end_program") == 0) return T_END_PROGRAM;
    if (strcmp(str, "if") == 0) return T_IF;
    if (strcmp(str, "end_if") == 0) return T_END_IF;
    if (strcmp(str, "loop") == 0) return T_LOOP;
    if (strcmp(str, "end_loop") == 0) return T_END_LOOP;
    return T_ID; //Fallback for non-keywords
}

//Add new token to global tokens[] array
void addToken(TokenType type, const char* lexeme) 
{
    tokens[tokenIndex].type = type;
    strncpy(tokens[tokenIndex].lexeme, lexeme, MAX_TOKEN_LEN);
    tokenIndex++;
}

//The Heart-read source code and build the token list
void lexer(const char* src) 
{
    int i = 0;
    while (src[i] != '\0') 
    {
        //Whitespace and comment skipping
        if (isspace(src[i])) { i++; continue; }
        if (src[i] == '/' && src[i+1] == '/') { while (src[i] != '\n' && src[i] != '\0') i++; continue; }
        //Word: make sure it starts with a letter, then iterate
        if (isalpha(src[i]) || src[i] == '_') {
            char buf[MAX_TOKEN_LEN] = {0};
            int j = 0;
            //Keep copying characters as long as letters or digits OR THE UNDERSCORE. HOLY MOTHER OF GOD THE UNDERSCORE WAS THE PROBLEM(I spent over 15 hours trying to fix this.)
            while (isalnum(src[i]) || src[i] == '_') buf[j++] = src[i++];
            //If a keyword, assign token type, store in token index
            if (isKeyword(buf)) addToken(keywordType(buf), buf);
            //Not a keyword then identifier, store in token index
            else addToken(T_ID, buf);
        }
        //Digit: get whole digit, store in token index
        else if (isdigit(src[i])) 
        {
            char buf[MAX_TOKEN_LEN] = {0};
            int j = 0;
            while (isdigit(src[i])) buf[j++] = src[i++];
            addToken(T_NUM, buf);
        }
        //Otherwise check for operands/errors
        else 
        {
            char buf[3] = {src[i], src[i+1], '\0'};
            switch (src[i]) 
            {
                case '=': addToken(src[i+1] == '=' ? (i++, T_EQ) : T_ASSIGN, src[i+1] == '=' ? "==" : "="); break;
                case '!': if (src[i+1] == '=') { addToken(T_NEQ, "!="); i++; } break;
                case '>': addToken(src[i+1] == '=' ? (i++, T_GTE) : T_GT, src[i+1] == '=' ? ">=" : ">"); break;
                case '<': addToken(src[i+1] == '=' ? (i++, T_LTE) : T_LT, src[i+1] == '=' ? "<=" : "<"); break;
                case '&': if (src[i+1] == '&') { addToken(T_AND, "&&"); i++; } break;
                case '|': if (src[i+1] == '|') { addToken(T_OR, "||"); i++; } break;
                case '+': addToken(T_PLUS, "+"); break;
                case '-': addToken(T_MINUS, "-"); break;
                case '*': addToken(T_MUL, "*"); break;
                case '/': addToken(T_DIV, "/"); break;
                case '%': addToken(T_MOD, "%"); break;
                case ';': addToken(T_SEMI, ";"); break;
                case ':': addToken(T_COLON, ":"); break;
                case '(': addToken(T_LPAREN, "("); break;
                case ')': addToken(T_RPAREN, ")"); break;
                default: addToken(T_INVALID, buf);
            }
            i++;
        }
    }
    //End of file
    addToken(T_EOF, "eof");
}


//-------------------------------
//Parser: PARSING TIME(RDP)
//-------------------------------

//Collect a log of errors for display
void error(const char* msg) 
{
    errorOccurred = 1;
    errorCount++;
    strncat(errorLog, msg, sizeof(errorLog) - strlen(errorLog) - 1);
    strncat(errorLog, "\n", sizeof(errorLog) - strlen(errorLog) - 1);
}
//See current token
Token peek() { return tokens[current]; }
//Move to next token
Token advance() { return tokens[current++]; }
//If token matches expeceted type, eat it
int match(TokenType type) 
{
    if (peek().type == type) { advance(); return 1; }
    return 0;
}
//Force token match or error
const char* tokenTypeName(TokenType type) 
{
    switch (type) 
    {
        case T_PROGRAM: return "program";
        case T_END_PROGRAM: return "end_program";
        case T_END_IF: return "end_if";
        case T_END_LOOP: return "end_loop";
        case T_IF: return "if";
        case T_LOOP: return "loop";
        case T_ASSIGN: return "=";
        case T_PLUS: return "+";
        case T_MINUS: return "-";
        case T_MUL: return "*";
        case T_DIV: return "/";
        case T_MOD: return "%";
        case T_LPAREN: return "(";
        case T_RPAREN: return ")";
        case T_SEMI: return ";";
        case T_COLON: return ":";
        case T_EQ: return "==";
        case T_NEQ: return "!=";
        case T_GT: return ">";
        case T_LT: return "<";
        case T_GTE: return ">=";
        case T_LTE: return "<=";
        case T_AND: return "&&";
        case T_OR: return "||";
        case T_ID: return "identifier";
        case T_NUM: return "number";
        case T_EOF: return "end of file";
        case T_INVALID: return "invalid token";
        default: return "unknown";
    }
}
//Specific error messaging
void expect(TokenType type) 
{
    if (!match(type)) 
    {
        char msg[128];
        snprintf(msg, sizeof(msg), "Expected '%s', but found '%s' (%s)",
                 tokenTypeName(type), peek().lexeme, tokenTypeName(peek().type));
        error(msg);
        advance();
    }
}

//RDP TIME: Setting precedence
void parseExpression();
void parseStatement();
void parseStatements();

void parseFactor() 
{
    if (match(T_ID) || match(T_NUM)) return;
    else if (match(T_LPAREN)) { parseExpression(); expect(T_RPAREN); }
    else error("Expected number, identifier or '('");
}

void parseTerm() 
{
    parseFactor();
    while (peek().type == T_MUL || peek().type == T_DIV || peek().type == T_MOD) {
        advance(); parseFactor();
    }
}

void parseExpression() 
{
    parseTerm();
    while (peek().type == T_PLUS || peek().type == T_MINUS) {
        advance(); parseTerm();
    }
}
//Logical and conditional parsing
void parseCondition() 
{
    //Left side
    parseExpression();
    //Condition
    if (!(match(T_EQ) || match(T_NEQ) || match(T_GT) || match(T_LT) || match(T_GTE) || match(T_LTE))) 
    {
        error("Expected comparison operator");
    }
    //Right side
    parseExpression();
}

void parseLogicExpression() 
{
    parseCondition();
    while (match(T_AND) || match(T_OR)) 
    {
        parseCondition();
    }
}

void parseIf() 
{
    expect(T_IF);
    expect(T_LPAREN);
    parseLogicExpression();
    expect(T_RPAREN);
    parseStatements();
    expect(T_END_IF);
}

void parseLoop() 
{
    expect(T_LOOP);
    expect(T_LPAREN);
    expect(T_ID);
    expect(T_ASSIGN);
    parseExpression();
    expect(T_COLON);
    parseExpression();
    expect(T_RPAREN);
    parseStatements();
    expect(T_END_LOOP);
}
//Assignment operation parsing
void parseAssignment() 
{
    expect(T_ID);
    expect(T_ASSIGN);
    parseExpression();
    expect(T_SEMI);
}
//General statement parsing
void parseStatement() 
{
    if (peek().type == T_IF) parseIf();
    else if (peek().type == T_LOOP) parseLoop();
    else if (peek().type == T_ID) parseAssignment();
    else 
    {
        char msg[128];
        snprintf(msg, sizeof(msg), "Unexpected statement starting with '%s' (%s)",
                 peek().lexeme, tokenTypeName(peek().type));
        error(msg);
        advance();  //Prevent infinite loop on bad statement
    }
}

void parseStatements() 
{
    while (peek().type != T_END_PROGRAM && peek().type != T_END_IF && peek().type != T_END_LOOP && peek().type != T_EOF) 
    {
        parseStatement();
    }
}
//Program entry
void parseProgram() 
{
    expect(T_PROGRAM);
    parseStatements();
    expect(T_END_PROGRAM);
}

//-------------------------------
//GUI: WINDOW TIME(GTK) + Main
//-------------------------------

void on_parse_clicked(GtkButton *button, gpointer user_data) 
{
    //Get data from text box(user input)
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(user_data));
    GtkTextIter start, end;
    gtk_text_buffer_get_bounds(buffer, &start, &end);
    gchar *text = gtk_text_buffer_get_text(buffer, &start, &end, FALSE);
    //Reset states
    tokenIndex = current = 0;
    errorOccurred = 0;
    lexer(text);
    if (!errorOccurred) 
    {
        parseProgram();
        if (!errorOccurred) 
        {
            //Only show success if no parser error either
            GtkWidget *dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL,
                GTK_MESSAGE_INFO, GTK_BUTTONS_OK,
                "Program parsed successfully!");
            gtk_dialog_run(GTK_DIALOG(dialog));
            gtk_widget_destroy(dialog);
        }
    }
    //Gimme errors in a log so you don't stop at the first one
    if (errorOccurred) 
    {
        GtkWidget *dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL,
            GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE,
            "Parser found %d error(s):\n\n%s", errorCount, errorLog);
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);

        //Flush log to prevent error buildup(bug)
        errorLog[0] = '\0';
        errorCount = 0;
    }
    g_free(text);
}

int main(int argc, char *argv[]) 
{
    //Initialize GTK runtime(idk what i am doing)
    gtk_init(&argc, &argv);
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "CSC 4101 LGP");
    gtk_window_set_default_size(GTK_WINDOW(window), 600, 400);
    //Layout setup
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    GtkWidget *textview = gtk_text_view_new();
    GtkWidget *button = gtk_button_new_with_label("Parse");
    //Quit button
    GtkWidget *quit_button = gtk_button_new_with_label("Quit");
    gtk_box_pack_start(GTK_BOX(vbox), quit_button, FALSE, FALSE, 0);
    g_signal_connect(quit_button, "clicked", G_CALLBACK(gtk_main_quit), NULL);

    gtk_box_pack_start(GTK_BOX(vbox), textview, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), button, FALSE, FALSE, 0);
    gtk_container_add(GTK_CONTAINER(window), vbox);
    //Event connections(what even is this)
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    g_signal_connect(button, "clicked", G_CALLBACK(on_parse_clicked), textview);
    //Startup
    gtk_widget_show_all(window);
    gtk_main();
    return 0;
}
