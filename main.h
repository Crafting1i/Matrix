#pragma once

#include <map>
#include <string>
#include <Windows.h>
#include <vector>
#include <random>

std::vector<std::wstring> logo = {
L"           #           ",
L"          ###          ",
L"         #####         ",
L"        #######        ",
L"       ###  ####       ",
L"      ###    ####      ",
L"     ###      ####     ",
L"    ###        ####    ",
L"   ###          ####   ",
L"  AMESU SOFTWARE ####  ",
L" ##################### ",
L"#######################"
};

const std::vector<TCHAR> possible_symbols = {
    L'ア', L'イ', L'エ', L'オ', L'ガ', L'カ', L'キ', L'ギ', L'ク', L'グ', L'ケ', L'ゲ', L'コ', L'ゴ', L'サ', L'ザ',
    L'シ', L'ジ', L'ス', L'ズ', L'セ', L'ゼ', L'ソ', L'ゾ', L'タ', L'ダ', L'チ', L'ヂ', L'ツ', L'ヅ', L'テ', L'デ',
    L'ト', L'ド', L'ナ', L'ニ', L'ヌ', L'ネ', L'ノ', L'ハ', L'バ', L'パ', L'ヒ', L'ビ', L'ピ', L'フ', L'ブ', L'プ',
    L'ヘ', L'ベ', L'ペ', L'ホ', L'ボ', L'ポ', L'マ', L'ミ', L'ム', L'メ', L'モ', L'ヤ', L'ユ', L'ヨ', L'ラ', L'リ',
    L'ル', L'レ', L'ロ', L'ワ', L'ヰ', L'ヱ', L'ヲ', L'ン', L'ヴ', L'ヷ', L'ヸ', L'ヹ', L'ヺ'
};

typedef std::map<char, std::string> chars_map_t;
using dist6_t = std::uniform_int_distribution<unsigned int>;
std::random_device dev;
std::mt19937 rng(dev());

struct pixel {
public:
    TCHAR symbol = 0;
    bool is_const = false;

    explicit pixel(TCHAR symbol)
        : symbol(symbol) {}

    TCHAR operator=(TCHAR c) {
        return this->symbol = c;
    }
};

class row {
    std::vector<pixel> pixels;
    int columns;
public:
    explicit row(int c): columns(c) {
        for (int i = 0; i < c; i += 1) {
            pixel p(' ');
            this->pixels.push_back(p);
        }
    };

    pixel* operator[](int index) {
        return &this->pixels[index];
    }
};

class field {
    std::vector<row> buffer;
    int columns, rows;
public:
    field(int c, int r) : columns(c), rows(r) {
        for (int i = 0; i < r; i += 1) buffer.push_back(row(c));
    }

    const TCHAR* get_buffer() {
        size_t size = (size_t)this->columns * this->rows;
        TCHAR* buf = (TCHAR*)calloc(size, sizeof(TCHAR));

        for (int row = 0; row < this->rows; row += 1) {
            for (int column = 0; column < this->columns; column += 1) {
                buf[row * this->columns + column] = this->buffer[this->rows - 1 - row][column]->symbol;
            }
        }

        return buf;
    }

    void set_pixel(int x, int y, TCHAR symbol) {
        if (x >= columns) throw std::out_of_range("x out of range");
        if (y >= rows) throw std::out_of_range("y out of range");

        *this->buffer[y][x] = symbol;
    }
    pixel* get_pixel(int x, int y) {
        if (x >= columns) throw std::out_of_range("x out of range");
        if (y >= rows) throw std::out_of_range("y out of range");

        return this->buffer[y][x];
    }
    
    COORD get_size() {
        COORD size;
        size.X = this->columns;
        size.Y = this->rows;

        return size;
    }

    row* operator[](int index) {
        return &this->buffer[index];
    }
};

class snake {
    field* f;
public:
    pixel* body;
    int length;
    COORD head_pos;

    snake(field* f):
        snake(
            f, dist6_t(0, f->get_size().X - 1)(rng) // last param is random x pos
        ) {};
    snake(field* f, int x) {
        dist6_t dist6_len(5, 8);
        this->length = dist6_len(rng); // random length (from 5 to 8 points);

        this->f = f;
        COORD fsize = f->get_size();

        if (x >= fsize.X) throw std::out_of_range("x out of range");
        this->head_pos.X = x;
        this->head_pos.Y = fsize.Y - 1;

        this->body = (pixel*)calloc(length, sizeof(pixel));
        for (int i = 0; i < length; i += 1) {
            this->body[i] = pixel('@');
        }
    }

    void step() {
        COORD screen_size = this->f->get_size();
        if (screen_size.X <= head_pos.X || head_pos.X < 0) throw std::runtime_error("snake in out of screen");

        std::uniform_int_distribution<size_t> dist6(0, possible_symbols.size() - 1);

        int y;
        for (int i = 0; i < this->length; i += 1) {
            pixel p = this->body[i];
            
            p.symbol = possible_symbols[dist6(rng)];
            y = head_pos.Y + i;

            if(screen_size.Y > y && y >= 0 && !f->get_pixel(head_pos.X, y)->is_const)
                this->f->set_pixel(head_pos.X, y, p.symbol);
        }
        
        y = head_pos.Y + this->length;
        if (screen_size.Y > y && y >= 0 && !f->get_pixel(head_pos.X, y)->is_const)
            this->f->set_pixel(head_pos.X, head_pos.Y + this->length, ' ');
        if (y == 0); // That's end....
        
        this->head_pos.Y -= 1;
    }

    bool operator==(snake s) {
        return this->head_pos.X == s.head_pos.X && this->head_pos.Y == s.head_pos.Y
            && this->length == s.length;
    }
};

class snake_spawner {
    snake* snakes;
    int count = 0;
    field* f;
public:
    snake_spawner(field* f): f(f) {
        spawn();
    }

    void spawn() {
        int tmp_count = this->count;
        snake* tmp = this->snakes;
        
        this->count += 1;
        this->snakes = (snake*)calloc(this->count, sizeof(snake));
        
        for (int i = 0; i < tmp_count; i += 1) {
            this->snakes[i] = tmp[i];
        }
        delete[] tmp;

        this->snakes[tmp_count] = snake(f);
    }

    void step() {
        COORD size = f->get_size();

        std::vector<snake> snakes_to_delete;
        for (int i = 0; i < this->count; i += 1) {
            this->snakes[i].step();
        }

        this->spawn();
        //double spawn?
        //+ spawn rate
        if (dist6_t(0, 1)(rng) == 1) this->spawn();
    }
};
