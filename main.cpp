#include <bits/stdc++.h>

using namespace std;

// check git

// Параметры мира

const int PROGRAM_SIZE = 64;
const int STAY_POINT = PROGRAM_SIZE / 2;
const int INF = 1e9;
const int DISPLAY = 1, SKIP = 0;

int born_energy, max_energy, initial_number, world_height, world_width, cnt_of_operations, lab, experiment_time;
string delay;
int display_mode = DISPLAY;

vector<int> init_program;

int random_int();
bool try_move_cell(int64_t);
void add_background(int, int, int, int);
void add_cell(int, int);

struct Cell {
    int current_energy, program_pointer = 0, data_pointer = 0;
    int directX, directY;
    int64_t my_id;
    vector<int> program;
    vector<int> data;
    Cell() {
        random_turn();
        current_energy = born_energy;
        program.resize(PROGRAM_SIZE / 2, STAY_POINT);
        data.resize(PROGRAM_SIZE / 4, 0);
    }
    void random_turn() {
        directX = (random_int() % 3) - 1;
        directY = (random_int() % 3) - 1;
    }
};

struct Item {
    string type;
    int64_t id;
    Item() {}
    Item(string t, int64_t i) {
        type = t;
        id = i;
    }
};

struct Background {
    int inner_energy, hardness;
    Background() {}
    Background(int e, int h) {
        hardness = h;
        inner_energy = e;
    }
};

const int MAX_N = 1000;

string display[MAX_N][MAX_N];

void show_display(int h, int w) {
    system("clear");
    for (int i = 0; i < h; ++i) {
        for (int j = 0; j < w; ++j) {
            cout << display[i][j];
        }
        cout << endl;
    }
    const char * cur_delay = ("sleep " + delay).c_str();
    system(cur_delay);
}

int random_int() {
    return rand();
}

void input() {
    cout << "Энергия бота при рождении:\n";
    cin >> born_energy;
    cout << "Максимальная энергия бота:\n";
    cin >> max_energy;
    cout << "Количество ботов\n";
    cin >> initial_number;
    cout << "Высота мира:\n";
    cin >> world_height;
    cout << "Ширина мира:\n";
    cin >> world_width;
    cout << "Скорость течения времени:\n";
    cin >> delay;
    cout << "Количество операций за такт:\n";
    cin >> cnt_of_operations;
    cout << "Программа для первой партии (сначала длина затем код):\n";
    int len;
    cin >> len;
    init_program.resize(len);
    for (auto & x : init_program) {
        cin >> x;
    }
    cout << "Включить лабораторный режим (0/1)?\n";
    cin >> lab;
}

int64_t counter = 0;

Item world[MAX_N][MAX_N];
map<int64_t, pair<int, int> > coords;
map<int, Background> background_pool;
map<int, Cell> cell_pool;

bool try_move_cell(int64_t id) {
    Cell cur = cell_pool[id];
    pair<int, int> my_coord = coords[id];
    pair<int, int> want_coord = {my_coord.first + cur.directY, my_coord.second + cur.directX};
    Item want_item = world[want_coord.first][want_coord.second];
    if (want_item.type != "background") {
        return false;
    }
    Background & want_back = background_pool[want_item.id];
    if (want_back.hardness) {
        return false;
    }
    swap(world[my_coord.first][my_coord.second], world[want_coord.first][want_coord.second]);
    swap(coords[id], coords[want_item.id]);
    return true;
}

void try_eat_back(int64_t id) {
    Cell & cur = cell_pool[id];
    pair<int, int> my_coord = coords[id];
    pair<int, int> want_coord = {my_coord.first + cur.directY, my_coord.second + cur.directX};
    Item want_item = world[want_coord.first][want_coord.second];
    if (want_item.type != "background") {
        return;
    }
    Background & want_back = background_pool[want_item.id];
    int need = max_energy - cur.current_energy;
    if (need <= want_back.inner_energy) {
        cur.current_energy = need;
        want_back.inner_energy -= need;
    } else {
        cur.current_energy += want_back.inner_energy;
        want_back.inner_energy = 0;
    }
}

void eat(int64_t hunter_id, int64_t victim_id) {
    Cell & hunter = cell_pool[hunter_id];
    Cell & victim = cell_pool[victim_id];
    hunter.current_energy += victim.current_energy;
    add_background(coords[victim_id].first, coords[victim_id].second, 0, 0);
    cell_pool.erase(victim_id);
}

void try_eat_cell(int64_t id) {
    Cell & cur = cell_pool[id];
    pair<int, int> my_coord = coords[id];
    pair<int, int> want_coord = {my_coord.first + cur.directY, my_coord.second + cur.directX};
    Item want_item = world[want_coord.first][want_coord.second];
    if (want_item.type != "cell") {
        return;
    }
    Cell & want_cell = cell_pool[want_item.id];
    if (cur.current_energy == want_cell.current_energy) {
        if (random_int() % 2 == 0) {
            eat(id, want_item.id);
        }
        return;
    }
    if (cur.current_energy > want_cell.current_energy) {
        eat(id, want_item.id);
        return;
    }
}

void make_copy(int64_t id) {
    Cell & cur = cell_pool[id];
    if (cur.current_energy <= 0) {
        return;
    }
    if ((random_int() % (cur.current_energy + 1)) < born_energy && random_int() % 2 == 0) {
        return;
    }
    pair<int, int> my_coord = coords[id];
    pair<int, int> want_coord = {my_coord.first + cur.directY, my_coord.second + cur.directX};
    Item want_item = world[want_coord.first][want_coord.second];
    if (want_item.type != "background") {
        return;
    }
    Background & want_back = background_pool[want_item.id];
    if (want_back.hardness != 0) {
        return;
    }
    background_pool.erase(want_item.id);
    add_cell(want_coord.first, want_coord.second);
    Cell & child = cell_pool[world[want_coord.first][want_coord.second].id];
    for (int i = 0; i < PROGRAM_SIZE / 2; ++i) {
        if ((random_int() % max_energy) >= cur.current_energy) {
            if (random_int() % 3 == 0) {
                child.program[i] = random_int() % PROGRAM_SIZE;
            } else {
                child.program[i] = cur.program[i];
            }
        } else {
            child.program[i] = cur.program[i];
        }
    }
    child.current_energy = cur.current_energy / 2;
    cur.current_energy -= child.current_energy;
}

void execute_cell(int64_t id, int operations) {
    Cell & cur = cell_pool[id];
    if (cur.current_energy <= 0) {
        if (random_int() % 16 == 0) {
            cell_pool.erase(id);
            add_background(coords[id].first, coords[id].second,  born_energy, 0);
        }
        return;
    }
    --cur.current_energy;
    for (int i = 0; i < operations; ++i) {
        int command = cur.program[cur.program_pointer];
        if (command >= PROGRAM_SIZE / 2) {
            cur.program_pointer += command;
        } else if (command < PROGRAM_SIZE / 4) {
            if (command == 0) {
                cur.program_pointer = 0;
                break;
            } else if (command == 1) {
                cur.directX = -1;
                cur.directY = -1;
            } else if (command == 2) {
                cur.directX = 0;
                cur.directY = -1;
            } else if (command == 3) {
                cur.directX = 1;
                cur.directY = -1;
            } else if (command == 4) {
                cur.directX = 1;
                cur.directY = 0;
            } else if (command == 5) {
                cur.directX = 1;
                cur.directY = 11;
            } else if (command == 6) {
                cur.directX = 0;
                cur.directY = 1;
            } else if (command == 7) {
                cur.directX = -1;
                cur.directY = 1;
            } else if (command == 8) {
                cur.directX = -1;
                cur.directY = 0;
            } else if (command == 10) {
                bool res = try_move_cell(id);
                if (!res) {
                    cur.program_pointer += 1;
                }
                if (cur.directX || cur.directY)
                    --cur.current_energy;
            } else if (command == 9) {
                // cur.random_turn();   
            } else if (command == 11) {
                try_eat_back(id);
            } else if (command == 12) {
                make_copy(id);
            } else if (command == 13) {
                try_eat_cell(id);
            }
        }
        ++cur.program_pointer;
        cur.program_pointer %= STAY_POINT;
    }
}

void send_to_display() {
    for (int i = 0; i < world_height; ++i) {
        for (int j = 0; j < world_width; ++j) {
            string out;
            string type = world[i][j].type;
            if (type == "background") {
                out = ".";
            }
            if (type == "cell") {
                out = "\x1b[31m*\x1b[30m";
            }
            for (int y = 0; y <= 2; ++y) {
                for (int x = 0; x <= 2; ++x) {
                    display[3 * i + y][3 * j + x] = out;
                }
            }
            if (type == "cell") {
                int cy = 3 * i + 1, cx = 3 * j + 1;
                Cell cur = cell_pool[world[i][j].id];
                display[cy + cur.directY][cx + cur.directX] = 'O';
            }
        }
    }
    show_display(world_height * 3, world_width * 3);
}

void add_background(int y, int x, int energy, int hardness) {
    world[y][x] = Item("background", counter++);
    background_pool[world[y][x].id] = Background(energy, hardness);
    coords[world[y][x].id] = {y, x};
}

void add_cell(int y, int x) {
    world[y][x] = Item("cell", counter++);
    cell_pool[world[y][x].id] = Cell();
    coords[world[y][x].id] = {y, x};
}

void write_program_cell_front(int64_t id, const vector<int> & part) {
    Cell & cur = cell_pool[id];
    for (size_t i = 0; i < part.size(); ++i) {
        cur.program[i] = part[i];
    }
}

void add_time() {
    int add_time;
    if (DISPLAY) {
        cout << "Время эксперимента: \n";
    }
    cin >> add_time;
    experiment_time += add_time;
}

void change_display_mode() {
    string mode;
    cin >> mode;
    if (mode == "skip") {
        display_mode = SKIP;    
    }
    if (mode == "display") {
        display_mode = DISPLAY;
    }
}

void dump_bots() {
    for (auto & bot : cell_pool) {
        cout << "id: \t" << bot.first << "\t energy: \t" << bot.second.current_energy << "\tprogram:\t";
        for (int i = 0; i < PROGRAM_SIZE / 2; ++i) {
            if (i == bot.second.program_pointer) {
                cout << "\x1b[31m";
            }
            cout << bot.second.program[i] << ' ';
            cout << "\x1b[30m";
        }
        cout << endl;
    }
}

void run() {
     // Создание мира;
     for (int i = 0; i < world_height; ++i) {
        for (int j = 0; j < world_width; ++j) {
            add_background(i, j, random_int() % (2 * born_energy), 0);
            if (i == 0 || j == 0 || i == world_height - 1 || j == world_width - 1) {
                background_pool[world[i][j].id].hardness = INF;
            }
        }
     }
     // Заселение ботами
     int have_bots = initial_number;
     while (have_bots) {
        int x = 1 + random_int() % (world_width - 2);
        int y = 1 + random_int() % (world_height - 2);
        if (world[y][x].type == "background") {
            --have_bots;
            background_pool.erase(world[y][x].id);
            add_cell(y, x);
            write_program_cell_front(world[y][x].id, init_program);
        }
        send_to_display();
     }
     // Жизнь
     experiment_time = 1;
     for (int i = 0; i < experiment_time; ++i) { 
        if (i + 1 == experiment_time) {
            if (!lab) {
                add_time();
            } else {
                cout << "Done\n";
                string query;
                while (query != "run") {
                    cin >> query;
                    if (query == "mode") {
                        change_display_mode(); 
                    }
                    if (query == "time") {
                        add_time();
                    }
                    if (query == "dump_bots") {
                        dump_bots();
                    }
                    if (query == "show_map") {
                        send_to_display();
                    }
                }
            }
        }
        vector<pair<int, Cell*> > bot_order;
        for (auto & bot : cell_pool) {
            bot_order.push_back({bot.first, &(bot.second)});
        }
        random_shuffle(bot_order.begin(), bot_order.end());
        for (auto & bot : bot_order) {
            execute_cell(bot.first, cnt_of_operations);
        }
        if (display_mode == DISPLAY) {
            send_to_display(); 
        }
     }
}

int main() {
    // ios_base::sync_with_stdio(false);
    cin.tie(0);
    cout.tie(0);
    input();
    run();
}
