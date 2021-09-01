#include <allegro.h>

// funcoes
void sair(); // Sair
void tempo(); // Tempo
void game(); // Jogo
void control(); // Controles
int colidir(int Ax, int Ay, int Bx, int By, int Aw, int Ah, int Bw, int Bh); // Colisoes

// variaveis globais
int sai = 0;
int rnd = 0; // Obstaculo Aleatorio
int som = 0;
int cx1 = 0; // img inicio chao
int cx2 = 1185; // img final chao
int piso = 500;
int press = 0;
int gover = 1; // GameOver
int Bonus = 0;
int width = 800; // Largura
int dAnim = 0; // Dinossauro animação
int pAnim = 0; // Pterodáctilo animação
int score = 0; // Pontos
int timer = 0;
int speed = 12; // Velocidade Jogo
float vup = 0.5; // Velocidade Subida 
float vely = 0; // Velocidade eixo Y
int height = 600; // Altura
int hscore = 0; // Recorde de pontos
int caindo = 1; // Dino caindo
int pulando = 0; // Dino pulando

struct obj{int wx, wy, x, y, w, h;};
struct obj
  enemy[9]={
    0, 218, 1500, 415, 34, 78,
    35, 218, 0, 415, 74, 78,
    110, 218, 0, 415, 112, 78,
    222, 185, 0, 385, 49, 106,
    274, 185, 0, 385, 108, 106,
    383, 185, 0, 385, 166, 106,
    554, 212, 0, 305, 95, 80,
    554, 212, 0, 345, 95, 80,
    554, 212, 0, 430, 95, 80
  }, // Inimigos/Colisoes
  nuvem[3] = {
    765, 225, 0, 0, 110, 40,
    765, 225, 0, 0, 110, 40,
    765, 225, 0, 0, 110, 40
  }, // Nuvens Cenario
  dino = {0, 0, 100, 400, 92, 98}; // Personagem DINO

SAMPLE *pulo, *bonus, *morto;
BITMAP *buffer, *dinoimg;
// FONT *f28;

int main() {
  // iniciacao
  allegro_init();
  install_timer();
  install_keyboard();
  install_sound(DIGI_AUTODETECT, MIDI_AUTODETECT, NULL);
  install_mouse();
  set_color_depth(32);
  set_window_title("DINO");
  set_close_button_callback(sair);
  install_int_ex(tempo, MSEC_TO_TIMER(13));
  set_gfx_mode(GFX_AUTODETECT_WINDOWED, width, height, 0, 0);

  // variaveis locais
  buffer = create_bitmap(width, height);
  dinoimg = load_bitmap("dinoimg.bmp", NULL); // Carregar imagem em bitmap
  // Carrega os sons do Jogo
  pulo = load_sample("pulo.wav"); 
  morto = load_sample("morto.wav");
  bonus = load_sample("bonus.wav");
	
  while (!(sai || key[KEY_ESC])) {
    show_mouse(screen);

    rectfill(buffer, 0, 0, width, height, 0xffffffff);  // Janela Retangulo
    masked_blit(dinoimg, buffer, 0, 315, cx1, 475, 1185, 50); // Traçado chao
    masked_blit(dinoimg, buffer, 0, 315, cx2, 475, 1185, 50); // Traçado chao
    game();
    textprintf_ex(buffer, font, 500, 50, 0x555555, -1, "SCORE"); // Texto Score

    if (!hscore && !score) {
      textprintf_ex(buffer, font, 550, 50, 0x555555, -1, "00000 00000");
    } else {
      textprintf_right_ex(buffer, font, 770, 50, 0x555555, 0xffffff, " %i", score);
      if (!hscore) {
        textprintf_ex(buffer, font, 550, 50, 0x555555, -1, "00000");
      } else {
        textprintf_ex(buffer, font, 550, 50, 0x555555, 0xffffff, " %i", hscore);
      }
    }

  // Exibir na tela
    masked_blit(dinoimg, buffer, enemy[rnd].wx + (rnd > 5) * (pAnim/14) * enemy[rnd].w, enemy[rnd].wy, enemy[rnd].x, enemy[rnd].y, enemy[rnd].w, enemy[rnd].h); // Inimigo
    masked_blit(dinoimg, buffer, dino.wx + (dAnim/6) * dino.w, dino.wy, dino.x, dino.y, dino.w, dino.h); // Dino
    draw_sprite(screen, buffer, 0, 0);
    timer = 0;
    while(!timer);
    clear_bitmap(buffer);
  }

  // Finalizacao, destroi imagens e texto
  destroy_bitmap(buffer);
  destroy_bitmap(dinoimg);
  destroy_sample(bonus);
  destroy_sample(morto);
  destroy_sample(pulo);
	return 0;
}

END_OF_MAIN();

void control () {
  som = (pulando && dino.y > piso - 100);
  pulando = (key[KEY_SPACE] && !caindo && dino.y >= piso - dino.h * 2); // altura do pulo
  caindo = (!pulando && dino.y + dino.h < piso); // Dino caindo

  // Pulando
  if (pulando) {
    vely -= vup;
  }

  // Caindo
  if (caindo) {
    vely += vup;
  }

  // Dino abaixar
  if (key[KEY_DOWN] && !pulando && !caindo) {
    dino.wy = 102;
    dino.w = 126;
    dino.h = 59;
    dino.y = piso;
  } else {
    dino.wy = 0;
    dino.w = 92;
    dino.h = 98;
  }

  // Pulando ou caindo parar animação dos pezinhos
  if (pulando || caindo) {
    dino.y += vely;
    dino.wx = 184;
    dAnim = 0;
  } else { // Se não zera tudo
    vely = 0;
    dino.wx = 0;
  }

  // Forçar a ficar junto com o piso
  if (dino.y + dino.h >= piso) {
    dino.y = piso - dino.h;
  }
}

// Logica do Jogo
void game() {
  if (score > 1000) {
    MSEC_TO_TIMER(5);
  } // Se Dino colidir com qualquer obstaculo | 3 testes de colisao
  if (colidir(dino.x, dino.y + 34, enemy[rnd].x, enemy[rnd].y, 70, 32, enemy[rnd].w, enemy[rnd].h) ||
      colidir(dino.x + 44, dino.y, enemy[rnd].x, enemy[rnd].y, 46, 33, enemy[rnd].w, enemy[rnd].h) ||
      colidir(dino.x + 22, dino.y + 68, enemy[rnd].x, enemy[rnd].y, 30, 30, enemy[rnd].w, enemy[rnd].h)) {
    if(!gover) {
      som = 3; // Som de GameOver
    }
    gover = 1;
  }

  // Se nao precionar tecla e GameOver e precionar Spaço
  if (!press && gover && key[KEY_SPACE]) {
    if (hscore < score) { // Se pontos for maior q o recorde substitui
      hscore = score;
    }
    rnd = 0;
    gover = 0;
    score = 0;
    dino.y = piso - dino.h; // Dino no chão | piso - comprimento do Dino
    for (int i = 0; i < 9; i++) { // Obstaculos
      enemy[i].x = width * 2;
    }
  }

  if (gover) {  // Se GameOver
    dAnim = 0; // Zera animação Dino
    dino.wx = 184;  // Primeira img do Dino
    masked_blit(dinoimg, buffer, 290, 0, 200, 150, 455, 155); // Imagem GameOver
  } else {
    score++; // Pontos aumenta

    control(); // Controles

    if (dAnim++ > 10) { // Animação Dino
      dAnim = 0;
    }

    if (pAnim++ > 26) { // Animação Pterodáctilo
      pAnim = 0;
    }

    cx1 -= speed; // img chao
    cx2 -= speed; //img chao
    enemy[rnd].x -= speed; // Inimigo aleatorio voltando para tras
    if (cx1 < -1185) { // final da img chao | Animação chao movendo
      cx1 = 0;
      cx2 = 1185;
    }
    if (enemy[rnd].x < -112) { // Inimigos aparecendo
      if (score > 2000) {
        rnd = rand() % 9; // Cactos + Pterodáctilo
      } else {
        rnd = rand() % 6; // Cactos
      }
      enemy[rnd].x = (rand() % 20) * 100 + width; // Posição obstaculo aleatorio intervalo
    }

    // Animação nuvens de forma aleatoria
    for (int i = 0; i < 3; i++) {
      if (nuvem[i].x-- < -100) {
        nuvem[i].x = (rand() % 20) * 50 + width;
        nuvem[i].y = (rand() % 10) * 25 + 150;
      }

      // Exibir nuvens
      masked_blit(dinoimg, buffer, nuvem[i].wx, nuvem[i].wy, nuvem[i].x, nuvem[i].y, nuvem[i].w, nuvem[i].h);
    }
  }

  press = key[KEY_SPACE]; // Monitorando tecla espaço
  // sons
  if (score > Bonus + 1000) { // Som de bonus
    Bonus = score;
    som = 2;
  }

  switch (som) {
    case 1: play_sample(pulo, 80, 128, 2000, 0); break; // Som do salto
    case 2: play_sample(bonus, 255, 128, 1000, 0); break; // Som do bonus
    case 3: play_sample(morto, 100, 128, 2000, 0); break; // Som de GameOver
  }

  som = 0;
}

int colidir(int Ax, int Ay, int Bx, int By, int Aw, int Ah, int Bw, int Bh){
	if(Ax + Aw > Bx && Ax < Bx + Bw && Ay + Ah > By && Ay < By + Bh)
	return 1; return 0;
}

void sair() {
  sai = 1;
};
void tempo() {
  timer++;
};
END_OF_FUNCTION(sair);
