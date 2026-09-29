#include "raylib.h"
#include <stdlib.h>
#include <stdio.h>

#define MAX_ENTIDADES 50
#define ARQUIVO_PLACAR "placar.txt"
#define ARQUIVO_SAVE "save.bin"

typedef enum { JOGADOR, MOEDA, INIMIGO } TipoEntidade;

typedef struct {
    TipoEntidade tipo;
    Vector2 pos;
    Color cor;
    bool ativa;
} Entidade;

Entidade* vetorEntidades[MAX_ENTIDADES];
int totalEntidades = 0;
Entidade* jogador = NULL;
int pontuacao = 0;
char mensagem[50] = "Use as setas para mover!";
double tempoMensagem = 0.0;

void adicionarEntidade(Entidade* e) {
    if (totalEntidades < MAX_ENTIDADES) {
        vetorEntidades[totalEntidades] = e;
        totalEntidades++;
    }
}

void liberarTodasEntidades(void) {
    for (int i = 0; i < totalEntidades; i++) {
        if (vetorEntidades[i] != NULL) {
            free(vetorEntidades[i]);
            vetorEntidades[i] = NULL;
        }
    }
    totalEntidades = 0;
    jogador = NULL;
}

void inicializarJogo(void) {
    liberarTodasEntidades();
    pontuacao = 0;

    jogador = (Entidade*)malloc(sizeof(Entidade));
    jogador->tipo = JOGADOR;
    jogador->pos = (Vector2){ 400, 300 };
    jogador->cor = BLUE;
    jogador->ativa = true;
    adicionarEntidade(jogador);

    for (int i = 0; i < 5; i++) {
        Entidade* m = (Entidade*)malloc(sizeof(Entidade));
        m->tipo = MOEDA;
        m->pos = (Vector2){ (float)GetRandomValue(50, 750), (float)GetRandomValue(50, 400) };
        m->cor = GREEN;
        m->ativa = true;
        adicionarEntidade(m);
    }
}

void salvarPlacarTexto(const char* nome, int pontos) {
    FILE* arquivo = fopen(ARQUIVO_PLACAR, "a");
    if (arquivo == NULL) return;

    fprintf(arquivo, "%s %d\n", nome, pontos);
    fclose(arquivo);
}

int lerMelhorPontuacao(void) {
    FILE* arquivo = fopen(ARQUIVO_PLACAR, "r");
    if (arquivo == NULL) return 0;

    int melhor = 0, valor = 0;
    char nomeLido[16];

    while (fscanf(arquivo, "%15s %d", nomeLido, &valor) == 2) {
        if (valor > melhor) melhor = valor;
    }
    fclose(arquivo);
    return melhor;
}

bool salvarJogoBinario(void) {
    FILE* arquivo = fopen(ARQUIVO_SAVE, "wb");
    if (arquivo == NULL) return false;

    fwrite(&totalEntidades, sizeof(int), 1, arquivo);
    
    for (int i = 0; i < totalEntidades; i++) {
        fwrite(vetorEntidades[i], sizeof(Entidade), 1, arquivo);
    }

    fclose(arquivo);
    return true;
}

bool carregarJogoBinario(void) {
    FILE* arquivo = fopen(ARQUIVO_SAVE, "rb");
    if (arquivo == NULL) return false;

    int totalSalvo = 0;
    if (fread(&totalSalvo, sizeof(int), 1, arquivo) != 1) {
        fclose(arquivo);
        return false;
    }

    liberarTodasEntidades();

    for (int i = 0; i < totalSalvo; i++) {
        Entidade* e = (Entidade*)malloc(sizeof(Entidade));
        if (fread(e, sizeof(Entidade), 1, arquivo) != 1) {
            free(e);
            break;
        }
        adicionarEntidade(e);
    }

    fclose(arquivo);
    
    if (totalEntidades > 0) jogador = vetorEntidades[0];
    
    return true;
}

int main(void) {
    const int larguraTela = 800;
    const int alturaTela = 600;
    InitWindow(larguraTela, alturaTela, "ED1 - Atividade 6: Manipulacao de Arquivos");

    char nomeJogador[16] = "Eric";

    inicializarJogo();
    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        if (jogador != NULL) {
            if (IsKeyDown(KEY_RIGHT)) jogador->pos.x += 4;
            if (IsKeyDown(KEY_LEFT))  jogador->pos.x -= 4;
            if (IsKeyDown(KEY_UP))    jogador->pos.y -= 4;
            if (IsKeyDown(KEY_DOWN))  jogador->pos.y += 4;
        }

        for (int i = 0; i < totalEntidades; i++) {
            if (vetorEntidades[i]->tipo == MOEDA && vetorEntidades[i]->ativa) {
                if (CheckCollisionCircles(jogador->pos, 15, vetorEntidades[i]->pos, 10)) {
                    vetorEntidades[i]->ativa = false;
                    pontuacao += 10;
                    vetorEntidades[i]->pos = (Vector2){ (float)GetRandomValue(50, 750), (float)GetRandomValue(50, 400) };
                    vetorEntidades[i]->ativa = true;
                }
            }
        }

        if (IsKeyPressed(KEY_F5)) {
            salvarPlacarTexto(nomeJogador, pontuacao);
            TextCopy(mensagem, "Pontuacao adicionada ao txt!");
            tempoMensagem = GetTime() + 2.0;
        }

        if (IsKeyPressed(KEY_F6)) {
            if (salvarJogoBinario()) {
                TextCopy(mensagem, "Estado do jogo salvo em binario!");
            } else {
                TextCopy(mensagem, "Erro ao salvar jogo!");
            }
            tempoMensagem = GetTime() + 2.0;
        }

        if (IsKeyPressed(KEY_F9)) {
            if (carregarJogoBinario()) {
                TextCopy(mensagem, "Jogo carregado com sucesso!");
            } else {
                TextCopy(mensagem, "Falha ao carregar save!");
            }
            tempoMensagem = GetTime() + 2.0;
        }

        if (IsKeyPressed(KEY_DELETE)) {
            if (remove(ARQUIVO_SAVE) == 0) {
                TextCopy(mensagem, "Arquivo save.bin deletado do disco!");
            } else {
                TextCopy(mensagem, "Nenhum arquivo save encontrado.");
            }
            tempoMensagem = GetTime() + 2.0;
        }

        BeginDrawing();
        ClearBackground(RAYWHITE);

        DrawText(TextFormat("Jogador: %s  |  Pontos: %d", nomeJogador, pontuacao), 20, 20, 20, BLACK);
        DrawText(TextFormat("Maior Recorde Geral (Lido do TXT): %d", lerMelhorPontuacao()), 20, 50, 18, GOLD);
        DrawText("Comandos: [F5] Gravar Placar TXT | [F6] Save Bin | [F9] Load Bin | [DELETE] Limpar Save", 20, 80, 14, GRAY);

        if (GetTime() < tempoMensagem) {
            DrawText(mensagem, 20, 550, 18, RED);
        } else {
            DrawText("Use as setas para mover!", 20, 550, 18, LIGHTGRAY);
        }

        for (int i = 0; i < totalEntidades; i++) {
            if (vetorEntidades[i] != NULL && vetorEntidades[i]->ativa) {
                if (vetorEntidades[i]->tipo == JOGADOR) {
                    DrawCircleV(vetorEntidades[i]->pos, 15, vetorEntidades[i]->cor);
                } else if (vetorEntidades[i]->tipo == MOEDA) {
                    DrawCircleV(vetorEntidades[i]->pos, 10, vetorEntidades[i]->cor);
                }
            }
        }

        EndDrawing();
    }

    liberarTodasEntidades();
    CloseWindow();
    return 0;
}
