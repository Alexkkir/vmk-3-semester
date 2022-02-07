#include <list>
#include <iostream>
#include <cstdlib>

using namespace std;

#define LIMIT 1000 // время окончания моделирования
#define N_SERVERS 2 // количество серверов

// Очень простой пример построения имитационной модели с календарём событий
// Модель "самодостаточная" - не используются библиотеки для организации имитационного моделирования

// Возможности С++ используются недостаточно. Что можно улучшить в этой части?

class Event  // событие в календаре
{
public:
    float time; // время свершения события
    enum Type {
        EV_INIT, EV_REQ, EV_FIN
    } type;   // тип события
    int attr; // дополнительные сведения о событии в зависимости от типа
    int id_server = -1;

    Event(float t, Event::Type tt, int a) {
        time = t;
        type = tt;
        attr = a;
    }

    Event(float t, Event::Type tt, int a, int i) {
        time = t;
        type = tt;
        attr = a;
        id_server = i;
    }

    Event() {
        time = 0;
        type = EV_INIT;
        attr = 0;
    }
};
// типы событий
//#define Event::EV_INIT 1
//#define Event::EV_REQ 2

//#define Event::EV_FIN 3
// состояния
//#define RUN 1

//#define IDLE 0

class Request // задание в очереди
{
public:
    float time;     // время выполнения задания без прерываний
    int source_num; // номер источника заданий (1 или 2)
    Request(float t, int s) {
        time = t;
        source_num = s;
    }
};

class Calendar : public list<Event *> // календарь событий
{
public:
    void put(Event *ev); // вставить событие в список с упорядочением по полю time
    Event *get(); // извлечь первое событие из календаря (с наименьшим модельным временем)
};

void Calendar::put(Event *ev) {
    list<Event *>::iterator i;
    Event **e = new (Event *);
    *e = ev;
    if (empty()) {
        push_back(*e);
        return;
    }
    i = begin();
    while ((i != end()) && ((*i)->time <= ev->time)) {
        ++i;
    }
    insert(i, *e);
}

Event *Calendar::get() {
    if (empty()) return NULL;
    Event *e = front();
    pop_front();
    return e;
}


typedef list<Request *> Queue; // очередь заданий к процессору

float get_req_time(int source_num); // длительность задания
float get_pause_time(int source_num); // длительность паузы между заданиями

class Server {
public:
    enum CpuState {IDLE, RUN} cpu_state = IDLE;
    float run_begin;
    Queue queue;
    int id;

    void process_request(Event *curr_ev, float curr_time, float dt, Calendar &calendar) {
        if (cpu_state == IDLE) {
            cpu_state = RUN;
            calendar.put(new Event(curr_time + dt, Event::EV_FIN, curr_ev->attr, id));
            run_begin = curr_time;
        } else
            queue.push_back(new Request(dt, curr_ev->attr));
    }

    float stop_curr(float curr_time, Calendar &calendar) {
        cpu_state = IDLE;
        float run_begin_save = run_begin;
        if (!queue.empty()) {
            Request *rq = queue.front();
            queue.pop_front();
            calendar.put(new Event(curr_time + rq->time, Event::EV_FIN, rq->source_num, id));
            delete rq;
            run_begin = curr_time;
        }
        return run_begin_save;
    }
};

class ServerSystem {
public:
    static const int n_servers = N_SERVERS;
    Server server[n_servers];
    int next_to_use = 0;

    ServerSystem() {
        for (int i = 0; i < n_servers; i++) {
            server[i].id = i;
        }
    }

    void process_event(Event *curr_ev, float curr_time, float dt, Calendar &calendar) {
        curr_ev->id_server = next_to_use;
        server[next_to_use].process_request(curr_ev, curr_time, dt, calendar);
        calendar.put(new Event(curr_time + get_pause_time(curr_ev->attr), Event::EV_REQ, curr_ev->attr));
        next_to_use += 1;
        next_to_use %= n_servers;
    }

    float finish_processing(Event *ev, float curr_time, Calendar &calendar) {
        return server[ev->id_server].stop_curr(curr_time, calendar);
    }
};

int main(int argc, char **argv) {
    ServerSystem serv_sys;
//Server server;
    Calendar calendar;
    float curr_time = 0;
    Event *curr_ev;
    float dt;
    srand(2019);
// начальное событие и инициализация календаря
    curr_ev = new Event(curr_time, Event::EV_INIT, 0);
    calendar.put(curr_ev);
// цикл по событиям

    while ((curr_ev = calendar.get()) != nullptr) {
        cout << "time " << curr_ev->time << " type " << curr_ev->type + 1 << endl;
        curr_time = curr_ev->time; // продвигаем время
        // обработка события
        if (curr_time >= LIMIT) break; // типичное дополнительное условие останова моделирования
        switch (curr_ev->type) {
            case Event::EV_INIT:  // запускаем генераторы запросов
                calendar.put(new Event(curr_time, Event::EV_REQ, 1));
                calendar.put(new Event(curr_time, Event::EV_REQ, 2));
                break;
            case Event::EV_REQ:
                // планируем событие окончания обработки, если процессор свободен, иначе ставим в очередь
                dt = get_req_time(curr_ev->attr);
                cout << "dt " << dt << " num " << curr_ev->attr << endl;

                serv_sys.process_event(curr_ev, curr_time, dt, calendar);
                // планируем событие генерации следующего задания
                break;
            case Event::EV_FIN:
                // объявляем процессор свободным и размещаем задание из очереди, если таковое есть
                // выводим запись о рабочем интервале
                float run_begin = serv_sys.finish_processing(curr_ev, curr_time, calendar);
                cout << "Work with " << run_begin << " for " << curr_time << " duration " << (curr_time - run_begin)
                     << endl;
                break;
        } // switch
        delete curr_ev;
    } // while
} // main

int rc = 0;
int pc = 0;

float get_req_time(int source_num) {
// Для демонстрационных целей - выдаётся случайное значение
// при детализации модели функцию можно доработать
    double r = ((double) rand()) / RAND_MAX;
    cout << "req " << rc << endl;
    rc++;
    if (source_num == 1) return r * 10; else return r * 20;
}

float get_pause_time(int source_num) // длительность паузы между заданиями
{
// см. комментарий выше
    double p = ((double) rand()) / RAND_MAX;
    cout << "pause " << pc << endl;
    pc++;
    if (source_num == 1) return p * 20; else return p * 10;
}
