#include "dbms.h"

DBMS::DBMS(string dbFolder, bool isInit)
{
    // ���������� ����� � ������� � ��������� ����(��������� �������� �������)
    dbTableStruct[TABLE_CARS] = 0x80;
    dbTableStruct[TABLE_MANAGERS] = 0x38;
    dbTableStruct[TABLE_SALES] = 0x30;
    dbTableStruct[TABLE_CLIENTS] = 0x1E0;

    // ���������� ����� � ������� � ���������� �������� � ������ �� ������
    dbTableSize[TABLE_CARS] = 8;
    dbTableSize[TABLE_MANAGERS] = 6;
    dbTableSize[TABLE_SALES] = 6;
    dbTableSize[TABLE_CLIENTS] = 9;

    // ���������� ���� � ����
    dbPath = dbFolder;

    // ���������� ��
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

    // �������� �� �� ������ dbPath. � ����� ����������� ��� ����� �������(��������� � configurations.h)
    treader[TABLE_CARS].open(dbPath + TABLE_CARS_SPATH, ios::in);
    treader[TABLE_MANAGERS].open(dbPath + TABLE_MANAGERS_SPATH, ios::in);
    treader[TABLE_SALES].open(dbPath + TABLE_SALES_SPATH, ios::in);
    treader[TABLE_CLIENTS].open(dbPath + TABLE_CLIENTS_SPATH, ios::in);

    // ���� �� ������ �� ������
    for (int tid = 0; tid < NUM_OF_TABLES; tid++) {
        tdata[tid].clear();

        // ������ ����� �� ��� ��� ���� �� ������ �� ����� �����
        while (!(treader[tid].peek() == EOF)) {
            currData = new rowData();   // �������� ������ ���� � �������
            currData->tableID = tid;    // ������������ ���������� � �������������� ���� � ���������� �������

            // ���������� ���������� �������, ������������� �� ���������
            for (int col = dbTableSize[tid] - 1; col >= 0; col--) {
                getline(treader[tid], str);     // ����������� �� �������� ����� �������

                // �������� �� ������ � ������� ����� ��������� ���� Int, ���� String (���� � ����� 1 - Int, ����� String)
                if ((dbTableStruct[tid] >> col) & 0x01) {
                    // �������� �� ���������� � ������ ����� ��� �������������� ������ ���������� stoi
                    if (str.empty())
                        currData->ints.push_back(0);    // push_back <- ���������� � ����� ��������� ������
                    else
                        currData->ints.push_back(stoi(str));    // stoi <- ������� �������������� string � int(�� ����� ������ �������� string)
                }
                else
                    currData->strings.push_back(str);
            }

            tdata[tid].push_back(currData); // ���������� ���������� ��������� ������ � ������ �����(����� �������)
        }
    }

    // �������� ���� ������ ��� ������(����� ������� ��� ������)
    for (int tid = 0; tid < NUM_OF_TABLES; tid++)
        treader[tid].close();
}

void DBMS::saveDB()
{
    // �������� ���� ������ �� ��� ������ (�� ������ - ios::out)
    treader[TABLE_CARS].open(dbPath + TABLE_CARS_SPATH, ios::out);
    treader[TABLE_MANAGERS].open(dbPath + TABLE_MANAGERS_SPATH, ios::out);
    treader[TABLE_SALES].open(dbPath + TABLE_SALES_SPATH, ios::out);
    treader[TABLE_CLIENTS].open(dbPath + TABLE_CLIENTS_SPATH, ios::out);

    // ����������� ������ ������ �� ������ ������� � ��������������� ����
    for (int tid = 0; tid < NUM_OF_TABLES; tid++)
        // ����������� ��������� ������ ������ ������� � ����
        for (rowData *currData : tdata[tid]) {
            for (int i : currData->ints)        // ������� ������ ���� �������� �����
                treader[tid] << i << '\n';
            for (string s : currData->strings)  // ����� ������ ���� ��������� �����
                treader[tid] << s << '\n';
        }

    // �������� ���� ������ ��� ���������� ��������� � ��� ������
    for (int tid = 0; tid < NUM_OF_TABLES; tid++)
        treader[tid].close();
}

void DBMS::ADD(rowData data)
{
    // �������� ������ ���� � ���������� ��� ����������� �������
    rowData *tmp = new rowData;
    tmp->tableID = data.tableID;
    tmp->ints = data.ints;
    tmp->strings = data.strings;

    tdata[data.tableID].push_back(tmp); // ���������� ���� � �������
}

// ������� ������ ��� �����
vector<rowData *> DBMS::GET(rowData toGet)
{
    vector<rowData *> res;

    // � ������� SALES ����� ���������� �� ��������� ����
    if (toGet.tableID != TABLE_SALES) {
        for (rowData *currData : tdata[toGet.tableID])  // ��� ������� �������� �������
            if ((currData->strings.size() > 0) && (currData->strings[0] == toGet.strings[0]))   // ���������� �������� � ���� � �������
                res.push_back(currData);    // ���� �������� ���������, �� ��������� ���� � ������
    }
    // � ��������� �������� ����� ���������� �� ���������� ����
    else
        for (rowData *currData : tdata[toGet.tableID])  // ��� ������� �������� �������
            if ((currData->ints.size() > 0) && (currData->ints[0] == toGet.ints[0]))    // ���������� �������� � ���� � �������
                res.push_back(currData);    // ���� �������� ���������, �� ��������� ���� � ������

    return res; // ���������� ������
}

vector<rowData *> DBMS::GET(rowData toGet, short mask)
{
    vector<rowData *> res;
    bool fits;

    short vPosInt;
    short vPosString;

    short intsOffset;
    short stringsOffset;

    if (!mask)  // ���� �� ������ �����, ��������� ������ ������ ������� (mask == 0x00)
        return tdata[toGet.tableID];

    if (!tdata[toGet.tableID].size())   // ���� ������� ������� �����, ���������� ������ ������(�� ��������� ������)
        return res;

    intsOffset = dbTableSize[toGet.tableID] - 1;                        // �������� int � �����
    stringsOffset = intsOffset - tdata[toGet.tableID][0]->ints.size();  // �������� string � �����

    // ���� �� ������� ����(������) � �������
    for (rowData *currData : tdata[toGet.tableID]) {
        vPosInt = 0;
        vPosString = 0;
        fits = true;

        // ���� �� ������ ������� � ������
        for (int col = dbTableSize[toGet.tableID] - 1; col >= 0; col--) {
            if (!((mask >> col) & 0x01))    // ���� �������� ������� ������� �� ������������ ��� ������ - ���������� ���
                continue;

            // ���� ������� ������� �������� � ���� int
            if ((dbTableStruct[toGet.tableID] >> col) & 0x01) {
                if (currData->ints[intsOffset - col] != toGet.ints[vPosInt]) { // ���� int �� �������� ��� ���������, �� ������� ���� ��� �� ��������
                    fits = false;   // ���������� ���� ���� ��� ������� ���� ��� �� ��������
                    break;          // ���������� ��������� ���� ��������� ������� � ������ ����
                }

                vPosInt++;          // ����������� ��������� ������� ����� int��
            }
            else {
                if (currData->strings[stringsOffset - col] != toGet.strings[vPosString]) {  // ���� string �� �������� ��� ���������, �� ������� ���� ��� �� ��������
                    fits = false;   // ���������� ���� ���� ��� ������� ���� ��� �� ��������
                    break;          // ���������� ��������� ���� ��������� ������� � ������ ����
                }

                vPosString++;       // ����������� ��������� ������� ����� string��
            }
        }

        if (fits)   // ���� ������� ���� ������ ��� ��������, ��������� ��� � �������
            res.push_back(currData);
    }

    return res;
}

bool DBMS::REMOVE(rowData data)
{
    int startSize;

    startSize = tdata[data.tableID].size();

    // � ������� SALES ����� ���������� �� ��������� ����
    if (data.tableID != TABLE_SALES) {
        for (int jnx = 0; jnx < tdata[data.tableID].size(); jnx++)  // ��� ������� �������� �������
            if (tdata[data.tableID][jnx]->strings[0] == data.strings[0])    // ���������� �������� � ���� � �������
                tdata[data.tableID].erase(tdata[data.tableID].begin() + jnx);   // ���� �������� ���������, �� ������� ����
    }
    // � ��������� �������� ����� ���������� �� ���������� ����
    else
        for (int jnx = 0; jnx < tdata[data.tableID].size(); jnx++)  // ��� ������� �������� �������
            if (tdata[data.tableID][jnx]->ints[0] == data.ints[0])  // ���������� �������� � ���� � �������
                tdata[data.tableID].erase(tdata[data.tableID].begin() + jnx);   // ���� �������� ���������, �� ������� ����

    return startSize == tdata[data.tableID].size(); // ���������� true ���� ���� ����������� ���� ���� ��������
}
