#include "lib.h"
#include <allegro5/allegro.h>
#include <allegro5/allegro_native_dialog.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>
#include <allegro5/color.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
//comentarios sobre globais estao todos no header

const float VELOCIDADE = 5.0;
const float HP=100.0;
const float ATAQUE=10.0;
const float DEFESA=2;
//removido
const float RAIO_P=150;

bool obj1; bool obj2; bool obj3; bool obj4;
//numero de passivos em casa, numero necessario
int numObj1; int limObj1;
//numero de transformados, numero necessario
int numObj2; int limObj2;

int LARGURA, ALTURA;

int opcao;
int estado = estPreMenu;
int nEntidades = 0; int nBlocos = 0;
int nBalas = 0;
float limEntidades;
int nPassivos=0;
bool mostrarCaixa=false;
int bCaixa=0;
float dificuldade=1;

bool sexo;
bool atirando=false;

int largFase; int altFase;
float pxFundo = 0; float pyFundo = 0;
float escala = 1.6f; float escalaVelocidade = 0.0f;

ALLEGRO_DISPLAY *janela = NULL;
ALLEGRO_EVENT_QUEUE *filaEventos = NULL;
ALLEGRO_TIMER *timer = NULL;
ALLEGRO_TIMER *timerAlt = NULL;
ALLEGRO_TIMER *timerTeste=NULL;
ALLEGRO_FONT *retroFont = NULL;
ALLEGRO_FONT *retroFont32 = NULL;
ALLEGRO_BITMAP *fundo;
ALLEGRO_BITMAP *caixaDialogo;
ALLEGRO_TRANSFORM camera;

Entidade player;
Balas balasPlayer;
Entidade *entidades = NULL;
Balas *balasEntidades;
Bloco *blocos = NULL;
bool criaEnt;
bool mostraHitbox=false;
bool reinicio=false;
bool interac=false;

int max(int a,int b){
    if(a>b) return a;
    else return b;
}
int min(int a,int b){
    if(a>b) return b;
    else return a;
}

//destroi o que der
void destroi(){
    al_destroy_timer(timer);
    al_destroy_timer(timerAlt);
    al_destroy_timer(timerTeste);
    al_destroy_display(janela);
    al_destroy_event_queue(filaEventos);
    al_destroy_font(retroFont);
    al_destroy_font(retroFont32);
    for (int i = 0; i < nEntidades; i++){
        al_destroy_bitmap(entidades[i].sprite);
    }
    al_destroy_bitmap(fundo);
    for (int i = 0; i < nBlocos; i++){
        if(blocos[i].sprite) al_destroy_bitmap(blocos[i].sprite);

    }
    for (int i = 0; i < nBalas; i++){
        if(balasEntidades[i].sprite) al_destroy_bitmap(balasEntidades[i].sprite);
    }
    al_destroy_bitmap(balasPlayer.sprite);
    al_destroy_bitmap(player.sprite);
    al_destroy_bitmap(caixaDialogo);
    
}

//mensagem de erro, deve ser usado na verificação de inicializações
void msgErro(char *t){
    al_show_native_message_box(NULL,"ERRO","Ocorreu o seguinte erro:",t,NULL,ALLEGRO_MESSAGEBOX_ERROR);
    estado = estSaida;
}

//inicializa os addons
int inic(){
    if(!al_init()){
        msgErro("Falha ao inicializar a Allegro");
        return 0;
    }
    if(!al_init_font_addon()){
        msgErro("Falha ao inicializar o addon de fontes");
        return 0;
    }
    if(!al_init_ttf_addon()){
        msgErro("Falha ao inicializar o addon ttf");
        return 0;
    }
    if (!al_init_image_addon()){
        msgErro("Falha ao inicializar o addon de imagens");
        return 0;
    }
    if (!al_install_keyboard()){
        msgErro("Falha ao inicializar o teclado");
        return 0;
    }
    //addon de audio aparentemente bugado
    //if(!al_install_audio()){ msgErro("Falha ao inicializar o audio"); return 0;}
    //if(!al_init_acodec_addon()){msgErro("Falha ao inicializar o codec de audio");return 0;}
     if(!al_install_mouse()){
        msgErro("Falha ao iniciar o mouse");
        return 0;
    }
    if (!al_init_primitives_addon()){
        msgErro("Falha ao inicializar as primitivas");
        return 0;
    }
    return 1;
}

void geraMundo(){ 
    
    fundo=al_load_bitmap("./bin/backgrounds/mapa0.png");
     //apenas um teste
    if(!initBloco()) msgErro("Erro ao gerar os blocos!");
    if(!fundo) msgErro("Deu ruim nos fundos!");
    //pega a largura de cada tile, pra função de desenho
    largFase=2*al_get_bitmap_width(fundo);
    //pega a altura de cada tile , pra função de desenho
    altFase=2*al_get_bitmap_height(fundo);
    //defino as razoes, utilidade explicada na declaração

}

//cria tudo que será necessário no jogo
int cria(){
    timer = al_create_timer(1.0 / FPS);
    timerAlt = al_create_timer(8.0 / FPS);
    timerTeste = al_create_timer(60.0 / FPS);
    if((!timer) || (!timerAlt)){
        msgErro("Falha ao criar temporizador");
        return 0;
    }
    al_set_new_bitmap_flags(ALLEGRO_VIDEO_BITMAP);
    al_set_new_display_flags(ALLEGRO_FULLSCREEN_WINDOW); //flag do fullscreen
    janela = al_create_display(1280,720);
    if(!janela) {
        msgErro("Falha ao criar janela");
        return 0;
    }
    
    ALTURA = al_get_display_height(janela);
    LARGURA = al_get_display_width(janela);

    al_set_window_title(janela, "Jogo");
    //cria fila de eventos principal
    filaEventos = al_create_event_queue();
    if(!filaEventos) {
        msgErro("Falha ao criar fila de eventos");
        return 0;
    }
    retroFont = al_load_font("./fonts/retroGaming.ttf", 20, 0);
    retroFont32 = al_load_font("./fonts/retroGaming.ttf", 32, 0);
    if((!retroFont) || (!retroFont32)){
        msgErro("Falha ao carregar fonte");
        return 0;
    }
    al_register_event_source(filaEventos, al_get_display_event_source(janela));
    al_register_event_source(filaEventos, al_get_timer_event_source(timer));
    al_register_event_source(filaEventos, al_get_timer_event_source(timerAlt));
    al_register_event_source(filaEventos,al_get_timer_event_source(timerTeste));
    al_register_event_source(filaEventos, al_get_keyboard_event_source());
    
    return 1;
}

void aumentaEntidades(){
    nEntidades++;
    nBalas++;
    if(nEntidades>=limEntidades){
        nEntidades--;
        nBalas--;
    }
    else{
        entidades = (Entidade*)realloc(entidades,nEntidades*sizeof(Entidade));
        balasEntidades = (Balas*)realloc(balasEntidades,nBalas*sizeof(Balas));
        if(entidades==NULL || balasEntidades==NULL){
            msgErro("Deu ruim na alocação!");
            estado=estSaida;
        }
    }
}
void aumentaBlocos(){
    //aloca o espaço
    nBlocos=73;
    blocos = (Bloco*)realloc(blocos,nBlocos*sizeof(Bloco));
    if(blocos==NULL && nBlocos!=0){
        msgErro("Deu ruim na alocação!");
        estado=estSaida;
    }
}

//seta tudo antes do fluxo do menu
void preMenu(){
    int i = 0, j = 252;
    bool sair = false;
    al_start_timer(timerAlt);
    while(!sair){
        ALLEGRO_EVENT evento;
        al_wait_for_event(filaEventos,&evento);
        switch (evento.type){
            case ALLEGRO_EVENT_TIMER:
                al_draw_multiline_text(retroFont32, al_map_rgb(j, j, j), LARGURA / 2, ALTURA - i, LARGURA * 0.9, al_get_font_line_height(retroFont32) * 1.1, ALLEGRO_ALIGN_CENTER, "O ano é 20XX, e o mundo está no mais completo caos. Há meses, surgiram boatos de que a infame [...]--uma organização criminosa vinda do outro lado do mundo--havia liberado a Ameaça Vermelha, um vírus altamente contagioso que causava fome e frio em tamanha magnitude que chegava a ser letal. Entretanto, como ela havia sido extinta há quase 30 anos, a população desconsiderou o risco, pensando que aquela era apenas mais uma fake news.\n\nA solução para tal crise se encontra em Brasilia, onde o primeiro-ministro Adolf Solnorabo Mussolini é secretamente recrutado para proteger seus queridos cidadãos da terrível Ameaça. Ao questionar sobre o motivo de ser escolhido para realizar árdua tarefa, nosso bravo herói descobre que seu histórico de atleta o imunizou contra a nefária doença, tornando-o a pessoa mais qualificada para combatê-la.\n\n\n\nCabe a você salvar o mundo dessa terrível ameaça.");
                al_flip_display();
                al_clear_to_color(al_map_rgb(0, 0, 0));
                if(i >= 1.5 * ALTURA) {
                    if(j > 0){
                        j -= 6;
                    }
                }
                else i += 3;
                if(j <= 0) {
                    sair = true;
                    estado = estMenu;
                }
                break;
            case ALLEGRO_EVENT_KEY_DOWN: //aperta uma tecla
                switch(evento.keyboard.keycode){
                    case ALLEGRO_KEY_ENTER: 
                        sair = true;
                        estado = estMenu;
                        break;
                    case ALLEGRO_KEY_ESCAPE:
                        al_stop_timer(timerAlt);
                        if(al_show_native_message_box(janela,"","Tem certeza que quer sair do jogo?", "","Nope :D|Sim T^T",ALLEGRO_MESSAGEBOX_YES_NO)%2==0){
                            sair = true;
                            estado = estSaida;
                        }
                        al_start_timer(timerAlt);
                        break;
                }
                break;
        }
    }
    al_stop_timer(timerAlt);
    al_flush_event_queue(filaEventos);
}

//fluxo do menu
void menu(){
    opcao = 0; //indica qual botão que tá com highlight
    int i = 255, j = 120, temp;
    bool sair = false;
    al_start_timer(timer);
    while(!sair){
        ALLEGRO_EVENT evento;
        al_wait_for_event(filaEventos,&evento);
        switch (evento.type){
            case ALLEGRO_EVENT_TIMER:
                al_draw_text(retroFont32, al_map_rgb(i, i, i), LARGURA / 2, ALTURA / 2, ALLEGRO_ALIGN_CENTER, "Jogar");
                al_draw_text(retroFont32, al_map_rgb(j, j, j), LARGURA / 2, ALTURA / 2 + al_get_font_line_height(retroFont32), ALLEGRO_ALIGN_CENTER, "Sair");
                al_flip_display();
                al_clear_to_color(al_map_rgb(0, 0, 0));
                break;
            case ALLEGRO_EVENT_KEY_DOWN: //aperta uma tecla
                switch(evento.keyboard.keycode){
                    case ALLEGRO_KEY_DOWN: 
                        opcao++;
                        temp = i;
                        i = j;
                        j = temp;
                        break;
                    case ALLEGRO_KEY_UP: 
                        opcao++;
                        temp = i;
                        i = j;
                        j = temp;
                        break;
                    case ALLEGRO_KEY_ENTER:
                        sair = true;
                }
                break;
        }
    }
    al_stop_timer(timer);
    al_flush_event_queue(filaEventos);
    if(opcao % 2 == 0){
        estado = estCutscene;
    }
    else{
        estado = estSaida;
    }
}

char dialogo1A[150] = "Vejo que você tem um espírito nobre, eu escolhi você para salvar a humanidade. Se escolher aceitar a missão, compareça a [LUGAR] em [HORÁRIO].";
char dialogo1B[37] = "E então, o que você tem para mim?";
char dialogo1C[203] = "Eu investiguei toda sua vida, sei das suas convicções e do seu histórico de atleta, e é por isso que eu sei, que você é o único adequado para essa missão. Solnorabo, você é imune ao vírus...";
char dialogo1D[9] = "Vírus?";
char dialogo1E[172] = "Ao vírus da... Ameaça Comunista. Esse vírus está se espalhando silenciosamente pela sociedade, e em poucos anos, toda a humanidade como conhecemos estará destruída.";
char dialogo1F[118] = "Malditos comunistas, sempre soube que algo assim aconteceria, TÁ OK, eu aceito a missão, o que eu tenho que fazer?";
char dialogo1G[88] = "Vá e leve esse comunicador, eu manterei contato e avisarei sobre os próximos passos.";

void cutscene(){
    caixaDialogo = al_load_bitmap("./bin/misc/UI/caixaDialogo.png");
    int i = 0;
    int xCaixaSup = LARGURA / 15, yCaixaSup = ALTURA / 15;
    int xCaixaInf = LARGURA - (al_get_bitmap_width(caixaDialogo) + LARGURA / 15), yCaixaInf = ALTURA * 14/15 - al_get_bitmap_height(caixaDialogo);
    int dialogo = 0;
    bool sair = false;
    al_start_timer(timer);
    while(!sair){
        ALLEGRO_EVENT evento;
        al_wait_for_event(filaEventos,&evento);
        switch (evento.type){
            case ALLEGRO_EVENT_TIMER:
                al_draw_filled_rectangle(0,0,LARGURA,ALTURA,al_map_rgb(0,0,0));
                al_draw_bitmap(caixaDialogo, xCaixaSup, yCaixaSup, 0);
                al_draw_bitmap(caixaDialogo, xCaixaInf, yCaixaInf, 0);

                if(dialogo == 0){
                    al_draw_multiline_textf(retroFont, al_map_rgb(255, 255, 255), xCaixaSup + 20, yCaixaSup + 20, al_get_bitmap_width(caixaDialogo)-40, 30, ALLEGRO_ALIGN_LEFT, "%s", dialogo1A);
                }
                else if(dialogo == 1){
                    al_draw_multiline_textf(retroFont, al_map_rgb(255, 255, 255), xCaixaInf + 20, yCaixaInf + 20, al_get_bitmap_width(caixaDialogo)-40, 30, ALLEGRO_ALIGN_LEFT, "%s", dialogo1B);
                }
                else if(dialogo == 2){
                    al_draw_multiline_textf(retroFont, al_map_rgb(255, 255, 255), xCaixaSup + 20, yCaixaSup + 20, al_get_bitmap_width(caixaDialogo)-40, 30, ALLEGRO_ALIGN_LEFT, "%s", dialogo1C);
                }
                else if(dialogo == 3){
                    al_draw_multiline_textf(retroFont, al_map_rgb(255, 255, 255), xCaixaInf + 20, yCaixaInf + 20, al_get_bitmap_width(caixaDialogo)-40, 30, ALLEGRO_ALIGN_LEFT, "%s", dialogo1D);
                }
                else if(dialogo == 4){
                    al_draw_multiline_textf(retroFont, al_map_rgb(255, 255, 255), xCaixaSup + 20, yCaixaSup + 20, al_get_bitmap_width(caixaDialogo)-40, 30, ALLEGRO_ALIGN_LEFT, "%s", dialogo1E);
                }
                else if(dialogo == 5){
                    al_draw_multiline_textf(retroFont, al_map_rgb(255, 255, 255), xCaixaInf + 20, yCaixaInf + 20, al_get_bitmap_width(caixaDialogo)-40, 30, ALLEGRO_ALIGN_LEFT, "%s", dialogo1F);
                }
                else if(dialogo == 6){
                    al_draw_multiline_textf(retroFont, al_map_rgb(255, 255, 255), xCaixaSup + 20, yCaixaSup + 20, al_get_bitmap_width(caixaDialogo)-40, 30, ALLEGRO_ALIGN_LEFT, "%s", dialogo1G);
                }
                break;
            case ALLEGRO_EVENT_KEY_DOWN: //aperta uma tecla
                switch(evento.keyboard.keycode){
                    case ALLEGRO_KEY_ENTER:
                        dialogo++;
                        if(dialogo >= 7) sair = true;
                }
                break;
        }
        al_flip_display();
    }
    al_stop_timer(timer);
    al_flush_event_queue(filaEventos);
    estado = estPreJogo;
}

//seta tudo antes do fluxo do jogo, talvez possa ser repetido a cada mudança de fase(?)
void preJogo(){
    switch(al_show_native_message_box(janela,"Escolha de dificuldade","Temos pra todos os gostos:","EASY: descrição\nNormal: descrição\nHARD: descrição","NORMAL|EASY|HARD",ALLEGRO_MESSAGEBOX_YES_NO)){
        case 0:
            dificuldade=1;
            break;
        case 1:
            dificuldade=1;
            break;
        case 2:
            dificuldade=0.5;
            break;
        case 3:
            dificuldade=1.5;
            break;
    }
    geraMundo();
    //pergunta como a pessoa quer jogar
    if(al_show_native_message_box(janela,"É azul ou rosa?","Escolha seu covideiro","Um ou outro, ta ok?","Solnorabo|Salnaraba",ALLEGRO_MESSAGEBOX_OK_CANCEL)%2!=0){
        sexo=1;
    }
    else sexo=0;
    limEntidades=150*dificuldade;
    //setando os atributos iniciais
    player.hp=HP/dificuldade;
    player.atk=ATAQUE/dificuldade;
    player.def=DEFESA/dificuldade;
    if(sexo) player.sprite=al_load_bitmap("./bin/entities/Char/salnorabo.png");
    else player.sprite=al_load_bitmap("./bin/entities/Char/salnaraba.png");
    //tudo isso aqui debaixo é bem hardcoded, depende do sprite msm infelizmente (na vdd eu tenho preguiça de fazer usando matematica entao eh isso)
    player.larguraSprite = 31; player.alturaSprite =  49;
    player.puloColuna=64; player.puloLinha=64;
    player.totalFrames=9; player.frameAtual=0;
    player.nDirecao[dBaixo]=2; player.nDirecao[dCima]=0;
    player.nDirecao[dEsquerda]=3; player.nDirecao[dDireita]=1;
    for (int i = 0; i < 4; i++){
        player.direcao[i]=false;
    }
    //zera as posiçoes e velocidades
    player.px=LARGURA/4; player.py=ALTURA/2;
    player.vx=0; player.vy=0;
    //o player sempre esta na tela
    player.naTela=true;
    //0 utilidade, so pra ficar inicializado
    player.inimigo=false;
    player.escalaEntidade=1;
    //raio da hitbox de contato/hitbox de dano (com entidades)
    player.raio=player.alturaSprite/4+player.larguraSprite/4;
    player.dano=false;
    if(reinicio){ //se reinicio for true, reseta as entidades e blocos
        nEntidades=0;
        nBalas=0;
        //zera as entidades pro proximo jogo
        entidades=NULL;
        balasEntidades=NULL;
    }
    obj1=false; obj3=false;
    obj2=false; obj4=false;
    numObj1=0; limObj1=6*dificuldade;
    numObj2=0; limObj2=24*dificuldade;
    srand(al_get_timer_count(timer));
    if(!player.sprite) msgErro("Erro no sprite do player");
    al_convert_mask_to_alpha(player.sprite,al_map_rgb(0,0,0)); //isso aqui define a transparencia do sprite
    balasPlayer.px=0; balasPlayer.py=0;
    balasPlayer.vx=0; balasPlayer.vy=0;
    balasPlayer.sprite=al_load_bitmap("./bin/entities/Bullets/balaPlayer.png");
    if(!balasPlayer.sprite){
        msgErro("Erro ao carregar balas!");
    }
    balasPlayer.escalaEntidade=0.8/dificuldade;
    balasPlayer.alturaSprite=al_get_bitmap_height(balasPlayer.sprite)*balasPlayer.escalaEntidade;
    balasPlayer.larguraSprite=al_get_bitmap_width(balasPlayer.sprite)*balasPlayer.escalaEntidade;
    balasPlayer.raio=balasPlayer.alturaSprite;
    balasPlayer.atingiu=true;

    for (int i = 0; i < 4; i++){
        balasPlayer.direcao[i]=false;
    }
    
    al_flush_event_queue(filaEventos); //da um wipe na fila inteira
    estado = estJogo;
}
void fimDeJogo(){
    //bem besta
    if(al_show_native_message_box(janela,"Você morreu!","Que pena", "Os inimigos venceram, tentar novamente?","Claro!|Tô Fora",ALLEGRO_MESSAGEBOX_YES_NO)%2==0){
        estado = estSaida;
    }
    //caso ele reinicie
    else{
        al_flush_event_queue(filaEventos);
        reinicio=1;
        estado= estPreJogo;
    }
}

void pauseJogo(){
    al_flush_event_queue(filaEventos);
    ALLEGRO_BITMAP *janelaPause;
    retroFont = al_load_font("./fonts/retroGaming.ttf", 50/escala, 0);
    janelaPause=al_load_bitmap("./bin/misc/UI/TextBox.bmp"); //carrega um bitmap de uma caixa de texto
    if(!janelaPause || !retroFont){
        msgErro("Deu ruim no pause!");
    }
    int alturaPause=al_get_bitmap_height(janelaPause);
    int larguraPause=al_get_bitmap_width(janelaPause);
    al_draw_scaled_bitmap(janelaPause,0,0, //essa função desenha um bitmap na escala desejada
                        larguraPause,alturaPause,(player.px-5*larguraPause/(2*escala)),(player.py-5*alturaPause/(2*escala)), //largura do bitmap, altura do bitmap,posicao x na tela, posicao y na tela
                        5*larguraPause/escala,5*alturaPause/escala,0); //tamanho desejado, altura desejada, flag
    al_draw_text(retroFont,al_map_rgb(0,0,0),player.px,player.py,ALLEGRO_ALIGN_CENTRE,"PAUSADO");
    al_flip_display(); //atualiza a tela
    bool despausa=false;
    while(!despausa){
        ALLEGRO_EVENT pausa;
        al_wait_for_event(filaEventos,&pausa);
        if(pausa.type==ALLEGRO_EVENT_KEY_DOWN) despausa=true; //se apertar enter sai do loop
        if(pausa.type==ALLEGRO_EVENT_KEY_UP){
            player.vx=0;
            player.vy=0;
            escalaVelocidade=0;
        }
    }
    al_flush_event_queue(filaEventos);
    retroFont = al_load_font("./fonts/retroGaming.ttf", 20, 0);
    al_destroy_bitmap(janelaPause);

    al_flip_display();
}

void atualizaCamera(){ //atualiza as posiçoes das coisas
    pxFundo=-(LARGURA/2)+(player.px+player.larguraSprite/2); //magia negra (geometria)
    pyFundo=-(ALTURA/2)+(player.py+player.alturaSprite/2);
    if(pxFundo<0) pxFundo=0; //n deixa passar do 0
    if(pyFundo<0) pyFundo=0;
}

void desenhaMundo(){ //essa função tem que desenhar o mapa inteiro usando os tiles ja carregados
    al_draw_scaled_bitmap(fundo,0,0,largFase/2,altFase/2,0,0,largFase,altFase,0);
}
void colisaoEntidades(int i){
    for(int j=0; j<nBlocos;j++){
            bool colisao=false;
            if(j<0) j = 0; //evitar bugs no j--
            bool temHitx=true; bool temHity=true;
            if(blocos[j].naTela){  //n tem mt o que explicar aqui, so desenhando msm (serio)
                if(blocos[j].larguraHitbox==0) temHitx=false;
                if(blocos[j].alturaHitbox==0) temHity=false;
                if (entidades[i].px+entidades[i].larguraSprite > blocos[j].px && entidades[i].px < blocos[j].px+blocos[j].larguraHitbox && entidades[i].py+entidades[i].alturaSprite > blocos[j].py && entidades[i].py < blocos[j].py+blocos[j].alturaHitbox) colisao=true;
                else colisao=false;
            }
            if((entidades[i].direcao[dCima] || entidades[i].direcao[dBaixo]) && colisao && temHity){
                    entidades[i].py-=entidades[i].vy;
            } 
            if((entidades[i].direcao[dDireita] || entidades[i].direcao[dEsquerda]) && colisao && temHitx){
                    entidades[i].px-=entidades[i].vx;
            }
    }
}
void caixaTexto(int i){
    i=i-59;
    al_flush_event_queue(filaEventos);
    ALLEGRO_BITMAP *janelaTexto;
    retroFont = al_load_font("./fonts/retroGaming.ttf", 30/escala, 0);
    janelaTexto=al_load_bitmap("./bin/misc/UI/TextBox.bmp"); //carrega um bitmap de uma caixa de texto
    if(!janelaTexto || !retroFont){
        msgErro("Deu ruim no texto!");
    }
    int alturaPause=al_get_bitmap_height(janelaTexto);
    int larguraPause=al_get_bitmap_width(janelaTexto);
    float pxPause=(player.px-5*larguraPause/(2*escala));
    float pyPause=(player.py-5*alturaPause/(2*escala));
    al_draw_scaled_bitmap(janelaTexto,0,0, //essa função desenha um bitmap na escala desejada
                        larguraPause,alturaPause,pxPause+200/escala,pyPause+200/escala, //largura do bitmap, altura do bitmap,posicao x na tela, posicao y na tela
                        5*larguraPause/escala,5*alturaPause/escala,0); //tamanho desejado, altura desejada, flag
    switch(i){
        case 0: //placa deserto
            al_draw_multiline_text(retroFont,al_map_rgb(0,0,0),pxPause+200/escala,pyPause+200/escala,2*larguraPause*escala,30/escala,ALLEGRO_ALIGN_LEFT,"PAUSADO");
            break;
        case 1: //placa campo
            al_draw_multiline_text(retroFont,al_map_rgb(0,0,0),pxPause+200/escala,pyPause+200/escala,2*larguraPause*escala,30/escala,ALLEGRO_ALIGN_LEFT,"PAUSADO");
            break;
        case 2: //placa mansao
            al_draw_multiline_text(retroFont,al_map_rgb(0,0,0),pxPause+200/escala,pyPause+200/escala,2*larguraPause*escala,30/escala,ALLEGRO_ALIGN_LEFT,"PAUSADO");
            break;
        case 3: //placa praça
            al_draw_multiline_text(retroFont,al_map_rgb(0,0,0),pxPause+200/escala,pyPause+200/escala,2*larguraPause*escala,30/escala,ALLEGRO_ALIGN_LEFT,"PAUSADO");
            break;
        case 4: //placa praia 1
            al_draw_multiline_text(retroFont,al_map_rgb(0,0,0),pxPause+200/escala,pyPause+200/escala,2*larguraPause*escala,30/escala,ALLEGRO_ALIGN_LEFT,"PAUSADO");
            break;
        case 5: //placa praia 2
            al_draw_multiline_text(retroFont,al_map_rgb(0,0,0),pxPause+200/escala,pyPause+200/escala,2*larguraPause*escala,30/escala,ALLEGRO_ALIGN_LEFT,"PAUSADO");
            break;
        case 6: //placa praia 3
            al_draw_multiline_text(retroFont,al_map_rgb(0,0,0),pxPause+200/escala,pyPause+200/escala,2*larguraPause*escala,30/escala,ALLEGRO_ALIGN_LEFT,"PAUSADO");
            break;
        case 7: //placa porco
            al_draw_multiline_text(retroFont,al_map_rgb(0,0,0),pxPause+200/escala,pyPause+200/escala,2*larguraPause*escala,30/escala,ALLEGRO_ALIGN_LEFT,"PAUSADO");
            break;
        case 8: //placa fabrica
            al_draw_multiline_text(retroFont,al_map_rgb(0,0,0),pxPause+200/escala,pyPause+200/escala,2*larguraPause*escala,30/escala,ALLEGRO_ALIGN_LEFT,"PAUSADO");
            break;
        case 9: //placa escola
            al_draw_multiline_text(retroFont,al_map_rgb(0,0,0),pxPause+200/escala,pyPause+200/escala,2*larguraPause*escala,30/escala,ALLEGRO_ALIGN_LEFT,"");
            break;
        case 10: //placa 10, canto inferior direito na praia
            al_draw_multiline_text(retroFont,al_map_rgb(0,0,0),pxPause+200/escala,pyPause+200/escala,2*larguraPause*escala,30/escala,ALLEGRO_ALIGN_LEFT,"O jogo é inclusivo com todos,então reservamos uma área especialmente aos bugs gráficos:");
            break;
        case 11: //barril
            al_draw_multiline_text(retroFont,al_map_rgb(0,0,0),pxPause+200/escala,pyPause+200/escala,2*larguraPause*escala,30/escala,ALLEGRO_ALIGN_LEFT,"Barril do Zeca Urubu,o maior programador do país");
            break;
        case 13: //lim inf
            al_draw_multiline_text(retroFont,al_map_rgb(0,0,0),pxPause+200/escala,pyPause+200/escala,2*larguraPause*escala,30/escala,ALLEGRO_ALIGN_LEFT,"Você devia ter prestado atenção nos comerciais da marinha.");
            break;
        case 69: //obj 1
            al_draw_multiline_text(retroFont,al_map_rgb(0,0,0),pxPause+200/escala,pyPause+200/escala,2*larguraPause*escala,30/escala,ALLEGRO_ALIGN_LEFT,"Objetivo 1 completo");            
            break;
        case 70:
            al_draw_multiline_text(retroFont,al_map_rgb(0,0,0),pxPause+200/escala,pyPause+200/escala,2*larguraPause*escala,30/escala,ALLEGRO_ALIGN_LEFT,"Objetivo 2 completo");            
            break; //obj 2
        case 71: //obj 3
            al_draw_multiline_text(retroFont,al_map_rgb(0,0,0),pxPause+200/escala,pyPause+200/escala,2*larguraPause*escala,30/escala,ALLEGRO_ALIGN_LEFT,"Objetivo 3 completo");            
            break;
        case 72: //obj 4
            al_draw_multiline_text(retroFont,al_map_rgb(0,0,0),pxPause+200/escala,pyPause+200/escala,2*larguraPause*escala,30/escala,ALLEGRO_ALIGN_LEFT,"Objetivo 4 completo");            
            break;
    }
    al_flip_display(); //atualiza a tela
    bool despausa=false;
    while(!despausa){
        ALLEGRO_EVENT pausa;
        al_wait_for_event(filaEventos,&pausa);
        if(pausa.type==ALLEGRO_EVENT_KEY_DOWN){
            despausa=true;
            interac=false;
        } //se apertar enter sai do loop
        if(pausa.type==ALLEGRO_EVENT_KEY_UP){
            player.vx=0;
            player.vy=0;
            escalaVelocidade=0;
        }
    }
    interac=false;
    mostrarCaixa=false;
    al_flush_event_queue(filaEventos);
    retroFont = al_load_font("./fonts/retroGaming.ttf", 20, 0);
    al_destroy_bitmap(janelaTexto);

    al_flip_display();
    if(i==13){
        fimDeJogo();
    }
}

void colisaoJogador(){

    //verifica os blocos na tela
    for (int i = 0; i < nBlocos; i++){
        if(blocos[i].px+blocos[i].larguraSprite/2+blocos[i].larguraHitbox/2 >= pxFundo && blocos[i].py+blocos[i].alturaSprite/2+blocos[i].alturaHitbox/2 >= pyFundo && blocos[i].px-blocos[i].larguraSprite/2+blocos[i].larguraHitbox/2<=LARGURA+pxFundo && blocos[i].py-blocos[i].alturaSprite/2+blocos[i].alturaHitbox/2<=ALTURA+pyFundo){
            blocos[i].naTela=true;
        }
        else blocos[i].naTela=false;
    }

    for (int i = 0; i < nEntidades; i++){
        if(entidades[i].px+entidades[i].larguraSprite >= pxFundo && entidades[i].py+entidades[i].alturaSprite >= pyFundo && entidades[i].px<=LARGURA+pxFundo && entidades[i].py<=ALTURA+pyFundo && entidades[i].hp>0){
            entidades[i].naTela=true;
        }
        else entidades[i].naTela=false;
        
    }
    //algoritmo de colisao medindo em um raio do player se tem alguma entidade, vasculhando as posiçoes das entidades
    for (int i = 0; i < nEntidades; i++){ 
        if(entidades[i].naTela && entidades[i].hp>0){//se a entidade ta na tela
            //teorema de pitagoras (serio)
            bool colisaoEntidade=false;
            double distancia=sqrt(pow(player.px+player.larguraSprite/2-entidades[i].px-entidades[i].larguraSprite/2,2)+pow(player.py+player.alturaSprite/2-entidades[i].py-entidades[i].alturaSprite/2,2));
            if(distancia<=player.raio+entidades[i].raio){ //isso significa que as circunferencias são secantes
                if(entidades[i].inimigo){ //se for inimigo, apanha
                    player.hp-=1;
                    //animacao de dano/ataque aqui, por enquanto nao rola nd
                    if(player.px>entidades[i].px) player.px+=VELOCIDADE/3;
                    if(player.px<entidades[i].px) player.px-=VELOCIDADE/3;
                    if(player.py>entidades[i].py) player.py+=VELOCIDADE/3;
                    if(player.py<entidades[i].py) player.py-=VELOCIDADE/3;
                    player.dano=true;
                }
               colisaoEntidade=true;
            }
            else{
                colisaoEntidade=false;
            }
            if(distancia<= RAIO_P*entidades[i].escalaEntidade*(3*dificuldade/2) && !colisaoEntidade){ //se ele tiver dentro do raio de procura
                float seno=(player.py-entidades[i].py)/distancia; //olha ele ai
                float cosseno=(player.px-entidades[i].px)/distancia;
                if(entidades[i].inimigo){  
                    entidades[i].vx=+VELOCIDADE*dificuldade*cosseno/1.7; //geometria 
                    entidades[i].vy=+VELOCIDADE*dificuldade*seno/1.7;
                }
                else{
                    entidades[i].vx=-(VELOCIDADE*cosseno/1.7)*dificuldade; //geometria 
                    entidades[i].vy=-(VELOCIDADE*seno/1.7)*dificuldade;
                }
                if(balasEntidades[i].atingiu==true && entidades[i].inimigo == true){
                    balasEntidades[i].px=entidades[i].px;
                    balasEntidades[i].py=entidades[i].py;
                    balasEntidades[i].vx=3*entidades[i].vx;
                    balasEntidades[i].vy=3*entidades[i].vy;
                    balasEntidades[i].atingiu=false;
                }
            }
            else{
                entidades[i].vx=0; //se n tiver no raio, fica quietinho
                entidades[i].vy=0;
            }
            if(entidades[i].vx>0) entidades[i].direcao[dDireita]=true;
            else if(entidades[i].vx<0) entidades[i].direcao[dEsquerda]=true;
            else{
                entidades[i].direcao[dDireita]=false;
                entidades[i].direcao[dEsquerda]=false;
            }
            if(entidades[i].vy>0) entidades[i].direcao[dBaixo]=true;
            else if(entidades[i].vy<0) entidades[i].direcao[dCima]=true;
            else{
                entidades[i].direcao[dDireita]=false;
                entidades[i].direcao[dEsquerda]=false;
            }
            entidades[i].px+=entidades[i].vx; //atualiza a posição
            entidades[i].py+=entidades[i].vy;
            balasEntidades[i].px+=balasEntidades[i].vx;
            balasEntidades[i].py+=balasEntidades[i].vy;
        }
        colisaoEntidades(i);
    }
    //colisao do jogador com os blocos
    
    for(int j=0; j<nBlocos;j++){
        bool colisao=false;
        if(j<0) j = 0; //evitar bugs no j--
        bool temHitx=true; bool temHity=true;
        if(blocos[j].naTela){  //n tem mt o que explicar aqui, so desenhando msm (serio)
            if(blocos[j].larguraHitbox==0) temHitx=false;
            if(blocos[j].alturaHitbox==0) temHity=false;
            if (player.px+player.larguraSprite > blocos[j].px && player.px < blocos[j].px+blocos[j].larguraHitbox && player.py+player.alturaSprite > blocos[j].py && player.py < blocos[j].py+blocos[j].alturaHitbox) colisao=true;
            else colisao=false;
        }
        if((j==72 && colisao)){
            mostrarCaixa=true;
            bCaixa=72;
        }
        else if(blocos[j].naTela && blocos[j].interac && interac){
            float distancia = sqrt(pow(player.px+player.larguraSprite/2-blocos[j].px-blocos[j].larguraHitbox/2,2)+pow(player.py+player.alturaSprite/2-blocos[j].py-blocos[j].alturaHitbox/2,2));
            if(distancia<=player.raio+RAIO_P/5){
                mostrarCaixa=true;
                bCaixa=j;
            }
        }
        if((player.direcao[dCima] || player.direcao[dBaixo]) && colisao && temHity){
                player.py-=player.vy/abs(player.vy); //uma lenta porem eficiente solução ao problema das colisões
                j--;
        }
        if((player.direcao[dDireita] || player.direcao[dEsquerda]) && colisao && temHitx){
                player.px-=player.vx/abs(player.vx);
                j--;
        }
    }

}
int initBalas(){
    balasEntidades[nBalas-1].px=0; balasEntidades[nBalas-1].py=0;
    balasEntidades[nBalas-1].vx=0; balasEntidades[nBalas-1].vy=0;
    balasEntidades[nBalas-1].sprite=al_load_bitmap("./bin/entities/Bullets/balaInimigo.png");
    if(!balasEntidades[nBalas-1].sprite){
            msgErro("Erro ao carregar balas!");
            return 0;
    }   
    balasEntidades[nBalas-1].escalaEntidade=0.7*dificuldade;
    balasEntidades[nBalas-1].alturaSprite=al_get_bitmap_height(balasEntidades[nBalas-1].sprite)*balasEntidades[nBalas-1].escalaEntidade;
    balasEntidades[nBalas-1].larguraSprite=al_get_bitmap_width(balasEntidades[nBalas-1].sprite)*balasEntidades[nBalas-1].escalaEntidade;
    balasEntidades[nBalas-1].raio=balasEntidades[nBalas-1].larguraSprite;
    balasEntidades[nBalas-1].atingiu=true;

    for (int i = 0; i < 4; i++){
        balasEntidades[nBalas-1].direcao[i]=false;
    }
    return 1;
}

int initEntidade(){ //aqui é tudo hardcoded msm, n tem jeito
    if(rand()%2==0){
        entidades[nEntidades-1].sprite=al_load_bitmap("./bin/entities/Char/NPC1.png");
        if(!entidades[nEntidades-1].sprite){
            msgErro("Deu ruim nos blocos!");
            return 0;
        } //inicializando tudo (obs, nada definitivo)
        entidades[nEntidades-1].larguraSprite = 1.7*al_get_bitmap_width(entidades[nEntidades-1].sprite); 
        entidades[nEntidades-1].alturaSprite =  1.7*al_get_bitmap_height(entidades[nEntidades-1].sprite);
        entidades[nEntidades-1].px=player.px+((rand()%2?-1:1)*rand()%LARGURA); entidades[nEntidades-1].py=player.py+((rand()%2?-1:1)*rand()%ALTURA);
        entidades[nEntidades-1].pDesenhox=0; entidades[nEntidades-1].pDesenhoy=0;
        entidades[nEntidades-1].vx=0; entidades[nEntidades-1].vy=0;
        entidades[nEntidades-1].inimigo=true;
        entidades[nEntidades-1].dano=false;
        entidades[nEntidades-1].escalaEntidade=1.7;
        entidades[nEntidades-1].raio=entidades[nEntidades-1].larguraSprite/4+entidades[nEntidades-1].alturaSprite/4;
        entidades[nEntidades-1].hp = 10*dificuldade;
        for (int j = 0; j < 4; j++){
            entidades[nEntidades-1].direcao[j]=false;
        }
        //inicializa balas
        entidades[nEntidades-1].atk=ATAQUE*dificuldade;
        entidades[nEntidades-1].def=DEFESA*dificuldade;
        if(!initBalas()) msgErro("Erro ao iniciar balas!");
    }
    else if(nPassivos<=limEntidades/5){
        entidades[nEntidades-1].sprite=al_load_bitmap("./bin/entities/Char/NPC.png");
        if(!entidades[nEntidades-1].sprite){
            msgErro("Deu ruim nos blocos!");
            return 0;
        } //inicializando tudo (obs, nada definitivo)
        entidades[nEntidades-1].larguraSprite = 1.7*al_get_bitmap_width(entidades[nEntidades-1].sprite); 
        entidades[nEntidades-1].alturaSprite =  1.7*al_get_bitmap_height(entidades[nEntidades-1].sprite);
        entidades[nEntidades-1].px=player.px+((rand()%2?-1:1)*rand()%LARGURA); entidades[nEntidades-1].py=player.py+((rand()%2?-1:1)*rand()%ALTURA);
        entidades[nEntidades-1].pDesenhox=0; entidades[nEntidades-1].pDesenhoy=0;
        entidades[nEntidades-1].vx=0; entidades[nEntidades-1].vy=0;
        entidades[nEntidades-1].inimigo=false;
        entidades[nEntidades-1].dano=false;
        entidades[nEntidades-1].escalaEntidade=1.7;
        entidades[nEntidades-1].raio=entidades[nEntidades-1].larguraSprite/4+entidades[nEntidades-1].alturaSprite/4;
        entidades[nEntidades-1].hp = 10*dificuldade;
        for (int j = 0; j < 4; j++){
            entidades[nEntidades-1].direcao[j]=false;
        }
        //inicializa balas
        entidades[nEntidades-1].atk=ATAQUE*dificuldade;
        entidades[nEntidades-1].def=DEFESA*dificuldade;
        if(!initBalas()) msgErro("Erro ao iniciar balas!");
        nPassivos++;
    }
    else{
        entidades[nEntidades-1].sprite=al_load_bitmap("./bin/entities/Char/NPC1.png");
        if(!entidades[nEntidades-1].sprite){
            msgErro("Deu ruim nos blocos!");
            return 0;
        } //inicializando tudo (obs, nada definitivo)
        entidades[nEntidades-1].larguraSprite = 1.7*al_get_bitmap_width(entidades[nEntidades-1].sprite); 
        entidades[nEntidades-1].alturaSprite =  1.7*al_get_bitmap_height(entidades[nEntidades-1].sprite);
        entidades[nEntidades-1].px=player.px+((rand()%2?-1:1)*rand()%LARGURA); entidades[nEntidades-1].py=player.py+((rand()%2?-1:1)*rand()%ALTURA);
        entidades[nEntidades-1].pDesenhox=0; entidades[nEntidades-1].pDesenhoy=0;
        entidades[nEntidades-1].vx=0; entidades[nEntidades-1].vy=0;
        entidades[nEntidades-1].inimigo=true;
        entidades[nEntidades-1].dano=false;
        entidades[nEntidades-1].escalaEntidade=1.7;
        entidades[nEntidades-1].raio=entidades[nEntidades-1].larguraSprite/4+entidades[nEntidades-1].alturaSprite/4;
        entidades[nEntidades-1].hp = 10*dificuldade;
        for (int j = 0; j < 4; j++){
            entidades[nEntidades-1].direcao[j]=false;
        }
        //inicializa balas
        entidades[nEntidades-1].atk=ATAQUE*dificuldade;
        entidades[nEntidades-1].def=DEFESA*dificuldade;
        if(!initBalas()) msgErro("Erro ao iniciar balas!");

    }
    return 1;
    //aqui teoricamente deveriam ficar todas as possibilidades e tal
}

int initBloco(){ //template pra qualquer geração de blocos, bem hardcoded mas pelo menos ta organizado
    aumentaBlocos();
    int i=0;
    int j=0;
    //casaB
    for (i; i < 2 ; i++, j++){ //template da geração de blocos, depois eu explico por que fiz assim
        int psx[2]={4,452};
        int psy[2]={26,26};
        blocos[j].sprite=al_load_bitmap("./bin/entities/Buildings/casaB.png");
        if(!blocos[j].sprite){
            msgErro("Deu ruim nos blocos!");
            return 0;
        }
        blocos[j].px = 2*psx[i]-4; blocos[j].py = 2*psy[i]+18;
        blocos[j].escalaEntidade=2;
        blocos[j].alturaHitbox=al_get_bitmap_height(blocos[j].sprite)*blocos[j].escalaEntidade/1.2; 
        blocos[j].larguraHitbox=al_get_bitmap_width(blocos[j].sprite)*blocos[j].escalaEntidade/1.2;
        blocos[j].larguraSprite=al_get_bitmap_width(blocos[j].sprite)*blocos[j].escalaEntidade;
        blocos[j].alturaSprite=al_get_bitmap_height(blocos[j].sprite)*blocos[j].escalaEntidade;
        blocos[j].py+=(blocos[j].alturaSprite-blocos[j].alturaHitbox)/2;
        blocos[j].px+=(blocos[j].larguraSprite-blocos[j].larguraHitbox)/2; blocos[j].interac=false;
    }
    i=0;
    //casaR
    for (i; i < 1 ; i++, j++){ //template da geração de blocos, depois eu explico por que fiz assim
        int psx[1]={548};
        int psy[1]={26};
        blocos[j].sprite=al_load_bitmap("./bin/entities/Buildings/casaR.png");
        if(!blocos[j].sprite){
            msgErro("Deu ruim nos blocos!");
            return 0;
        }
        blocos[j].px = 2*psx[i]-4; blocos[j].py = 2*psy[i]+18;
        blocos[j].escalaEntidade=2;
        blocos[j].alturaHitbox=al_get_bitmap_height(blocos[j].sprite)*blocos[j].escalaEntidade/1.2; 
        blocos[j].larguraHitbox=al_get_bitmap_width(blocos[j].sprite)*blocos[j].escalaEntidade/1.2;
        blocos[j].larguraSprite=al_get_bitmap_width(blocos[j].sprite)*blocos[j].escalaEntidade;
        blocos[j].alturaSprite=al_get_bitmap_height(blocos[j].sprite)*blocos[j].escalaEntidade;
        blocos[j].py+=(blocos[j].alturaSprite-blocos[j].alturaHitbox)/2;
        blocos[j].px+=(blocos[j].larguraSprite-blocos[j].larguraHitbox)/2; blocos[j].interac=false;
    }
    i=0;
    //casa G
    for (i; i < 1 ; i++, j++){ //template da geração de blocos, depois eu explico por que fiz assim
        int psx[1]={228};
        int psy[1]={26};
        blocos[j].sprite=al_load_bitmap("./bin/entities/Buildings/casaG.png");
        if(!blocos[j].sprite){
            msgErro("Deu ruim nos blocos!");
            return 0;
        }
        blocos[j].px = 2*psx[i]-4; blocos[j].py = 2*psy[i]+18;
        blocos[j].escalaEntidade=2;
        blocos[j].alturaHitbox=al_get_bitmap_height(blocos[j].sprite)*blocos[j].escalaEntidade/1.2; 
        blocos[j].larguraHitbox=al_get_bitmap_width(blocos[j].sprite)*blocos[j].escalaEntidade/1.2;
        blocos[j].larguraSprite=al_get_bitmap_width(blocos[j].sprite)*blocos[j].escalaEntidade;
        blocos[j].alturaSprite=al_get_bitmap_height(blocos[j].sprite)*blocos[j].escalaEntidade;
        blocos[j].py+=(blocos[j].alturaSprite-blocos[j].alturaHitbox)/2;
        blocos[j].px+=(blocos[j].larguraSprite-blocos[j].larguraHitbox)/2; blocos[j].interac=false;
    }
    i=0;
    //casaGrandeB
    for (i; i < 1 ; i++, j++){ //template da geração de blocos, depois eu explico por que fiz assim
        int psx[1]={320};
        int psy[1]={20};
        blocos[j].sprite=al_load_bitmap("./bin/entities/Buildings/casaGrandeB.png");
        if(!blocos[j].sprite){
            msgErro("Deu ruim nos blocos!");
            return 0;
        }
        blocos[j].px = 2*psx[i]-4; blocos[j].py = 2*psy[i]+18;
        blocos[j].escalaEntidade=2;
        blocos[j].alturaHitbox=al_get_bitmap_height(blocos[j].sprite)*blocos[j].escalaEntidade/1.2; 
        blocos[j].larguraHitbox=al_get_bitmap_width(blocos[j].sprite)*blocos[j].escalaEntidade/1.2;
        blocos[j].larguraSprite=al_get_bitmap_width(blocos[j].sprite)*blocos[j].escalaEntidade;
        blocos[j].alturaSprite=al_get_bitmap_height(blocos[j].sprite)*blocos[j].escalaEntidade;
        blocos[j].py+=(blocos[j].alturaSprite-blocos[j].alturaHitbox)/2;
        blocos[j].px+=(blocos[j].larguraSprite-blocos[j].larguraHitbox)/2; blocos[j].interac=false;
    }
    i=0;
    //tree3
    for (i; i < 13 ; i++, j++){ //template da geração de blocos, depois eu explico por que fiz assim
        int psx[13]={129,129,129,193,193,193,385,385,385,449,449,449,961};
        int psy[13]={261,293,325,261,293,325,261,293,325,261,293,325,549};
        blocos[j].sprite=al_load_bitmap("./bin/entities/Trees/tree3.png");
        if(!blocos[j].sprite){
            msgErro("Deu ruim nos blocos!");
            return 0;
        }
        blocos[j].px = 2*psx[i]; blocos[j].py = 2*(psy[i]+20);
        blocos[j].escalaEntidade=2;
        blocos[j].alturaHitbox=al_get_bitmap_height(blocos[j].sprite)*blocos[j].escalaEntidade/3; 
        blocos[j].larguraHitbox=al_get_bitmap_width(blocos[j].sprite)*blocos[j].escalaEntidade/3;
        blocos[j].larguraSprite=al_get_bitmap_width(blocos[j].sprite)*blocos[j].escalaEntidade;
        blocos[j].alturaSprite=al_get_bitmap_height(blocos[j].sprite)*blocos[j].escalaEntidade;
        blocos[j].py+=(blocos[j].alturaSprite-blocos[j].alturaHitbox)/2;
        blocos[j].px+=(blocos[j].larguraSprite-blocos[j].larguraHitbox)/2; blocos[j].interac=false;
    }
    i=0;
    //casaGrandeR
    for (i; i < 3 ; i++, j++){ //template da geração de blocos, depois eu explico por que fiz assim
        int psx[3]={96,0,512};
        int psy[3]={19,276,276};
        blocos[j].sprite=al_load_bitmap("./bin/entities/Buildings/casaGrandeR.png");
        if(!blocos[j].sprite){
            msgErro("Deu ruim nos blocos!");
            return 0;
        }
        blocos[j].px = 2*psx[i]-4; blocos[j].py = 2*psy[i]+18;
        blocos[j].escalaEntidade=2;
        blocos[j].alturaHitbox=al_get_bitmap_height(blocos[j].sprite)*blocos[j].escalaEntidade/1.2; 
        blocos[j].larguraHitbox=al_get_bitmap_width(blocos[j].sprite)*blocos[j].escalaEntidade/1.2;
        blocos[j].larguraSprite=al_get_bitmap_width(blocos[j].sprite)*blocos[j].escalaEntidade;
        blocos[j].alturaSprite=al_get_bitmap_height(blocos[j].sprite)*blocos[j].escalaEntidade;
        blocos[j].py+=(blocos[j].alturaSprite-blocos[j].alturaHitbox)/2;
        blocos[j].px+=(blocos[j].larguraSprite-blocos[j].larguraHitbox)/2; blocos[j].interac=false;
    }
    i=0;
    //casaGrandeG
    for (i; i < 1 ; i++, j++){ //template da geração de blocos, depois eu explico por que fiz assim
        int psx[1]={256};
        int psy[1]={276};
        blocos[j].sprite=al_load_bitmap("./bin/entities/Buildings/casaGrandeG.png");
        if(!blocos[j].sprite){
            msgErro("Deu ruim nos blocos!");
            return 0;
        }
        blocos[j].px = 2*psx[i]-4; blocos[j].py = 2*psy[i]+18;
        blocos[j].escalaEntidade=2;
        blocos[j].alturaHitbox=al_get_bitmap_height(blocos[j].sprite)*blocos[j].escalaEntidade/1.2; 
        blocos[j].larguraHitbox=al_get_bitmap_width(blocos[j].sprite)*blocos[j].escalaEntidade/1.2;
        blocos[j].larguraSprite=al_get_bitmap_width(blocos[j].sprite)*blocos[j].escalaEntidade;
        blocos[j].alturaSprite=al_get_bitmap_height(blocos[j].sprite)*blocos[j].escalaEntidade;
        blocos[j].py+=(blocos[j].alturaSprite-blocos[j].alturaHitbox)/2;
        blocos[j].px+=(blocos[j].larguraSprite-blocos[j].larguraHitbox)/2; blocos[j].interac=false;
    }
    i=0;
    //tree2
    for (i; i < 2 ; i++, j++){ //template da geração de blocos, depois eu explico por que fiz assim
        int psx[2]={1356,1473};
        int psy[2]={450,835};
        blocos[j].sprite=al_load_bitmap("./bin/entities/Trees/tree2.png");
        if(!blocos[j].sprite){
            msgErro("Deu ruim nos blocos!");
            return 0;
        }
        blocos[j].px = 2*psx[i]+40; blocos[j].py = 2*psy[i]+50;
        blocos[j].escalaEntidade=2;
        blocos[j].alturaHitbox=al_get_bitmap_height(blocos[j].sprite)*blocos[j].escalaEntidade/4; 
        blocos[j].larguraHitbox=al_get_bitmap_width(blocos[j].sprite)*blocos[j].escalaEntidade/4;
        blocos[j].larguraSprite=al_get_bitmap_width(blocos[j].sprite)*blocos[j].escalaEntidade;
        blocos[j].alturaSprite=al_get_bitmap_height(blocos[j].sprite)*blocos[j].escalaEntidade;
        blocos[j].py+=(blocos[j].alturaSprite-blocos[j].alturaHitbox)/2;
        blocos[j].px+=(blocos[j].larguraSprite-blocos[j].larguraHitbox)/2; blocos[j].interac=false;
    }
    i=0;
    //porco1 mudar altura e largura
    for (i; i < 1 ; i++, j++){ //template da geração de blocos, depois eu explico por que fiz assim
        int psx[1]={1060};
        int psy[1]={940};
        blocos[j].sprite=al_load_bitmap("./bin/entities/Char/vendor1.png");
        al_convert_mask_to_alpha(blocos[j].sprite,al_map_rgb(255,255,255));
        if(!blocos[j].sprite){
            msgErro("Deu ruim nos blocos!");
            return 0;
        }
        blocos[j].px = 2*psx[i]; blocos[j].py = 2*psy[i];
        blocos[j].escalaEntidade=2;
        blocos[j].alturaHitbox=al_get_bitmap_height(blocos[j].sprite)*blocos[j].escalaEntidade; 
        blocos[j].larguraHitbox=al_get_bitmap_width(blocos[j].sprite)*blocos[j].escalaEntidade;
        blocos[j].larguraSprite=al_get_bitmap_width(blocos[j].sprite)*blocos[j].escalaEntidade;
        blocos[j].alturaSprite=al_get_bitmap_height(blocos[j].sprite)*blocos[j].escalaEntidade;
        blocos[j].py+=(blocos[j].alturaSprite-blocos[j].alturaHitbox)/2;
        blocos[j].px+=(blocos[j].larguraSprite-blocos[j].larguraHitbox)/2; blocos[j].interac=false;
    }
    i=0;
    //porco2 mudar altura e largura
    for (i; i < 1 ; i++, j++){ //template da geração de blocos, depois eu explico por que fiz assim
        int psx[1]={1060};
        int psy[1]={1004};
        blocos[j].sprite=al_load_bitmap("./bin/entities/Char/vendor2.png");
        al_convert_mask_to_alpha(blocos[j].sprite,al_map_rgb(255,255,255));
        if(!blocos[j].sprite){
            msgErro("Deu ruim nos blocos!");
            return 0;
        }
        blocos[j].px = 2*psx[i]; blocos[j].py = 2*psy[i];
        blocos[j].escalaEntidade=2;
        blocos[j].alturaHitbox=2*al_get_bitmap_height(blocos[j].sprite)*blocos[j].escalaEntidade; 
        blocos[j].larguraHitbox=al_get_bitmap_width(blocos[j].sprite)*blocos[j].escalaEntidade;
        blocos[j].larguraSprite=al_get_bitmap_width(blocos[j].sprite)*blocos[j].escalaEntidade;
        blocos[j].alturaSprite=2*al_get_bitmap_height(blocos[j].sprite)*blocos[j].escalaEntidade;
        blocos[j].py+=(blocos[j].alturaSprite-blocos[j].alturaHitbox)/2;
        blocos[j].px+=(blocos[j].larguraSprite-blocos[j].larguraHitbox)/2; blocos[j].interac=false;
    }
    i=0;
    //porco3 mudar altura e largura
    for (i; i < 1 ; i++, j++){ //template da geração de blocos, depois eu explico por que fiz assim
        int psx[1]={1733};
        int psy[1]={1133};
        blocos[j].sprite=al_load_bitmap("./bin/entities/Char/vendor3.png");
        al_convert_mask_to_alpha(blocos[j].sprite,al_map_rgb(255,255,255));
        if(!blocos[j].sprite){
            msgErro("Deu ruim nos blocos!");
            return 0;
        }
        blocos[j].px = 2*psx[i]; blocos[j].py = 2*psy[i];
        blocos[j].escalaEntidade=2;
        blocos[j].alturaHitbox=al_get_bitmap_height(blocos[j].sprite)*blocos[j].escalaEntidade;
        blocos[j].larguraHitbox=al_get_bitmap_width(blocos[j].sprite)*blocos[j].escalaEntidade;
        blocos[j].larguraSprite=al_get_bitmap_width(blocos[j].sprite)*blocos[j].escalaEntidade;
        blocos[j].alturaSprite=al_get_bitmap_height(blocos[j].sprite)*blocos[j].escalaEntidade;
        blocos[j].py+=(blocos[j].alturaSprite-blocos[j].alturaHitbox)/2;
        blocos[j].px+=(blocos[j].larguraSprite-blocos[j].larguraHitbox)/2; blocos[j].interac=false;
    }
    i=0;
    //fabrica
    for (i; i < 1 ; i++, j++){ //template da geração de blocos, depois eu explico por que fiz assim
        int psx[1]={1760};
        int psy[1]={220};
        blocos[j].sprite=al_load_bitmap("./bin/entities/Buildings/fabricaAlt3M.png");
        if(!blocos[j].sprite){
            msgErro("Deu ruim nos blocos!");
            return 0;
        }
        blocos[j].px = 2*psx[i]; blocos[j].py = 2*psy[i];
        blocos[j].escalaEntidade=2;
        blocos[j].alturaHitbox=al_get_bitmap_height(blocos[j].sprite)*blocos[j].escalaEntidade/8; 
        blocos[j].larguraHitbox=al_get_bitmap_width(blocos[j].sprite)*blocos[j].escalaEntidade;
        blocos[j].larguraSprite=al_get_bitmap_width(blocos[j].sprite)*blocos[j].escalaEntidade;
        blocos[j].alturaSprite=al_get_bitmap_height(blocos[j].sprite)*blocos[j].escalaEntidade;
        blocos[j].py+=(blocos[j].alturaSprite-blocos[j].alturaHitbox)/2;
        blocos[j].px+=(blocos[j].larguraSprite-blocos[j].larguraHitbox)/2; blocos[j].interac=false;
    }
    i=0;
    //mansao, caso especial .
    for (i; i < 1 ; i++, j++){ //template da geração de blocos, depois eu explico por que fiz assim
        int psx[1]={836};
        int psy[1]={0};
        blocos[j].sprite=al_load_bitmap("./bin/entities/Buildings/mansao.png");
        if(!blocos[j].sprite){
            msgErro("Deu ruim nos blocos!");
            return 0;
        }
        blocos[j].px = 2*psx[i]; blocos[j].py = 2*psy[i];
        blocos[j].escalaEntidade=2;
        blocos[j].alturaHitbox=al_get_bitmap_height(blocos[j].sprite)*blocos[j].escalaEntidade; 
        blocos[j].larguraHitbox=al_get_bitmap_width(blocos[j].sprite)*blocos[j].escalaEntidade;
        blocos[j].alturaSprite=al_get_bitmap_height(blocos[j].sprite)*blocos[j].escalaEntidade;
        blocos[j].larguraSprite=al_get_bitmap_width(blocos[j].sprite)*blocos[j].escalaEntidade;
        blocos[j].py+=(blocos[j].alturaSprite-blocos[j].alturaHitbox)/2;
        blocos[j].px+=(blocos[j].larguraSprite-blocos[j].larguraHitbox)/2; blocos[j].interac=false;
    }
    i=0;
    //tree1
    for (i; i < 25 ; i++, j++){ //template da geração de blocos, depois eu explico por que fiz assim
        int psx[25]={1026,1026,1154,1154,1410,962,962,1506,1506,1218,1250,1186,1281,1153,1314,1122,1346,1122,1346,1153,1314,1186,1281,1281,1250};
        int psy[25]={330,364,330,364,490,587,619,777,811,586,586,618,618,649,649,682,682,715,715,746,746,778,778,810,810};
        blocos[j].sprite=al_load_bitmap("./bin/entities/Trees/tree1.png");
        if(!blocos[j].sprite){
            msgErro("Deu ruim nos blocos!");
            return 0;
        }
        blocos[j].px = 2*psx[i]-2; blocos[j].py = 2*psy[i]+40;
        blocos[j].escalaEntidade=2;
        blocos[j].alturaHitbox=al_get_bitmap_height(blocos[j].sprite)*blocos[j].escalaEntidade/5; 
        blocos[j].larguraHitbox=al_get_bitmap_width(blocos[j].sprite)*blocos[j].escalaEntidade/5;
        blocos[j].larguraSprite=al_get_bitmap_width(blocos[j].sprite)*blocos[j].escalaEntidade;
        blocos[j].alturaSprite=al_get_bitmap_height(blocos[j].sprite)*blocos[j].escalaEntidade;
        blocos[j].py+=(blocos[j].alturaSprite-blocos[j].alturaHitbox)/2;
        blocos[j].px+=(blocos[j].larguraSprite-blocos[j].larguraHitbox)/2; blocos[j].interac=false;
    }
    i=0;
    //escola
    for (i; i < 1 ; i++, j++){ //template da geração de blocos, depois eu explico por que fiz assim
        int psx[1]={1792};
        int psy[1]={702};
        blocos[j].sprite=al_load_bitmap("./bin/entities/Buildings/escola.png");
        if(!blocos[j].sprite){
            msgErro("Deu ruim nos blocos!");
            return 0;
        }
        blocos[j].px = 2*psx[i]; blocos[j].py = 2*psy[i];
        blocos[j].escalaEntidade=2;
        blocos[j].alturaHitbox=al_get_bitmap_height(blocos[j].sprite)*blocos[j].escalaEntidade/8; 
        blocos[j].larguraHitbox=al_get_bitmap_width(blocos[j].sprite)*blocos[j].escalaEntidade;
        blocos[j].alturaSprite=al_get_bitmap_height(blocos[j].sprite)*blocos[j].escalaEntidade;
        blocos[j].larguraSprite=al_get_bitmap_width(blocos[j].sprite)*blocos[j].escalaEntidade;
        blocos[j].py+=(blocos[j].alturaSprite-blocos[j].alturaHitbox)/2;
        blocos[j].px+=(blocos[j].larguraSprite-blocos[j].larguraHitbox)/2; blocos[j].interac=false;
    }
    i=0;
    //cerca1
    for (i; i < 2 ; i++, j++){ //template da geração de blocos, depois eu explico por que fiz assim
        int psx[2]={810,1387};
        int psy[2]={0,0};
        blocos[j].sprite=al_load_bitmap("./bin/entities/Buildings/cercaV.png");
        al_convert_mask_to_alpha(blocos[j].sprite,al_map_rgb(255,255,255));
        if(!blocos[j].sprite){
            msgErro("Deu ruim nos blocos!");
            return 0;
        }
        blocos[j].px = 2*psx[i]; blocos[j].py = 2*psy[i];
        blocos[j].escalaEntidade=2;
        blocos[j].alturaHitbox=al_get_bitmap_height(blocos[j].sprite)*blocos[j].escalaEntidade; 
        blocos[j].larguraHitbox=al_get_bitmap_width(blocos[j].sprite)*blocos[j].escalaEntidade;
        blocos[j].alturaSprite=al_get_bitmap_height(blocos[j].sprite)*blocos[j].escalaEntidade;
        blocos[j].larguraSprite=al_get_bitmap_width(blocos[j].sprite)*blocos[j].escalaEntidade;
        blocos[j].py+=(blocos[j].alturaSprite-blocos[j].alturaHitbox)/2;
        blocos[j].px+=(blocos[j].larguraSprite-blocos[j].larguraHitbox)/2; blocos[j].interac=false;
    }
    i=0;
    //cerca2
    for (i; i < 2 ; i++, j++){ //template da geração de blocos, depois eu explico por que fiz assim
        int psx[2]={811,1162};
        int psy[2]={416,416};
        blocos[j].sprite=al_load_bitmap("./bin/entities/Buildings/cercaH.png");
        al_convert_mask_to_alpha(blocos[j].sprite,al_map_rgb(255,255,255));
        if(!blocos[j].sprite){
            msgErro("Deu ruim nos blocos!");
            return 0;
        }
        blocos[j].px = 2*psx[i]; blocos[j].py = 2*psy[i];
        blocos[j].escalaEntidade=2;
        blocos[j].alturaHitbox=al_get_bitmap_height(blocos[j].sprite)*blocos[j].escalaEntidade; 
        blocos[j].larguraHitbox=al_get_bitmap_width(blocos[j].sprite)*blocos[j].escalaEntidade;
        blocos[j].alturaSprite=al_get_bitmap_height(blocos[j].sprite)*blocos[j].escalaEntidade;
        blocos[j].larguraSprite=al_get_bitmap_width(blocos[j].sprite)*blocos[j].escalaEntidade;
        blocos[j].py+=(blocos[j].alturaSprite-blocos[j].alturaHitbox)/2;
        blocos[j].px+=(blocos[j].larguraSprite-blocos[j].larguraHitbox)/2; blocos[j].interac=false;
    }
    i=0;
    //placas
    for (i; i < 11 ; i++, j++){ //template da geração de blocos, depois eu explico por que fiz assim
        int psx[11]={617,776,1097,1193,969,1001,1033,1065,2152,2025,2473};
        int psy[11]={552,616,361,841,1193,1193,1193,1129,590,873,1250};
        blocos[j].sprite=al_load_bitmap("./bin/entities/Clutter/props_sign.png");
        if(!blocos[j].sprite){
            msgErro("Deu ruim nos blocos!");
            return 0;
        }
        blocos[j].px = 2*psx[i]-18; blocos[j].py = 2*psy[i]-10;
        blocos[j].escalaEntidade=2;
        blocos[j].alturaHitbox=al_get_bitmap_height(blocos[j].sprite)*blocos[j].escalaEntidade/-2; 
        blocos[j].larguraHitbox=al_get_bitmap_width(blocos[j].sprite)*blocos[j].escalaEntidade/-2;
        blocos[j].alturaSprite=al_get_bitmap_height(blocos[j].sprite)*blocos[j].escalaEntidade;
        blocos[j].larguraSprite=al_get_bitmap_width(blocos[j].sprite)*blocos[j].escalaEntidade;
        blocos[j].py+=(blocos[j].alturaSprite-blocos[j].alturaHitbox)/2;
        blocos[j].px+=(blocos[j].larguraSprite-blocos[j].larguraHitbox)/2; blocos[j].interac=true;
    }
    i=0;
    //barril
    for (i; i < 1 ; i++, j++){ //template da geração de blocos, depois eu explico por que fiz assim
        int psx[1]={612};
        int psy[1]={2};
        blocos[j].sprite=al_load_bitmap("./bin/entities/Clutter/Barrel.png");
        if(!blocos[j].sprite){
            msgErro("Deu ruim nos blocos!");
            return 0;
        }
        blocos[j].px = 2*psx[i]-10; blocos[j].py = 2*psy[i]-5;
        blocos[j].escalaEntidade=2;
        blocos[j].alturaHitbox=al_get_bitmap_height(blocos[j].sprite)*blocos[j].escalaEntidade; 
        blocos[j].larguraHitbox=al_get_bitmap_width(blocos[j].sprite)*blocos[j].escalaEntidade;
        blocos[j].alturaSprite=al_get_bitmap_height(blocos[j].sprite)*blocos[j].escalaEntidade;
        blocos[j].larguraSprite=al_get_bitmap_width(blocos[j].sprite)*blocos[j].escalaEntidade;
        blocos[j].py+=(blocos[j].alturaSprite-blocos[j].alturaHitbox)/2;
        blocos[j].px+=(blocos[j].larguraSprite-blocos[j].larguraHitbox)/2; blocos[j].interac=true;
    }
    i=0;
    //lim dir
    for (i; i < 1 ; i++, j++){ //template da geração de blocos, depois eu explico por que fiz assim
        int psx[1]={2560};
        int psy[1]={0};
        blocos[j].sprite=al_load_bitmap("./bin/entities/Buildings/limdir.png");
        if(!blocos[j].sprite){
            msgErro("Deu ruim nos blocos!");
            return 0;
        }
        blocos[j].px = 2*psx[i]; blocos[j].py = 2*psy[i];
        blocos[j].escalaEntidade=2;
        blocos[j].alturaHitbox=al_get_bitmap_height(blocos[j].sprite)*blocos[j].escalaEntidade; 
        blocos[j].larguraHitbox=al_get_bitmap_width(blocos[j].sprite)*blocos[j].escalaEntidade;
        blocos[j].alturaSprite=al_get_bitmap_height(blocos[j].sprite)*blocos[j].escalaEntidade;
        blocos[j].larguraSprite=al_get_bitmap_width(blocos[j].sprite)*blocos[j].escalaEntidade;
        blocos[j].py+=(blocos[j].alturaSprite-blocos[j].alturaHitbox)/2;
        blocos[j].px+=(blocos[j].larguraSprite-blocos[j].larguraHitbox)/2; blocos[j].interac=false;
    }
    i=0;
    //lim inf
    for (i; i < 1 ; i++, j++){ //template da geração de blocos, depois eu explico por que fiz assim
        int psx[1]={0};
        int psy[1]={1356};
        blocos[j].sprite=al_load_bitmap("./bin/entities/Buildings/liminf.png");
        if(!blocos[j].sprite){
            msgErro("Deu ruim nos blocos!");
            return 0;
        }
        blocos[j].px = 2*psx[i]; blocos[j].py = 2*psy[i];
        blocos[j].escalaEntidade=2;
        blocos[j].alturaHitbox=al_get_bitmap_height(blocos[j].sprite)*blocos[j].escalaEntidade; 
        blocos[j].larguraHitbox=al_get_bitmap_width(blocos[j].sprite)*blocos[j].escalaEntidade;
        blocos[j].alturaSprite=al_get_bitmap_height(blocos[j].sprite)*blocos[j].escalaEntidade;
        blocos[j].larguraSprite=al_get_bitmap_width(blocos[j].sprite)*blocos[j].escalaEntidade;
        blocos[j].py+=(blocos[j].alturaSprite-blocos[j].alturaHitbox)/2;
        blocos[j].px+=(blocos[j].larguraSprite-blocos[j].larguraHitbox)/2; blocos[j].interac=true;
    }
    i=0;
    return 1;
}


void geraEntidades(){ //gera as entidades, pode ser preciso usar varios casos
    if(rand()%50==0 && nEntidades<=limEntidades){ //gera entidades aleatoriamente
        aumentaEntidades();
        if(!initEntidade()) estado=estSaida;

    }
    //tirei daqui o gera blocos, eles vao ser desenhados automaticamente
}
void jogadorAtaque(){
    if(balasPlayer.atingiu==true){
        balasPlayer.px = player.px; balasPlayer.py = player.py;
        if(balasPlayer.direcao[dCima]) balasPlayer.vy=-VELOCIDADE*8;
        if(balasPlayer.direcao[dBaixo]) balasPlayer.vy=VELOCIDADE*8;
        if(balasPlayer.direcao[dDireita]) balasPlayer.vx=VELOCIDADE*8;
        if(balasPlayer.direcao[dEsquerda]) balasPlayer.vx=-VELOCIDADE*8;
        balasPlayer.atingiu=false;
    }
}

void colisaoBalasP(){
    for (int i = 0; i < nBalas; i++){
        if(balasEntidades[i].atingiu==false){
            float distancia = sqrt(pow(player.px+player.larguraSprite/2-balasEntidades[i].px-balasEntidades[i].larguraSprite/2,2)+pow(player.py+player.alturaSprite/2-balasEntidades[i].py-balasEntidades[i].alturaSprite/2,2));
            if(distancia<=player.raio+balasEntidades[i].raio){
                player.hp-=entidades[i].atk/player.def;
                balasEntidades[i].atingiu=true;
                player.dano=true;
            }
            else{
                for (int j = 0; j < nBlocos; j++){
                    if (balasEntidades[i].px+balasEntidades[i].larguraSprite > blocos[j].px && balasEntidades[i].px < blocos[j].px+blocos[j].larguraHitbox && balasEntidades[i].py+player.alturaSprite > blocos[j].py && balasEntidades[i].py < blocos[j].py+blocos[j].alturaHitbox) balasEntidades[i].atingiu=true;
                }
            }
            
        }

    }
}

void colisaoBalasE(){
    for (int i = 0; i < nEntidades && !balasPlayer.atingiu; i++){
        if(entidades[i].naTela && entidades[i].inimigo && entidades[i].hp>0){
            float distancia = sqrt(pow(entidades[i].px+entidades[i].larguraSprite/2-balasPlayer.px-balasPlayer.larguraSprite/2,2)+pow(entidades[i].py+entidades[i].alturaSprite/2-balasPlayer.py-balasPlayer.alturaSprite/2,2));
            if(distancia<=entidades[i].raio+balasPlayer.raio){
                entidades[i].hp-=player.atk/entidades[i].def;
                balasPlayer.atingiu=true;
                entidades[i].dano=true;
            }

        }
        if(!balasPlayer.atingiu){
            for (int j = 0; j < nBlocos; j++){
                    if (balasPlayer.px+balasPlayer.larguraSprite > blocos[j].px && balasPlayer.px < blocos[j].px+blocos[j].larguraHitbox && balasPlayer.py+player.alturaSprite > blocos[j].py && balasPlayer.py < blocos[j].py+blocos[j].alturaHitbox) balasPlayer.atingiu=true;
                }
        }

    }

}

//atualiza a posição das balas
void atualizaBalas(){
    if(balasPlayer.atingiu==false){
        if(balasPlayer.px<0 || balasPlayer.px>largFase || balasPlayer.py<0 || balasPlayer.py>altFase) balasPlayer.atingiu=true;
            else al_draw_scaled_bitmap(balasPlayer.sprite,0,0,
                                   balasPlayer.larguraSprite/balasPlayer.escalaEntidade,balasPlayer.alturaSprite/balasPlayer.escalaEntidade,
                                   balasPlayer.px,balasPlayer.py,
                                   balasPlayer.larguraSprite,
                                   balasPlayer.alturaSprite,0);

    }
    else{
        balasPlayer.vx=0;
        balasPlayer.vy=0;
    }
    al_hold_bitmap_drawing(true);
    for (int i = 0; i < nBalas; i++){
        if(balasEntidades[i].atingiu==false && entidades[i].hp>0){
            if(balasEntidades[i].px<0 || balasEntidades[i].px>largFase || balasEntidades[i].py<0 || balasEntidades[i].py>altFase) balasEntidades[i].atingiu=true;
            else al_draw_scaled_bitmap( balasEntidades[i].sprite,0,0,
                                   balasEntidades[i].larguraSprite/balasEntidades[i].escalaEntidade,balasEntidades[i].alturaSprite/balasEntidades[i].escalaEntidade,
                                   balasEntidades[i].px,balasEntidades[i].py,
                                   balasEntidades[i].larguraSprite,
                                   balasEntidades[i].alturaSprite,0);
        }
        else{
            balasEntidades[i].vx=0;
            balasEntidades[i].vy=0;
        }

    }

    
    al_hold_bitmap_drawing(false);
    
}


void atualizaEntidades(){
    int naTela=0; 
    int *iNaTela=NULL;
    iNaTela = (int*)realloc(iNaTela,nEntidades*sizeof(int));
    al_hold_bitmap_drawing(true);
    for (int i = 0; i < nEntidades; i++){
        if(entidades[i].naTela && entidades[i].hp>0){ //desenha entidades escaladas e seus hitboxes
            if(entidades[i].dano){
                        al_draw_tinted_scaled_bitmap(entidades[i].sprite,al_map_rgba(0,0,255,0.5),0,0,
                                 entidades[i].larguraSprite/entidades[i].escalaEntidade,entidades[i].alturaSprite/entidades[i].escalaEntidade,
                                 entidades[i].px,entidades[i].py,
                                 entidades[i].larguraSprite,
                                 entidades[i].alturaSprite,0);
                        entidades[i].dano=false;
            }
            else al_draw_scaled_bitmap(entidades[i].sprite,0,0,
                                 entidades[i].larguraSprite/entidades[i].escalaEntidade,entidades[i].alturaSprite/entidades[i].escalaEntidade,
                                 entidades[i].px,entidades[i].py,
                                 entidades[i].larguraSprite,
                                 entidades[i].alturaSprite,0);
            
            iNaTela[naTela]=i;
            naTela++;

        }
    }
    al_hold_bitmap_drawing(false);
    for(int i = 0; i< naTela && mostraHitbox; i++){ 
        if(mostraHitbox) al_draw_circle(entidades[iNaTela[i]].px+entidades[iNaTela[i]].larguraSprite/2,entidades[iNaTela[i]].alturaSprite/2+entidades[iNaTela[i]].py,entidades[iNaTela[i]].raio,al_map_rgb(0,0,i),1);
        if(mostraHitbox) al_draw_circle(entidades[iNaTela[i]].px+entidades[iNaTela[i]].larguraSprite/2,entidades[iNaTela[i]].alturaSprite/2+entidades[iNaTela[i]].py,entidades[iNaTela[i]].escalaEntidade*RAIO_P*(3*dificuldade/2),al_map_rgb(0,0,i),1);
    }
    naTela=0;
    iNaTela=(int*)realloc(iNaTela,nBlocos*sizeof(int));
    al_hold_bitmap_drawing(true);
    for (int i = 0; i < nBlocos; i++){
        if(blocos[i].naTela){
            al_draw_scaled_bitmap(blocos[i].sprite,0,0,
                                    blocos[i].larguraSprite/blocos[i].escalaEntidade,blocos[i].alturaSprite/blocos[i].escalaEntidade,
                                    blocos[i].px+(-blocos[i].larguraSprite+blocos[i].larguraHitbox)/2,blocos[i].py+(-blocos[i].alturaSprite+blocos[i].alturaHitbox),
                                    blocos[i].larguraSprite,
                                    blocos[i].alturaSprite,0);
            iNaTela[naTela]=i;
            naTela++;

        }
    }
    al_hold_bitmap_drawing(false);
    for(int i = 0; i< naTela && mostraHitbox;i++){ 
        if(mostraHitbox) al_draw_rectangle(blocos[iNaTela[i]].px,blocos[iNaTela[i]].py,blocos[iNaTela[i]].px+blocos[iNaTela[i]].larguraHitbox,blocos[iNaTela[i]].py+blocos[iNaTela[i]].alturaHitbox,al_map_rgb(0,0,i),1);
    }
}
int contaFrames=1;
void atualizaJogador(){ //desenha e anima o jogador
    if((player.vx!=0 || player.vy!=0) && contaFrames%5==0){ //anima o sprite usando vx, vy, e a estrutura do player (dps eu implemento)
        contaFrames=1;
        player.frameAtual++;
        if(player.frameAtual==player.totalFrames){
            player.frameAtual=0;
        }
        player.pDesenhox=player.frameAtual*player.puloColuna;
    }
    else if(player.vx==0 && player.vy==0) player.pDesenhox=8*player.puloColuna;
    contaFrames++;
    if(player.direcao[dCima]){
            player.pDesenhoy=player.puloLinha*player.nDirecao[dCima];
    }
    if(player.direcao[dBaixo]){
            player.pDesenhoy=player.puloLinha*player.nDirecao[dBaixo];

    }
    if(player.direcao[dDireita]){
            player.pDesenhoy=player.puloLinha*player.nDirecao[dDireita];

    }
    if(player.direcao[dEsquerda]){
            player.pDesenhoy=player.puloLinha*player.nDirecao[dEsquerda];

    }

    if(player.dano && contaFrames%5!=0) al_draw_tinted_bitmap_region(player.sprite,al_map_rgba(255,0,0,0.5),player.pDesenhox,player.pDesenhoy,
                          player.larguraSprite,player.alturaSprite,
                          player.px,player.py,0);
    else al_draw_bitmap_region(player.sprite, //muito feio filho
                          player.pDesenhox,player.pDesenhoy,
                          player.larguraSprite,player.alturaSprite,
                          player.px,player.py,0);
    if(mostraHitbox) al_draw_circle(player.px+player.larguraSprite/2,player.py+player.larguraSprite/2,player.raio,al_map_rgb(0,0,0),1);
    
}

void UI(){

    ALLEGRO_FONT *fonteUI = al_load_font("./fonts/retroGaming.ttf",25/escala,0);
    al_draw_textf(fonteUI,al_map_rgb(255,255,255),pxFundo,pyFundo,ALLEGRO_ALIGN_LEFT,"HP: %.0f",player.hp);
    al_draw_textf(fonteUI,al_map_rgb(255,255,255),pxFundo,pyFundo+30,ALLEGRO_ALIGN_LEFT,"DEF: %.1f",player.def);
    al_draw_textf(fonteUI,al_map_rgb(255,255,255),pxFundo,pyFundo+60,ALLEGRO_ALIGN_LEFT,"ATK: %.1f",player.atk);
    al_destroy_font(fonteUI);
}


//fluxo do jogo
void jogo(){
    al_flush_event_queue(filaEventos);
    al_start_timer(timer);
    escala = 1.6f;
    //sai do loop do while
    bool sair=false;
    bool teste=false; //modo de teste, frame unico
    //unica serventia desse bool é pra animação de movimento do personagem
    bool desenhe=false; //desenha o proximo frame
    while (!sair && estado==estJogo){
        ALLEGRO_EVENT evento;
        al_wait_for_event(filaEventos,&evento);
        switch (evento.type){
            case ALLEGRO_EVENT_KEY_DOWN: //aperta uma tecla
                switch(evento.keyboard.keycode){
                    case ALLEGRO_KEY_UP:
                        player.vy-=VELOCIDADE;
                        player.direcao[dCima]=true;
                         
                        break;
                    case ALLEGRO_KEY_DOWN:
                        player.vy+=VELOCIDADE;
                        player.direcao[dBaixo]=true;
                         
                        break;
                    case ALLEGRO_KEY_RIGHT:
                        player.vx+=VELOCIDADE;
                        player.direcao[dDireita]=true;
                         
                        break;
                    case ALLEGRO_KEY_LEFT:
                        player.vx-=VELOCIDADE;
                        player.direcao[dEsquerda]=true;
                         
                        break;
                    case ALLEGRO_KEY_ESCAPE:
                        //a função retorna 0, 1 ou 2. 0 se a janela for fechada, 1 se for OK e 2 se for cancelar
                        switch(al_show_native_message_box(janela,"Saída","Deseja sair ou ir ao menu?","O progresso será perdido, salvo ou não","Deixa Quieto|Menu|Sair",ALLEGRO_MESSAGEBOX_OK_CANCEL)){
                            case 3:
                                sair=true;
                                estado = estSaida;
                                break;
                            case 2:
                                sair=true;
                                estado = estPreMenu;
                                escala = 1.0f;
                                player.px = 0;
                                player.py = 0;
                                pxFundo = 0;
                                pyFundo = 0;
                                al_identity_transform(&camera);
                                al_translate_transform(&camera,-(player.px+player.larguraSprite/2),-(player.py+player.larguraSprite/2)); //basicamente transforma tudo que ta na tela de acordo com esses parametros, eu vou mandar os videos que eu vi ensinando isso pq admito que nem eu entendi direito kkkkkk 
                                al_scale_transform(&camera,escala,escala); //esse é o mais simples
                                al_translate_transform(&camera,-pxFundo+(player.px+player.larguraSprite/2),-pyFundo+(player.py+player.larguraSprite/2)); 
                                al_use_transform(&camera); //simplesmente torna a transformação "canonica"
                                break;
                            defaut:
                                break;
                        }
                        player.vx=0;player.vy=0;escalaVelocidade=0;
                        al_flush_event_queue(filaEventos);
                        break;
                    case ALLEGRO_KEY_Q:
                        escalaVelocidade+= 0.1f;
                        break;
                    case ALLEGRO_KEY_E:
                        escalaVelocidade-=0.1f;
                        break;
                    case ALLEGRO_KEY_ENTER:
                        al_stop_timer(timer);
                        pauseJogo();
                        al_start_timer(timer);
                        break;

                    case ALLEGRO_KEY_C:
                        if(!mostraHitbox) mostraHitbox=true;
                        else mostraHitbox=false;
                        break;
                    case ALLEGRO_KEY_T: //entra no modo de teste, bom pra desbugar as coisas
                        if(!teste){
                            al_stop_timer(timer);
                            al_flush_event_queue(filaEventos);
                            al_start_timer(timerTeste);
                            teste=true;
                        }              
                        else{
                            al_stop_timer(timerTeste);
                            al_flush_event_queue(filaEventos);
                            al_start_timer(timer);
                            teste=false;
                        }
                    case ALLEGRO_KEY_W:
                        balasPlayer.direcao[dCima]=true;
                        jogadorAtaque();
                        break;
                    case ALLEGRO_KEY_A:
                        balasPlayer.direcao[dEsquerda]=true;
                        jogadorAtaque();
                        break;
                    case ALLEGRO_KEY_S:
                        balasPlayer.direcao[dBaixo]=true;
                        jogadorAtaque();
                        break;
                    case ALLEGRO_KEY_D:
                        balasPlayer.direcao[dDireita]=true;
                        jogadorAtaque();
                        break;
                    case ALLEGRO_KEY_SPACE:
                        interac=true;
                        break;
                }
                break;
            case ALLEGRO_EVENT_KEY_UP: //solta a tecla
                switch(evento.keyboard.keycode){
                    case ALLEGRO_KEY_UP:
                        if(player.vy!=0 || player.direcao[dCima]) player.vy+=VELOCIDADE;
                        player.direcao[dCima]=false;
                         
                        break;
                    case ALLEGRO_KEY_DOWN:
                        if(player.vy!=0 || player.direcao[dBaixo])player.vy-=VELOCIDADE;
                        player.direcao[dBaixo]=false;
                         
                        break;
                    case ALLEGRO_KEY_RIGHT:
                        if(player.vx!=0 || player.direcao[dDireita])player.vx-=VELOCIDADE;
                        player.direcao[dDireita]=false;
                         
                        break;
                    case ALLEGRO_KEY_LEFT:
                        if(player.vx!=0 || player.direcao[dEsquerda])player.vx+=VELOCIDADE;
                        player.direcao[dEsquerda]=false;
                         
                        break;
                    case ALLEGRO_KEY_Q:
                        escalaVelocidade-= 0.1f;
                        break;
                    case ALLEGRO_KEY_E:
                        escalaVelocidade+= 0.1f;
                        break;
                    case ALLEGRO_KEY_W:
                        balasPlayer.direcao[dCima]=false;
                        
                        break;
                    case ALLEGRO_KEY_A:
                        balasPlayer.direcao[dEsquerda]=false;
                        
                        break;
                    case ALLEGRO_KEY_S:
                        balasPlayer.direcao[dBaixo]=false;
                        
                        break;
                    case ALLEGRO_KEY_D:
                        balasPlayer.direcao[dDireita]=false;
                        
                        break;
                }
                break;
            case ALLEGRO_EVENT_TIMER:
                desenhe=true;
                
                escala+=escalaVelocidade; //soma a velocidade da escala à escala, analogo ao movimento normal
                if(escala<1.0f) escala=1.0f; //limita o zoom pra n bugar 
                if(escala>5.0f) escala=5.0f; //zoom maximo
        
                geraEntidades();
                player.dano=false;
                player.px+=player.vx;
                player.py+=player.vy;
                if(balasPlayer.atingiu==false){
                    balasPlayer.py+=balasPlayer.vy;
                    balasPlayer.px+=balasPlayer.vx;
                }
                atualizaCamera(); //aqui começa a magia negra
                colisaoBalasE();
                colisaoBalasP();
                colisaoJogador();
                if(mostrarCaixa){
                    al_stop_timer(timer);
                    caixaTexto(bCaixa);
                    al_start_timer(timer);
                }
                mostrarCaixa=false;
                interac=false;
                if(player.px<0) player.px=0;
                else if(player.px> largFase)player.px=largFase;
                if(player.py<0) player.py=0;
                else if(player.py>altFase) player.py=altFase;
                al_identity_transform(&camera);
                al_translate_transform(&camera,-(player.px+player.larguraSprite/2),-(player.py+player.larguraSprite/2)); //basicamente transforma tudo que ta na tela de acordo com esses parametros, eu vou mandar os videos que eu vi ensinando isso pq admito que nem eu entendi direito kkkkkk 
                al_scale_transform(&camera,escala,escala); //esse é o mais simples
                al_translate_transform(&camera,-pxFundo+(player.px+player.larguraSprite/2),-pyFundo+(player.py+player.larguraSprite/2)); 
                al_use_transform(&camera); //simplesmente torna a transformação "canonica"
                if(player.hp<=0) fimDeJogo();
                if(obj1){
                    caixaTexto(59+69);
                    player.hp=HP/dificuldade;
                    player.def=DEFESA/dificuldade;
                }
                if(obj2){
                    caixaTexto(59+70);
                    player.hp=HP/dificuldade;
                    player.def=DEFESA/dificuldade;
                }
                if(obj3){
                    caixaTexto(59+71);
                    player.hp=HP/dificuldade;
                    player.def=DEFESA/dificuldade;
                }
                if(obj4){
                    caixaTexto(59+72);
                    player.hp=HP/dificuldade;
                    player.def=DEFESA/dificuldade;
                }
                break;
            
            case ALLEGRO_EVENT_DISPLAY_CLOSE:
                sair = true;
                desenhe = false;
                break;
        }
        if(desenhe){
            desenhaMundo(); //desenha o fundo da fase atual
            atualizaJogador();
            atualizaBalas();
            atualizaEntidades();
            UI();
            al_draw_textf(retroFont,al_map_rgb(0,0,0),player.px+player.larguraSprite/2,player.py+player.alturaSprite,ALLEGRO_ALIGN_CENTER,"Escala =%.2f x=%.2f y=%.2f tx=%d ty=%d",escala,pxFundo,pyFundo,al_get_display_width(janela),al_get_display_height(janela));
            if(obj1 && obj2 && obj3 && obj4){
                sair=1;
                al_flush_event_queue(filaEventos);
                al_stop_timer(timer);
                estado = estFinal;
            }
            al_flip_display();
            desenhe=0;
        }
    }
}

void final(){

}