#include <iostream>
#include <fstream>
#include <Windows.h>
#include <conio.h>
#include <random>
#include <time.h>

#define U 72
#define D 80
#define L 75
#define R 77

#define RED 12
#define YELLOW 14
#define VIOLET 13
#define GREEN 10
#define BLUE 9
#define CYAN 11
#define WHITE 15

typedef long long i64;
typedef unsigned long long u64;

using namespace std;

const string dataFile = "data.acerixx";

const int tick = 10;

void color(int color)
{
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, color);
}

const string car[7] = { "/^^\\",
                        "|  |",
                        "|/\\|",
                        "||||",
                        "||||",
                        "|\\/|",
                        "|__|" };

void xy(int x, int y) {
    COORD c;
    c.X = x;
    c.Y = y;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), c);
}

struct dimensions {
    const int x, y;
};

struct cursor {
    int x, y;
    void show(bool showFlag) {
        HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);
        CONSOLE_CURSOR_INFO cursorInfo;
        GetConsoleCursorInfo(out, &cursorInfo);
        cursorInfo.bVisible = showFlag;
        SetConsoleCursorInfo(out, &cursorInfo);
    }
};

struct obstacle {
    bool origin;
    bool toBeDeleted = false;
    int id;
    int lane;
    int dist;
    obstacle* next;
};

struct player {
    int lane;
    int lanes;
    const int dist = 35;
    player(int lane, int lanes) : lane(lane), lanes(lanes) {}
    void changeLane(char dir) {
        for (int i = 0; i < 7; i++) {
            color(GREEN);
            xy(lane * lanes + 2, dist + i);
            cout << "    ";
        }
        if (dir == R && lane < lanes - 2) {
            lane++;
        }
        if (dir == L && lane > 0) {
            lane--;
        }
    }
};

class screen {
private:
    dimensions Dimensions;
public:
    cursor Cursor;
    screen(int scr_x, int scr_y) : Dimensions{ scr_x, scr_y } {
        clear();
    }
    void clear(int x1, int y1, int x2, int y2) {
        for (int y = y1; y <= y2; y++) {
            for (int x = x1; x <= x2; x++) {
                xy(x, y);
                cout << " ";
            }
        }
        xy(0, 0);
    }
    void clear() {
        for (int y = 0; y <= Dimensions.y; y++) {
            for (int x = 0; x <= Dimensions.x; x++) {
                xy(x, y);
                cout << " ";
            }
        }
        xy(0, 0);
    }
};

class map {
private:
    int lanes = 7;
    dimensions Dimensions;

    obstacle* origin = new obstacle{ true, false, 0, 0, 0, NULL };
    obstacle* lastObstacle() {
        obstacle* iter = origin;
        while (iter->next != NULL) {
            iter = iter->next;
        }
        return iter;
    }

    player* Player;

    template<typename F>
    obstacle* allObstacles(F f) {
        if (origin->next != NULL && origin->next->toBeDeleted) {
            obstacle* temp = origin->next;
            origin->next = temp->next;
            delete temp;
            points++;
        }
        obstacle* iter = origin;
        while (iter->next != NULL) {
            iter = iter->next;
            f(iter);
        }
        return iter;
    }
    void drawMap() {
        for (int i = 6; i < Dimensions.y; i++) {
            color(WHITE);
            xy(0, i);
            cout << "|";
            xy(Dimensions.x - 1, i);
            cout << "|";
        }
        xy(Dimensions.x, Dimensions.y);
        for (int i = 0; i < Dimensions.x; i++) {
            color(VIOLET);
            xy(i, 44);
            cout << "=";
            xy(i, 48);
            cout << "=";
        }
    }
    void updateMap() {
        for (int i = 1; i < lanes - 1; i++) {
            for (int j = 6; j < 43; j++) {
                color(WHITE);
                xy(lanes * i, j);
                if ((j - (time * speed / 100000 - 40)) % 14 < lanes) {
                    cout << "|";
                }
                else {
                    cout << " ";
                }
            }
        }
        for (int i = 0; i < 7; i++) {
            color(GREEN);
            xy(Player->lane * lanes + 2, Player->dist + i);
            cout << car[i];
        }
        allObstacles( [=] (obstacle* current) {
            color(RED);
            if (current->dist / 100 < Dimensions.y) {
                for (int i = 0; i < 7; i++) {
                    xy(current->lane * lanes + 2, current->dist / 100 + i);
                    if (current->dist / 100 + i < Dimensions.y && current->dist / 100 + i > 5)
                        cout << car[i];
                }
                for (int i = current->dist / 100 - 1; i >= 0; i--) {
                    xy(current->lane * lanes + 2, i);
                    cout << "    ";
                }
            }
            else {
                for (int i = current->dist / 100 - 1; i >= 0; i--) {
                    xy(current->lane * lanes + 2, i);
                    cout << "    ";
                }
                current->toBeDeleted = true;
            }
            current->dist += speed * tick / 1000;

            if (current->lane == Player->lane && current->dist / 100 >= Player->dist - 6)
                lost = true;
        });
        color(GREEN);
        xy(1, 46);
        cout << "Pkt.: " << points;
        color(CYAN);
        xy(10, 46);
        cout << " Prd.: " << speed / 100 << " km/h";
        color(YELLOW);
        xy(26, 46);
        cout << "HIGHWAY DISORDER";
    }
public:
    uint16_t points = 0;
    bool lost = false;

    int speed = 5000;
    u64 time = 0;

    void newObstacle(int lane) {
        obstacle* iter = origin;
        while (iter->next != NULL) {
            iter = iter->next;
        }
        iter->next = new obstacle{ false, false, iter->id + 1, lane, 0, NULL };
    }

    map(screen* Screen, player* Player, int scr_x, int scr_y) : Dimensions{ scr_x, scr_y }, Player(Player) {
        Screen->clear();
        drawMap();
    }
    void newFrame(int delay) {
        time += delay;
        speed += 10;
        updateMap();
        if (time % (200) == 0)
            newObstacle(rand() % 6);
        Sleep(delay);
    }
};

int main() {
    ifstream inp(dataFile, ios::in | ios::binary);
    srand(time(NULL));
    bool quit = false;
    while (!quit) {
        char choice;

        const int window_x = 43, window_y = 43;

        screen Screen(window_x, window_y);
        Screen.Cursor.show(false);

        color(YELLOW);
        xy(1, 10);
        cout << "Przed rozpoczeciem, zmien rozmiar okna do";
        xy(1, 12);
        cout << (int)window_x << "x" << (int)window_y + 6 << ", czcionke na rastrowa 8x8 i wcisnij";
        xy(4, 14);
        cout << "dowolny klawisz. Steruj strzalkami.";

        while (!_kbhit());

        player Player(2, 7);

        map Map(&Screen, &Player, window_x, window_y);

        while (!Map.lost) {
            Map.newFrame(tick);
            if (_kbhit()) {
                Player.changeLane(_getch());
            }
        }
        char topScore[2];
        inp.read(topScore, 2);
        int topScoreNum = 256 * (int)topScore[0] + topScore[1];
        if (topScoreNum < Map.points) {
            topScoreNum = Map.points;
            topScore[0] = topScoreNum / 256;
            topScore[1] = topScoreNum % 256;
            ofstream out(dataFile, ios::out | ios::binary | ios::trunc);
            out.write(topScore, 2);
        }
        color(RED);
        system("cls");
        xy(10, 10);
        cout << "PRZEGRANA!";
        xy(10, 12);
        color(GREEN);
        cout << "Zdobyte punkty: " << Map.points;
        xy(10, 14);
        color(CYAN);
        cout << "Rekord: " << (topScore[0] * 256) + (topScore[1]);
        xy(10, 16);
        color(WHITE);
        cout << "Q - wyjscie";
        xy(10, 17);
        cout << "R - ponowna gra";
        while (!_kbhit());
        switch (_getch()) {
            case 'q':
                quit = true;
                break;
            default:
                system("cls");
        }
    }
    system("exit");
}