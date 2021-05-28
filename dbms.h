#pragma once
#include <iostream>
#include <fstream>
#include "constants.h"
#include <string>
#include <vector>

using namespace std;

class DBMS
{
private:
    void readDB();  // Функция считвания базы данных
    void saveDB();  // Функция записи базы данных

    int dbTableStruct[NUM_OF_TABLES];   // Битовое поле с информацией о структуре таблицы базы данных
    short dbTableSize[NUM_OF_TABLES];   // Поле с информацией о количестве колонок в таблице базы данных

    fstream treader[NUM_OF_TABLES];         // stream для чтения и записи
    vector<rowData*> tdata[NUM_OF_TABLES];  // Структура, хранящая данные из таблицы для их обработки во время работы программы   

public:
    string dbPath;  // Путь к базе данных

    DBMS(string dbFolder, bool isInit); // Конструктор
    ~DBMS();    // Деструктор

    vector<rowData *> GET(rowData toGet);               // Поиск по таблице по определяющим полям
    vector<rowData *> GET(rowData toGet, short mask);   // Поиск по таблице по нескольким полям

    void ADD(rowData data);     // Добавление поля в таблицу
    bool REMOVE(rowData data);  // Удаление поля из таблицы
};
