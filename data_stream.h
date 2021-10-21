#ifndef DATA_STREAM_H
#define DATA_STREAM_H

#include <iostream>

/*
 * класс навигации по данным todo: организовать через std::ostream
 */
class data_stream
{
    const char* _data_ptr = nullptr;//скользящий указатель на текущую позицию
    int _line = -1;                 //строка
    int _pos = -1;                  //символ в строке
public:
    data_stream() = default;

    data_stream(const char* data);

    data_stream(const data_stream& other);

    //проверяет что класс инициализирован
    operator bool() const;

    //выбирает символ и двигает указатель на следующий
    char operator++();

    //выбирает символ оставляя указатель где был
    char operator*() const;

    //показывает какой из инстансов указывает дальше todo: предусмотреть ситуацию сравнения разных строк
    bool operator>(const data_stream& o) const;

    /* обеспечивает объединение указателей на поток из разных парсеров
     * если один из двух невалидный, возвращает валидный
     * иначе выбирает тот что указывает дальше если оба парсера успешны (priority = 0)
     * либо тот, что указывает ближе, если хотя бы один из парсеров не успешен (priority = -1)
     */
    static data_stream combine(const data_stream& data1, const data_stream& data2, int priority);

    /* выбирает поток при ошибке (тот что валиден либо указывает ближе)
     * 1 - выбрать первый
     * 2 - выбрать второй
     * 0 - нет валидных потоков
     */
    static int validest(const data_stream& data1, const data_stream& data2);

    //выводит текущую позицию и символ - для сообщений об ошибке
    friend std::ostream& operator<<(std::ostream& o, const data_stream& data);
};

struct parsing_fault_data
{
    bool _valid = false;
    std::string _what {"null object"};
    data_stream _fault_point;

    parsing_fault_data() = default;
    explicit parsing_fault_data(bool x) : _valid(true), _what() {}
    parsing_fault_data(std::string what, data_stream& fault_poing) : _what(std::move(what)), _fault_point(fault_poing),
                                                                     _valid(false) { }
    friend std::ostream& operator<<(std::ostream& o, const parsing_fault_data& data);
};

/*
 * класс состояния парсера
 */
class parsing_state
{
public:
    parsing_fault_data _pfd;
    data_stream _ds;

    parsing_state(const data_stream& ds, const parsing_fault_data &pfd);

    bool is_valid() const;

    bool is_EOF() const;

    //объединяет два состояния с приоритетом полученного состояния
    static parsing_state join(const parsing_state& initial, const parsing_state& result);

    //объединяет два состония требуя чтобы оба были валидными
    static parsing_state both(const parsing_state& ps1, const parsing_state& ps2);

    void fault(const std::string& why, bool ass_ds);

    friend std::ostream& operator<<(std::ostream& o, const parsing_state& ps);
};

#endif /***DATASTREAM_H***/