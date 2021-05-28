#include "dbms.h"

DBMS::DBMS(string dbFolder, bool isInit)
{
    // Заполнение полей с данными о структуре базы(структура известна заранее)
    dbTableStruct[TABLE_CARS] = 0x80;
    dbTableStruct[TABLE_MANAGERS] = 0x38;
    dbTableStruct[TABLE_SALES] = 0x30;
    dbTableStruct[TABLE_CLIENTS] = 0x1E0;

    // Заполнение полей с данными о количестве столбцов в каждой из таблиц
    dbTableSize[TABLE_CARS] = 8;
    dbTableSize[TABLE_MANAGERS] = 6;
    dbTableSize[TABLE_SALES] = 6;
    dbTableSize[TABLE_CLIENTS] = 9;

    // Сохранение пути к базе
    dbPath = dbFolder;

    // Считывание БД
    if (isInit)
        readDB();
}

DBMS::~DBMS()
{
    saveDB();
}

void DBMS::readDB()
{
    string str;
    rowData *currData;

    // Открытие БД по адресу dbPath. В конце добавляется имя файла таблицы(прописаны в configurations.h)
    treader[TABLE_CARS].open(dbPath + TABLE_CARS_SPATH, ios::in);
    treader[TABLE_MANAGERS].open(dbPath + TABLE_MANAGERS_SPATH, ios::in);
    treader[TABLE_SALES].open(dbPath + TABLE_SALES_SPATH, ios::in);
    treader[TABLE_CLIENTS].open(dbPath + TABLE_CLIENTS_SPATH, ios::in);

    // Цикл по каждой из таблиц
    for (int tid = 0; tid < NUM_OF_TABLES; tid++) {
        tdata[tid].clear();

        // Чтение полей до тех пор пока не дойдем до конца файла
        while (!(treader[tid].peek() == EOF)) {
            currData = new rowData();   // Создание нового поля с данными
            currData->tableID = tid;    // Прописывание информации о принадлежности поля к конкретной таблице

            // Считывание количества колонок, определенного по структуре
            for (int col = dbTableSize[tid] - 1; col >= 0; col--) {
                getline(treader[tid], str);     // Считываение из текущего файла таблицы

                // Опираясь на данные в битовой маске считываем либо Int, либо String (если в маске 1 - Int, иначе String)
                if ((dbTableStruct[tid] >> col) & 0x01) {
                    // Проверка на содержание в строке данны для предотвращения ошибки исполнения stoi
                    if (str.empty())
                        currData->ints.push_back(0);    // push_back <- добавление в конец структуры данных
                    else
                        currData->ints.push_back(stoi(str));    // stoi <- функция преобразования string в int(из файла всегда читается string)
                }
                else
                    currData->strings.push_back(str);
            }

            tdata[tid].push_back(currData); // Добавление заполненой структуры данных в список полей(строк таблицы)
        }
    }

    // Закрытие всех таблиц для чтения(потом откроем для записи)
    for (int tid = 0; tid < NUM_OF_TABLES; tid++)
        treader[tid].close();
}

void DBMS::saveDB()
{
    // Открытие всех таблиц БД для записи (от начала - ios::out)
    treader[TABLE_CARS].open(dbPath + TABLE_CARS_SPATH, ios::out);
    treader[TABLE_MANAGERS].open(dbPath + TABLE_MANAGERS_SPATH, ios::out);
    treader[TABLE_SALES].open(dbPath + TABLE_SALES_SPATH, ios::out);
    treader[TABLE_CLIENTS].open(dbPath + TABLE_CLIENTS_SPATH, ios::out);

    // Поочередная запись данных из каждой таблицы в соответствующий файл
    for (int tid = 0; tid < NUM_OF_TABLES; tid++)
        // Поочередное занесение каждой строки таблицы в файл
        for (rowData *currData : tdata[tid]) {
            for (int i : currData->ints)        // Сначала запись всех числовых полей
                treader[tid] << i << '\n';
            for (string s : currData->strings)  // Затем запись всех текстовых полей
                treader[tid] << s << '\n';
        }

    // Закрытие всех файлов для сохранения записаных в них данных
    for (int tid = 0; tid < NUM_OF_TABLES; tid++)
        treader[tid].close();
}

void DBMS::ADD(rowData data)
{
    // Создание нового поля и заполнение его переданными данными
    rowData *tmp = new rowData;
    tmp->tableID = data.tableID;
    tmp->ints = data.ints;
    tmp->strings = data.strings;

    tdata[data.tableID].push_back(tmp); // Добавление поля в таблицу
}

// Функция поиска без маски
vector<rowData *> DBMS::GET(rowData toGet)
{
    vector<rowData *> res;

    // В табилце SALES поиск происходит по числовому полю
    if (toGet.tableID != TABLE_SALES) {
        for (rowData *currData : tdata[toGet.tableID])  // Для каждого элемента таблицы
            if ((currData->strings.size() > 0) && (currData->strings[0] == toGet.strings[0]))   // Сравниваем значение в поле с искомым
                res.push_back(currData);    // Если значение совпадает, то добавляем поле в выдачу
    }
    // В остальных таблицах поиск происходит по текстовому полю
    else
        for (rowData *currData : tdata[toGet.tableID])  // Для каждого элемента таблицы
            if ((currData->ints.size() > 0) && (currData->ints[0] == toGet.ints[0]))    // Сравниваем значение в поле с искомым
                res.push_back(currData);    // Если значение совпадает, то добавляем поле в выдачу

    return res; // Возвращаем выдачу
}

vector<rowData *> DBMS::GET(rowData toGet, short mask)
{
    vector<rowData *> res;
    bool fits;

    short vPosInt;
    short vPosString;

    short intsOffset;
    short stringsOffset;

    if (!mask)  // Если не задана маска, выполняем полную выдачу таблицы (mask == 0x00)
        return tdata[toGet.tableID];

    if (!tdata[toGet.tableID].size())   // Если текущая таблица пуста, возвращаем пустую выдачу(во избажании ошибок)
        return res;

    intsOffset = dbTableSize[toGet.tableID] - 1;                        // Смещение int в маске
    stringsOffset = intsOffset - tdata[toGet.tableID][0]->ints.size();  // Смещение string в маске

    // Цикл по каждому полю(строке) в таблице
    for (rowData *currData : tdata[toGet.tableID]) {
        vPosInt = 0;
        vPosString = 0;
        fits = true;

        // Цикл по каждой колонке в строке
        for (int col = dbTableSize[toGet.tableID] - 1; col >= 0; col--) {
            if (!((mask >> col) & 0x01))    // Если значение текущей колонки не сравнивается для выдачи - пропускаем шаг
                continue;

            // Если текущая колонка содержит в себе int
            if ((dbTableStruct[toGet.tableID] >> col) & 0x01) {
                if (currData->ints[intsOffset - col] != toGet.ints[vPosInt]) { // Если int не подходит под сравнение, то текущее поле нам не подходит
                    fits = false;   // Выставляем флаг того что текущее поле нам не подходит
                    break;          // Пропускаем сравнение всех остальных колонок в данном поле
                }

                vPosInt++;          // Увеличиваем указатель позиции среди intов
            }
            else {
                if (currData->strings[stringsOffset - col] != toGet.strings[vPosString]) {  // Если string не подходит под сравнение, то текущее поле нам не подходит
                    fits = false;   // Выставляем флаг того что текущее поле нам не подходит
                    break;          // Пропускаем сравнение всех остальных колонок в данном поле
                }

                vPosString++;       // Увеличиваем указатель позиции среди stringов
            }
        }

        if (fits)   // Если текущее поле прошло все проверки, добавляем его в выборку
            res.push_back(currData);
    }

    return res;
}

bool DBMS::REMOVE(rowData data)
{
    int startSize;

    startSize = tdata[data.tableID].size();

    // В табилце SALES поиск происходит по числовому полю
    if (data.tableID != TABLE_SALES) {
        for (int jnx = 0; jnx < tdata[data.tableID].size(); jnx++)  // Для каждого элемента таблицы
            if (tdata[data.tableID][jnx]->strings[0] == data.strings[0])    // Сравниваем значение в поле с искомым
                tdata[data.tableID].erase(tdata[data.tableID].begin() + jnx);   // Если значение совпадает, то удаляем поле
    }
    // В остальных таблицах поиск происходит по текстовому полю
    else
        for (int jnx = 0; jnx < tdata[data.tableID].size(); jnx++)  // Для каждого элемента таблицы
            if (tdata[data.tableID][jnx]->ints[0] == data.ints[0])  // Сравниваем значение в поле с искомым
                tdata[data.tableID].erase(tdata[data.tableID].begin() + jnx);   // Если значение совпадает, то удаляем поле

    return startSize == tdata[data.tableID].size(); // Возвращаем true если было произведено хоть одно удаление
}
