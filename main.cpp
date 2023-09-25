#include <iostream>
#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h> 
#include <allegro5/allegro_image.h>
#include <vector>
#include <algorithm>
#include <ctime>
#include <cstdlib>
#define _USE_MATH_DEFINES
#include <math.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>

const int boardSize = 10;                                           // rozmiar planszy
const int cellSize = 50;                                            // rozmiar komorki
const int screenWidth = 1280;                                       // szerokosc okna
const int screenHeight = 720;                                       // wysokosc okna
const ALLEGRO_COLOR COLOR_CELL_EMPTY = al_map_rgb(30, 127, 195);    // kolor pustego pola
const ALLEGRO_COLOR COLOR_CELL_VALID = al_map_rgb(124, 252, 0);     // kolor poprawnego pola

/// <summary>
/// Struktura przedstawiajaca pojedynczy statek.
/// 
/// Statek posiada id i rozmiar (od 1 do 3). 
/// W zmiennych x i y przechowywane sa wspolrzedne 1 elementu statku, jesli statek jest dluzszy niz jeden jego dalsze wspolrzedne obliczane sa dzieki zmiennym: size i isVertical. 
/// Zmienna isSunk przyjmuje wartosc true, gdy statek zostanie trafiny we wszystkie jego wspolrzedne.
/// </summary>
struct Ship {
    int id; 		   // id statku
    int size;           // rozmiar statku
    int x, y;           // wspolrzedne statku
    bool isVertical;    // czy statek jest pionowy
    bool isSunk;        // czy statek jest zatopiony
};

/// <summary>
/// Struktura przedstawiajaca jednego z graczy.
/// 
/// Kazdy z graczy ma zadeklarowana plansze, ktora jest 2 wymiarowa tablica vectorow. 
/// Wartosci w tablicy to: 0 - dla pustego pola; 1 - dla pola zajetego przez statek; 2 - dla pola pustego, ostrzelanego; 3 - dla pola zajetego przez statek, ostrzelanego; 4 - dla pola, w ktorym statek zostal zatopiony.
/// Zmienna ships to vector statkow (typu "Ship"). Dla kazdego gracza przewidziane jest 6 statkow. Zmienna hitCount okresla liczbe trafien, ktore wykonal gracz na przeciwnku.
/// Jesli liczba trafien przekroczy 12, gra powinna zostac zakonczona. Nastepnie jest konstruktor, ktory tworzy plansze oraz resetuje liczbe trafien.
/// </summary>
struct Player {
    std::vector<std::vector<int>> board;                            // plansza gracza
    std::vector<Ship> ships;                                        // statki gracza
    int hitCount;                                                   // liczba trafien

    Player() {                                                      // konstruktor
        hitCount = 0;                                               // poczatkowa liczba trafien
        board.resize(boardSize, std::vector<int>(boardSize, 0));    // ustawienie rozmiaru planszy (10x10 z wartoscia "0")
    }
};

static const struct Player EmptyPlayer;

/// <summary>
/// Klasa odpowiadajaca za przebieg gry.
/// 
/// Za pomoca tej klasy oraz jej metod przebiega cala gra. Odbywa sie to dzieki motodzie run(). Deklarowane sa rowniez metody oraz zmienne. Utworzeni zostaja gracze oraz vector przechowujacy wielkosci statkow.
/// </summary>
class Game {
public:
    Game();     // konstruktor
    ~Game();    // destruktor
    void run(); // funkcja uruchamiajaca gre

private:
    void drawBoards();                                                                          // funkcja rysujaca plansze
    void update();                                                                              // glowna petla gry
    bool humanPlaceShips();                                                                     // funkcja umieszczajaca statki przez gracza
    void computerPlaceShips();                                                                  // funkcja umieszczajaca statki przez komputer
    bool isValidPlacement(const Ship& ship, const std::vector<std::vector<int>>& board) const;  // funkcja sprawdzajaca czy mozna umiescic statek
    bool isGameOver() const;                                                                    // funkcja sprawdzajaca czy gra sie skonczyla
    void gameOverScreen();
    void checkShipSunk(std::vector<std::vector<int>>& board, std::vector<Ship>& ships, int cellY, int cellX) const;    // funkcja sprawdzajaca czy statek zostal zatopiony
    bool menu();                                                                                // funkcja menu

    ALLEGRO_DISPLAY* display;           // okno gry
    ALLEGRO_FONT* font; 	            // czcionka
    ALLEGRO_BITMAP* oneMast;
    ALLEGRO_BITMAP* twoMastFirst;
    ALLEGRO_BITMAP* twoMastSecond;
    ALLEGRO_BITMAP* threeMastFirst;
    ALLEGRO_BITMAP* threeMastSecond;
    ALLEGRO_BITMAP* threeMastThird;
    ALLEGRO_BITMAP* fire;
    ALLEGRO_BITMAP* grajButton;
    ALLEGRO_BITMAP* zamknijButton;
    ALLEGRO_BITMAP* powrotButton;
    ALLEGRO_BITMAP* resetButton;
    ALLEGRO_BITMAP* poddajButton;
    ALLEGRO_BITMAP* menuBackground;
    ALLEGRO_BITMAP* sunk;
    ALLEGRO_BITMAP* miss;
    ALLEGRO_BITMAP* background;
    ALLEGRO_BITMAP* winScreen;
    ALLEGRO_BITMAP* loseScreen;
    ALLEGRO_BITMAP* startScreen;
    ALLEGRO_SAMPLE* missSFX;
    ALLEGRO_SAMPLE* hitSFX;
    ALLEGRO_SAMPLE* clickSFX;
    ALLEGRO_SAMPLE* shipSFX;
    ALLEGRO_SAMPLE* winSFX;
    ALLEGRO_SAMPLE* loseSFX;
    ALLEGRO_SAMPLE* menuMusic;
    ALLEGRO_SAMPLE_ID idMusic;
    ALLEGRO_EVENT_QUEUE* eventQueue;    // kolejka zdarzen
    ALLEGRO_TIMER* timer;			    // timer

    Player human; 					    // gracz
    Player computer;				    // komputer
    bool humanTurn;					    // zmienna przechowujaca status tur
    bool gameOver;					    // zmienna przechowujaca status konca gry
    bool powrotStatus;
    bool poddajStatus;
    bool resetStatus;
    std::vector<int> shipSizes = { 3, 3, 2, 2, 1, 1 }; 	// rozmiary statkow
};

/// <summary>
/// Konstruktor klasy Game.
/// 
/// Najpierw inicjowane sa elementy biblioteki Allegro 5 oraz sama bilioteka. Oprocz biblioteki inicjowane jest oblugiwanie: myszy, dzwieku, rysowania figur i czcionek, obrazow.
/// Do wczesniej zadeklarowanych zmiennych zostaja przypisane: okno gry, czcionka (Absolute Empire), bitmapy, dzwieki, kolejka zdarzen, oraz timer.
/// Zostaje zarezerowowana ilosc sampli. Do kolejki zdarzen zostaja zarejstrowane zdarzenia.
/// </summary>
Game::Game() {
    al_init(); 																	    // inicjalizacja biblioteki allegro
    al_install_mouse(); 														    // inicjalizacja myszki
    al_install_audio(); 														    // inicjalizacja audio
    al_init_acodec_addon(); 													    // inicjalizacja kodekow audio
    al_init_primitives_addon(); 												    // inicjalizacja rysowania figur
    al_init_font_addon(); 														    // inicjalizacja czcionek
    al_init_ttf_addon(); 														    // inicjalizacja ttf
    al_init_image_addon(); 														    // inicjalizacja obrazkow

    display = al_create_display(screenWidth, screenHeight); 						// tworzenie okna gry
    font = al_load_ttf_font("arial.ttf", 50, 0); 									// wczytywanie czcionki
    oneMast = al_load_bitmap("Images/1Mast.png");
    twoMastFirst = al_load_bitmap("Images/2Mast-1.png");
    twoMastSecond = al_load_bitmap("Images/2Mast-2.png");
    threeMastFirst = al_load_bitmap("Images/3Mast-1.png");
    threeMastSecond = al_load_bitmap("Images/3Mast-2.png");
    threeMastThird = al_load_bitmap("Images/3Mast-3.png");
    fire = al_load_bitmap("Images/fire.png");
    grajButton = al_load_bitmap("Images/grajButton.png");
    zamknijButton = al_load_bitmap("Images/zamknijButton.png");
    powrotButton = al_load_bitmap("Images/powrotButton.png");
    resetButton = al_load_bitmap("Images/resetButton.png");
    poddajButton = al_load_bitmap("Images/poddajButton.png");
    menuBackground = al_load_bitmap("Images/5-720p.jpg");
    sunk = al_load_bitmap("Images/sunk.png");
    miss = al_load_bitmap("Images/miss.png");
    background = al_load_bitmap("Images/background.png");
    winScreen = al_load_bitmap("Images/winScreen.png");
    loseScreen = al_load_bitmap("Images/loseScreen.png");
    startScreen = al_load_bitmap("Images/twoja_tura.png");
    missSFX = al_load_sample("SFX/miss_SFX.mp3");
    hitSFX = al_load_sample("SFX/hit_SFX.mp3");
    clickSFX = al_load_sample("SFX/click_SFX.mp3");
    shipSFX = al_load_sample("SFX/ship_SFX.mp3");
    winSFX = al_load_sample("SFX/win_SFX.mp3");
    loseSFX = al_load_sample("SFX/lose_SFX.mp3");
    menuMusic = al_load_sample("SFX/themeMusic.mp3");
    eventQueue = al_create_event_queue(); 											// tworzenie kolejki zdarzen
    timer = al_create_timer(1.0 / 15); 												// tworzenie timera (15 fps)

    al_reserve_samples(7); 														    // rezerwowanie 7 sampli
    al_register_event_source(eventQueue, al_get_mouse_event_source()); 				// rejestrowanie zdarzen z myszki
    al_register_event_source(eventQueue, al_get_display_event_source(display)); 	// rejestrowanie zdarzen z okna gry
    al_register_event_source(eventQueue, al_get_timer_event_source(timer)); 		// rejestrowanie zdarzen z timera

    humanTurn = true; 															    // poczatkowa wartosc zmiennej humanTurn
    gameOver = false; 															    // poczatkowa wartosc zmiennej gameOver
    powrotStatus = true;
    poddajStatus = false;
    resetStatus = true;
}

/// <summary>
/// Destruktor klasy Game.
/// </summary>
Game::~Game() {
    al_destroy_display(display); 		    // usuwanie okna gry
    al_destroy_font(font); 				    // usuwanie czcionki
    al_destroy_bitmap(menuBackground); 		    // usuwanie obrazka statku
    al_destroy_bitmap(oneMast);
    al_destroy_bitmap(twoMastFirst);
    al_destroy_bitmap(twoMastSecond);
    al_destroy_bitmap(threeMastFirst);
    al_destroy_bitmap(threeMastSecond);
    al_destroy_bitmap(threeMastThird);
    al_destroy_bitmap(fire);
    al_destroy_bitmap(grajButton);
    al_destroy_bitmap(zamknijButton);
    al_destroy_bitmap(powrotButton);
    al_destroy_bitmap(resetButton);
    al_destroy_bitmap(poddajButton);
    al_destroy_bitmap(sunk);
    al_destroy_bitmap(miss);
    al_destroy_bitmap(background);
    al_destroy_bitmap(winScreen);
    al_destroy_bitmap(loseScreen);
    al_destroy_bitmap(startScreen);
    al_destroy_sample(missSFX);
    al_destroy_sample(hitSFX);
    al_destroy_sample(clickSFX);
    al_destroy_sample(shipSFX);
    al_destroy_sample(winSFX);
    al_destroy_sample(loseSFX);
    al_destroy_sample(menuMusic);
    al_destroy_event_queue(eventQueue); 	// usuwanie kolejki zdarzen
    al_destroy_timer(timer); 			    // usuwanie timera
}

/// <summary>
/// Rysowanie ekranu gry.
/// 
/// Na t³o zostaje wczytana bitmapa. Nastepnie zostaj¹ narysowane kwadraty odpowiadajace za tlo plansz. Nad planszami wyswietlaja sie napisy "Gracz" i "komputer", w zale¿noœci od tury jeden napis jest czerwony, a drugi bia³y.
/// Za pomoca 2 petli for na kazda plansze odczytywane sa wartosci pol z tablicy, a nastepnie rysowane sa: ustawione statki, nietrafione pola, trafione statki i zatopione statki.
/// Za pomoca instrukcji warunkowych, bitmapy statkow wyznaczone sa w odpowiednim kierunku. Na koncu funkcji rysowane sa bitmpay przyciskow oraz ekran koncowy w przypadku zakonczenia gry.
/// </summary>
void Game::drawBoards() {
    al_draw_bitmap(background, 0, 0, NULL);
    al_draw_filled_rectangle(70, 100, 570, 600, COLOR_CELL_EMPTY);
    al_draw_filled_rectangle(710, 100, 1210, 600, COLOR_CELL_EMPTY);

    if (humanTurn)
    {
        al_draw_text(font, al_map_rgb(255, 100, 100), screenWidth / 4, 25, ALLEGRO_ALIGN_CENTER, "Gracz");											// napis "Gracz"
        al_draw_text(font, al_map_rgb(255, 255, 255), 3 * screenWidth / 4, 25, ALLEGRO_ALIGN_CENTER, "Komputer");									// napis "Komputer"
    }
    else if (!humanTurn)
    {
        al_draw_text(font, al_map_rgb(255, 255, 255), screenWidth / 4, 25, ALLEGRO_ALIGN_CENTER, "Gracz");											// napis "Gracz"
        al_draw_text(font, al_map_rgb(255, 100, 100), 3 * screenWidth / 4, 25, ALLEGRO_ALIGN_CENTER, "Komputer");									// napis "Komputer"
    }

    // rysowanie planszy gracza
    for (int i = 0; i < boardSize; ++i) {
        for (int j = 0; j < boardSize; ++j) {
            if (human.board[i][j] == 1 && (i == 0 || (human.board[i - 1][j] != 1 && human.board[i - 1][j] != 3)) && (j == 0 || (human.board[i][j - 1] != 1 && human.board[i][j - 1] != 3)) && (i == boardSize - 1 || (human.board[i + 1][j] != 1 && human.board[i + 1][j] != 3)) && (j == boardSize - 1 || (human.board[i][j + 1] != 1 && human.board[i][j + 1] != 3))) {
                al_draw_bitmap(oneMast, j * cellSize + 70, i * cellSize + 100, NULL);           // jednomasztowiec
            }
            else if (human.board[i][j] == 1 && (i == 0 || (human.board[i - 1][j] != 1 && human.board[i - 1][j] != 3)) && (j == 0 || (human.board[i][j - 1] != 1 && human.board[i][j - 1] != 3)) && (i == boardSize - 1 || (human.board[i + 1][j] == 1 || human.board[i + 1][j] == 3)) && (j == boardSize - 1 || (human.board[i][j + 1] != 1 && human.board[i][j + 1] != 3)) && (i > boardSize - 3 || (human.board[i + 2][j] != 1 && human.board[i + 2][j] != 3))) {
                al_draw_bitmap(twoMastFirst, j * cellSize + 70, i * cellSize + 100, NULL); // dwumastowiec-1 (pionowo)
            }
            else if (human.board[i][j] == 1 && (i == 0 || (human.board[i - 1][j] == 1 || human.board[i - 1][j] == 3)) && (j == 0 || (human.board[i][j - 1] != 1 && human.board[i][j - 1] != 3)) && (i == boardSize - 1 || (human.board[i + 1][j] != 1 && human.board[i + 1][j] != 3)) && (j == boardSize - 1 || (human.board[i][j + 1] != 1 && human.board[i][j + 1] != 3)) && (i < 2 || (human.board[i - 2][j] != 1 && human.board[i - 2][j] != 3))) {
                al_draw_bitmap(twoMastSecond, j * cellSize + 70, i * cellSize + 100, NULL); // dwumastowiec-2 (pionowo)
            }
            else if (human.board[i][j] == 1 && (i == 0 || (human.board[i - 1][j] != 1 && human.board[i - 1][j] != 3)) && (j == 0 || (human.board[i][j - 1] != 1 && human.board[i][j - 1] != 3)) && (i == boardSize - 1 || (human.board[i + 1][j] != 1 && human.board[i + 1][j] != 3)) && (j == boardSize - 1 || (human.board[i][j + 1] == 1 || human.board[i][j + 1] == 3)) && (j > boardSize - 3 || (human.board[i][j + 2] != 1 && human.board[i][j + 2] != 3))) {
                al_draw_rotated_bitmap(twoMastFirst, 25, 25, j * cellSize + 70 + 25, i * cellSize + 100 + 25, 3 * M_PI / 2, NULL); // dwumastowiec-1 (poziomo)
            }
            else if (human.board[i][j] == 1 && (i == 0 || (human.board[i - 1][j] != 1 && human.board[i - 1][j] != 3)) && (j == 0 || (human.board[i][j - 1] == 1 || human.board[i][j - 1] == 3)) && (i == boardSize - 1 || (human.board[i + 1][j] != 1 && human.board[i + 1][j] != 3)) && (j == boardSize - 1 || (human.board[i][j + 1] != 1 && human.board[i][j + 1] != 3)) && (j < 2 || (human.board[i][j - 2] != 1 && human.board[i][j - 2] != 3))) {
                al_draw_rotated_bitmap(twoMastSecond, 25, 25, j * cellSize + 70 + 25, i * cellSize + 100 + 25, 3 * M_PI / 2, NULL); // dwumastowiec-2 (poziomo)
            }
            else if (human.board[i][j] == 1 && (i == 0 || (human.board[i - 1][j] != 1 && human.board[i - 1][j] != 3)) && (j == 0 || (human.board[i][j - 1] != 1 && human.board[i][j - 1] != 3)) && (i == boardSize - 1 || (human.board[i + 1][j] == 1 || human.board[i + 1][j] == 3)) && (j == boardSize - 1 || (human.board[i][j + 1] != 1 && human.board[i][j + 1] != 3)) && (i > boardSize - 3 || (human.board[i + 2][j] == 1 || human.board[i + 2][j] == 3))) {
                al_draw_bitmap(threeMastFirst, j * cellSize + 70, i * cellSize + 100, NULL); // trzymasztowiec-1 (pionowo)
            }
            else if (human.board[i][j] == 1 && (i == 0 || (human.board[i - 1][j] == 1 || human.board[i - 1][j] == 3)) && (j == 0 || (human.board[i][j - 1] != 1 && human.board[i][j - 1] != 3)) && (i == boardSize - 1 || (human.board[i + 1][j] == 1 || human.board[i + 1][j] == 3)) && (j == boardSize - 1 || (human.board[i][j + 1] != 1 && human.board[i][j + 1] != 3)) && (i < 2 || (human.board[i - 2][j] != 1 && human.board[i - 2][j] != 3))) {
                al_draw_bitmap(threeMastSecond, j * cellSize + 70, i * cellSize + 100, NULL); // trzymasztowiec-2 (pionowo)
            }
            else if (human.board[i][j] == 1 && (i == 0 || (human.board[i - 1][j] == 1 || human.board[i - 1][j] == 3)) && (j == 0 || (human.board[i][j - 1] != 1 && human.board[i][j - 1] != 3)) && (i == boardSize - 1 || (human.board[i + 1][j] != 1 && human.board[i + 1][j] != 3)) && (j == boardSize - 1 || (human.board[i][j + 1] != 1 && human.board[i][j + 1] != 3)) && (i < 2 || (human.board[i - 2][j] == 1 || human.board[i - 2][j] == 3))) {
                al_draw_bitmap(threeMastThird, j * cellSize + 70, i * cellSize + 100, NULL); // trzymasztowiec-3 (pionowo)
            }
            else if (human.board[i][j] == 1 && (i == 0 || (human.board[i - 1][j] != 1 && human.board[i - 1][j] != 3)) && (j == 0 || (human.board[i][j - 1] != 1 && human.board[i][j - 1] != 3)) && (i == boardSize - 1 || (human.board[i + 1][j] != 1 && human.board[i + 1][j] != 3)) && (j == boardSize - 1 || (human.board[i][j + 1] == 1 || human.board[i][j + 1] == 3)) && (j > boardSize - 3 || (human.board[i][j + 2] == 1 || human.board[i][j + 2] == 3))) {
                al_draw_rotated_bitmap(threeMastFirst, 25, 25, j * cellSize + 70 + 25, i * cellSize + 100 + 25, 3 * M_PI / 2, NULL); // trzymasztowiec-1 (poziomo)
            }
            else if (human.board[i][j] == 1 && (i == 0 || (human.board[i - 1][j] != 1 && human.board[i - 1][j] != 3)) && (j == 0 || (human.board[i][j - 1] == 1 || human.board[i][j - 1] == 3)) && (i == boardSize - 1 || (human.board[i + 1][j] != 1 && human.board[i + 1][j] != 3)) && (j == boardSize - 1 || (human.board[i][j + 1] == 1 || human.board[i][j + 1] == 3)) && (j < 2 || (human.board[i][j - 2] != 1 && human.board[i][j - 2] != 3))) {
                al_draw_rotated_bitmap(threeMastSecond, 25, 25, j * cellSize + 70 + 25, i * cellSize + 100 + 25, 3 * M_PI / 2, NULL); // trzymasztowiec-2 (poziomo)
            }
            else if (human.board[i][j] == 1 && (i == 0 || (human.board[i - 1][j] != 1 && human.board[i - 1][j] != 3)) && (j == 0 || (human.board[i][j - 1] == 1 || human.board[i][j - 1] == 3)) && (i == boardSize - 1 || (human.board[i + 1][j] != 1 && human.board[i + 1][j] != 3)) && (j == boardSize - 1 || (human.board[i][j + 1] != 1 && human.board[i][j + 1] != 3)) && (j < 2 || (human.board[i][j - 2] == 1 || human.board[i][j - 2] == 3))) {
                al_draw_rotated_bitmap(threeMastThird, 25, 25, j * cellSize + 70 + 25, i * cellSize + 100 + 25, 3 * M_PI / 2, NULL); // trzymasztowiec-3 (poziomo)
            }
            else if (human.board[i][j] == 2) {      // jesli pole jest puste (nie trafione)
                al_draw_bitmap(miss, j * cellSize + 70, i * cellSize + 100, NULL);
            }
            else if (human.board[i][j] == 4 && (i == 0 || (human.board[i - 1][j] == 0 || human.board[i - 1][j] == 2)) && (j == 0 || (human.board[i][j - 1] == 0 || human.board[i][j - 1] == 2)) && (i == boardSize - 1 || (human.board[i + 1][j] == 0 || human.board[i + 1][j] == 2)) && (j == boardSize - 1 || (human.board[i][j + 1] == 0 || human.board[i][j + 1] == 2))) {
                al_draw_bitmap(oneMast, j * cellSize + 70, i * cellSize + 100, NULL);           // jednomasztowiec
                al_draw_bitmap(sunk, j * cellSize + 70, i * cellSize + 100, NULL);
            }
            else if (human.board[i][j] == 3 && (i == 0 || (human.board[i - 1][j] != 1 && human.board[i - 1][j] != 3)) && (j == 0 || (human.board[i][j - 1] != 1 && human.board[i][j - 1] != 3)) && (i == boardSize - 1 || (human.board[i + 1][j] == 1 || human.board[i + 1][j] == 3)) && (j == boardSize - 1 || (human.board[i][j + 1] != 1 && human.board[i][j + 1] != 3)) && (i > boardSize - 3 || (human.board[i + 2][j] != 1 && human.board[i + 2][j] != 3))) {
                al_draw_bitmap(twoMastFirst, j * cellSize + 70, i * cellSize + 100, NULL); // dwumastowiec-1 (pionowo)
                al_draw_bitmap(fire, j * cellSize + 70, i * cellSize + 100, NULL);
            }
            else if (human.board[i][j] == 3 && (i == 0 || (human.board[i - 1][j] == 1 || human.board[i - 1][j] == 3)) && (j == 0 || (human.board[i][j - 1] != 1 && human.board[i][j - 1] != 3)) && (i == boardSize - 1 || (human.board[i + 1][j] != 1 && human.board[i + 1][j] != 3)) && (j == boardSize - 1 || (human.board[i][j + 1] != 1 && human.board[i][j + 1] != 3)) && (i < 2 || (human.board[i - 2][j] != 1 && human.board[i - 2][j] != 3))) {
                al_draw_bitmap(twoMastSecond, j * cellSize + 70, i * cellSize + 100, NULL); // dwumastowiec-2 (pionowo)
                al_draw_bitmap(fire, j * cellSize + 70, i * cellSize + 100, NULL);
            }
            else if (human.board[i][j] == 3 && (i == 0 || (human.board[i - 1][j] != 1 && human.board[i - 1][j] != 3)) && (j == 0 || (human.board[i][j - 1] != 1 && human.board[i][j - 1] != 3)) && (i == boardSize - 1 || (human.board[i + 1][j] != 1 && human.board[i + 1][j] != 3)) && (j == boardSize - 1 || (human.board[i][j + 1] == 1 || human.board[i][j + 1] == 3)) && (j > boardSize - 3 || (human.board[i][j + 2] != 1 && human.board[i][j + 2] != 3))) {
                al_draw_rotated_bitmap(twoMastFirst, 25, 25, j * cellSize + 70 + 25, i * cellSize + 100 + 25, 3 * M_PI / 2, NULL); // dwumastowiec-1 (poziomo)
                al_draw_bitmap(fire, j * cellSize + 70, i * cellSize + 100, NULL);
            }
            else if (human.board[i][j] == 3 && (i == 0 || (human.board[i - 1][j] != 1 && human.board[i - 1][j] != 3)) && (j == 0 || (human.board[i][j - 1] == 1 || human.board[i][j - 1] == 3)) && (i == boardSize - 1 || (human.board[i + 1][j] != 1 && human.board[i + 1][j] != 3)) && (j == boardSize - 1 || (human.board[i][j + 1] != 1 && human.board[i][j + 1] != 3)) && (j < 2 || (human.board[i][j - 2] != 1 && human.board[i][j - 2] != 3))) {
                al_draw_rotated_bitmap(twoMastSecond, 25, 25, j * cellSize + 70 + 25, i * cellSize + 100 + 25, 3 * M_PI / 2, NULL); // dwumastowiec-2 (poziomo)
                al_draw_bitmap(fire, j * cellSize + 70, i * cellSize + 100, NULL);
            }
            else if (human.board[i][j] == 3 && (i == 0 || (human.board[i - 1][j] != 1 && human.board[i - 1][j] != 3)) && (j == 0 || (human.board[i][j - 1] != 1 && human.board[i][j - 1] != 3)) && (i == boardSize - 1 || (human.board[i + 1][j] == 1 || human.board[i + 1][j] == 3)) && (j == boardSize - 1 || (human.board[i][j + 1] != 1 && human.board[i][j + 1] != 3)) && (i > boardSize - 3 || (human.board[i + 2][j] == 1 || human.board[i + 2][j] == 3))) {
                al_draw_bitmap(threeMastFirst, j * cellSize + 70, i * cellSize + 100, NULL); // trzymasztowiec-1 (pionowo)
                al_draw_bitmap(fire, j * cellSize + 70, i * cellSize + 100, NULL);
            }
            else if (human.board[i][j] == 3 && (i == 0 || (human.board[i - 1][j] == 1 || human.board[i - 1][j] == 3)) && (j == 0 || (human.board[i][j - 1] != 1 && human.board[i][j - 1] != 3)) && (i == boardSize - 1 || (human.board[i + 1][j] == 1 || human.board[i + 1][j] == 3)) && (j == boardSize - 1 || (human.board[i][j + 1] != 1 && human.board[i][j + 1] != 3)) && (i < 2 || (human.board[i - 2][j] != 1 && human.board[i - 2][j] != 3))) {
                al_draw_bitmap(threeMastSecond, j * cellSize + 70, i * cellSize + 100, NULL); // trzymasztowiec-2 (pionowo)
                al_draw_bitmap(fire, j * cellSize + 70, i * cellSize + 100, NULL);
            }
            else if (human.board[i][j] == 3 && (i == 0 || (human.board[i - 1][j] == 1 || human.board[i - 1][j] == 3)) && (j == 0 || (human.board[i][j - 1] != 1 && human.board[i][j - 1] != 3)) && (i == boardSize - 1 || (human.board[i + 1][j] != 1 && human.board[i + 1][j] != 3)) && (j == boardSize - 1 || (human.board[i][j + 1] != 1 && human.board[i][j + 1] != 3)) && (i < 2 || (human.board[i - 2][j] == 1 || human.board[i - 2][j] == 3))) {
                al_draw_bitmap(threeMastThird, j * cellSize + 70, i * cellSize + 100, NULL); // trzymasztowiec-3 (pionowo)
                al_draw_bitmap(fire, j * cellSize + 70, i * cellSize + 100, NULL);
            }
            else if (human.board[i][j] == 3 && (i == 0 || (human.board[i - 1][j] != 1 && human.board[i - 1][j] != 3)) && (j == 0 || (human.board[i][j - 1] != 1 && human.board[i][j - 1] != 3)) && (i == boardSize - 1 || (human.board[i + 1][j] != 1 && human.board[i + 1][j] != 3)) && (j == boardSize - 1 || (human.board[i][j + 1] == 1 || human.board[i][j + 1] == 3)) && (j > boardSize - 3 || (human.board[i][j + 2] == 1 || human.board[i][j + 2] == 3))) {
                al_draw_rotated_bitmap(threeMastFirst, 25, 25, j * cellSize + 70 + 25, i * cellSize + 100 + 25, 3 * M_PI / 2, NULL); // trzymasztowiec-1 (poziomo)
                al_draw_bitmap(fire, j * cellSize + 70, i * cellSize + 100, NULL);
            }
            else if (human.board[i][j] == 3 && (i == 0 || (human.board[i - 1][j] != 1 && human.board[i - 1][j] != 3)) && (j == 0 || (human.board[i][j - 1] == 1 || human.board[i][j - 1] == 3)) && (i == boardSize - 1 || (human.board[i + 1][j] != 1 && human.board[i + 1][j] != 3)) && (j == boardSize - 1 || (human.board[i][j + 1] == 1 || human.board[i][j + 1] == 3)) && (j < 2 || (human.board[i][j - 2] != 1 && human.board[i][j - 2] != 3))) {
                al_draw_rotated_bitmap(threeMastSecond, 25, 25, j * cellSize + 70 + 25, i * cellSize + 100 + 25, 3 * M_PI / 2, NULL); // trzymasztowiec-2 (poziomo)
                al_draw_bitmap(fire, j * cellSize + 70, i * cellSize + 100, NULL);
            }
            else if (human.board[i][j] == 3 && (i == 0 || (human.board[i - 1][j] != 1 && human.board[i - 1][j] != 3)) && (j == 0 || (human.board[i][j - 1] == 1 || human.board[i][j - 1] == 3)) && (i == boardSize - 1 || (human.board[i + 1][j] != 1 && human.board[i + 1][j] != 3)) && (j == boardSize - 1 || (human.board[i][j + 1] != 1 && human.board[i][j + 1] != 3)) && (j < 2 || (human.board[i][j - 2] == 1 || human.board[i][j - 2] == 3))) {
                al_draw_rotated_bitmap(threeMastThird, 25, 25, j * cellSize + 70 + 25, i * cellSize + 100 + 25, 3 * M_PI / 2, NULL); // trzymasztowiec-3 (poziomo)
                al_draw_bitmap(fire, j * cellSize + 70, i * cellSize + 100, NULL);
            }
            else if (human.board[i][j] == 4 && (i == 0 || human.board[i - 1][j] != 4) && (j == 0 || human.board[i][j - 1] != 4) && (i == boardSize - 1 || human.board[i + 1][j] == 4) && (j == boardSize - 1 || human.board[i][j + 1] != 4) && (i > boardSize - 3 || human.board[i + 2][j] != 4)) {
                al_draw_bitmap(twoMastFirst, j * cellSize + 70, i * cellSize + 100, NULL); // dwumastowiec-1 (pionowo)
                al_draw_bitmap(sunk, j * cellSize + 70, i * cellSize + 100, NULL);
            }
            else if (human.board[i][j] == 4 && (i == 0 || human.board[i - 1][j] == 4) && (j == 0 || human.board[i][j - 1] != 4) && (i == boardSize - 1 || human.board[i + 1][j] != 4) && (j == boardSize - 1 || human.board[i][j + 1] != 4) && (i < 2 || human.board[i - 2][j] != 4)) {
                al_draw_bitmap(twoMastSecond, j * cellSize + 70, i * cellSize + 100, NULL); // dwumastowiec-2 (pionowo)
                al_draw_bitmap(sunk, j * cellSize + 70, i * cellSize + 100, NULL);
            }
            else if (human.board[i][j] == 4 && (i == 0 || human.board[i - 1][j] != 4) && (j == 0 || human.board[i][j - 1] != 4) && (i == boardSize - 1 || human.board[i + 1][j] != 4) && (j == boardSize - 1 || human.board[i][j + 1] == 4) && (j > boardSize - 3 || human.board[i][j + 2] != 4)) {
                al_draw_rotated_bitmap(twoMastFirst, 25, 25, j * cellSize + 70 + 25, i * cellSize + 100 + 25, 3 * M_PI / 2, NULL); // dwumastowiec-1 (poziomo)
                al_draw_bitmap(sunk, j * cellSize + 70, i * cellSize + 100, NULL);
            }
            else if (human.board[i][j] == 4 && (i == 0 || human.board[i - 1][j] != 4) && (j == 0 || human.board[i][j - 1] == 4) && (i == boardSize - 1 || human.board[i + 1][j] != 4) && (j == boardSize - 1 || human.board[i][j + 1] != 4) && (j < 2 || human.board[i][j - 2] != 4)) {
                al_draw_rotated_bitmap(twoMastSecond, 25, 25, j * cellSize + 70 + 25, i * cellSize + 100 + 25, 3 * M_PI / 2, NULL); // dwumastowiec-2 (poziomo)
                al_draw_bitmap(sunk, j * cellSize + 70, i * cellSize + 100, NULL);
            }
            else if (human.board[i][j] == 4 && (i == 0 || human.board[i - 1][j] != 4) && (j == 0 || human.board[i][j - 1] != 4) && (i == boardSize - 1 || human.board[i + 1][j] == 4) && (j == boardSize - 1 || human.board[i][j + 1] != 4) && (i > boardSize - 3 || human.board[i + 2][j] == 4)) {
                al_draw_bitmap(threeMastFirst, j * cellSize + 70, i * cellSize + 100, NULL); // trzymasztowiec-1 (pionowo)
                al_draw_bitmap(sunk, j * cellSize + 70, i * cellSize + 100, NULL);
            }
            else if (human.board[i][j] == 4 && (i == 0 || human.board[i - 1][j] == 4) && (j == 0 || human.board[i][j - 1] != 4) && (i == boardSize - 1 || human.board[i + 1][j] == 4) && (j == boardSize - 1 || human.board[i][j + 1] != 4) && (i < 2 || human.board[i - 2][j] != 4)) {
                al_draw_bitmap(threeMastSecond, j * cellSize + 70, i * cellSize + 100, NULL); // trzymasztowiec-2 (pionowo)
                al_draw_bitmap(sunk, j * cellSize + 70, i * cellSize + 100, NULL);
            }
            else if (human.board[i][j] == 4 && (i == 0 || human.board[i - 1][j] == 4) && (j == 0 || human.board[i][j - 1] != 4) && (i == boardSize - 1 || human.board[i + 1][j] != 4) && (j == boardSize - 1 || human.board[i][j + 1] != 4) && (i < 2 || human.board[i - 2][j] == 4)) {
                al_draw_bitmap(threeMastThird, j * cellSize + 70, i * cellSize + 100, NULL); // trzymasztowiec-3 (pionowo)
                al_draw_bitmap(sunk, j * cellSize + 70, i * cellSize + 100, NULL);
            }
            else if (human.board[i][j] == 4 && (i == 0 || human.board[i - 1][j] != 4) && (j == 0 || human.board[i][j - 1] != 4) && (i == boardSize - 1 || human.board[i + 1][j] != 4) && (j == boardSize - 1 || human.board[i][j + 1] == 4) && (j > boardSize - 3 || human.board[i][j + 2] == 4)) {
                al_draw_rotated_bitmap(threeMastFirst, 25, 25, j * cellSize + 70 + 25, i * cellSize + 100 + 25, 3 * M_PI / 2, NULL); // trzymasztowiec-1 (poziomo)
                al_draw_bitmap(sunk, j * cellSize + 70, i * cellSize + 100, NULL);
            }
            else if (human.board[i][j] == 4 && (i == 0 || human.board[i - 1][j] != 4) && (j == 0 || human.board[i][j - 1] == 4) && (i == boardSize - 1 || human.board[i + 1][j] != 4) && (j == boardSize - 1 || human.board[i][j + 1] == 4) && (j < 2 || human.board[i][j - 2] != 4)) {
                al_draw_rotated_bitmap(threeMastSecond, 25, 25, j * cellSize + 70 + 25, i * cellSize + 100 + 25, 3 * M_PI / 2, NULL); // trzymasztowiec-2 (poziomo)
                al_draw_bitmap(sunk, j * cellSize + 70, i * cellSize + 100, NULL);
            }
            else if (human.board[i][j] == 4 && (i == 0 || human.board[i - 1][j] != 4) && (j == 0 || human.board[i][j - 1] == 4) && (i == boardSize - 1 || human.board[i + 1][j] != 4) && (j == boardSize - 1 || human.board[i][j + 1] != 4) && (j < 2 || human.board[i][j - 2] == 4)) {
                al_draw_rotated_bitmap(threeMastThird, 25, 25, j * cellSize + 70 + 25, i * cellSize + 100 + 25, 3 * M_PI / 2, NULL); // trzymasztowiec-3 (poziomo)
                al_draw_bitmap(sunk, j * cellSize + 70, i * cellSize + 100, NULL);
            }


            al_draw_rectangle(j * cellSize + 70, i * cellSize + 100, (j + 1) * cellSize + 70, (i + 1) * cellSize + 100, al_map_rgb(0, 0, 0), 1);	// rysowanie ramki
        }
    }

    // rysowanie planszy komputera
    for (int i = 0; i < boardSize; ++i) {
        for (int j = 0; j < boardSize; ++j) {
            if (computer.board[i][j] == 2) {																								// jesli pole jest puste (nie trafione)
                al_draw_bitmap(miss, j * cellSize + 710, i * cellSize + 100, NULL);
            }
            else if (computer.board[i][j] == 3) {																								// jesli pole jest zajete przez statek (trafione)
                al_draw_bitmap(fire, j * cellSize + 710, i * cellSize + 100, NULL);
            }
            else if (computer.board[i][j] == 4 && (i == 0 || computer.board[i - 1][j] != 4) && (j == 0 || computer.board[i][j - 1] != 4) && (i == boardSize - 1 || computer.board[i + 1][j] != 4) && (j == boardSize - 1 || computer.board[i][j + 1] != 4)) {
                al_draw_bitmap(oneMast, j * cellSize + 710, i * cellSize + 100, NULL);           // jednomasztowiec
                al_draw_bitmap(sunk, j * cellSize + 710, i * cellSize + 100, NULL);
            }
            else if (computer.board[i][j] == 4 && (i == 0 || computer.board[i - 1][j] != 4) && (j == 0 || computer.board[i][j - 1] != 4) && (i == boardSize - 1 || computer.board[i + 1][j] == 4) && (j == boardSize - 1 || computer.board[i][j + 1] != 4) && (i > boardSize - 3 || computer.board[i + 2][j] != 4)) {
                al_draw_bitmap(twoMastFirst, j * cellSize + 710, i * cellSize + 100, NULL); // dwumastowiec-1 (pionowo)
                al_draw_bitmap(sunk, j * cellSize + 710, i * cellSize + 100, NULL);
            }
            else if (computer.board[i][j] == 4 && (i == 0 || computer.board[i - 1][j] == 4) && (j == 0 || computer.board[i][j - 1] != 4) && (i == boardSize - 1 || computer.board[i + 1][j] != 4) && (j == boardSize - 1 || computer.board[i][j + 1] != 4) && (i < 2 || computer.board[i - 2][j] != 4)) {
                al_draw_bitmap(twoMastSecond, j * cellSize + 710, i * cellSize + 100, NULL); // dwumastowiec-2 (pionowo)
                al_draw_bitmap(sunk, j * cellSize + 710, i * cellSize + 100, NULL);
            }
            else if (computer.board[i][j] == 4 && (i == 0 || computer.board[i - 1][j] != 4) && (j == 0 || computer.board[i][j - 1] != 4) && (i == boardSize - 1 || computer.board[i + 1][j] != 4) && (j == boardSize - 1 || computer.board[i][j + 1] == 4) && (j > boardSize - 3 || computer.board[i][j + 2] != 4)) {
                al_draw_rotated_bitmap(twoMastFirst, 25, 25, j * cellSize + 710 + 25, i * cellSize + 100 + 25, 3 * M_PI / 2, NULL); // dwumastowiec-1 (poziomo)
                al_draw_bitmap(sunk, j * cellSize + 710, i * cellSize + 100, NULL);
            }
            else if (computer.board[i][j] == 4 && (i == 0 || computer.board[i - 1][j] != 4) && (j == 0 || computer.board[i][j - 1] == 4) && (i == boardSize - 1 || computer.board[i + 1][j] != 4) && (j == boardSize - 1 || computer.board[i][j + 1] != 4) && (j < 2 || computer.board[i][j - 2] != 4)) {
                al_draw_rotated_bitmap(twoMastSecond, 25, 25, j * cellSize + 710 + 25, i * cellSize + 100 + 25, 3 * M_PI / 2, NULL); // dwumastowiec-2 (poziomo)
                al_draw_bitmap(sunk, j * cellSize + 710, i * cellSize + 100, NULL);
            }
            else if (computer.board[i][j] == 4 && (i == 0 || computer.board[i - 1][j] != 4) && (j == 0 || computer.board[i][j - 1] != 4) && (i == boardSize - 1 || computer.board[i + 1][j] == 4) && (j == boardSize - 1 || computer.board[i][j + 1] != 4) && (i > boardSize - 3 || computer.board[i + 2][j] == 4)) {
                al_draw_bitmap(threeMastFirst, j * cellSize + 710, i * cellSize + 100, NULL); // trzymasztowiec-1 (pionowo)
                al_draw_bitmap(sunk, j * cellSize + 710, i * cellSize + 100, NULL);
            }
            else if (computer.board[i][j] == 4 && (i == 0 || computer.board[i - 1][j] == 4) && (j == 0 || computer.board[i][j - 1] != 4) && (i == boardSize - 1 || computer.board[i + 1][j] == 4) && (j == boardSize - 1 || computer.board[i][j + 1] != 4) && (i < 2 || computer.board[i - 2][j] != 4)) {
                al_draw_bitmap(threeMastSecond, j * cellSize + 710, i * cellSize + 100, NULL); // trzymasztowiec-2 (pionowo)
                al_draw_bitmap(sunk, j * cellSize + 710, i * cellSize + 100, NULL);
            }
            else if (computer.board[i][j] == 4 && (i == 0 || computer.board[i - 1][j] == 4) && (j == 0 || computer.board[i][j - 1] != 4) && (i == boardSize - 1 || computer.board[i + 1][j] != 4) && (j == boardSize - 1 || computer.board[i][j + 1] != 4) && (i < 2 || computer.board[i - 2][j] == 4)) {
                al_draw_bitmap(threeMastThird, j * cellSize + 710, i * cellSize + 100, NULL); // trzymasztowiec-3 (pionowo)
                al_draw_bitmap(sunk, j * cellSize + 710, i * cellSize + 100, NULL);
            }
            else if (computer.board[i][j] == 4 && (i == 0 || computer.board[i - 1][j] != 4) && (j == 0 || computer.board[i][j - 1] != 4) && (i == boardSize - 1 || computer.board[i + 1][j] != 4) && (j == boardSize - 1 || computer.board[i][j + 1] == 4) && (j > boardSize - 3 || computer.board[i][j + 2] == 4)) {
                al_draw_rotated_bitmap(threeMastFirst, 25, 25, j * cellSize + 710 + 25, i * cellSize + 100 + 25, 3 * M_PI / 2, NULL); // trzymasztowiec-1 (poziomo)
                al_draw_bitmap(sunk, j * cellSize + 710, i * cellSize + 100, NULL);
            }
            else if (computer.board[i][j] == 4 && (i == 0 || computer.board[i - 1][j] != 4) && (j == 0 || computer.board[i][j - 1] == 4) && (i == boardSize - 1 || computer.board[i + 1][j] != 4) && (j == boardSize - 1 || computer.board[i][j + 1] == 4) && (j < 2 || computer.board[i][j - 2] != 4)) {
                al_draw_rotated_bitmap(threeMastSecond, 25, 25, j * cellSize + 710 + 25, i * cellSize + 100 + 25, 3 * M_PI / 2, NULL); // trzymasztowiec-2 (poziomo)
                al_draw_bitmap(sunk, j * cellSize + 710, i * cellSize + 100, NULL);
            }
            else if (computer.board[i][j] == 4 && (i == 0 || computer.board[i - 1][j] != 4) && (j == 0 || computer.board[i][j - 1] == 4) && (i == boardSize - 1 || computer.board[i + 1][j] != 4) && (j == boardSize - 1 || computer.board[i][j + 1] != 4) && (j < 2 || computer.board[i][j - 2] == 4)) {
                al_draw_rotated_bitmap(threeMastThird, 25, 25, j * cellSize + 710 + 25, i * cellSize + 100 + 25, 3 * M_PI / 2, NULL); // trzymasztowiec-3 (poziomo)
                al_draw_bitmap(sunk, j * cellSize + 710, i * cellSize + 100, NULL);
            }

            al_draw_rectangle(j * cellSize + 710, i * cellSize + 100, (j + 1) * cellSize + 710, (i + 1) * cellSize + 100, al_map_rgb(0, 0, 0), 1); // rysowanie ramki
        }
    }

    if (!gameOver)
    {
        al_draw_bitmap(powrotButton, 70, 625, NULL);
        al_draw_bitmap(resetButton, 448, 625, NULL);
        al_draw_bitmap(poddajButton, 665, 625, NULL);
    }
    else if (gameOver)
    {
        if (human.hitCount == 12)
            al_draw_bitmap(winScreen, 0, 0, NULL);
        else
            al_draw_bitmap(loseScreen, 0, 0, NULL);

        al_draw_bitmap(powrotButton, 70, 625, NULL);
        al_draw_bitmap(resetButton, 448, 625, NULL);
    }

    al_flip_display(); 																														    // odswiezanie ekranu
}

/// <summary>
/// Czesc glowna gry.
/// 
/// Calosc wykonywana jest w petli. Tworzona jest zmienna, ktora przechowuje zdarzenia i dodaje je do kolejki. Za pomoca timera obraz jest caly czas odswiezany. Zamkniecie okna przerywa program.
/// W przypadku klikniecia LPM pobierane sa wspolrzedne myszy oraz wybierana jest akcja. Komputer natomiast wybiera pole do zaatakowania w sposob losowy. Na koncu zostaje sprawdzone czy gra zsotala zakonczona.
/// </summary>
void Game::update() {
    while (!gameOver) { 																    // petla gry
        ALLEGRO_EVENT event; 																// zmienna przechowujaca zdarzenie
        al_wait_for_event(eventQueue, &event); 												// oczekiwanie na zdarzenie
        if (event.type == ALLEGRO_EVENT_TIMER && humanTurn) {
            drawBoards(); 																	// odswiezanie plansz
        }
        else if (event.type == ALLEGRO_EVENT_DISPLAY_CLOSE) { 								// zamkniete okno gry
            break;
        }
        else if (event.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP && event.mouse.button == 1 && humanTurn) { 		    	// oddanie strzalu przez gracza
            int mouseX = event.mouse.x;
            int mouseY = event.mouse.y;

            int cellX = (mouseX - 710) / cellSize; 											// obliczanie wspolrzednych pola
            int cellY = (mouseY - 100) / cellSize; 											// obliczanie wspolrzednych pola

            if (cellX >= 0 && cellX < boardSize && cellY >= 0 && cellY < boardSize) { 		// sprawdzanie czy kliknieto w plansze
                if (computer.board[cellY][cellX] == 1) { 									// jesli trafiono statek
                    computer.board[cellY][cellX] = 3;
                    al_play_sample(hitSFX, 0.5, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL); 	// odtwarzanie dzwieku trafienia
                    checkShipSunk(computer.board, computer.ships, cellY, cellX);
                    human.hitCount++;
                }
                else if (computer.board[cellY][cellX] == 0) { 								// jesli nie trafiono statku
                    computer.board[cellY][cellX] = 2;
                    al_play_sample(missSFX, 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL); // odtwarzanie dzwieku pudla
                    humanTurn = false;
                }
            }
            else if (event.mouse.x >= 70 && event.mouse.x <= 287 && event.mouse.y >= 625 && event.mouse.y <= 695) // powrot
            {
                al_play_sample(clickSFX, 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL); // odtworzenie dzwieku klikniecia
                human = EmptyPlayer;
                computer = EmptyPlayer;
                al_stop_sample(&idMusic);
                powrotStatus = true;
            }
            else if (event.mouse.x >= 448 && event.mouse.x <= 615 && event.mouse.y >= 625 && event.mouse.y <= 695) // reset
            {
                al_play_sample(clickSFX, 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL); // odtworzenie dzwieku klikniecia
                human = EmptyPlayer;
                computer = EmptyPlayer;
                resetStatus = true;
            }
            else if (event.mouse.x >= 665 && event.mouse.x <= 832 && event.mouse.y >= 625 && event.mouse.y <= 695) // poddanie
            {
                al_play_sample(clickSFX, 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL); // odtworzenie dzwieku klikniecia
                computer.hitCount = 12;
            }
        }

        if (powrotStatus || resetStatus)
            break;

        if (!humanTurn && !gameOver)
        { 														// oddanie strzalu przez komputer
            drawBoards();
            al_rest(0.5); 																	// opoznienie

            int cellX, cellY;

            cellX = rand() % boardSize;
            cellY = rand() % boardSize;

            for (int i = 0; i < boardSize; ++i) {
                for (int j = 0; j < boardSize; ++j) {
                    if (human.board[i][j] == 3)
                    {
                        if (i > 0 && (human.board[i - 1][j] == 0 || human.board[i - 1][j] == 1)) {
                            cellX = j;
                            cellY = i - 1;
                        }
                        else if (i < boardSize - 1 && (human.board[i + 1][j] == 0 || human.board[i + 1][j] == 1)) {
                            cellX = j;
                            cellY = i + 1;
                        }
                        else if (j > 0 && (human.board[i][j - 1] == 0 || human.board[i][j - 1] == 1)) {
                            cellX = j - 1;
                            cellY = i;
                        }
                        else if (j < boardSize - 1 && (human.board[i][j + 1] == 0 || human.board[i][j + 1] == 1)) {
                            cellX = j + 1;
                            cellY = i;
                        }
                    }
                }
            }

            if (human.board[cellY][cellX] == 1) { 											// jesli trafiono statek
                human.board[cellY][cellX] = 3;
                al_play_sample(hitSFX, 0.5, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL); 		// odtwarzanie dzwieku trafienia
                checkShipSunk(human.board, human.ships, cellY, cellX);
                computer.hitCount++;
            }
            else if (human.board[cellY][cellX] == 0) { 										// jesli nie trafiono statku
                human.board[cellY][cellX] = 2;
                al_play_sample(missSFX, 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL); 		// odtwarzanie dzwieku pudla
                humanTurn = true;
            }
        }

        if (isGameOver()) { 																// sprawdzanie czy gra sie skonczyla
            gameOver = true;
            if (human.hitCount >= 12)
                al_play_sample(winSFX, 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL); 	// odtwarzanie dzwieku wygranej
            else
                al_play_sample(loseSFX, 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL); 	// odtwarzanie dzwieku przegranej
            break;
        }
    }
}

/// <summary>
/// Ustawianie statkow gracza.
/// 
/// Za pomoca petli for tworzone sa nowe statki. Ich wielkosc pobierana jest z wczesniej zadeklarowanej tablicy. Nastepnie za pomoca LPM mozemy wybrac miejsce ulokowania statku. 
/// Jesli funkcja isValidPlacement zwroci wartosc true, bedziemy mogli ulokowac nastepny statek, az umiescimy wszystkie 6. 
/// Podczas przsuwania kursorem rysowany jest prostokat, ktory ktory pokazuje, w ktorym miejscu zostanie polozony statek. Za pomoca PPM mozemy zmienic orientacje statku.
/// Zwraca wartosc true jesli uzytkownik umiesci wszystkie statki.
/// </summary>
bool Game::humanPlaceShips() {
    for (int z = 0; z < 6; z++) { 													// petla dla kazdego statku
        int shipSize = shipSizes[z]; 													// pobieranie rozmiaru statku
        Ship ship; 																		    // tworzenie statku
        ship.id = z;
        ship.isSunk = false;
        ship.size = shipSize; 															    // ustawianie rozmiaru statku
        ship.isVertical = true; 														    // ustawianie orientacji statku
        bool placed = false; 															    // zmienna przechowujaca informacje czy statek zostal umieszczony
        int mouseX = 0, mouseY = 0; 													    // zmienne przechowujace wspolrzedne myszy

        while (!placed) { 																// petla umieszczania statku
            bool shipSet = false; 														    // zmienna przechowujaca informacje czy statek zostal ustawiony

            while (!shipSet) { 															    // petla ustawiania statku
                ALLEGRO_EVENT event;
                al_wait_for_event(eventQueue, &event); 										// oczekiwanie na zdarzenie
                if (event.type == ALLEGRO_EVENT_TIMER) {
                    drawBoards(); 															// odswiezanie plansz
                    if (ship.isVertical && isValidPlacement(ship, human.board)) { 			// rysowanie propozycji umieszczenia statku
                        al_draw_rectangle(ship.x * cellSize + 70, ship.y * cellSize + 100, (ship.x + 1) * cellSize + 70, (ship.y + 1) * cellSize + 100 + (ship.size - 1) * cellSize, COLOR_CELL_VALID, 3);
                    }
                    else if (!ship.isVertical && isValidPlacement(ship, human.board))
                    {
                        al_draw_rectangle(ship.x * cellSize + 70, ship.y * cellSize + 100, (ship.x + 1) * cellSize + 70 + (ship.size - 1) * cellSize, (ship.y + 1) * cellSize + 100, COLOR_CELL_VALID, 3);
                    }
                    al_flip_display(); 														// odswiezanie ekranu
                }
                else if (event.type == ALLEGRO_EVENT_DISPLAY_CLOSE) { 						// zamkniete okno gry
                    return false;
                }
                else if (event.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP && event.mouse.button == 1) { 	// wybranie pozycji statku (prawy przycisk myszy
                    mouseX = event.mouse.x;
                    mouseY = event.mouse.y;

                    ship.x = (mouseX - 70) / cellSize; 										// obliczanie wspolrzednych statku
                    ship.y = (mouseY - 100) / cellSize; 									// obliczanie wspolrzednych statku

                    shipSet = true; 														// ustawienie statku

                    if (event.mouse.x >= 70 && event.mouse.x <= 287 && event.mouse.y >= 625 && event.mouse.y <= 695) // powrot
                    {
                        al_play_sample(clickSFX, 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL); // odtworzenie dzwieku klikniecia
                        human = EmptyPlayer;
                        computer = EmptyPlayer;
                        al_stop_sample(&idMusic);
                        powrotStatus = true;
                    }
                    else if (event.mouse.x >= 448 && event.mouse.x <= 615 && event.mouse.y >= 625 && event.mouse.y <= 695) // reset
                    {
                        al_play_sample(clickSFX, 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL); // odtworzenie dzwieku klikniecia
                        human = EmptyPlayer;
                        computer = EmptyPlayer;
                        resetStatus = true;
                    }
                    else if (event.mouse.x >= 665 && event.mouse.x <= 832 && event.mouse.y >= 625 && event.mouse.y <= 695) // poddanie
                    {
                        al_play_sample(clickSFX, 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL); // odtworzenie dzwieku klikniecia
                        computer.hitCount = 12;
                        poddajStatus = true;
                    }
                }
                else if (event.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP && event.mouse.button == 2) { 	// zmiana orientacji statku (lewy przycisk myszy)
                    if (ship.isVertical) {
                        ship.isVertical = false;
                    }
                    else {
                        ship.isVertical = true;
                    }
                }
                else if (event.type == ALLEGRO_EVENT_MOUSE_AXES) { 							// ruch myszy (zmienne do porpozycji statku)
                    ship.x = (event.mouse.x - 70) / cellSize;
                    ship.y = (event.mouse.y - 100) / cellSize;
                }
            }

            if (isValidPlacement(ship, human.board)) { 										// jesli statek zostal ustawiony poprawnie - wczytanie go do planszy
                for (int i = 0; i < ship.size; ++i) {
                    if (ship.isVertical) { 													// ustawienie statku w zaleznosci od orientacji
                        human.board[ship.y + i][ship.x] = 1;
                    }
                    else {
                        human.board[ship.y][ship.x + i] = 1;
                    }
                }

                human.ships.push_back(ship); 												// dodanie statku do wektora statkow gracza
                placed = true;
            }

            if (powrotStatus || poddajStatus || resetStatus)
                break;
        }
        if (powrotStatus || poddajStatus || resetStatus)
        {
            poddajStatus = false;
            break;
        }

        al_play_sample(shipSFX, 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL); 				// odtwarzanie dzwieku umieszczenia statku
    }
    al_draw_bitmap(startScreen, 0, 0, NULL);
    al_flip_display();
    al_rest(1);
    return true;
}

/// <summary>
/// Ustawianie statkow gracza.
/// 
/// Za pomoca petli for tworzone sa nowe statki. Ich wielkosc pobierana jest z wczesniej zadeklarowanej tablicy. Nastepnie za pomoca LPM mozemy wybrac miejsce ulokowania statku. 
/// Jesli funkcja isValidPlacement zwroci wartosc true, bedziemy mogli ulokowac nastepny statek, az umiescimy wszystkie 6. 
/// Podczas przsuwania kursorem rysowany jest prostokat, ktory ktory pokazuje, w ktorym miejscu zostanie polozony statek. Za pomoca PPM mozemy zmienic orientacje statku.
/// </summary>
void Game::computerPlaceShips() {
    for (int z = 0; z < 6; z++) { 													// petla dla kazdego statku
        int shipSize = shipSizes[z]; 													// pobieranie rozmiaru statku
        Ship ship; 																		    // tworzenie statku
        ship.id = z;
        ship.isSunk = false;
        ship.size = shipSize; 															    // ustawianie rozmiaru statku
        bool placed = false;
        while (!placed) { 																// petla umieszczania statku w sposob losowy
            ship.isVertical = rand() % 2;
            ship.x = rand() % (ship.isVertical ? boardSize : boardSize - ship.size + 1);
            ship.y = rand() % (ship.isVertical ? boardSize - ship.size + 1 : boardSize);

            if (isValidPlacement(ship, computer.board)) { 								// jesli statek zostal ustawiony poprawnie - wczytanie go do planszy
                for (int i = 0; i < ship.size; ++i) {
                    if (ship.isVertical) { 												// ustawienie statku w zaleznosci od orientacji
                        computer.board[ship.y + i][ship.x] = 1;
                    }
                    else {
                        computer.board[ship.y][ship.x + i] = 1;
                    }
                }

                computer.ships.push_back(ship); 										// dodanie statku do wektora statkow komputera
                placed = true;
            }
        }
    }
}

/// <summary>
/// Sprawdzanie poprawnosci umieszczonego statku.
/// 
/// Funkcja sluzy do sprawdzania czy pole, w ktorym ma byc statek oraz sasiednie pola sa puste. W tablicach dx i dy znajduja sie zmiany wspolrzednych, dzieki ktorym mozemy sprawdzic sasiednie wspolrzedne.
/// Sprawdzane jest kazde pole statku z osobna. Zwraca wartosc true jest wspolrzedne spelniaja wymagania do umieszenia statku.
/// </summary>
bool Game::isValidPlacement(const Ship& ship, const std::vector<std::vector<int>>& board) const {
    int dx[] = { -1, 0, 1, 0, -1, 1, 1, -1 }; 												        // tablica wspolrzednych x sasiadow (lewo, dol, prawo, gora, lewo-gora, prawo-dol, prawo-gora, lewo-dol)
    int dy[] = { 0, 1, 0, -1, -1, 1, -1, 1 }; 												        // tablica wspolrzednych y sasiadow

    for (int i = 0; i < ship.size; ++i) { 													        // petla dla kazdego pola statku
        int x = ship.x + (ship.isVertical ? 0 : i); 											    // ustalanie wspolrzednej x
        int y = ship.y + (ship.isVertical ? i : 0); 											    // ustalanie wspolrzednej y

        if (x < 0 || x >= board[0].size() || y < 0 || y >= board.size() || board[y][x] != 0) { 	    // sprawdzanie czy pole dla statku jest puste
            return false;
        }

        for (int j = 0; j < 8; ++j) { 														        // petla dla kazdego sasiada
            int nx = x + dx[j];
            int ny = y + dy[j];

            if (nx >= 0 && ny >= 0 && nx < board[0].size() && ny < board.size() && board[ny][nx] != 0) { // sprawdzanie czy pole sasiadujace nie jest zajete
                return false;
            }
        }
    }

    return true;
}

/// <summary>
/// Sprawdzane Game Over.
/// 
/// Funkcja sluzy do sprawdzania, czy ktorys z graczy osiagnal minimalna ilosc punktow potrzebna do wygrania. Po jej osiagnieciu nastepuje Game Over.
/// Zwraca true jesli, jeden z graczy zdobyl wymagana liczbe punktow.
/// </summary>
bool Game::isGameOver() const {
    return human.hitCount == 12 || computer.hitCount >= 12; 					// sprawdzanie czy gra sie skonczyla (gry ktoras ze stron ma 12 pumktow)
}

/// <summary>
/// Wyswietlanie ekranu Game Over.
/// 
/// Ekran jest wyswiatlany po zakonczeniu gry. Wyswietla sie ekran, ktory jest zawarty w funkcji drawBoards dla "gameOver = true". Gracze i ich plansze zostaja zresetowani.
/// </summary>
void Game::gameOverScreen() {
    while (true) { 																    // petla gry
        drawBoards();

        ALLEGRO_EVENT event; 																// zmienna przechowujaca zdarzenie
        al_wait_for_event(eventQueue, &event); 												// oczekiwanie na zdarzenie
        if (event.type == ALLEGRO_EVENT_TIMER) {
        }
        else if (event.type == ALLEGRO_EVENT_DISPLAY_CLOSE) { 								// zamkniete okno gry
            break;
        }
        else if (event.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP) { 		    	// oddanie strzalu przez gracza
            if (event.mouse.x >= 70 && event.mouse.x <= 287 && event.mouse.y >= 625 && event.mouse.y <= 695) // powrot
            {
                al_play_sample(clickSFX, 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL); // odtworzenie dzwieku klikniecia
                human = EmptyPlayer;
                computer = EmptyPlayer;
                al_stop_sample(&idMusic);
                powrotStatus = true;
                break;
            }
            else if (event.mouse.x >= 448 && event.mouse.x <= 615 && event.mouse.y >= 625 && event.mouse.y <= 695) // reset
            {
                al_play_sample(clickSFX, 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL); // odtworzenie dzwieku klikniecia
                human = EmptyPlayer;
                computer = EmptyPlayer;
                resetStatus = true;
                break;
            }
        }
    }
}

/// <summary>
/// Sprawdzanie zatopienia statku.
/// 
/// Funkcja po kolei sprawdza, ktory ze statkow znajduje sie w miejscu trafienia i sprawdza czy pozostale pola statku rowniez sa trafione. Jesli tak to zmienia jego oznaczenie w tablicy na 4 dla kazdego pola, ktore zajmuje.
/// </summary>
void Game::checkShipSunk(std::vector<std::vector<int>>& board, std::vector<Ship>& ships, int cellY, int cellX) const {
    for (int i = 0; i < 6; i++)
    {
        int count = 0;
        if (ships[i].isVertical && ships[i].x == cellX && ships[i].y <= cellY && ships[i].y + ships[i].size > cellY) {
            for (int j = 0; j < ships[i].size; j++)
            {
                if (board[ships[i].y + j][ships[i].x] == 3)
                    count++;
                else
                    break;
            }
            if (count == ships[i].size)
            {
                ships[i].isSunk = true;
                for (int j = 0; j < ships[i].size; j++)
                    board[ships[i].y + j][ships[i].x] = 4;
            }
            else
                break;
        }
        else if (!ships[i].isVertical && ships[i].y == cellY && ships[i].x <= cellX && ships[i].x + ships[i].size > cellX) {
            for (int j = 0; j < ships[i].size; j++)
            {
                if (board[ships[i].y][ships[i].x + j] == 3)
                    count++;
                else
                    break;
            }
            if (count == ships[i].size)
            {
                ships[i].isSunk = true;
                for (int j = 0; j < ships[i].size; j++)
                    board[ships[i].y][ships[i].x + j] = 4;
            }
            else
                break;
        }
    }
}

/// <summary>
/// Wyswietlanie menu.
/// 
/// Wyswietla ekran menu. Tlem menu jest bitmapa. Po lewej stronie znajduja sie 2 przyciski. Jeden jest odpowiedzialny za przejscie do gry, drugi za wyjscie z gry.
/// Zwraca true, jesli uzytkownik nacisnie przycisk GRAJ.
/// </summary>
bool Game::menu() {
    al_draw_bitmap(menuBackground, 0, 0, NULL);
    al_play_sample(menuMusic, 0.1, 0.0, 1.0, ALLEGRO_PLAYMODE_LOOP, &idMusic); // odtwarzanie dzwieku w petli

    // Rysowanie przycisków
    al_draw_bitmap(grajButton, 100, 100, NULL);
    al_draw_bitmap(zamknijButton, 100, 250, NULL);

    font = al_load_ttf_font("arial.ttf", 50, NULL); // ustawienie czcionki

    bool running = true;
    while (running) // petla menu
    {
        ALLEGRO_EVENT event;   // zmienna przechowujaca zdarzenie
        al_wait_for_event(eventQueue, &event);      // oczekiwanie na zdarzenie
        if (event.type == ALLEGRO_EVENT_DISPLAY_CLOSE) { // jesli zostalo zamkniete okno
            return false;
        }
        else if (event.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP) // jesli zostal wcisniety przycisk myszy
        {
            if (event.mouse.button == 1) { // lewy przycisk myszy
                int x = event.mouse.x; // pobranie wspolrzednej x
                int y = event.mouse.y; // pobranie wspolrzednej y
                if (x >= 100 && x <= 410 && y >= 100 && y <= 200) { // jesli zostal wcisniety przycisk GRAJ
                    running = false;
                    al_play_sample(clickSFX, 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL); // odtworzenie dzwieku klikniecia
                    return true;
                }
                else if (x >= 100 && x <= 410 && y >= 250 && y <= 350) { // jesli zostal wcisniety przycisk WYJDZ
                    running = false;
                    al_play_sample(clickSFX, 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL); // odtworzenie dzwieku klikniecia
                    return false;
                }
            }
        }
        al_flip_display(); // odswiezenie ekranu
    }
    return false;
}

/// <summary>
/// Wyswietlanie etapow gry w odpowiedniej kolejnosci.
/// 
/// Wyswietla etapy gry w odpowiedniej kolejnosci, gdzie na poczatku powinno byc menu, a na koncu ekran koncowy.
/// </summary>
void Game::run() {
    al_start_timer(timer); // rozpoczecie timera
    while (powrotStatus)
    {
        powrotStatus = false;
        resetStatus = true;
        if (menu()) // jesli zostal wybrany przycisk GRAJ program przechodzi do gry
        {
            while (resetStatus)
            {
                resetStatus = false;
                if (humanPlaceShips()) // umieszczenie statkow przez gracza
                {
                    computerPlaceShips(); // umieszczenie statkow przez komputer
                    update(); // glowna petla gry
                    if (gameOver)
                    {
                        gameOverScreen();
                        gameOver = false;
                    }
                }
            }
        }
    }
}

/// <summary>
/// Stworzenie gry.
/// 
/// Tworzy obiekt klasy Game, dzieki ktoremu mozna uruchomic gre.
/// </summary>
int main(void) {
    srand(time(NULL)); // ustawienie ziarna losowania
    Game game; // utworzenie obiektu gry
    game.run(); // uruchomienie gry

    return 0;
}